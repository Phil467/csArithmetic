// Limbs are in base 2^32. Decimal text exists only on input and on display.
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cmath>
#ifndef CS_Q_BASE
#define CS_Q_BASE (4294967296LL)
#endif

namespace {

enum { QLIMB_STACK = 256 };

struct QLimbs {
  uint32_t small[QLIMB_STACK];
  uint32_t* p;
  size_t n;
  size_t cap;
  bool heap;

  CS_INLINE QLimbs() : n(1), cap(QLIMB_STACK), heap(false) {
    p = small;
    small[0] = 0;
  }
  CS_INLINE ~QLimbs() {
    if (heap && p)
      free(p);
  }
  QLimbs(const QLimbs&) = delete;
  QLimbs& operator=(const QLimbs&) = delete;

  CS_INLINE void reserve(size_t need) {
    if (need < 1)
      need = 1;
    if (need <= (size_t)QLIMB_STACK) {
      if (heap) {
        if (n && n <= (size_t)QLIMB_STACK)
          memcpy(small, p, n * sizeof(uint32_t));
        free(p);
        heap = false;
      }
      p = small;
      cap = QLIMB_STACK;
      return;
    }
    if (heap && cap >= need)
      return;
    uint32_t* np = (uint32_t*)malloc(need * sizeof(uint32_t));
    if (p && n)
      memcpy(np, p, n * sizeof(uint32_t));
    if (heap)
      free(p);
    p = np;
    heap = true;
    cap = need;
  }
  CS_INLINE void trim() {
    while (n > 1 && p[n - 1] == 0)
      --n;
  }
  CS_INLINE void setZero() {
    reserve(1);
    p[0] = 0;
    n = 1;
  }
  CS_INLINE void setOne() {
    reserve(1);
    p[0] = 1;
    n = 1;
  }
  CS_INLINE bool isZero() const { return n == 0 || (n == 1 && p[0] == 0); }
  CS_INLINE bool isOne() const { return n == 1 && p[0] == 1; }
  CS_INLINE void copyFrom(const uint32_t* s, size_t sn) {
    if (!s || sn == 0) {
      setZero();
      return;
    }
    while (sn > 1 && s[sn - 1] == 0)
      --sn;
    reserve(sn);
    memcpy(p, s, sn * sizeof(uint32_t));
    n = sn;
  }
  CS_INLINE void copy(const QLimbs& o) { copyFrom(o.p, o.n); }
  CS_INLINE void setU64(uint64_t v) {
    if (v == 0) {
      setZero();
      return;
    }
    uint32_t t[2];
    t[0] = (uint32_t)v;
    t[1] = (uint32_t)(v >> 32);
    copyFrom(t, t[1] ? 2 : 1);
  }
  CS_INLINE char* leak() const {
    size_t m = n ? n : 1;
    uint32_t* o = (uint32_t*)malloc(m * sizeof(uint32_t));
    if (!p || n == 0)
      o[0] = 0;
    else
      memcpy(o, p, n * sizeof(uint32_t));
    return (char*)o;
  }
};

CS_INLINE int qCmpLimbs(const uint32_t* a, size_t na, const uint32_t* b, size_t nb) {
  if (na != nb)
    return na < nb ? -1 : 1;
  for (size_t i = na; i-- > 0; ) {
    if (a[i] != b[i])
      return a[i] < b[i] ? -1 : 1;
  }
  return 0;
}

CS_INLINE void qAdd(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, QLimbs& r) {
  size_t m = na > nb ? na : nb;
  r.reserve(m + 1);
  uint64_t carry = 0;
  for (size_t i = 0; i < m; ++i) {
    uint64_t cur = carry;
    if (i < na)
      cur += a[i];
    if (i < nb)
      cur += b[i];
    r.p[i] = (uint32_t)cur;
    carry = cur >> 32;
  }
  if (carry) {
    r.p[m] = (uint32_t)carry;
    r.n = m + 1;
  } else {
    r.n = m;
  }
  r.trim();
}

CS_INLINE void qSub(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, QLimbs& r) {
  r.reserve(na);
  int64_t borrow = 0;
  for (size_t i = 0; i < na; ++i) {
    int64_t cur = (int64_t)a[i] - (i < nb ? (int64_t)b[i] : 0) - borrow;
    if (cur < 0) {
      cur += CS_Q_BASE;
      borrow = 1;
    } else {
      borrow = 0;
    }
    r.p[i] = (uint32_t)cur;
  }
  r.n = na;
  r.trim();
}

CS_INLINE void qSubAbs(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, QLimbs& r, bool& neg) {
  int c = qCmpLimbs(a, na, b, nb);
  neg = c < 0;
  if (c == 0) {
    r.setZero();
    return;
  }
  if (c > 0)
    qSub(a, na, b, nb, r);
  else
    qSub(b, nb, a, na, r);
}

static int qKaraCutoff = 32;
static int qKaraOverflow = 0;

struct QArena {
  uint32_t* p;
  size_t used;
  size_t cap;
  CS_INLINE QArena(size_t n) : used(0), cap(n) {
    p = (uint32_t*)malloc(n * sizeof(uint32_t));
  }
  CS_INLINE ~QArena() { free(p); }
  CS_INLINE uint32_t* take(size_t n) {
    if (n < 1) n = 1;
    if (used + n > cap) {
      qKaraOverflow = 1;
      return p;
    }
    uint32_t* r = p + used;
    used += n;
    return r;
  }
  CS_INLINE size_t mark() const { return used; }
  CS_INLINE void rewind(size_t m) { used = m; }
};

CS_INLINE size_t qSigLen(const uint32_t* a, size_t n) {
  while (n > 1 && a[n - 1] == 0)
    --n;
  if (n == 1 && a[0] == 0)
    return 0;
  return n;
}

CS_INLINE void qMulSchoolRaw(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, uint32_t* r) {
  memset(r, 0, (na + nb) * sizeof(uint32_t));
  for (size_t i = 0; i < na; ++i) {
    uint64_t carry = 0;
    for (size_t j = 0; j < nb; ++j) {
      uint64_t cur = (uint64_t)r[i + j] + (uint64_t)a[i] * b[j] + carry;
      r[i + j] = (uint32_t)cur;
      carry = cur >> 32;
    }
    r[i + nb] = (uint32_t)carry;
  }
}

CS_INLINE void qAddRaw(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, uint32_t* r, size_t& nr) {
  if (na == 0 && nb == 0) {
    nr = 0;
    return;
  }
  size_t m = na > nb ? na : nb;
  uint64_t carry = 0;
  size_t i = 0;
  for (; i < m; ++i) {
    uint64_t cur = carry;
    if (i < na)
      cur += a[i];
    if (i < nb)
      cur += b[i];
    r[i] = (uint32_t)cur;
    carry = cur >> 32;
  }
  if (carry)
    r[i++] = (uint32_t)carry;
  nr = qSigLen(r, i);
}

CS_INLINE void qSubRaw(uint32_t* a, size_t& na, size_t acap, const uint32_t* b, size_t nb) {
  if (nb == 0)
    return;
  size_t n = na > nb ? na : nb;
  if (n > acap) {
    qKaraOverflow = 1;
    return;
  }
  int borrow = 0;
  for (size_t i = 0; i < n; ++i) {
    __int128 cur = (i < na ? (__int128)a[i] : 0) - (i < nb ? (__int128)b[i] : 0) - borrow;
    if (cur < 0) {
      cur += CS_Q_BASE;
      borrow = 1;
    } else {
      borrow = 0;
    }
    a[i] = (uint32_t)cur;
  }
  if (borrow) {
    qKaraOverflow = 1;
    return;
  }
  na = qSigLen(a, n);
}

CS_INLINE void qAddInto(uint32_t* dst, size_t dstLen, const uint32_t* src, size_t ns, size_t offset) {
  uint64_t carry = 0;
  size_t i = 0;
  while (i < ns || carry) {
    if (offset + i >= dstLen) {
      qKaraOverflow = 1;
      return;
    }
    uint64_t cur = carry + dst[offset + i] + (i < ns ? src[i] : 0);
    dst[offset + i] = (uint32_t)cur;
    carry = cur >> 32;
    ++i;
  }
}

static int qToomCutoff = 4096;

struct SLim {
  uint32_t* p;
  size_t n;
  size_t cap;
  int neg;
};

CS_INLINE void qMulRec(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, uint32_t* r, QArena& ar);

CS_INLINE void sNorm(SLim& a) {
  while (a.n > 1 && a.p[a.n - 1] == 0)
    --a.n;
  if (a.n == 0 || (a.n == 1 && a.p[0] == 0)) {
    a.n = 0;
    a.neg = 0;
  }
}

CS_INLINE SLim sBuf(QArena& ar, size_t cap) {
  if (cap < 1)
    cap = 1;
  SLim a;
  a.p = ar.take(cap);
  memset(a.p, 0, cap * sizeof(uint32_t));
  a.n = 0;
  a.cap = cap;
  a.neg = 0;
  return a;
}

CS_INLINE void sCopyTo(SLim& d, const SLim& s) {
  if (!s.n) {
    d.n = 0;
    d.neg = 0;
    return;
  }
  memcpy(d.p, s.p, s.n * sizeof(uint32_t));
  d.n = s.n;
  d.neg = s.neg;
}

CS_INLINE void sAdd(const SLim& a, const SLim& b, SLim& r) {
  if (!a.n) {
    sCopyTo(r, b);
    return;
  }
  if (!b.n) {
    sCopyTo(r, a);
    return;
  }
  if (a.neg == b.neg) {
    size_t nr = 0;
    qAddRaw(a.p, a.n, b.p, b.n, r.p, nr);
    r.n = nr;
    r.neg = a.neg;
    sNorm(r);
    return;
  }
  int c = qCmpLimbs(a.p, a.n, b.p, b.n);
  if (c == 0) {
    r.n = 0;
    r.neg = 0;
    return;
  }
  const SLim& hi = c > 0 ? a : b;
  const SLim& lo = c > 0 ? b : a;
  memcpy(r.p, hi.p, hi.n * sizeof(uint32_t));
  size_t rn = hi.n;
  qSubRaw(r.p, rn, r.cap, lo.p, lo.n);
  r.n = rn;
  r.neg = hi.neg;
  sNorm(r);
}

CS_INLINE void sSub(const SLim& a, SLim b, SLim& r) {
  if (b.n)
    b.neg = !b.neg;
  sAdd(a, b, r);
}

CS_INLINE void sShl(const SLim& a, int bits, SLim& r) {
  if (!a.n || bits <= 0) {
    sCopyTo(r, a);
    return;
  }
  uint32_t carry = 0;
  for (size_t i = 0; i < a.n; ++i) {
    uint64_t cur = ((uint64_t)a.p[i] << bits) | carry;
    r.p[i] = (uint32_t)cur;
    carry = (uint32_t)(cur >> 32);
  }
  size_t n = a.n;
  if (carry)
    r.p[n++] = carry;
  r.n = n;
  r.neg = a.neg;
  sNorm(r);
}

CS_INLINE void sFrom(SLim& d, const uint32_t* p, size_t n) {
  n = p ? qSigLen(p, n) : 0;
  if (!n) {
    d.n = 0;
    d.neg = 0;
    return;
  }
  memcpy(d.p, p, n * sizeof(uint32_t));
  d.n = n;
  d.neg = 0;
}

CS_INLINE void sMul(const SLim& a, const SLim& b, SLim& r, QArena& ar) {
  if (!a.n || !b.n) {
    r.n = 0;
    r.neg = 0;
    return;
  }
  memset(r.p, 0, (a.n + b.n) * sizeof(uint32_t));
  qMulRec(a.p, a.n, b.p, b.n, r.p, ar);
  r.n = qSigLen(r.p, a.n + b.n);
  r.neg = (a.neg != b.neg);
  if (!r.n)
    r.neg = 0;
}

CS_INLINE void sDivSmall(SLim& a, uint32_t div) {
  if (!a.n)
    return;
  uint64_t rem = 0;
  for (size_t i = a.n; i-- > 0; ) {
    uint64_t cur = (rem << 32) | a.p[i];
    a.p[i] = (uint32_t)(cur / div);
    rem = cur % div;
  }
  if (rem)
    qKaraOverflow = 1;
  sNorm(a);
}

CS_INLINE void qPart(const uint32_t* a, size_t na, size_t off, size_t len, const uint32_t*& p, size_t& n) {
  if (!a || off >= na || len == 0) {
    p = 0;
    n = 0;
    return;
  }
  size_t m = na - off;
  if (m > len)
    m = len;
  p = a + off;
  n = qSigLen(p, m);
}

CS_INLINE void sAddShift(uint32_t* dst, size_t dstLen, const SLim& c, size_t shift) {
  if (!c.n)
    return;
  if (!c.neg) {
    qAddInto(dst, dstLen, c.p, c.n, shift);
    return;
  }
  int borrow = 0;
  size_t i = 0;
  while (i < c.n || borrow) {
    if (shift + i >= dstLen) {
      qKaraOverflow = 1;
      return;
    }
    int64_t cur = (int64_t)dst[shift + i] - (i < c.n ? (int64_t)c.p[i] : 0) - borrow;
    if (cur < 0) {
      cur += CS_Q_BASE;
      borrow = 1;
    } else {
      borrow = 0;
    }
    dst[shift + i] = (uint32_t)cur;
    ++i;
  }
}

CS_INLINE void qToom3(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, uint32_t* r, QArena& ar) {
  size_t n = na > nb ? na : nb;
  size_t s = (n + 2) / 3;
  if (s < 1)
    s = 1;
  const uint32_t *a0p, *a1p, *a2p, *b0p, *b1p, *b2p;
  size_t a0n, a1n, a2n, b0n, b1n, b2n;
  qPart(a, na, 0, s, a0p, a0n);
  qPart(a, na, s, s, a1p, a1n);
  qPart(a, na, 2 * s, na, a2p, a2n);
  qPart(b, nb, 0, s, b0p, b0n);
  qPart(b, nb, s, s, b1p, b1n);
  qPart(b, nb, 2 * s, nb, b2p, b2n);

  size_t saved = ar.mark();
  size_t pc = s + 4;
  SLim A0 = sBuf(ar, pc), A1 = sBuf(ar, pc), A2 = sBuf(ar, pc);
  SLim B0 = sBuf(ar, pc), B1 = sBuf(ar, pc), B2 = sBuf(ar, pc);
  sFrom(A0, a0p, a0n);
  sFrom(A1, a1p, a1n);
  sFrom(A2, a2p, a2n);
  sFrom(B0, b0p, b0n);
  sFrom(B1, b1p, b1n);
  sFrom(B2, b2p, b2n);

  SLim t1 = sBuf(ar, pc), t2 = sBuf(ar, pc), accP = sBuf(ar, pc);
  SLim Ap = sBuf(ar, pc), Am = sBuf(ar, pc), A2e = sBuf(ar, pc);
  SLim Bp = sBuf(ar, pc), Bm = sBuf(ar, pc), B2e = sBuf(ar, pc);

  sAdd(A0, A1, t1);
  sAdd(t1, A2, Ap);
  sSub(A0, A1, t1);
  sAdd(t1, A2, Am);
  sShl(A1, 1, t1);
  sShl(A2, 2, t2);
  sAdd(A0, t1, accP);
  sAdd(accP, t2, A2e);

  sAdd(B0, B1, t1);
  sAdd(t1, B2, Bp);
  sSub(B0, B1, t1);
  sAdd(t1, B2, Bm);
  sShl(B1, 1, t1);
  sShl(B2, 2, t2);
  sAdd(B0, t1, accP);
  sAdd(accP, t2, B2e);

  size_t mc = pc * 2 + 2;
  SLim P0 = sBuf(ar, mc), P1 = sBuf(ar, mc), Pm = sBuf(ar, mc), P2 = sBuf(ar, mc), Pi = sBuf(ar, mc);
  sMul(A0, B0, P0, ar);
  sMul(Ap, Bp, P1, ar);
  sMul(Am, Bm, Pm, ar);
  sMul(A2e, B2e, P2, ar);
  sMul(A2, B2, Pi, ar);

  SLim c0 = sBuf(ar, mc), c1 = sBuf(ar, mc), c2 = sBuf(ar, mc), c3 = sBuf(ar, mc), c4 = sBuf(ar, mc);
  SLim U = sBuf(ar, mc), V = sBuf(ar, mc), W = sBuf(ar, mc), X = sBuf(ar, mc), Y = sBuf(ar, mc);
  sCopyTo(c0, P0);
  sCopyTo(c4, Pi);

  sAdd(P1, Pm, U);
  sDivSmall(U, 2);
  sSub(U, c0, V);
  sSub(V, c4, c2);

  sSub(P1, Pm, U);
  sDivSmall(U, 2);

  sShl(c2, 2, V);
  sShl(c4, 4, W);
  sSub(P2, c0, X);
  sSub(X, V, Y);
  sSub(Y, W, X);
  sDivSmall(X, 2);

  sSub(X, U, c3);
  sDivSmall(c3, 3);
  sSub(U, c3, c1);

  size_t big = 6 * s + 8;
  if (big < na + nb)
    big = na + nb;
  uint32_t* acc = ar.take(big);
  memset(acc, 0, big * sizeof(uint32_t));
  sAddShift(acc, big, c0, 0);
  sAddShift(acc, big, c1, s);
  sAddShift(acc, big, c2, 2 * s);
  sAddShift(acc, big, c3, 3 * s);
  sAddShift(acc, big, c4, 4 * s);
  for (size_t i = na + nb; i < big; ++i) {
    if (acc[i])
      qKaraOverflow = 1;
  }
  memcpy(r, acc, (na + nb) * sizeof(uint32_t));
  ar.rewind(saved);
}

CS_INLINE void qMulRec(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, uint32_t* r, QArena& ar) {
  if (na == 0 || nb == 0) {
    if (na + nb)
      memset(r, 0, (na + nb) * sizeof(uint32_t));
    return;
  }
  if (na < (size_t)qKaraCutoff || nb < (size_t)qKaraCutoff) {
    qMulSchoolRaw(a, na, b, nb, r);
    return;
  }
  if (na >= (size_t)qToomCutoff && nb >= (size_t)qToomCutoff) {
    qToom3(a, na, b, nb, r, ar);
    return;
  }
  size_t n = na > nb ? na : nb;
  size_t k = n >> 1;
  size_t a0n = qSigLen(a, na < k ? na : k);
  size_t b0n = qSigLen(b, nb < k ? nb : k);
  size_t a1n = na > k ? qSigLen(a + k, na - k) : 0;
  size_t b1n = nb > k ? qSigLen(b + k, nb - k) : 0;
  const uint32_t* a1 = a + k;
  const uint32_t* b1 = b + k;
  size_t saved = ar.mark();

  uint32_t* z0 = 0;
  size_t z0n = 0;
  size_t z0cap = 0;
  if (a0n && b0n) {
    z0cap = a0n + b0n;
    z0 = ar.take(z0cap + 1);
    z0[z0cap] = 0;
    qMulRec(a, a0n, b, b0n, z0, ar);
    if (z0[z0cap])
      qKaraOverflow = 1;
    z0n = qSigLen(z0, z0cap);
  }

  uint32_t* z2 = 0;
  size_t z2n = 0;
  size_t z2cap = 0;
  if (a1n && b1n) {
    z2cap = a1n + b1n;
    z2 = ar.take(z2cap + 1);
    z2[z2cap] = 0;
    qMulRec(a1, a1n, b1, b1n, z2, ar);
    if (z2[z2cap])
      qKaraOverflow = 1;
    z2n = qSigLen(z2, z2cap);
  }

  uint32_t* sa = ar.take(k + 2);
  uint32_t* sb = ar.take(k + 2);
  size_t san = 0, sbn = 0;
  qAddRaw(a, a0n, a1n ? a1 : 0, a1n, sa, san);
  qAddRaw(b, b0n, b1n ? b1 : 0, b1n, sb, sbn);

  uint32_t* z1 = 0;
  size_t z1n = 0;
  size_t z1cap = 0;
  if (san && sbn) {
    z1cap = san + sbn;
    z1 = ar.take(z1cap + 1);
    z1[z1cap] = 0;
    qMulRec(sa, san, sb, sbn, z1, ar);
    if (z1[z1cap])
      qKaraOverflow = 1;
    z1n = qSigLen(z1, z1cap);
    if (z0n)
      qSubRaw(z1, z1n, z1cap, z0, z0n);
    if (z2n)
      qSubRaw(z1, z1n, z1cap, z2, z2n);
  }

  memset(r, 0, (na + nb) * sizeof(uint32_t));
  if (z0n)
    qAddInto(r, na + nb, z0, z0n, 0);
  if (z1n)
    qAddInto(r, na + nb, z1, z1n, k);
  if (z2n)
    qAddInto(r, na + nb, z2, z2n, k * 2);
  ar.rewind(saved);
}

CS_INLINE void qMul(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, QLimbs& r) {
  if (!a || !b || na == 0 || nb == 0 || (na == 1 && a[0] == 0) || (nb == 1 && b[0] == 0)) {
    r.setZero();
    return;
  }
  r.reserve(na + nb + 1);
  r.p[na + nb] = 0;
  if (na < (size_t)qKaraCutoff || nb < (size_t)qKaraCutoff)
    qMulSchoolRaw(a, na, b, nb, r.p);
  else {
    QArena ar((na + nb) * 32 + 512);
    qMulRec(a, na, b, nb, r.p, ar);
  }
  if (r.p[na + nb] != 0)
    qKaraOverflow = 1;
  r.n = na + nb;
  r.trim();
}

CS_INLINE void qMulSmall(const uint32_t* a, size_t n, uint32_t m, uint32_t* dst, size_t keep) {
  uint64_t carry = 0;
  for (size_t i = 0; i < n; ++i) {
    uint64_t cur = (uint64_t)a[i] * m + carry;
    dst[i] = (uint32_t)cur;
    carry = cur >> 32;
  }
  if (n < keep)
    dst[n] = (uint32_t)carry;
  for (size_t i = n + 1; i < keep; ++i)
    dst[i] = 0;
}

CS_INLINE void qDivModKnuth(const uint32_t* u0, size_t un, const uint32_t* v0, size_t vn, QLimbs& q, QLimbs& r) {
  if (!v0 || vn == 0 || (vn == 1 && v0[0] == 0)) {
    q.setZero();
    r.setZero();
    return;
  }
  while (un > 1 && u0[un - 1] == 0)
    --un;
  while (vn > 1 && v0[vn - 1] == 0)
    --vn;
  if (qCmpLimbs(u0, un, v0, vn) < 0) {
    q.setZero();
    r.copyFrom(u0, un);
    return;
  }
  if (vn == 1) {
    uint32_t vd = v0[0];
    q.reserve(un);
    uint64_t rem = 0;
    for (size_t i = un; i-- > 0; ) {
      uint64_t cur = (rem << 32) + u0[i];
      q.p[i] = (uint32_t)(cur / vd);
      rem = cur % vd;
    }
    q.n = un;
    q.trim();
    r.reserve(1);
    r.p[0] = (uint32_t)rem;
    r.n = 1;
    return;
  }
  uint32_t d = (uint32_t)((uint64_t)CS_Q_BASE / ((uint64_t)v0[vn - 1] + 1));
  QLimbs u, v;
  u.reserve(un + 1);
  qMulSmall(u0, un, d, u.p, un + 1);
  u.n = un + 1;
  v.reserve(vn);
  qMulSmall(v0, vn, d, v.p, vn);
  v.n = vn;
  size_t qn = un - vn + 1;
  q.reserve(qn);
  memset(q.p, 0, qn * sizeof(uint32_t));
  q.n = qn;
  for (int j = (int)un - (int)vn; j >= 0; --j) {
    uint64_t num = ((uint64_t)u.p[j + vn] << 32) + u.p[j + vn - 1];
    uint64_t qhat, rhat;
    if (u.p[j + vn] >= v.p[vn - 1]) {
      qhat = CS_Q_BASE - 1;
      rhat = num - qhat * v.p[vn - 1];
    } else {
      qhat = num / v.p[vn - 1];
      rhat = num % v.p[vn - 1];
    }
    while (qhat >= (uint64_t)CS_Q_BASE || (rhat < (uint64_t)CS_Q_BASE && qhat * (uint64_t)v.p[vn - 2] > (rhat << 32) + u.p[j + vn - 2])) {
      --qhat;
      rhat += v.p[vn - 1];
    }
    uint64_t carry = 0;
    int borrow = 0;
    for (size_t i = 0; i < vn; ++i) {
      uint64_t prod = qhat * (uint64_t)v.p[i] + carry;
      carry = prod >> 32;
      uint32_t digit = (uint32_t)prod;
      int64_t cur = (int64_t)u.p[j + i] - (int64_t)digit - borrow;
      if (cur < 0) {
        cur += CS_Q_BASE;
        borrow = 1;
      } else {
        borrow = 0;
      }
      u.p[j + i] = (uint32_t)cur;
    }
    int64_t last = (int64_t)u.p[j + vn] - (int64_t)carry - borrow;
    if (last < 0) {
      --qhat;
      uint64_t c = 0;
      for (size_t i = 0; i < vn; ++i) {
        uint64_t s = (uint64_t)u.p[j + i] + v.p[i] + c;
        u.p[j + i] = (uint32_t)s;
        c = s >> 32;
      }
      last += (int64_t)CS_Q_BASE + (int64_t)c;
    }
    u.p[j + vn] = (uint32_t)last;
    q.p[j] = (uint32_t)qhat;
  }
  q.trim();
  r.reserve(vn);
  uint64_t rem = 0;
  for (size_t i = vn; i-- > 0; ) {
    uint64_t cur = (rem << 32) + u.p[i];
    r.p[i] = (uint32_t)(cur / d);
    rem = cur % d;
  }
  r.n = vn;
  r.trim();
}

static int qDivCutoff = 48;
static int qDivOverflow = 0;
static int qDivFallback = 0;

CS_INLINE int qCmpShifted(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, size_t shift) {
  size_t bl = nb == 0 ? 0 : shift + nb;
  if (na != bl)
    return na < bl ? -1 : 1;
  for (size_t i = na; i-- > 0; ) {
    uint32_t bv = (i >= shift && i - shift < nb) ? b[i - shift] : 0;
    if (a[i] != bv)
      return a[i] < bv ? -1 : 1;
  }
  return 0;
}

CS_INLINE void qAddShifted(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, size_t shift, uint32_t* dst, size_t cap, size_t& nd) {
  size_t m = na > shift + nb ? na : shift + nb;
  if (m + 1 > cap) {
    qDivOverflow = 1;
    nd = 0;
    return;
  }
  uint64_t carry = 0;
  size_t i = 0;
  for (; i < m; ++i) {
    uint64_t cur = carry;
    if (i < na)
      cur += a[i];
    if (i >= shift && i - shift < nb)
      cur += b[i - shift];
    dst[i] = (uint32_t)cur;
    carry = cur >> 32;
  }
  if (carry)
    dst[i++] = (uint32_t)carry;
  nd = qSigLen(dst, i);
}

CS_INLINE void qSubShifted(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, size_t shift, uint32_t* dst, size_t& nd) {
  int borrow = 0;
  size_t m = na > shift + nb ? na : shift + nb;
  for (size_t i = 0; i < m; ++i) {
    int64_t cur = (i < na ? (int64_t)a[i] : 0) - (i >= shift && i - shift < nb ? (int64_t)b[i - shift] : 0) - borrow;
    if (cur < 0) {
      cur += CS_Q_BASE;
      borrow = 1;
    } else {
      borrow = 0;
    }
    dst[i] = (uint32_t)cur;
  }
  if (borrow)
    qDivOverflow = 1;
  nd = qSigLen(dst, m);
}

CS_INLINE void qSubFromShifted(const uint32_t* b, size_t nb, size_t shift, const uint32_t* a, size_t na, uint32_t* dst, size_t& nd) {
  int borrow = 0;
  size_t m = shift + nb;
  if (na > m)
    m = na;
  for (size_t i = 0; i < m; ++i) {
    int64_t bv = (i >= shift && i - shift < nb) ? (int64_t)b[i - shift] : 0;
    int64_t cur = bv - (i < na ? (int64_t)a[i] : 0) - borrow;
    if (cur < 0) {
      cur += CS_Q_BASE;
      borrow = 1;
    } else {
      borrow = 0;
    }
    dst[i] = (uint32_t)cur;
  }
  if (borrow)
    qDivOverflow = 1;
  nd = qSigLen(dst, m);
}

CS_INLINE void qDecLimb(uint32_t* a, size_t& n) {
  if (n == 0) {
    qDivOverflow = 1;
    return;
  }
  size_t i = 0;
  while (i < n && a[i] == 0) {
    a[i] = (uint32_t)(CS_Q_BASE - 1);
    ++i;
  }
  if (i >= n) {
    qDivOverflow = 1;
    return;
  }
  --a[i];
  n = qSigLen(a, n);
}

CS_INLINE void qDivKnuthInto(const uint32_t* u, size_t un, const uint32_t* v, size_t vn, uint32_t* Q, size_t qcap, size_t& qn, uint32_t* R, size_t rcap, size_t& rn) {
  QLimbs qq, rr;
  qDivModKnuth(u, un, v, vn, qq, rr);
  if (qq.n > qcap || rr.n > rcap) {
    qDivOverflow = 1;
    qn = 0;
    rn = 0;
    return;
  }
  qn = qSigLen(qq.p, qq.n);
  rn = qSigLen(rr.p, rr.n);
  if (qn)
    memcpy(Q, qq.p, qn * sizeof(uint32_t));
  if (rn)
    memcpy(R, rr.p, rn * sizeof(uint32_t));
}

CS_INLINE void qDivRec(const uint32_t* A, size_t aLen, const uint32_t* B, size_t bLen, uint32_t* Q, size_t qcap, size_t& qn, uint32_t* R, size_t rcap, size_t& rn, QArena& ar);

CS_INLINE void qDivUnbal(const uint32_t* A, size_t aLen, const uint32_t* B, size_t n, uint32_t* Q, size_t qcap, size_t& qn, uint32_t* R, size_t rcap, size_t& rn, QArena& ar) {
  size_t m0 = aLen - n;
  size_t m = m0;
  size_t saved = ar.mark();
  uint32_t* cur = ar.take(aLen + 2);
  memset(cur, 0, (aLen + 2) * sizeof(uint32_t));
  memcpy(cur, A, aLen * sizeof(uint32_t));
  size_t cn = aLen;
  size_t qAccCap = m0 + n + 8;
  uint32_t* Qacc = ar.take(qAccCap);
  memset(Qacc, 0, qAccCap * sizeof(uint32_t));
  size_t qAccN = 0;
  while (m > n && !qDivOverflow) {
    size_t drop = m - n;
    size_t topN = cn > drop ? qSigLen(cur + drop, cn - drop) : 0;
    size_t step = ar.mark();
    size_t qcap2 = n + 4;
    size_t rcap2 = n + 2;
    uint32_t* q = ar.take(qcap2);
    uint32_t* r = ar.take(rcap2);
    size_t qq = 0, rr = 0;
    qDivRec(topN ? cur + drop : cur, topN, B, n, q, qcap2, qq, r, rcap2, rr, ar);
    if (qAccN) {
      if (qAccN + n > qAccCap) {
        qDivOverflow = 1;
        break;
      }
      memmove(Qacc + n, Qacc, qAccN * sizeof(uint32_t));
      memset(Qacc, 0, n * sizeof(uint32_t));
      qAccN += n;
    }
    if (qq)
      qAddInto(Qacc, qAccCap, q, qq, 0);
    qAccN = qSigLen(Qacc, qAccCap);
    uint32_t* nxt = ar.take(drop + n + 2);
    memset(nxt, 0, (drop + n + 2) * sizeof(uint32_t));
    size_t lowN = cn < drop ? cn : drop;
    if (lowN)
      memcpy(nxt, cur, lowN * sizeof(uint32_t));
    if (rr)
      memcpy(nxt + drop, r, rr * sizeof(uint32_t));
    size_t nn = qSigLen(nxt, drop + rr);
    memset(cur, 0, (aLen + 2) * sizeof(uint32_t));
    if (nn)
      memcpy(cur, nxt, nn * sizeof(uint32_t));
    cn = nn;
    ar.rewind(step);
    m -= n;
  }
  if (!qDivOverflow) {
    size_t qcap2 = m + 4;
    size_t rcap2 = n + 2;
    uint32_t* q = ar.take(qcap2);
    uint32_t* r = ar.take(rcap2);
    size_t qq = 0, rr = 0;
    qDivRec(cur, qSigLen(cur, cn), B, n, q, qcap2, qq, r, rcap2, rr, ar);
    if (qAccN && m) {
      if (qAccN + m > qAccCap)
        qDivOverflow = 1;
      else {
        memmove(Qacc + m, Qacc, qAccN * sizeof(uint32_t));
        memset(Qacc, 0, m * sizeof(uint32_t));
        qAccN += m;
      }
    }
    if (qq && !qDivOverflow)
      qAddInto(Qacc, qAccCap, q, qq, 0);
    qAccN = qSigLen(Qacc, qAccCap);
    if (qAccN > qcap || rr > rcap)
      qDivOverflow = 1;
    else {
      qn = qAccN;
      if (qn)
        memcpy(Q, Qacc, qn * sizeof(uint32_t));
      rn = rr;
      if (rn)
        memcpy(R, r, rn * sizeof(uint32_t));
    }
  }
  ar.rewind(saved);
}

CS_INLINE void qDivRec(const uint32_t* A, size_t aLen, const uint32_t* B, size_t bLen, uint32_t* Q, size_t qcap, size_t& qn, uint32_t* R, size_t rcap, size_t& rn, QArena& ar) {
  size_t nA = qSigLen(A, aLen);
  size_t n = qSigLen(B, bLen);
  qn = 0;
  rn = 0;
  if (n == 0 || (n == 1 && B[0] == 0)) {
    qDivOverflow = 1;
    return;
  }
  if (nA < n) {
    if (nA > rcap) {
      qDivOverflow = 1;
      return;
    }
    rn = nA;
    if (rn)
      memcpy(R, A, rn * sizeof(uint32_t));
    return;
  }
  size_t m = nA - n;
  if (n < (size_t)qDivCutoff || m + 1 < (size_t)qDivCutoff || m < 2) {
    qDivKnuthInto(A, nA, B, n, Q, qcap, qn, R, rcap, rn);
    return;
  }
  if (m > n) {
    qDivUnbal(A, nA, B, n, Q, qcap, qn, R, rcap, rn, ar);
    return;
  }
  size_t k = m >> 1;
  const uint32_t* B1 = B + k;
  size_t nB1 = n - k;
  size_t nB0 = qSigLen(B, k);
  size_t saved = ar.mark();
  size_t shift2 = k * 2;
  size_t nAhi = nA > shift2 ? nA - shift2 : 0;
  const uint32_t* Ahi = nAhi ? A + shift2 : A;
  size_t q1cap = (m - k) + 4;
  size_t r1cap = nB1 + 2;
  uint32_t* Q1 = ar.take(q1cap);
  uint32_t* R1 = ar.take(r1cap);
  size_t q1n = 0, r1n = 0;
  qDivRec(Ahi, nAhi, B1, nB1, Q1, q1cap, q1n, R1, r1cap, r1n, ar);
  size_t lowN = nA < shift2 ? nA : shift2;
  size_t concCap = shift2 + r1n + 2;
  uint32_t* conc = ar.take(concCap);
  memset(conc, 0, concCap * sizeof(uint32_t));
  if (lowN)
    memcpy(conc, A, lowN * sizeof(uint32_t));
  if (r1n)
    memcpy(conc + shift2, R1, r1n * sizeof(uint32_t));
  size_t concN = qSigLen(conc, shift2 + r1n);
  QLimbs prod;
  if (q1n && nB0)
    qMul(Q1, q1n, B, nB0, prod);
  else
    prod.setZero();
  size_t prodN = (prod.n == 1 && prod.p[0] == 0) ? 0 : prod.n;
  size_t apCap = concCap + prodN + k + 4;
  uint32_t* Ap = ar.take(apCap);
  size_t apN = 0;
  int neg = 0;
  if (qCmpShifted(conc, concN, prod.p, prodN, k) >= 0)
    qSubShifted(conc, concN, prod.p, prodN, k, Ap, apN);
  else {
    qSubFromShifted(prod.p, prodN, k, conc, concN, Ap, apN);
    neg = 1;
  }
  int fixes = 0;
  while (neg && !qDivOverflow) {
    if (++fixes > 6) {
      qDivOverflow = 1;
      break;
    }
    qDecLimb(Q1, q1n);
    uint32_t* tmp = ar.take(apCap + n + k + 2);
    if (qCmpShifted(Ap, apN, B, n, k) <= 0) {
      qSubFromShifted(B, n, k, Ap, apN, tmp, apN);
      if (apN)
        memcpy(Ap, tmp, apN * sizeof(uint32_t));
      neg = 0;
    } else {
      size_t nn = 0;
      qSubShifted(Ap, apN, B, n, k, tmp, nn);
      if (nn)
        memcpy(Ap, tmp, nn * sizeof(uint32_t));
      apN = nn;
    }
  }
  size_t nMid = apN > k ? apN - k : 0;
  const uint32_t* Amid = nMid ? Ap + k : Ap;
  size_t q0cap = k + 4;
  size_t r0cap = nB1 + 2;
  uint32_t* Q0 = ar.take(q0cap);
  uint32_t* R0 = ar.take(r0cap);
  size_t q0n = 0, r0n = 0;
  if (!qDivOverflow)
    qDivRec(Amid, nMid, B1, nB1, Q0, q0cap, q0n, R0, r0cap, r0n, ar);
  size_t low2 = apN < k ? apN : k;
  size_t conc2cap = k + r0n + 2;
  uint32_t* conc2 = ar.take(conc2cap);
  memset(conc2, 0, conc2cap * sizeof(uint32_t));
  if (low2)
    memcpy(conc2, Ap, low2 * sizeof(uint32_t));
  if (r0n)
    memcpy(conc2 + k, R0, r0n * sizeof(uint32_t));
  size_t conc2N = qSigLen(conc2, k + r0n);
  QLimbs prod0;
  if (q0n && nB0)
    qMul(Q0, q0n, B, nB0, prod0);
  else
    prod0.setZero();
  size_t p0n = (prod0.n == 1 && prod0.p[0] == 0) ? 0 : prod0.n;
  uint32_t* App = ar.take(conc2cap + p0n + 4);
  size_t appN = 0;
  int neg2 = 0;
  if (qCmpShifted(conc2, conc2N, prod0.p, p0n, 0) >= 0)
    qSubShifted(conc2, conc2N, prod0.p, p0n, 0, App, appN);
  else {
    qSubFromShifted(prod0.p, p0n, 0, conc2, conc2N, App, appN);
    neg2 = 1;
  }
  fixes = 0;
  while (neg2 && !qDivOverflow) {
    if (++fixes > 6) {
      qDivOverflow = 1;
      break;
    }
    qDecLimb(Q0, q0n);
    uint32_t* tmp = ar.take(appN + n + 4);
    if (qCmpShifted(App, appN, B, n, 0) <= 0) {
      qSubFromShifted(B, n, 0, App, appN, tmp, appN);
      if (appN)
        memcpy(App, tmp, appN * sizeof(uint32_t));
      neg2 = 0;
    } else {
      size_t nn = 0;
      qSubShifted(App, appN, B, n, 0, tmp, nn);
      if (nn)
        memcpy(App, tmp, nn * sizeof(uint32_t));
      appN = nn;
    }
  }
  if (qDivOverflow) {
    ar.rewind(saved);
    return;
  }
  if (k + q1n + 2 > qcap || appN > rcap) {
    qDivOverflow = 1;
    ar.rewind(saved);
    return;
  }
  memset(Q, 0, qcap * sizeof(uint32_t));
  if (q0n)
    qAddInto(Q, qcap, Q0, q0n, 0);
  if (q1n)
    qAddInto(Q, qcap, Q1, q1n, k);
  qn = qSigLen(Q, qcap);
  rn = appN;
  if (rn)
    memcpy(R, App, rn * sizeof(uint32_t));
  if (rn && qCmpLimbs(R, rn, B, n) >= 0)
    qDivOverflow = 1;
  ar.rewind(saved);
}

CS_INLINE void qDivMod(const uint32_t* u0, size_t un, const uint32_t* v0, size_t vn, QLimbs& q, QLimbs& r) {
  if (!v0 || vn == 0 || (vn == 1 && v0[0] == 0)) {
    q.setZero();
    r.setZero();
    return;
  }
  while (un > 1 && u0[un - 1] == 0)
    --un;
  while (vn > 1 && v0[vn - 1] == 0)
    --vn;
  if (qCmpLimbs(u0, un, v0, vn) < 0) {
    q.setZero();
    r.copyFrom(u0, un);
    return;
  }
  if (vn == 1 || vn < (size_t)qDivCutoff || un - vn < (size_t)qDivCutoff) {
    qDivModKnuth(u0, un, v0, vn, q, r);
    return;
  }
  uint32_t d = (uint32_t)((uint64_t)CS_Q_BASE / ((uint64_t)v0[vn - 1] + 1));
  QLimbs u, v;
  u.reserve(un + 1);
  qMulSmall(u0, un, d, u.p, un + 1);
  u.n = qSigLen(u.p, un + 1);
  v.reserve(vn + 1);
  qMulSmall(v0, vn, d, v.p, vn);
  v.n = qSigLen(v.p, vn);
  size_t qcap = u.n - v.n + 4;
  size_t rcap = v.n + 2;
  int prevKara = qKaraOverflow;
  qKaraOverflow = 0;
  qDivOverflow = 0;
  int ok = 0;
  {
    QArena ar((u.n + v.n) * 64 + 256);
    uint32_t* Qb = ar.take(qcap);
    uint32_t* Rb = ar.take(rcap);
    size_t qn = 0, rn = 0;
    qDivRec(u.p, u.n, v.p, v.n, Qb, qcap, qn, Rb, rcap, rn, ar);
    if (qKaraOverflow)
      qDivOverflow = 1;
    if (!qDivOverflow) {
      q.copyFrom(Qb, qn);
      r.reserve(rn + 1);
      uint64_t rem = 0;
      for (size_t i = rn; i-- > 0; ) {
        uint64_t cur = (rem << 32) + Rb[i];
        r.p[i] = (uint32_t)(cur / d);
        rem = cur % d;
      }
      if (rn == 0) {
        r.p[0] = 0;
        r.n = 1;
      } else {
        r.n = rn;
        r.trim();
      }
      if (rem != 0)
        qDivOverflow = 1;
      else
        ok = 1;
    }
  }
  qKaraOverflow = prevKara;
  if (!ok) {
    ++qDivFallback;
    qDivOverflow = 0;
    qDivModKnuth(u0, un, v0, vn, q, r);
  }
}

CS_INLINE void qGcd(const uint32_t* a0, size_t na, const uint32_t* b0, size_t nb, QLimbs& g) {
  if ((na == 1 && a0 && a0[0] == 1) || (nb == 1 && b0 && b0[0] == 1)) {
    g.setOne();
    return;
  }
  QLimbs a, b, q, r;
  a.copyFrom(a0, na);
  b.copyFrom(b0, nb);
  while (!b.isZero()) {
    qDivMod(a.p, a.n, b.p, b.n, q, r);
    a.copy(b);
    b.copy(r);
  }
  g.copy(a);
}

CS_INLINE void qNormalize(QLimbs& num, QLimbs& den) {
  if (num.isZero()) {
    den.setOne();
    return;
  }
  if (num.isOne() || den.isOne())
    return;
  QLimbs g, q, r;
  qGcd(num.p, num.n, den.p, den.n, g);
  if (g.isOne())
    return;
  qDivMod(num.p, num.n, g.p, g.n, q, r);
  num.copy(q);
  qDivMod(den.p, den.n, g.p, g.n, q, r);
  den.copy(q);
}

CS_INLINE void qFromDec(const char* s, char*& out, size_t& n) {
  if (!s || !s[0])
    s = "0";
  size_t len = strlen(s);
  size_t i = 0;
  while (i + 1 < len && s[i] == '0')
    ++i;
  if (s[i] == '0') {
    uint32_t* p = (uint32_t*)malloc(sizeof(uint32_t));
    p[0] = 0;
    out = (char*)p;
    n = 1;
    return;
  }
  size_t digits = len - i;
  size_t cap = digits / 9 + 3;
  uint32_t* p = (uint32_t*)malloc(cap * sizeof(uint32_t));
  memset(p, 0, cap * sizeof(uint32_t));
  size_t nlimb = 0;
  size_t k = i;
  while (k < len) {
    size_t take = len - k;
    if (take > 9)
      take = 9;
    uint32_t chunk = 0;
    uint32_t scale = 1;
    for (size_t t = 0; t < take; ++t) {
      chunk = chunk * 10u + (uint32_t)(s[k + t] - '0');
      scale *= 10u;
    }
    uint64_t carry = chunk;
    size_t j = 0;
    while (j < nlimb || carry) {
      if (j >= cap) {
        cap = cap * 2 + 2;
        uint32_t* np = (uint32_t*)realloc(p, cap * sizeof(uint32_t));
        if (np)
          p = np;
      }
      uint64_t cur = carry;
      if (j < nlimb)
        cur += (uint64_t)p[j] * scale;
      p[j] = (uint32_t)cur;
      carry = cur >> 32;
      ++j;
    }
    nlimb = j;
    k += take;
  }
  while (nlimb > 1 && p[nlimb - 1] == 0)
    --nlimb;
  if (nlimb == 0) {
    p[0] = 0;
    nlimb = 1;
  }
  out = (char*)p;
  n = nlimb;
}

CS_INLINE char* qDupLimbs(const char* s, size_t n) {
  if (!s || n == 0) {
    uint32_t* p = (uint32_t*)malloc(sizeof(uint32_t));
    p[0] = 0;
    return (char*)p;
  }
  uint32_t* p = (uint32_t*)malloc(n * sizeof(uint32_t));
  memcpy(p, s, n * sizeof(uint32_t));
  return (char*)p;
}

CS_INLINE char* qLimbOne() {
  uint32_t* p = (uint32_t*)malloc(sizeof(uint32_t));
  p[0] = 1;
  return (char*)p;
}

CS_INLINE uint64_t qAbsLong(long v) {
  if (v >= 0)
    return (uint64_t)v;
  return (uint64_t)0 - (uint64_t)v;
}

CS_INLINE const uint32_t* qL(const char* p) { return (const uint32_t*)p; }

CS_INLINE double qLimbsToDouble(const char* raw, size_t n) {
  if (!raw || n == 0)
    return 0;
  const uint32_t* p = (const uint32_t*)raw;
  double v = 0;
  for (size_t i = n; i-- > 0; )
    v = v * (double)CS_Q_BASE + (double)p[i];
  return v;
}

#ifndef QBASE_TEST
CS_INLINE bool qRawZero(const char* p, size_t n) {
  if (!p || n == 0)
    return true;
  const uint32_t* a = (const uint32_t*)p;
  return n == 1 && a[0] == 0;
}

CS_INLINE int qCmpAbsQ(const CSARITHMETIC::csQNUMBER& a, const CSARITHMETIC::csQNUMBER& b) {
  QLimbs n1, n2;
  qMul(qL(a.numerator), a.numSize, qL(b.denominator), b.denomSize, n1);
  qMul(qL(a.denominator), a.denomSize, qL(b.numerator), b.numSize, n2);
  return qCmpLimbs(n1.p, n1.n, n2.p, n2.n);
}

CS_INLINE int qCmpSigned(const CSARITHMETIC::csQNUMBER& a, const CSARITHMETIC::csQNUMBER& b) {
  bool az = qRawZero(a.numerator, a.numSize);
  bool bz = qRawZero(b.numerator, b.numSize);
  if (az && bz)
    return 0;
  if (az)
    return b.sign ? 1 : -1;
  if (bz)
    return a.sign ? -1 : 1;
  if (a.sign != b.sign)
    return a.sign ? -1 : 1;
  int c = qCmpAbsQ(a, b);
  return a.sign ? -c : c;
}

CS_INLINE void qChopDec(char* s, size_t keep) {
  size_t n = strlen(s);
  if (keep >= n)
    return;
  if (keep == 0) {
    s[0] = '0';
    s[1] = 0;
    return;
  }
  s[keep] = 0;
}

CS_INLINE void qReducePair(char*& numerator, size_t& numSize, char*& denominator, size_t& denomSize, size_t sizeCondition) {
  char* ns = CSARITHMETIC::csLimbsToDec(numerator, numSize);
  char* ds = CSARITHMETIC::csLimbsToDec(denominator, denomSize);
  size_t nd = strlen(ns), dd = strlen(ds);
  size_t maxd = nd > dd ? nd : dd;
  size_t mind = nd < dd ? nd : dd;
  if (maxd > sizeCondition && mind > maxd - sizeCondition) {
    size_t diff = maxd - sizeCondition;
    qChopDec(ns, nd > dd ? sizeCondition : nd - diff);
    qChopDec(ds, dd > nd ? sizeCondition : dd - diff);
    free(numerator);
    free(denominator);
    qFromDec(ns, numerator, numSize);
    qFromDec(ds, denominator, denomSize);
  }
  free(ns);
  free(ds);
}
#endif

#ifndef QBASE_TEST
CS_INLINE void rFromDigits(const char* s, QLimbs& o) {
  char* raw = 0;
  size_t n = 0;
  qFromDec(s && s[0] ? s : "0", raw, n);
  o.copyFrom((const uint32_t*)raw, n);
  free(raw);
}

CS_INLINE void rMulPow10(QLimbs& a, int k);

CS_INLINE void rLoad(const CSARITHMETIC::csRNUMBER& a, QLimbs& o) {
  if (!a.mantissa || a.mantSize == 0) {
    o.setZero();
    return;
  }
  o.copyFrom((const uint32_t*)a.mantissa, a.mantSize);
}

CS_INLINE void rSave(CSARITHMETIC::csRNUMBER& a, const QLimbs& v) {
  size_t n = v.n ? v.n : 1;
  uint32_t* p = (uint32_t*)malloc(n * sizeof(uint32_t));
  if (!v.p || v.n == 0)
    p[0] = 0;
  else
    memcpy(p, v.p, n * sizeof(uint32_t));
  if (a.mantissa)
    free(a.mantissa);
  a.mantissa = (char*)p;
  a.mantSize = n;
}

CS_INLINE char* rExportDigits(const CSARITHMETIC::csRNUMBER& a) {
  if (!a.mantissa || a.mantSize == 0)
    return CSARITHMETIC::csLimbsToDec(0, 0);
  return CSARITHMETIC::csLimbsToDec(a.mantissa, a.mantSize);
}

CS_INLINE int rDecDigits(const QLimbs& a) {
  if (a.isZero())
    return 1;
  if (a.n <= 2) {
    uint64_t v = a.p[0];
    if (a.n == 2)
      v += (uint64_t)a.p[1] << 32;
    int d = 1;
    while (v >= 10) {
      v /= 10;
      ++d;
    }
    return d;
  }
  double hi = (double)a.p[a.n - 1] * 4294967296.0 + (double)a.p[a.n - 2];
  double lg = std::log10(hi) + (double)(a.n - 2) * (32.0 * std::log10(2.0));
  if (lg < 0)
    lg = 0;
  return (int)std::floor(lg) + 1;
}

CS_INLINE void rDivPow10(QLimbs& a, int k) {
  while (k > 0 && !a.isZero()) {
    int take = k > 9 ? 9 : k;
    uint32_t scale = 1;
    for (int i = 0; i < take; ++i)
      scale *= 10u;
    uint64_t rem = 0;
    for (size_t i = a.n; i-- > 0; ) {
      uint64_t cur = (rem << 32) | a.p[i];
      a.p[i] = (uint32_t)(cur / scale);
      rem = cur % scale;
    }
    a.trim();
    k -= take;
  }
}

CS_INLINE int rCmpValue(const CSARITHMETIC::csRNUMBER& a, const CSARITHMETIC::csRNUMBER& b) {
  QLimbs A, B;
  rLoad(a, A);
  rLoad(b, B);
  if (A.isZero() && B.isZero())
    return 0;
  if (A.isZero())
    return b.sign ? 1 : -1;
  if (B.isZero())
    return a.sign ? -1 : 1;
  int ea = a.exponent, eb = b.exponent;
  int e = ea < eb ? ea : eb;
  if (ea > e)
    rMulPow10(A, ea - e);
  if (eb > e)
    rMulPow10(B, eb - e);
  int c = qCmpLimbs(A.p, A.n, B.p, B.n);
  if (!c)
    return 0;
  if (a.sign != b.sign)
    return a.sign ? -1 : 1;
  return a.sign ? -c : c;
}

CS_INLINE void rMulPow10(QLimbs& a, int k) {
  if (k <= 0 || a.isZero())
    return;
  while (k > 0) {
    int take = k > 9 ? 9 : k;
    uint32_t scale = 1;
    for (int i = 0; i < take; ++i)
      scale *= 10u;
    QLimbs t;
    t.reserve(a.n + 1);
    qMulSmall(a.p, a.n, scale, t.p, a.n + 1);
    t.n = a.n + 1;
    t.trim();
    a.copy(t);
    k -= take;
  }
}

CS_INLINE int rTrim10(QLimbs& a) {
  int k = 0;
  while (!a.isZero()) {
    uint64_t rem = 0;
    for (size_t i = a.n; i-- > 0; )
      rem = ((rem << 32) | a.p[i]) % 10u;
    if (rem)
      break;
    rem = 0;
    for (size_t i = a.n; i-- > 0; ) {
      uint64_t cur = (rem << 32) | a.p[i];
      a.p[i] = (uint32_t)(cur / 10u);
      rem = cur % 10u;
    }
    a.trim();
    ++k;
  }
  return k;
}

CS_INLINE void rStoreMant(CSARITHMETIC::csRNUMBER& rn, const QLimbs& v) {
  if (rn.mantissa)
    free(rn.mantissa);
  rn.mantissa = CSARITHMETIC::csLimbsToDec(v.p, v.n ? v.n : 1);
  rn.mantSize = strlen(rn.mantissa);
}

CS_INLINE void rCombine(const QLimbs& left, int leftNeg, const QLimbs& right, int rightNeg, QLimbs& out, int& outNeg) {
  if (!leftNeg) {
    if (rightNeg) {
      bool neg = false;
      qSubAbs(left.p, left.n, right.p, right.n, out, neg);
      outNeg = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
    } else {
      qAdd(left.p, left.n, right.p, right.n, out);
      outNeg = CS_POSITIVE_NUMBER;
    }
  } else if (rightNeg) {
    qAdd(left.p, left.n, right.p, right.n, out);
    outNeg = CS_NEGATIVE_NUMBER;
  } else {
    bool neg = false;
    qSubAbs(right.p, right.n, left.p, left.n, out, neg);
    outNeg = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  }
  if (out.isZero())
    outNeg = CS_POSITIVE_NUMBER;
}
#endif

}
