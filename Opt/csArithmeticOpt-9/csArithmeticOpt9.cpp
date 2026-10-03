#include "csArithmeticOpt9.h"

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


enum { CS_STACK_MAX = 2048 };

struct csScratch {
  char small[CS_STACK_MAX + 1];
  char* p;
  bool heap;
  CS_FORCE_INLINE csScratch() {
    p = 0;
    heap = false;
  }
  CS_FORCE_INLINE void alloc(size_t n, char fill) {
    if (n <= (size_t)CS_STACK_MAX) {
      p = small;
      heap = false;
    } else {
      p = (char*)malloc(n + 1);
      heap = true;
    }
    if (fill)
      for (size_t i = 0; i < n; ++i)
        p[i] = fill;
    p[n] = '\0';
  }
  CS_FORCE_INLINE void copyFrom(const char* s, size_t n) {
    alloc(n, 0);
    for (size_t i = 0; i < n; ++i)
      p[i] = s[i];
    p[n] = '\0';
  }
  CS_FORCE_INLINE void release() {
    if (heap && p)
      free(p);
    p = 0;
    heap = false;
  }
  CS_FORCE_INLINE ~csScratch() { release(); }
};

CS_FORCE_INLINE static void csPlaceAligned(const char* src, size_t srcSize, char* dst, size_t opSize) {
  size_t delta = opSize - srcSize;
  for (size_t i = 0; i < srcSize; ++i)
    dst[delta + i] = src[i];
  dst[opSize] = '\0';
}

CS_FORCE_INLINE static bool csAlignSub(const char* a0, size_t aSize, const char* b0, size_t bSize,
                       char*& a, char*& b, size_t opSize) {
  csPlaceAligned(a0, aSize, a, opSize);
  csPlaceAligned(b0, bSize, b, opSize);
  bool neg = false;
  if (aSize < bSize)
    neg = true;
  else if (aSize == bSize) {
    for (size_t i = 0; i < opSize; ++i) {
      if (a[i] < b[i]) { neg = true; break; }
      if (a[i] > b[i]) break;
    }
  }
  if (neg) {
    char* tmp = a;
    a = b;
    b = tmp;
  }
  return neg;
}

CS_FORCE_INLINE CSARITHMETIC::csQNUMBER::csQNUMBER(CSARITHMETIC::csQRaw) {
  numerator = 0;
  denominator = 0;
  numSize = 0;
  denomSize = 0;
  sign = CS_POSITIVE_NUMBER;
}

CS_FORCE_INLINE static void csRemainZero(char*& remain, size_t& remSize) {
  if (remain && remSize >= 1) {
    remain[0] = '0';
    remain[1] = '\0';
  } else {
    remain = (char*)malloc(2);
    remain[0] = '0';
    remain[1] = '\0';
  }
  remSize = 1;
}

CS_FORCE_INLINE static void csGcdModStep(char*& a, char*& b, char*& remain,
                        size_t& aSize, size_t& bSize, size_t& remSize,
                        csScratch buf[3], int& ia, int& ib, int& ir, size_t cap) {
  remain = buf[ir].p;
  remSize = cap;
  CSARITHMETIC::makeModulusQ(a, b, remain, aSize, bSize, remSize);
  if (remain != buf[ir].p) {
    buf[ir].p = remain;
    buf[ir].heap = true;
  }
  int dead = ia;
  ia = ib;
  a = b;
  aSize = bSize;
  ib = ir;
  b = remain;
  bSize = remSize;
  ir = dead;
}


namespace {
struct AddStorage {
  csBIDIGITS v[58][58];
  constexpr AddStorage() : v{} {
    for (int j = 0; j < 58; ++j)
      for (int i = 0; i < 58; ++i) {
        if (j >= 48 && i >= 48) {
          int s = (i - 48) + (j - 48);
          v[j][i] = csBIDIGITS{ (uchar)(s / 10 + 48), (uchar)(s % 10 + 48) };
        } else {
          v[j][i] = csBIDIGITS{ '0', '0' };
        }
      }
  }
};
struct MulStorage {
  csBIDIGITS v[58][58];
  constexpr MulStorage() : v{} {
    for (int j = 0; j < 58; ++j)
      for (int i = 0; i < 58; ++i) {
        if (j >= 48 && i >= 48) {
          int s = (i - 48) * (j - 48);
          v[j][i] = csBIDIGITS{ (uchar)(s / 10 + 48), (uchar)(s % 10 + 48) };
        } else {
          v[j][i] = csBIDIGITS{ '0', '0' };
        }
      }
  }
};
struct SubStorage {
  csBIDIGITS v[58][58];
  constexpr SubStorage() : v{} {
    for (int j = 0; j < 58; ++j)
      for (int i = 0; i < 58; ++i) {
        if (j >= 47 && i >= 47) {
          int d = j - i;
          if (d >= 0)
            v[j][i] = csBIDIGITS{ 48, (uchar)(d + 48) };
          else
            v[j][i] = csBIDIGITS{ 49, (uchar)(58 + d) };
        } else {
          v[j][i] = csBIDIGITS{ '0', '0' };
        }
      }
  }
};
struct DivStorage {
  char v[256][256];
  constexpr DivStorage() : v{} {
    for (int j = 0; j < 256; ++j)
      for (int i = 0; i < 256; ++i) {
        if (j > 48 && i >= 49)
          v[j][i] = (char)((j - 48) / (i - 48) + 48);
        else
          v[j][i] = '0';
      }
  }
};
constexpr AddStorage gAdd{};
constexpr MulStorage gMul{};
constexpr SubStorage gSub{};
constexpr DivStorage gDiv{};
static_assert(gAdd.v[48][48].tens == '0' && gAdd.v[48][48].units == '0', "0+0");
static_assert(gAdd.v[57][57].tens == '1' && gAdd.v[57][57].units == '8', "9+9");
static_assert(gMul.v[57][57].tens == '8' && gMul.v[57][57].units == '1', "9*9");
static_assert(gMul.v[48][57].units == '0', "0*9");
static_assert(gSub.v[50][50].tens == 48 && gSub.v[50][50].units == 48, "0");
static_assert(gSub.v[48][49].tens == 49 && gSub.v[48][49].units == 57, "borrow");
static_assert(gDiv.v[57][49] == '9', "9/1");
static_assert(gDiv.v[57][57] == '1', "9/9");
static_assert(gDiv.v[48][49] == '0', "row zero");
}
#define addTable gAdd.v
#define mulTable gMul.v
#define subTable gSub.v
#define divTable gDiv.v


/* ---- src/csArithUtils.cpp ---- */
#include <string.h>

using namespace __mem_man;
using namespace __ar_man;

CS_FORCE_INLINE void CSARITHMETIC_API __mem_man::csReallocString(char** str, size_t size)
{
  *str = (char*)realloc(*str, size+1);
  str[0][size]= '\0';
}

CS_FORCE_INLINE void CSARITHMETIC_API __mem_man::csReallocString(char** str, size_t size, size_t newSize, char cFill)
{
  *str = (char*)realloc(*str, newSize+1);
  for(size_t i=size; i<newSize; i++)
  {
    (*str)[i] = cFill;
  }
  str[0][newSize]= '\0';
}

CS_FORCE_INLINE char* CSARITHMETIC_API __mem_man::csAllocCharPtr(size_t n,size_t n1, char init)
{
    //n1 = n+1
  char*t = (char*)malloc(n1);
  for(size_t i=0; i<n; i++)
  {
    t[i] = init;
  }
  t[n] = '\0';
  return t;
}

CS_FORCE_INLINE char* CSARITHMETIC_API __mem_man::csAllocCharPtr(size_t n, char init)
{
    //n1 = n+1
  char*t = (char*)malloc(n+1);
  for(size_t i=0; i<n; i++)
  {
    t[i] = init;
  }
  t[n] = '\0';
  return t;
}


CS_FORCE_INLINE char* CSARITHMETIC_API __mem_man::newString(const char*cstr)
{
  size_t sz = strlen(cstr);
  char* str = csAlloc<char>(sz+1);
  for(size_t i=0; i<sz; i++)
  {
    str[i] = cstr[i];
  }
  str[sz] = '\0';
  return str;
}

CS_FORCE_INLINE char* CSARITHMETIC_API __mem_man::newString(const char*cstr, size_t size)
{
  char* str = csAlloc<char>(size+1);
  for(size_t i=0; i<size; i++)
  {
    str[i] = cstr[i];
  }
  str[size] = '\0';
  return str;
}

CS_FORCE_INLINE char* CSARITHMETIC_API __mem_man::newString(char*cstr, size_t size)
{
  char* str = csAlloc<char>(size+1);
  for(size_t i=0; i<size; i++)
  {
    str[i] = cstr[i];
  }
  str[size] = '\0';
  return str;
}

CS_FORCE_INLINE char* CSARITHMETIC_API __mem_man::newString(const char*cstr, size_t begin, size_t end)
{
  size_t size = end-begin;
  
    char* str = csAlloc<char>(size+1);
    for(size_t i=begin, j= 0; i<end; i++, j++)
    {
      str[j] = cstr[i];
    }
    str[size] = '\0';
    return str;
  
}


CS_FORCE_INLINE char* __mem_man::intToString(int nb, size_t& sz)
{
  char tmp[32];
  sprintf(tmp, "%d", nb);
  sz = strlen(tmp);
  char* str = csAlloc<char>(sz + 1);
  for (size_t i = 0; i < sz; ++i)
    str[i] = tmp[i];
  str[sz] = '\0';
  return str;
}

CS_FORCE_INLINE char* __mem_man::uLongToString(size_t nb, size_t& sz)
{
  char tmp[32];
  sprintf(tmp, "%ld", nb);
  sz = strlen(tmp);
  char* str = csAlloc<char>(sz + 1);
  for (size_t i = 0; i < sz; ++i)
    str[i] = tmp[i];
  str[sz] = '\0';
  return str;
}

CS_FORCE_INLINE void __ar_man::getReady(char*&a,char*&b, size_t&opSize)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  opSize = aSize>bSize?aSize:bSize;

  if(opSize > bSize)
  {
    size_t delta = opSize-bSize;
    b = (char*)realloc(b,opSize+1);
    for(size_t i=bSize-1,j=0; j<bSize; j++,i--)
    {
      b[i+delta] = b[i];
    }
    for(size_t i=0; i<delta; i++)
    {
      b[i] = '0';
    }
  }
  else if(opSize > aSize)
  {
    size_t delta = opSize-aSize;
    a = (char*)realloc(a,opSize+1);
    for(size_t i=aSize-1,j=0; j<aSize; j++,i--)
    {
      a[i+delta] = a[i];
    }
    for(size_t i=0; i<delta; i++)
    {
      a[i] = '0';
    }
  }
}

CS_FORCE_INLINE bool __ar_man::isaGreater(char*a,char*b,size_t opSize, size_t ibegin)
{
  int j = 0, k = 0;
  bool bl = 0;
  for(size_t i=0; i<opSize; i++)
  {
    int aid=i+ibegin;
    if(a[aid]<b[i])
    {
      j++;
    }
    else if(a[aid] > b[i])
    {
      k++;
    }
    if(k>j)
    {
      bl = 1;
      break;
    }
    else if(k<j)
    {
      break;
    }
  }
  return bl;
}

CS_FORCE_INLINE bool __ar_man::isaGreaterEqual(char*a,char*b,size_t opSize, size_t ibegin)
{
  int i, j = 0, k = 0;
  bool bl = 0;
  for(i=0; i<opSize; i++)
  {
    int aid=i+ibegin;
    if(a[aid]<b[i])
    {
      j++;
    }
    else if(a[aid]>b[i])
    {
      k++;
    }
    if(k>j)
    {
      bl = 1;
      break;
    }
    else if(k<j)
    {
      break;
    }
  }
  if(i == opSize)
    bl = 1;

  return bl;
}

CS_FORCE_INLINE bool __ar_man::isaGreater2(char*a,char*b,size_t aSize,size_t bSize, size_t ibegin)
{

  bool bl = 0;
  if(aSize > bSize)
  {
    bl=1;
  }
  else if(aSize==bSize)
  {
  int j = 0, k = 0;
  for(size_t i=0; i<aSize; i++)
  {
    int aid=i+ibegin;
    if(a[aid]<b[i])
    {
      j++;
    }
    else if(a[aid] > b[i])
    {
      k++;
    }
    if(k>j)
    {
      bl = 1;
      break;
    }
    else if(k<j)
    {
      break;
    }
  }
  }
  return bl;
}

CS_FORCE_INLINE bool __ar_man::isaGreaterEqual2(char*a,char*b,size_t aSize, size_t bSize, size_t ibegin)
{
  if(aSize > bSize)
  {
    return 1;
  }
  else if(aSize < bSize)
  {
    return 0;
  }
  
  for(size_t i=0; i<aSize; i++)
  {
    int aid=i+ibegin;
    if(a[aid]<b[i])
    {
      return 0;
    }
    else if(a[aid]>b[i])
    {
      return 1;
    }

  }
  
  return 1;
}

CS_FORCE_INLINE int __ar_man::getReadySub(char*&a,char*&b, size_t&opSize)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  opSize = aSize>bSize?aSize:bSize;

  int sign = 1;
  if(opSize > bSize)
  {
    size_t delta = opSize-bSize;
    b = (char*)realloc(b,opSize+1);
    for(size_t i=bSize-1,j=0; j<bSize; j++,i--)
    {
      b[i+delta] = b[i];
    }
    for(size_t i=0; i<delta; i++)
    {
      b[i] = '0';
    }

  }
  else if(opSize > aSize)
  {
    size_t delta = opSize-aSize;
    a = (char*)realloc(a,opSize+1);
    for(size_t i=aSize-1,j=0; j<aSize; j++,i--)
    {
      a[i+delta] = a[i];
    }
    for(size_t i=0; i<delta; i++)
    {
      a[i] = '0';
    }

    sign = -1;
    char*tmp = a;
    a = b;
    b = tmp;
  }
  else
  {
    for(size_t i=0; i<opSize; i++)
    {
      if(a[i]<b[i])
      {
        sign = -1;
        char*tmp = a;
        a = b;
        b = tmp;
        break;
      }
    }
  }
  return sign;
}

CS_FORCE_INLINE void __ar_man::getReady2(const char*a0,const char*b0, char*&a, char*&b, size_t&opSize)
{
  size_t aSize = strlen(a0);
  size_t bSize = strlen(b0);
  b = csAlloc<char>(bSize+1);
  a = csAlloc<char>(aSize+1);
  opSize = aSize>bSize?aSize:bSize;
  if(aSize == bSize)
  {
    for(size_t i=0; i<aSize; i++)
      a[i] = a0[i];
    a[aSize] = '\0';
    for(size_t i=0; i<bSize; i++)
      b[i] = b0[i];
    b[bSize] = '\0';
  }
  else if(opSize > bSize)
  {
    size_t delta = opSize-bSize;
    b = (char*)realloc(b,opSize+1);
    b[opSize]='\0';
    for(size_t i=bSize-1,j=0; j<bSize; j++,i--)
    {
      b[i+delta] = b0[i];
    }
    for(size_t i=0; i<delta; i++)
    {
      b[i] = '0';
    }

    for(size_t i=0; i<aSize; i++)
      a[i] = a0[i];
    a[aSize] = '\0';
  }
  else if(opSize > aSize)
  {
    size_t delta = opSize-aSize;
    a = (char*)realloc(a,opSize+1);
    a[opSize]='\0';
    for(size_t i=aSize-1,j=0; j<aSize; j++,i--)
    {
      a[i+delta] = a0[i];
    }
    for(size_t i=0; i<delta; i++)
    {
      a[i] = '0';
    }
    aSize = opSize;

    for(size_t i=0; i<bSize; i++)
      b[i] = b0[i];
    b[bSize] = '\0';
  }
}


CS_FORCE_INLINE bool __ar_man::getReadySub2(const char*a0,const char*b0, char*&a, char*&b, size_t&opSize)
{// avoid putting zeros in front of numbers !!!!!
  size_t aSize = strlen(a0);
  size_t bSize = strlen(b0);
  b = csAlloc<char>(bSize+1);
  a = csAlloc<char>(aSize+1);
  bool sign = 0;
  opSize = aSize>bSize?aSize:bSize;
  if(aSize == bSize)
  {
    for(size_t i=0; i<aSize; i++)
      a[i] = a0[i];
    a[aSize] = '\0';
    for(size_t i=0; i<bSize; i++)
      b[i] = b0[i];
    b[bSize] = '\0';

    for(size_t i=0; i<opSize; i++)
    {
      if(a[i]<b[i])
      {
        sign = 1;
        char*tmp = a;
        a = b;
        b = tmp;
        break;
      }
      else if(a[i]>b[i])
      {
        break;
      }
    }
  }
  else if(opSize > bSize)
  {
    size_t delta = opSize-bSize;
    b = (char*)realloc(b,opSize+1);
    b[opSize]='\0';
    for(size_t i=bSize-1,j=0; j<bSize; j++,i--)
    {
      b[i+delta] = b0[i];
    }
    for(size_t i=0; i<delta; i++)
    {
      b[i] = '0';
    }

    for(size_t i=0; i<aSize; i++)
      a[i] = a0[i];
    a[aSize] = '\0';
  }
  else if(opSize > aSize)
  {
    size_t delta = opSize-aSize;
    a = (char*)realloc(a,opSize+1);
    a[opSize]='\0';
    for(size_t i=aSize-1,j=0; j<aSize; j++,i--)
    {
      a[i+delta] = a0[i];
    }
    for(size_t i=0; i<delta; i++)
    {
      a[i] = '0';
    }
    aSize = opSize;

    for(size_t i=0; i<bSize; i++)
      b[i] = b0[i];
    b[bSize] = '\0';


     sign = 1;
     char*tmp = a;
     a = b;
     b = tmp;

  }
  return sign;
}

CS_FORCE_INLINE bool __ar_man::getReadySub3(const char*a0,const char*b0, char*&a, char*&b, size_t aSize, size_t bSize, size_t&opSize)
{// avoid putting zeros in front of numbers !!!!!


  int sign = 0;
  opSize = aSize>bSize?aSize:bSize;
  if(aSize == bSize)
  {
    b = csAlloc<char>(bSize+1);
    a = csAlloc<char>(aSize+1);
    for(size_t i=0; i<aSize; i++)
      a[i] = a0[i];
    a[aSize] = '\0';
    for(size_t i=0; i<bSize; i++)
      b[i] = b0[i];
    b[bSize] = '\0';


    for(size_t i=0; i<opSize; i++)
    {
      if(a[i]<b[i])
      {
        sign = 1;
        char*tmp = a;
        a = b;
        b = tmp;
        break;
      }
      else if(a[i]>b[i])
      {
        break;
      }
    }
  }
  else if(opSize > bSize)
  {
    size_t delta = opSize-bSize;
    b = csAlloc<char>(opSize+1);
    b[opSize]='\0';
    for(size_t i=bSize-1,j=0; j<bSize; j++,i--)
    {
      b[i+delta] = b0[i];
    }
    for(size_t i=0; i<delta; i++)
    {
      b[i] = '0';
    }
    a = csAlloc<char>(aSize+1);
    for(size_t i=0; i<aSize; i++)
      a[i] = a0[i];
    a[aSize] = '\0';
  }
  else if(opSize > aSize)
  {
    size_t delta = opSize-aSize;
    a = csAlloc<char>(opSize+1);
    a[opSize]='\0';
    for(size_t i=0; i<delta; i++)
    {
      a[i] = '0';
    }
    for(size_t i=aSize-1,j=0; j<aSize; j++,i--)
    {
      a[i+delta] = a0[i];
    }

    aSize = opSize;

    b = csAlloc<char>(bSize+1);
    for(size_t i=0; i<bSize; i++)
      b[i] = b0[i];
    b[bSize] = '\0';


     sign = 1;
     char*tmp = a;
     a = b;
     b = tmp;

  }
  return sign;
}


CS_FORCE_INLINE void __ar_man::shiftLeft(char*&nb, size_t size, size_t nShift)
{
  for(size_t i=nShift,j=0; i<size; i++, j++)
  {
    nb[j] = nb[i];
  }
  for(size_t j=size-nShift; j<size; j++)
  {
    nb[j] = '0';
  }
}

CS_FORCE_INLINE void __ar_man::removeLeft(char*&nb, size_t& size, size_t remLen)
{
  size_t diff = size - remLen;
  if (remLen && diff)
    memmove(nb, nb + remLen, diff);
  nb[diff] = '\0';
  size = diff;
}
CS_FORCE_INLINE void __ar_man::removeRight(char*&nb, size_t& size, size_t remLen)
{
  size_t diff = size - remLen;
  nb[diff] = '\0';
  size = diff;
}


CS_FORCE_INLINE void __ar_man::shiftLeftCopy(char* nb, char*&nbCopy, size_t nbSize, size_t nbCopyPos)
{
  for(size_t i=0; i<nbSize; i++)
  {
    nbCopy[nbCopyPos+i] = nb[i];
  }
  for(size_t j=0; j<nbCopyPos; j++)
  {
    nbCopy[j] = '0';
  }
}

CS_FORCE_INLINE char* __ar_man::shiftRightCopy(char* nb, size_t nbSize, size_t nbCopySize)
{
  char* nbCopy = csAllocCharPtr(nbCopySize, '0');
  size_t nbCopyPos = nbCopySize-nbSize;
  for(size_t i=0; i<nbSize; i++)
  {
    nbCopy[nbCopyPos+i] = nb[i];
  }
  return nbCopy;
}


CS_FORCE_INLINE void __ar_man::shiftRight(char*&nb, size_t size, size_t nShift)
{
  size_t n1 = size-1, n = n1-nShift, m = size-nShift;
  for(size_t i=0; i<m; i++)
  {
    nb[n-i] = nb[n1-i];
  }
  for(size_t j=0; j<nShift; j++)
  {
    nb[j] = '0';
  }
}


CS_FORCE_INLINE void __ar_man::skipZeros(char*a, size_t aSize, size_t& skipLen)
{
  for(size_t i=skipLen; i<aSize; i++)
  {
    if(a[i] != '0')
    {
      skipLen = i;
      return;
    }
  }
  skipLen = aSize-1;
}


CS_FORCE_INLINE void __ar_man::skipZeros2(char*a, size_t aSize, size_t& skipLen, size_t&incr)
{
  for(size_t i=0; i<aSize; i++)
  {
    if(a[i] != '0')
    {
      skipLen = i;
      incr++;
      break;
    }
  }
}


CS_FORCE_INLINE bool __ar_man::removeFrontZeros(char*&a, size_t& size)
{
  for(size_t i=0; i<size; i++)
  {
    if(a[i] != '0')
    {
      if(i != 0)
        removeLeft(a,size,i);
      return 1;
    }
  }
  
  // reveals some bugs
  if(size > 1)
  {
    a[0] = '0';
    a[1] = '\0';
    size = 1;
  }
  return 0;
}

CS_FORCE_INLINE void __ar_man::addFrontZeros(char*&a, size_t& size, size_t nZeros)
{
  size = size+nZeros;
  a = (char*)realloc(a,size+1);
  a[size] = '\0';
  shiftRight(a,size,nZeros);
}

CS_FORCE_INLINE void __ar_man::fillString(char*&str, const char* cstr, size_t size)
{
  for(size_t i=0; i<size; i++)
  {
    str[i] = cstr[i];
  }
  str[size] = '\0';
}

CS_FORCE_INLINE char* __ar_man::filledString(const char* cstr, size_t size)
{
  char*str = (char*)malloc(size+1);
  
  for(size_t i=0; i<size; i++)
  {
    str[i] = cstr[i];
  }
  str[size] = '\0';
  return str;
}

/* ---- src/csAddition.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::createAdditionTable()
{
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeAddition(char*a, char*b, char*&result, size_t opSize, size_t& resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  size_t m=opSize-1;

  for(size_t i=m,j=0; j<opSize; j++, i-=1)
  {
    csBIDIGITS bd1 = addTable[int(a[i])][int(b[i])];

    csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
    prevCarry = addTable[bd1.tens][bd2.tens];
    *(result+i+1) = bd2.units;
  }
  result[0] = prevCarry.units;//important !!
  removeFrontZeros(result, resSize);

}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeAddition(char*a,char*b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t opSize = aSize > bSize ? aSize : bSize;
  csScratch aBuf, bBuf;
  aBuf.alloc(opSize, '0');
  bBuf.alloc(opSize, '0');
  csPlaceAligned(a, aSize, aBuf.p, opSize);
  csPlaceAligned(b, bSize, bBuf.p, opSize);
  size_t resSize = opSize + 1;
  char* result = csAllocCharPtr(resSize, '0');
  makeAddition(aBuf.p, bBuf.p, result, opSize, resSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeAddition(char*a,char*b, size_t&resSize)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t opSize = aSize > bSize ? aSize : bSize;
  csScratch aBuf, bBuf;
  aBuf.alloc(opSize, '0');
  bBuf.alloc(opSize, '0');
  csPlaceAligned(a, aSize, aBuf.p, opSize);
  csPlaceAligned(b, bSize, bBuf.p, opSize);
  resSize = opSize + 1;
  char* result = csAllocCharPtr(resSize, '0');
  makeAddition(aBuf.p, bBuf.p, result, opSize, resSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeAddition(const char*a,const char*b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t opSize = aSize > bSize ? aSize : bSize;
  csScratch aBuf, bBuf;
  aBuf.alloc(opSize, '0');
  bBuf.alloc(opSize, '0');
  csPlaceAligned(a, aSize, aBuf.p, opSize);
  csPlaceAligned(b, bSize, bBuf.p, opSize);
  size_t resSize = opSize + 1;
  char* result = csAllocCharPtr(resSize, '0');
  makeAddition(aBuf.p, bBuf.p, result, opSize, resSize);
  return result;
}

/* ---- src/csSubstraction.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeSubstractionTable()
{
}


CS_FORCE_INLINE csBIDIGITS CSARITHMETIC_API CSARITHMETIC::substractionTransform(int i)
{
  csBIDIGITS bd;
  if(i>=0)
  {
    bd = {48,(uchar)(i+48)};
  }
  else
  {
    bd = {49, (uchar)(58 + i)};
  }
  return bd;
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a, char*b, char*&result, size_t opSize)
{
  csBIDIGITS prevCarry={'0','0'};
  size_t m=opSize-1;
  
  for(size_t i=m,j=0; j<opSize; j++, i-=1)
  {

    prevCarry = subTable[int(a[i])][prevCarry.tens];

    csBIDIGITS bd = subTable[prevCarry.units][b[i]];
    prevCarry.tens = prevCarry.tens>bd.tens?prevCarry.tens:bd.tens;
    result[i] = bd.units;
  }
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a,char*b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t opSize = aSize > bSize ? aSize : bSize;
  csScratch aBuf, bBuf;
  aBuf.alloc(opSize, '0');
  bBuf.alloc(opSize, '0');
  char* pa = aBuf.p;
  char* pb = bBuf.p;
  csAlignSub(a, aSize, b, bSize, pa, pb, opSize);
  char* result = csAllocCharPtr(opSize, '0');
  makeSubstraction(pa, pb, result, opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a,char*b, bool& sign)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t opSize = aSize > bSize ? aSize : bSize;
  csScratch aBuf, bBuf;
  aBuf.alloc(opSize, '0');
  bBuf.alloc(opSize, '0');
  char* pa = aBuf.p;
  char* pb = bBuf.p;
  sign = csAlignSub(a, aSize, b, bSize, pa, pb, opSize);
  char* result = csAllocCharPtr(opSize, '0');
  makeSubstraction(pa, pb, result, opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a,char*b, size_t aSize, size_t bSize, bool& sign)
{
  size_t opSize = aSize > bSize ? aSize : bSize;
  csScratch aBuf, bBuf;
  aBuf.alloc(opSize, '0');
  bBuf.alloc(opSize, '0');
  char* pa = aBuf.p;
  char* pb = bBuf.p;
  sign = csAlignSub(a, aSize, b, bSize, pa, pb, opSize);
  char* result = csAllocCharPtr(opSize, '0');
  makeSubstraction(pa, pb, result, opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a,char*b, size_t aSize, size_t bSize, size_t& resSize, bool& sign)
{
  size_t opSize = aSize > bSize ? aSize : bSize;
  csScratch aBuf, bBuf;
  aBuf.alloc(opSize, '0');
  bBuf.alloc(opSize, '0');
  char* pa = aBuf.p;
  char* pb = bBuf.p;
  sign = csAlignSub(a, aSize, b, bSize, pa, pb, opSize);
  resSize = opSize;
  char* result = csAllocCharPtr(resSize, '0');
  makeSubstraction(pa, pb, result, resSize);
  return result;
}


CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(const char*a,const char*b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t opSize = aSize > bSize ? aSize : bSize;
  csScratch aBuf, bBuf;
  aBuf.alloc(opSize, '0');
  bBuf.alloc(opSize, '0');
  char* pa = aBuf.p;
  char* pb = bBuf.p;
  csAlignSub(a, aSize, b, bSize, pa, pb, opSize);
  char* result = csAlloc<char>(opSize + 1);
  result[opSize] = '\0';
  makeSubstraction(pa, pb, result, opSize);
  return result;
}

/* ---- src/csMultiplication.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationTable()
{
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationBase(char*a, uchar b, char*&result, size_t aSize, size_t resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  size_t m=aSize-1, i = 0, j=0, n=resSize-1;
  
  for(i=m,j=0; j<aSize; j++, i-=1)
  {
    csBIDIGITS bd1 = mulTable[int(a[i])][b];

    csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
    prevCarry = addTable[bd1.tens][bd2.tens];
    result[n-j] = bd2.units;
  }
  
  result[n-j] = prevCarry.units;
  free(result); // ----------------------------------------------------- check latter !!!!! 
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationBase2(char*a, uchar b, char*&result, size_t aSize, size_t resSize
    ,csBIDIGITS& prevCarry, size_t m, size_t n)
{
  prevCarry={'0','0'};
  size_t j, i;
  for(i=m,j=0; j<aSize; j++, i-=1)
  {
    csBIDIGITS bd1 = mulTable[int(a[i])][b];

    csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
    prevCarry = addTable[bd1.tens][bd2.tens];
    result[n-j] = bd2.units;
  }
  result[n-j] = prevCarry.units;
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeAdditionForMul(char*a, char*b, char*&result, size_t opSize, size_t resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  size_t m=opSize-1, n=resSize-opSize;

  for(size_t i=m,j=0; j<opSize; j++, i-=1)
  {
    size_t id = i+n;
    csBIDIGITS bd1 = addTable[int(a[id])][int(b[id])];
    csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
    prevCarry = addTable[bd1.tens][bd2.tens];
    result[id] = bd2.units;
    
  }
  
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeMultiplication(char*a, char*b, char*&result, char*& tmpResult,
 size_t aSize, size_t bSize, size_t resSize)
{
  
  size_t m=bSize-1, n = aSize + 1;

  csBIDIGITS prevCarry;
  size_t m1=aSize-1, n1=resSize-1;

  for(size_t i=m,j=0; j<bSize; j++, i-=1)
  {
    
    makeMultiplicationBase2(a, b[i], tmpResult, aSize, resSize,
                            prevCarry, m1, n1);
    shiftLeft(tmpResult, resSize, j);
    makeAdditionForMul(result, tmpResult, result, n+j, resSize);

  }
  
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeMultiplication(char*a, char*b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  //for more performances
  if(aSize < bSize)
  {
    size_t t = aSize;
    aSize = bSize;
    bSize = t;

    char*c = a;
    a = b;
    b = c;
  }
  //
  size_t opSize = aSize + bSize;
  size_t sz = opSize + 1;
  char*result = csAllocCharPtr(opSize, sz, '0');
  csScratch tmpBuf;
  tmpBuf.alloc(opSize, '0');
  char*tmpResult = tmpBuf.p;

  makeMultiplication(a,b,result,tmpResult,aSize,bSize,opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeMultiplication(char*a, char*b, size_t aSize, size_t bSize)
{
  size_t opSize = aSize + bSize;
  size_t sz = opSize + 1;
  char*result = csAllocCharPtr(opSize, sz, '0');
  csScratch tmpBuf;
  tmpBuf.alloc(opSize, '0');
  char*tmpResult = tmpBuf.p;

  makeMultiplication(a,b,result,tmpResult,aSize,bSize,opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeMultiplication(char*a, char*b, size_t aSize, size_t bSize, size_t& resSize)
{
  resSize = aSize + bSize;
  size_t sz = resSize + 1;
  char*result = csAllocCharPtr(resSize, sz, '0');
  csScratch tmpBuf;
  tmpBuf.alloc(resSize, '0');
  char*tmpResult = tmpBuf.p;
  
  makeMultiplication(a,b,result,tmpResult,aSize,bSize,resSize);
   
  return result;
}


CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeMultiplication(const char*a, const char*b)
{
  return makeMultiplication((char*)a,(char*)b);
}

/* ---- src/csDivision.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeDivisionTable()
{
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeSubstractionForDiv(char*a, char*b, char*&result, size_t opSize, size_t frontOffset)
{
  csBIDIGITS prevCarry={'0','0'};
  size_t m=opSize-1, n=m+frontOffset;
  
  for(size_t i=n,j=0; j<opSize; j++, i-=1)
  {

    prevCarry = subTable[int(a[i])][prevCarry.tens];

    csBIDIGITS bd = subTable[prevCarry.units][b[m-j]];
    prevCarry.tens = prevCarry.tens>bd.tens?prevCarry.tens:bd.tens;
    result[i] = bd.units;
  }
  
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationForDivBase1(char*a, uchar b, char*&result, size_t aSize, size_t resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  size_t m=aSize-1, i = 0, j=0, n=resSize-1;
  
  for(i=m,j=0; j<aSize; j++, i-=1)
  {
    csBIDIGITS bd1 = mulTable[int(a[i])][b];

    csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
    prevCarry = addTable[bd1.tens][bd2.tens];
    result[n-j] = bd2.units;
    
  }
  // la case pour la retenue n'existe pas ici, car b est choisi tel qu'il n'y aura pas de retenue
  
}
CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationForDivBase2(char*a, uchar b, char*&result, size_t aSize, size_t resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  size_t m=aSize-1, i = 0, j=0, n=resSize-1;
  
  for(i=m,j=0; j<aSize; j++, i-=1)
  {
    csBIDIGITS bd1 = mulTable[int(a[i])][b];

    csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
    prevCarry = addTable[bd1.tens][bd2.tens];
    result[n-j] = bd2.units;
    
  }
  // la case pour la retenue existe bien ici, car b est choisi tel qu'il est possible d'avoir la retenue
  result[n-j] = prevCarry.units;
  
}


CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*_a, char* _b, char*&result,
        size_t& aSize, size_t& bSize, size_t& resSize)
{
    csScratch aBuf;
  aBuf.copyFrom(_a, aSize);
  char*a = aBuf.p;
    csScratch bBuf;
  bBuf.copyFrom(_b, bSize);
  char*b = bBuf.p;
  // dangerous when changing addresses of a or b by reallocating eg with removeFrontZeros()
  if(removeFrontZeros(b,bSize)==0)
  {  
    resSize = 1;
    result = newString("0", 1);
    //this function. make sure not to free it outside of here
    return "Invalid input !";
  }
  if(removeFrontZeros(a,aSize)==0)
  {
    resSize = 1;
    result = newString("0", 1);
    //this function. make sure not to free it outside of here
    return "0";
  }

  if(bSize == 1)
  {
    if(b[0] == '1')
    {
      resSize = aSize;
      result = newString(a, resSize);
      return 0;
    }
    if(b[0] == '0')
    {
      resSize = 1;
      if(aSize == 1 && a[0] == '0')
      {
        result = newString("ind", 3);
        return "Indeterminate";
      }
      else
      {
        result = newString("inf", 3);
        return "Infinite";
      }
    }
  }

  size_t tmpResSize = bSize+1;
  csScratch tmpBuf;
  tmpBuf.alloc(tmpResSize, '0');
  char*tmpRes = tmpBuf.p;
  size_t skipZerosLen = 0, lastSzl=0;
  bool cmp;

  size_t resSize1= resSize-1;

  uchar quotientDigit = '1';// or 49

  size_t tmpResSizeForSub=bSize, qid=0;
  csScratch cpyBuf;
  cpyBuf.alloc(tmpResSize, '0');
  char* bCpy = cpyBuf.p;
  for(size_t _i = 0, _pos = tmpResSize - bSize; _i < bSize; ++_i) bCpy[_pos + _i] = b[_i];


  size_t nextSize = bSize-1;
  
  while(1)
  {
    skipZeros(a, aSize, skipZerosLen);
    size_t diff = aSize-skipZerosLen;
    nextSize++;
    tmpResSizeForSub = nextSize-skipZerosLen;
    cmp = (nextSize>skipZerosLen&&nextSize<=aSize)&&isaGreaterEqual2(a,b,tmpResSizeForSub,bSize,skipZerosLen);
    if(diff < bSize || (diff==bSize && !cmp) || nextSize>aSize)
    {
      /* The non-zero digit targeted by skipZeros has not been lowered yet. */
      if(nextSize <= skipZerosLen && a[skipZerosLen] != '0' && diff >= bSize)
      {
        qid++;
        continue;
      }
      /* The last digit of the remainder is not yet part of the comparison. */
      if(diff == bSize && !cmp && nextSize < aSize && nextSize > skipZerosLen && qid < resSize)
      {
        qid++;
        continue;
      }
      if(nextSize < skipZerosLen)
      {
        resSize = qid + skipZerosLen-nextSize+2; // +2 cause nextSize was increased
        csReallocString(&result,qid,resSize, '0');
        
        break;
      }
      
      resSize = strlen(result);
      result[resSize] = '\0';
      break;
    }

    if(cmp)
    { 
      if(tmpResSizeForSub == bSize)
      {

        if(bSize == 1)
          quotientDigit = (divTable[a[skipZerosLen]][b[0]]-1);
        else
          quotientDigit = (divTable[bicharIndex(a[skipZerosLen], a[skipZerosLen+1])][bicharIndex(b[0],b[1])]-1);

        if(quotientDigit == 48) quotientDigit = 49;
        
        makeMultiplicationForDivBase1(b, quotientDigit, tmpRes, tmpResSizeForSub, tmpResSizeForSub);
        makeSubstractionForDiv(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);

        while(isaGreaterEqual(a,b,tmpResSizeForSub,skipZerosLen))
        {
           makeSubstractionForDiv(a, b, a, tmpResSizeForSub, skipZerosLen);
           quotientDigit += 1;
        }
        
      }
      else
      {
        if(bSize == 1)
          quotientDigit = divTable[bicharIndex(a[skipZerosLen], a[skipZerosLen+1])][b[0]]-2;
        else
          quotientDigit = (divTable[tricharIndex(a[skipZerosLen], a[skipZerosLen+1], a[skipZerosLen+2])]
                                   [bicharIndex(b[0],b[1])]-1);

        if(quotientDigit <= 48) quotientDigit = 49;
        if(quotientDigit > 57) quotientDigit = 57;
      
        makeMultiplicationForDivBase2(b, quotientDigit, tmpRes, bSize, tmpResSizeForSub);
        makeSubstractionForDiv(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);

        while(isaGreaterEqual(a,bCpy,tmpResSizeForSub,skipZerosLen))
        {
           makeSubstractionForDiv(a, bCpy, a, tmpResSizeForSub, skipZerosLen);
           quotientDigit += 1;
        }
        
      }

      result[qid] = quotientDigit;
      qid++;
      
    }
    else
    {
      
      qid++;
    }

  }
  
  return 0;
}

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*_a, char* _b, char*&result, char*&remain,
        size_t& aSize, size_t& bSize, size_t& resSize, size_t& remSize)
{
  
 csScratch aBuf;
  aBuf.copyFrom(_a, aSize);
  char*a = aBuf.p;
 csScratch bBuf;
  bBuf.copyFrom(_b, bSize);
  char*b = bBuf.p;
 
  if(removeFrontZeros(b,bSize)==0)
  {  
    remSize = 1;
    remain = newString("0", 1);

    resSize = 1;
    result = newString("0", 1);
    return "Invalid input !";
  }
  if(removeFrontZeros(a,aSize)==0)
  {
    remSize = 1;
    remain = newString("0", 1);
    resSize = 1;
    result = newString("0", 1);
    return "0";
  }

  if(bSize == 1)
  {
    if(b[0] == '1')
    {
      remSize = 1;
      remain = newString("0", 1);

      resSize = aSize;
      result = newString(a, resSize);
      return 0;
    }
    if(b[0] == '0')
    {
      remSize = 1;
      remain = newString("0", 1);

      resSize = 1;
      if(aSize == 1 && a[0] == '0')
      {
        result = newString("ind", 3);
        
        return "Indeterminate";
      }
      else
      {
        result = newString("inf", 3);
        
        return "Infinite";
      }
    }
  }

  size_t tmpResSize = bSize+1;
  csScratch tmpBuf;
  tmpBuf.alloc(tmpResSize, '0');
  char*tmpRes = tmpBuf.p;
  size_t skipZerosLen = 0;
  bool cmp;

  size_t resSize1= resSize-1;

  uchar quotientDigit = '1';// or 49

  size_t tmpResSizeForSub=bSize, qid=0;
  csScratch cpyBuf;
  cpyBuf.alloc(tmpResSize, '0');
  char* bCpy = cpyBuf.p;
  for(size_t _i = 0, _pos = tmpResSize - bSize; _i < bSize; ++_i) bCpy[_pos + _i] = b[_i];
  

  size_t nextSize = bSize-1;
  
  while(1)
  {

    skipZeros(a, aSize, skipZerosLen);
    size_t diff = aSize-skipZerosLen;
    
    nextSize++;
    
    tmpResSizeForSub = nextSize-skipZerosLen;
    cmp =  (nextSize>skipZerosLen&&nextSize<=aSize)&&isaGreaterEqual2(a,b,tmpResSizeForSub,bSize,skipZerosLen);
    if(diff < bSize || (diff==bSize && !cmp) || nextSize>aSize)
    {
      if(nextSize <= skipZerosLen && a[skipZerosLen] != '0' && diff >= bSize)
      {
        qid++;
        continue;
      }
      if(diff == bSize && !cmp && nextSize < aSize && nextSize > skipZerosLen && qid < resSize)
      {
        qid++;
        continue;
      }

      if(diff > remSize) remain = (char*)realloc(remain, diff+1);
      for(size_t i=0; i<diff; i++)
      {
        remain[i] = a[i+skipZerosLen];
      }
      remain[diff] = '\0';
      remSize = diff;

      if(nextSize < skipZerosLen)
      {
        resSize = qid + skipZerosLen-nextSize+2; // +2 cause nextSize was increased
        csReallocString(&result,qid,resSize, '0');
        
        break;
      }

      resSize = strlen(result);
      result[resSize] = '\0';
      
      break;
    }


    if(cmp)
    {
      
      if(tmpResSizeForSub == bSize)
      {
        if(bSize == 1)
          quotientDigit = (divTable[a[skipZerosLen]][b[0]]-1);
        else
          quotientDigit = (divTable[bicharIndex(a[skipZerosLen], a[skipZerosLen+1])][bicharIndex(b[0],b[1])]-1);

        if(quotientDigit == 48) quotientDigit = 49;


        makeMultiplicationForDivBase1(b, quotientDigit, tmpRes, tmpResSizeForSub, tmpResSizeForSub);
        makeSubstractionForDiv(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);

        while(isaGreaterEqual(a,b,tmpResSizeForSub,skipZerosLen))
        {
           makeSubstractionForDiv(a, b, a, tmpResSizeForSub, skipZerosLen);
           quotientDigit += 1;
        }
        
      }
      else
      {
        
      //convert the first and second digits to a number, to find the quotient with the first digit of b

        if(bSize == 1)
          quotientDigit = divTable[bicharIndex(a[skipZerosLen], a[skipZerosLen+1])][b[0]]-2;
        else
          quotientDigit = (divTable[tricharIndex(a[skipZerosLen], a[skipZerosLen+1], a[skipZerosLen+2])]
                                 [bicharIndex(b[0],b[1])]-1);

        if(quotientDigit <= 48) quotientDigit = 49;
        if(quotientDigit > 57) quotientDigit = 57;
        
        makeMultiplicationForDivBase2(b, quotientDigit, tmpRes, bSize, tmpResSizeForSub);
        makeSubstractionForDiv(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);

        while(isaGreaterEqual(a,bCpy,tmpResSizeForSub,skipZerosLen))
        {
           makeSubstractionForDiv(a, bCpy, a, tmpResSizeForSub, skipZerosLen);
           quotientDigit += 1;
        }
        
      }

      result[qid] = quotientDigit;
      qid++;
    }
    else
    {
      qid++;
    }

  }
  
  return 0;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*a, char* b, char*&remain)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  int rs;
  size_t resSize = (rs=(aSize-bSize+1))>1?rs:1;
  size_t remSize = bSize;

  remain = csAllocCharPtr(remSize, '0');

  char*quotient = csAllocCharPtr(resSize, '0');

  makeDivisionQ(a, b, quotient, remain, aSize, bSize, resSize, remSize);

  return quotient;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*a, char* b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  int rs;
  size_t resSize = (rs=(aSize-bSize+1))>1?rs:1;
  size_t remSize = bSize;

  char*quotient = csAllocCharPtr(resSize, '0');

  makeDivisionQ(a, b, quotient, aSize, bSize, resSize);

  return quotient;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*a, char* b, size_t aSize, size_t bSize, size_t& resSize)
{
  int rs;
  resSize = (rs=(aSize-bSize+1))>1?rs:1;
  size_t remSize = bSize, remSize1 = remSize+1;

  char*quotient = csAllocCharPtr(resSize, '0');
  
  makeDivisionQ(a, b, quotient, aSize, bSize, resSize);

  return quotient;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(const char*_a, const char* _b, char*&remain)
{

  size_t aSize = strlen(_a);
  size_t bSize = strlen(_b);
  char*a = filledString(_a,aSize);
  char*b = filledString(_b,bSize);

  size_t rs, resSize = (rs=(aSize-bSize+1))>1?rs:1;
  
  size_t remSize = bSize;

  remain = csAllocCharPtr(remSize, '0');

  char*quotient = csAllocCharPtr(resSize, '0');

  makeDivisionQ(a, b, quotient, remain, aSize, bSize, resSize, remSize);
  free(a);
  free(b);
  return quotient;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(const char*_a, const char* _b)
{

  size_t aSize = strlen(_a);
  size_t bSize = strlen(_b);
  char*a = filledString(_a,aSize);
  char*b = filledString(_b,bSize);
  

  size_t rs, resSize = (rs=(aSize-bSize+1))>1?rs:1;
  
  size_t remSize = bSize, remSize1 = remSize+1;

  char*quotient = csAllocCharPtr(resSize, '0');

  makeDivisionQ(a, b, quotient, aSize, bSize, resSize);
  free(a);
  free(b);
  return quotient;
}

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionR(char*_a, char* _b, char*&resInt, char*&resDec, char*&remain,
                    size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t&remSize, size_t nDecimals)
{
  size_t aSize = _aSize+nDecimals;
  char* a = csAlloc<char>(aSize+1);
  a[aSize] = '\0';
  for(size_t i=0; i<_aSize; i++)
  {
    a[i] = _a[i];
  }
  for(size_t i=_aSize; i<aSize; i++)
  {
    a[i] = '0';
  }
  
  resDecSize = nDecimals;
  resDec = csAllocCharPtr(nDecimals,nDecimals+1, '0');
  int rs = (aSize-bSize+1); // int est important
  resSize = (rs>1)?rs:1;
  resInt = csAllocCharPtr(resSize, '0');
  resInt[resSize] = '\0';

  remSize = bSize;
  remain = csAllocCharPtr(remSize, '0');
  remain[remSize] = '\0';

  const char* status = makeDivisionQ(a, _b, resInt, remain, aSize, bSize, resSize, remSize);


  if(status)
  {
    return status;
  }
  if(resSize<=nDecimals)
  {

    for(size_t i=0; i<resSize; i++)
    {
      resDec[i] = resInt[i];
    }
    resIntSize = 1;
    resInt = (char*)realloc(resInt, 2);
    resInt[0] = '0';
    resInt[1] = '\0';
  }
  else
  {
    size_t nDec = nDecimals-1;
    size_t n = resSize-1;
    for(size_t i=0; i<nDecimals; i++)
    {
      resDec[nDec-i] = resInt[n-i];
    }
    resIntSize = resSize-nDecimals;
    resInt = (char*)realloc(resInt,resIntSize+1);
    resInt[resIntSize] = '\0';
  }

  free(a);
  return 0;
}

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionR2(char*_a, char* _b, char*&resInt, char*&resDec, char*&remain,
                    size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t&remSize, size_t nDecimals)
{
  
  size_t aSize = _aSize+nDecimals;
  char* a = csAlloc<char>(aSize+1);
  a[aSize] = '\0';
  for(size_t i=0; i<_aSize; i++)
  {
    a[i] = _a[i];
  }
  for(size_t i=_aSize; i<aSize; i++)
  {
    a[i] = '0';
  }
  
  resDecSize = nDecimals;
  resDec = csAllocCharPtr(nDecimals,nDecimals+1, '0');
  int rs = (aSize-bSize+1); // int est important
  resSize = (rs>1)?rs:1;
  resInt = csAllocCharPtr(resSize, '0');
  resInt[resSize] = '\0';

  remSize = bSize;
  remain = csAllocCharPtr(remSize, '0');
  remain[remSize] = '\0';

  const char* status = makeDivisionQ(a, _b, resInt, remain, aSize, bSize, resSize, remSize);


  if(status)
  {
    return status;
  }
  if(resSize<=nDecimals)
  {

    size_t diff = nDecimals-resSize;//--------------------------------- ajout, consideration des zeros au debut des decimales
    for(size_t i=0, j=diff; i<resSize; i++,j++)
    {
      resDec[j] = resInt[i];
    }
    resIntSize = 1;
    resInt = (char*)realloc(resInt, 2);
    resInt[0] = '0';
    resInt[1] = '\0';
  }
  else
  {
    size_t nDec = nDecimals-1;
    size_t n = resSize-1;
    for(size_t i=0; i<nDecimals; i++)
    {
      resDec[nDec-i] = resInt[n-i];
    }
    resIntSize = resSize-nDecimals;
    resInt = (char*)realloc(resInt,resIntSize+1);
    resInt[resIntSize] = '\0';
  }


  free(a);
  return 0;
}

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionR2(char*_a, char* _b, char*&resInt, char*&resDec,
                    size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t nDecimals)
{
  
  size_t aSize = _aSize+nDecimals;
  char* a = csAlloc<char>(aSize+1);
  a[aSize] = '\0';
  for(size_t i=0; i<_aSize; i++)
  {
    a[i] = _a[i];
  }
  for(size_t i=_aSize; i<aSize; i++)
  {
    a[i] = '0';
  }
  
  resDecSize = nDecimals;
  resDec = csAllocCharPtr(nDecimals,nDecimals+1, '0');
  int rs = (aSize-bSize+1); // int est important
  resSize = (rs>1)?rs:1;
  resInt = csAllocCharPtr(resSize, '0');
  resInt[resSize] = '\0';

  const char* status = makeDivisionQ(a, _b, resInt, aSize, bSize, resSize);


  if(status)
  {
    return status;
  }
  if(resSize<=nDecimals)
  {

    size_t diff = nDecimals-resSize;//--------------------------------- ajout, consideration des zeros au debut des decimales
    for(size_t i=0, j=diff; i<resSize; i++,j++)
    {
      resDec[j] = resInt[i];
    }
    resIntSize = 1;
    resInt = (char*)realloc(resInt, 2);
    resInt[0] = '0';
    resInt[1] = '\0';
  }
  else
  {
    size_t nDec = nDecimals-1;
    size_t n = resSize-1;
    for(size_t i=0; i<nDecimals; i++)
    {
      resDec[nDec-i] = resInt[n-i];
    }
    resIntSize = resSize-nDecimals;
    resInt = (char*)realloc(resInt,resIntSize+1);
    resInt[resIntSize] = '\0';
  }


  free(a);
  return 0;
}

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionR(char*a, char* b, char*&resInt, char*&resDec, char*&remain, size_t nDecimals)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t remSize=0, resSize=0, resIntSize, resDecSize;
  return makeDivisionR(a, b, resInt, resDec, remain,
                    aSize, bSize, resSize, resIntSize, resDecSize, remSize, nDecimals);

}

/* ---- src/csModulus.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;


CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeModulusQ(char*a, char* b, char*&remain,
        size_t& aSize, size_t& bSize, size_t& remSize)
{
  
  if(removeFrontZeros(b,bSize)==0)
  {
    csRemainZero(remain, remSize);
    return "Invalid Input !";
  }
  if(removeFrontZeros(a,aSize)==0)
  {
    csRemainZero(remain, remSize);
    return "0";
  }

  if((bSize == 1 && b[0] == '1')||(aSize == 1 && a[0] == '1'))
  {
    csRemainZero(remain, remSize);
    return "0";
  }

  size_t tmpResSize = bSize+1;
  csScratch tmpBuf;
  tmpBuf.alloc(tmpResSize, '0');
  char*tmpRes = tmpBuf.p;

  size_t skipZerosLen = 0;
  bool cmp;

  uchar quotientDigit = '1';// or 49

  size_t tmpResSizeForSub=bSize;
  csScratch cpyBuf;
  cpyBuf.alloc(tmpResSize, '0');
  char* bCpy = cpyBuf.p;
  for(size_t _i = 0, _pos = tmpResSize - bSize; _i < bSize; ++_i) bCpy[_pos + _i] = b[_i];

  size_t nextSize = bSize-1;
  while(1)
  {

    skipZeros(a, aSize, skipZerosLen);
    
    size_t diff = aSize-skipZerosLen;
    
    nextSize++;
    
    tmpResSizeForSub = nextSize-skipZerosLen;
    
    cmp =  (nextSize>skipZerosLen&&nextSize<=aSize)&&isaGreaterEqual2(a,b,tmpResSizeForSub,bSize,skipZerosLen);
    
    if(diff < bSize || (diff==bSize && !cmp) || nextSize>aSize)
    {
      if(nextSize <= skipZerosLen && a[skipZerosLen] != '0' && diff >= bSize)
        continue;
      if(diff == bSize && !cmp && nextSize < aSize && nextSize > skipZerosLen)
        continue;

      if(diff > remSize) remain = (char*)realloc(remain, diff+1);
      remain[diff] = '\0';
      
      for(size_t i=0; i<diff; i++)
      {
        remain[i] = a[i+skipZerosLen];
      }

      remSize = diff;
      break;
    }

    if(cmp)
    {
      if(tmpResSizeForSub == bSize)
      {
        if(bSize == 1)
          quotientDigit = (divTable[a[skipZerosLen]][b[0]]-1);
        else
          quotientDigit = (divTable[bicharIndex(a[skipZerosLen], a[skipZerosLen+1])][bicharIndex(b[0],b[1])]-1);

        if(quotientDigit == 48) quotientDigit = 49;

  
        makeMultiplicationForDivBase1(b, quotientDigit, tmpRes, tmpResSizeForSub, tmpResSizeForSub);
        makeSubstractionForDiv(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);
        
        while(isaGreaterEqual(a,b,tmpResSizeForSub,skipZerosLen))
          makeSubstractionForDiv(a, b, a, tmpResSizeForSub, skipZerosLen);

      }
      else
      {
      //convert the first and second digits to a number, to find the quotient with the first digit of b
        if(bSize == 1)
          quotientDigit = divTable[bicharIndex(a[skipZerosLen], a[skipZerosLen+1])][b[0]]-2;
        else
          quotientDigit = (divTable[tricharIndex(a[skipZerosLen], a[skipZerosLen+1], a[skipZerosLen+2])]
                                 [bicharIndex(b[0],b[1])]-1);

        if(quotientDigit <= 48) quotientDigit = 49;
        if(quotientDigit > 57) quotientDigit = 57;
        
        makeMultiplicationForDivBase2(b, quotientDigit, tmpRes, bSize, tmpResSizeForSub);
        makeSubstractionForDiv(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);
        while(isaGreaterEqual(a,bCpy,tmpResSizeForSub,skipZerosLen))
          makeSubstractionForDiv(a, bCpy, a, tmpResSizeForSub, skipZerosLen);


      }
    }

  }

  
  return 0;
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeModulusQ(char*a, char* b, char*&remain)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t remSize = 0;
  remSize = bSize;
  remain = csAllocCharPtr(remSize, '0');
  makeModulusQ(a, b, remain, aSize, bSize, remSize);
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeModulusQ(char*a, char* b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t remSize = 0;
  remSize = bSize;
  char* remain = csAllocCharPtr(remSize, '0');
  makeModulusQ(a, b, remain, aSize, bSize, remSize);
  return remain;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeModulusQ(const char*a, const char* b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  char* _a = newString(a, aSize);
  char* _b = newString(b, bSize);
  size_t remSize = 0;
  char* remain = 0;
  remSize = bSize;
  remain = csAllocCharPtr(remSize, '0');
  makeModulusQ(_a, _b, remain, aSize, bSize, remSize);
  free(_a);
  free(_b);
  return remain;
}

/* ---- src/csARITHMETIC.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;
using namespace CSARITHMETIC;


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::init()
{
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::printNumber(char*nb, const char*seperator)
{
  cout<<nb<<seperator;
}

CS_FORCE_INLINE csBIDIGITS** CSARITHMETIC_API CSARITHMETIC::toBIDIGITS(int** opTable)
{
  csBIDIGITS** op = csAlloc<csBIDIGITS*>(58);
  for(size_t j=0; j<58; j++)
  {
    op[j] = csAlloc2<csBIDIGITS>(58,{'0','0'});
  }
  for(size_t j=48; j<58; j++)
  {
    for(size_t i=48; i<58; i++)
    {
      op[j][i].tens = opTable[j][i]/10+48;
      op[j][i].units = opTable[j][i]%10+48;
    }
  }
  return op;
}


CS_FORCE_INLINE uchar CSARITHMETIC_API CSARITHMETIC::bicharIndex(uchar tens, uchar units)
{
  return (tens-48)*10+units;
}

CS_FORCE_INLINE uchar CSARITHMETIC_API CSARITHMETIC::tricharIndex(int cents, uchar tens, uchar units)
{
  return uchar((cents-48)*100+(tens-48)*10+units);
}

//unsigned uchar

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::printTable(char*opName)
{
  char* title;
  const csBIDIGITS (*tab)[58] = nullptr;
  const char (*ctab)[256] = nullptr;
  bool b=0;
  size_t max = 58;
  if(strcmp(opName, "mul")==0)
  {
    title = "Multiplication table : \n";
    tab = mulTable;
  }
  else if(strcmp(opName, "add")==0)
  {
    title = "Addition table : \n";
    tab = addTable;
  }
  else if(strcmp(opName, "sub")==0)
  {
    title = "Substraction table : \n";
    tab = subTable;
  }
  else if(strcmp(opName, "div")==0)
  {
    title = "Division table : \n";
    ctab = divTable;
    b=1;
    max = 148;
  }
  cout<<title;
  
  size_t min = 48;
  cout<<"    ";
  for(size_t i=min; i<58; i++)
    cout<<" "<<char(i)<<"  ";
  cout<<"\n\n";
  for(size_t j=48; j<max; j++)
  {
    cout<< " " << char(j) << "  ";
    for(size_t i=min; i<58; i++)
    {
      if(!b)
      {
        cout<<tab[j][i].tens<<tab[j][i].units<<"  ";
      }
      else
        cout<<ctab[j][i]<<"  ";;
    }
    cout<<"\n\n";
  }

}


CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::gcd(char*_a, char*_b, size_t aSize, size_t bSize, size_t& gdcSize)
{
  if ((aSize == 1 && _a[0] == '1') || (bSize == 1 && _b[0] == '1'))
  {
    gdcSize = 1;
    char* one = csAlloc<char>(2);
    one[0] = '1';
    one[1] = '\0';
    return one;
  }

  size_t cap = aSize > bSize ? aSize : bSize;
  if (cap < 1)
    cap = 1;
  csScratch buf[3];
  buf[0].alloc(cap, '0');
  buf[1].alloc(cap, '0');
  buf[2].alloc(cap, '0');
  char* a = buf[0].p;
  char* b = buf[1].p;
  int ia = 0, ib = 1, ir = 2;
  for (size_t i = 0; i < aSize; ++i)
    a[i] = _a[i];
  a[aSize] = '\0';
  for (size_t i = 0; i < bSize; ++i)
    b[i] = _b[i];
  b[bSize] = '\0';

  char* remain = b;
  size_t remSize = bSize;
  do
  {
    csGcdModStep(a, b, remain, aSize, bSize, remSize, buf, ia, ib, ir, cap);
  } while (remSize > 1);

  while (!(remain[0] == '0' || remain[0] == '1'))
  {
    csGcdModStep(a, b, remain, aSize, bSize, remSize, buf, ia, ib, ir, cap);
  }

  char* srcDigits = (remain[0] == '1') ? b : a;
  size_t srcSize = (remain[0] == '1') ? bSize : aSize;
  removeFrontZeros(srcDigits, srcSize);
  gdcSize = srcSize;
  char* out = csAlloc<char>(srcSize + 1);
  for (size_t i = 0; i < srcSize; ++i)
    out[i] = srcDigits[i];
  out[srcSize] = '\0';
  return out;
}

CS_FORCE_INLINE char*CSARITHMETIC_API CSARITHMETIC::gcd(char*a, char*b,size_t& gdcSize)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  char* res = gcd(a,b,aSize,bSize,gdcSize);
  return res;
}

CS_FORCE_INLINE char*CSARITHMETIC_API CSARITHMETIC::gcd(const char*a, const char*b,size_t& gdcSize)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  char* _a = newString(a, aSize);
  char* _b = newString(b, bSize);
  char * g = gcd(_a,_b,aSize,bSize,gdcSize);
  free(_a);
  free(_b);
  return g;
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csSortMinR(csRNUMBER*& rn, size_t size)
{
  size_t s1 = size-1;
  csRNUMBER tmp;
  for(size_t j=0; j<size; j++)
  {
    for(size_t i=j+1; i<s1; i++)
    {
      if(rn[j] < rn[i])
      {
        tmp.copy(rn[j]);
        rn[j].copy(rn[i]);
        rn[i].copy(tmp);
      }
    }
  }
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csSortMin_CoordsByRNumber(csFCOORDS*& fc, csRNUMBER*& rn, size_t size)
{
  size_t s1 = size-1;
  csRNUMBER tmp;
  csFCOORDS tfc;
  for(size_t j=0; j<s1; j++)
  {
    for(size_t i=j+1; i<size; i++)
    {
      if(rn[j] > rn[i])
      {
        tmp.copy(rn[j]);
        rn[j].copy(rn[i]);
        rn[i].copy(tmp);

        tfc = fc[j];
        fc[j] = fc[i];
        fc[i] = tfc;
      }
    }
  }
  
  tmp.clear();
}
CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csSortMax_CoordsByRNumber(csFCOORDS*& fc, csRNUMBER*& rn, size_t size)
{
  size_t s1 = size-1;
  csRNUMBER tmp;
  csFCOORDS tfc;
  for(size_t j=0; j<s1; j++)
  {
    for(size_t i=j+1; i<size; i++)
    {
      if(rn[j] < rn[i])
      {
        tmp.copy(rn[j]);
        rn[j].copy(rn[i]);
        rn[i].copy(tmp);

        tfc = fc[j];
        fc[j] = fc[i];
        fc[i] = tfc;
      }
    }
  }
  for(size_t j=0; j<size; j++)
  {
    cout<<rn[j]<< " ";
  }
  cout<<"\n";
  tmp.clear();
}

/* ---- src/csQNUMBER.cpp ---- */
#include <cstddef>
#include <cstdio>
#include <cstring>

using namespace __mem_man;
using namespace __ar_man;
using namespace CSARITHMETIC;

extern int RPRECISION;

CS_FORCE_INLINE csQNUMBER::csQNUMBER(const char* _numerator, const char* _denominator, bool _sign)
{
  qFromDec(_numerator, numerator, numSize);
  qFromDec(_denominator, denominator, denomSize);
  sign = _sign;
}

CS_FORCE_INLINE csQNUMBER::csQNUMBER(size_t num, size_t denom, bool _sign)
{
  sign = _sign;
  QLimbs n, d;
  n.setU64((uint64_t)num);
  d.setU64((uint64_t)denom);
  numerator = n.leak();
  numSize = n.n;
  denominator = d.leak();
  denomSize = d.n;
}

CS_FORCE_INLINE void csQNUMBER::init()
{
  numerator=0;
  denominator=0;
}

CS_FORCE_INLINE csQNUMBER* CSARITHMETIC_API CSARITHMETIC::csQNUMBER_PTR_ALLOC(size_t nb)
{
  csQNUMBER *qn = csAlloc<csQNUMBER>(nb);
  for(size_t i=0; i<nb; i++)
  {
    qn[i].init();
  }
  return qn;
}

CS_FORCE_INLINE csQNUMBER* CSARITHMETIC_API CSARITHMETIC::csQNUMBER_PTR_ALLOC(size_t nb, csQNUMBER init)
{
  csQNUMBER *qn = csAlloc<csQNUMBER>(nb);
  for (size_t i = 0; i < nb; i++) {
    qn[i].sign = init.sign;
    if (init.numerator && init.numSize) {
      qn[i].numerator = qDupLimbs(init.numerator, init.numSize);
      qn[i].numSize = init.numSize;
    } else {
      qn[i].numerator = qDupLimbs(0, 0);
      qn[i].numSize = 1;
    }
    if (init.denominator && init.denomSize) {
      qn[i].denominator = qDupLimbs(init.denominator, init.denomSize);
      qn[i].denomSize = init.denomSize;
    } else {
      qn[i].denominator = qLimbOne();
      qn[i].denomSize = 1;
    }
  }
  return qn;
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csQNUMBER_PTR_FREE(csQNUMBER*& qn, size_t nb)
{
  for(size_t i=0; i<nb; i++)
  {
    qn[i].clear();
  }
  free(qn);
}


CS_FORCE_INLINE void csQNUMBER::set(const char* _numerator, const char* _denominator, bool _sign)
{
  if (numerator) {
    free(numerator);
    free(denominator);
  }
  qFromDec(_numerator, numerator, numSize);
  qFromDec(_denominator, denominator, denomSize);
  sign = _sign;
}

CS_FORCE_INLINE void csQNUMBER::set(size_t num, size_t denom, bool _sign)
{
  if (numerator) {
    free(numerator);
    free(denominator);
  }
  sign = _sign;
  QLimbs n, d;
  n.setU64((uint64_t)num);
  d.setU64((uint64_t)denom);
  numerator = n.leak();
  numSize = n.n;
  denominator = d.leak();
  denomSize = d.n;
}

CS_FORCE_INLINE void csQNUMBER::setl(long num, size_t denom)
{
  if (numerator) {
    free(numerator);
    free(denominator);
  }
  sign = num < 0 ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  QLimbs n, d;
  n.setU64(qAbsLong(num));
  d.setU64((uint64_t)denom);
  numerator = n.leak();
  numSize = n.n;
  denominator = d.leak();
  denomSize = d.n;
}

CS_FORCE_INLINE size_t csQNUMBER::maxSize()
{
  return numSize>denomSize?numSize:denomSize;
}

CS_FORCE_INLINE void csQNUMBER::reduce(size_t sizeCondition)
{
  qReducePair(numerator, numSize, denominator, denomSize, sizeCondition);
}

CS_FORCE_INLINE csQNUMBER csReduce(csQNUMBER a, size_t sizeCondition)
{
  char* n = qDupLimbs(a.numerator, a.numSize);
  char* d = qDupLimbs(a.denominator, a.denomSize);
  size_t ns = a.numSize ? a.numSize : 1;
  size_t ds = a.denomSize ? a.denomSize : 1;
  qReducePair(n, ns, d, ds, sizeCondition);
  a.numerator = n;
  a.denominator = d;
  a.numSize = ns;
  a.denomSize = ds;
  return a;
}

CS_FORCE_INLINE bool csQNUMBER::isAbsDifferent(csQNUMBER a)
{
  return !isAbsEqual(a);
}

CS_FORCE_INLINE bool csQNUMBER::operator==(csQNUMBER a)
{
  return qCmpSigned(*this, a) == 0;
}

CS_FORCE_INLINE bool csQNUMBER::isAbsEqual(csQNUMBER a)
{
  return qCmpAbsQ(*this, a) == 0;
}

CS_FORCE_INLINE bool csQNUMBER::operator!=(csQNUMBER a)
{
  return !(*this == a);
}

CS_FORCE_INLINE bool csQNUMBER::operator>(csQNUMBER a)
{
  return qCmpSigned(*this, a) > 0;
}

CS_FORCE_INLINE bool csQNUMBER::isAbsGreater(csQNUMBER a)
{
  return qCmpAbsQ(*this, a) > 0;
}

CS_FORCE_INLINE bool csQNUMBER::operator>=(csQNUMBER a)
{
  return qCmpSigned(*this, a) >= 0;
}

CS_FORCE_INLINE bool csQNUMBER::isAbsGreaterEqual(csQNUMBER a)
{
  return qCmpAbsQ(*this, a) >= 0;
}

CS_FORCE_INLINE bool csQNUMBER::operator<(csQNUMBER a)
{
  return qCmpSigned(*this, a) < 0;
}

CS_FORCE_INLINE bool csQNUMBER::isAbsLess(csQNUMBER a)
{
  return qCmpAbsQ(*this, a) < 0;
}

CS_FORCE_INLINE bool csQNUMBER::operator<=(csQNUMBER a)
{
  return qCmpSigned(*this, a) <= 0;
}

CS_FORCE_INLINE bool csQNUMBER::isAbsLessEqual(csQNUMBER a)
{
  return qCmpAbsQ(*this, a) <= 0;
}

CS_FORCE_INLINE void csQNUMBER::print(const char*title)
{
  char* ns = csLimbsToDec(numerator, numSize);
  char* ds = csLimbsToDec(denominator, denomSize);
  if (sign == CS_POSITIVE_NUMBER)
    cout << "\n " << title << ns << " / " << ds << "\n";
  else
    cout << "\n " << title << "-" << ns << " / " << ds << "\n";
  free(ns);
  free(ds);
}

CS_FORCE_INLINE void csQNUMBER::clear(const source_location loc)
{
  if(numerator)
  {
    free(numerator);
    free(denominator);
    numerator = 0;
    denominator = 0;
    numSize = 0;
    denomSize = 0;

  }
}

CS_FORCE_INLINE csQNUMBER csQNUMBER::operator+(csQNUMBER a)
{
  csQNUMBER qn(csQRaw{});
  QLimbs n1, n2, den, num;
  qMul(qL(numerator), numSize, qL(a.denominator), a.denomSize, n1);
  qMul(qL(denominator), denomSize, qL(a.numerator), a.numSize, n2);
  qMul(qL(denominator), denomSize, qL(a.denominator), a.denomSize, den);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (a.sign == CS_NEGATIVE_NUMBER)
      qSubAbs(n1.p, n1.n, n2.p, n2.n, num, neg);
    else
      qAdd(n1.p, n1.n, n2.p, n2.n, num);
  } else if (a.sign == CS_NEGATIVE_NUMBER) {
    qAdd(n1.p, n1.n, n2.p, n2.n, num);
    neg = true;
  } else {
    qSubAbs(n2.p, n2.n, n1.p, n1.n, num, neg);
  }
  if (num.isZero())
    neg = false;
  qNormalize(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
}

CS_FORCE_INLINE csQNUMBER csQNUMBER::operator-(csQNUMBER a)
{
  csQNUMBER qn(csQRaw{});
  QLimbs n1, n2, den, num;
  qMul(qL(numerator), numSize, qL(a.denominator), a.denomSize, n1);
  qMul(qL(denominator), denomSize, qL(a.numerator), a.numSize, n2);
  qMul(qL(denominator), denomSize, qL(a.denominator), a.denomSize, den);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (a.sign == CS_NEGATIVE_NUMBER)
      qAdd(n1.p, n1.n, n2.p, n2.n, num);
    else
      qSubAbs(n1.p, n1.n, n2.p, n2.n, num, neg);
  } else if (a.sign == CS_NEGATIVE_NUMBER) {
    qSubAbs(n2.p, n2.n, n1.p, n1.n, num, neg);
  } else {
    qAdd(n1.p, n1.n, n2.p, n2.n, num);
    neg = true;
  }
  if (num.isZero())
    neg = false;
  qNormalize(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
}

CS_FORCE_INLINE csQNUMBER csQNUMBER::operator*(csQNUMBER a)
{
  csQNUMBER qn(csQRaw{});
  QLimbs num, den;
  qMul(qL(numerator), numSize, qL(a.numerator), a.numSize, num);
  qMul(qL(denominator), denomSize, qL(a.denominator), a.denomSize, den);
  bool neg = !num.isZero() && (sign != a.sign);
  qNormalize(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
}

CS_FORCE_INLINE csQNUMBER csQNUMBER::operator/(csQNUMBER a)
{
  csQNUMBER qn(csQRaw{});
  QLimbs num, den;
  qMul(qL(numerator), numSize, qL(a.denominator), a.denomSize, num);
  qMul(qL(denominator), denomSize, qL(a.numerator), a.numSize, den);
  bool neg = !num.isZero() && (sign != a.sign);
  qNormalize(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
}

CS_FORCE_INLINE csQNUMBER& csQNUMBER::operator=(csQNUMBER a)
{
  if (numerator) {
    free(numerator);
    free(denominator);
    numerator = 0;
  }
  sign = a.sign;
  if (a.numerator && a.numSize) {
    numerator = qDupLimbs(a.numerator, a.numSize);
    numSize = a.numSize;
  } else {
    numerator = qDupLimbs(0, 0);
    numSize = 1;
  }
  if (a.denominator && a.denomSize) {
    denominator = qDupLimbs(a.denominator, a.denomSize);
    denomSize = a.denomSize;
  } else {
    denominator = qLimbOne();
    denomSize = 1;
  }
  a.clear();
  return *this;
}

CS_FORCE_INLINE csQNUMBER& csQNUMBER::operator=(long a)
{
  if (numerator) {
    free(numerator);
    free(denominator);
    numerator = 0;
  }
  sign = a < 0 ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  QLimbs n;
  n.setU64(qAbsLong(a));
  numerator = n.leak();
  numSize = n.n;
  denomSize = 1;
  denominator = qLimbOne();
  return *this;
}



CS_FORCE_INLINE csQNUMBER csQNUMBER::operator+(long a)
{
  csQNUMBER qn(csQRaw{});
  bool asign = a < 0;
  QLimbs mag, prod, num;
  mag.setU64(qAbsLong(a));
  qMul(qL(denominator), denomSize, mag.p, mag.n, prod);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (asign)
      qSubAbs(qL(numerator), numSize, prod.p, prod.n, num, neg);
    else
      qAdd(qL(numerator), numSize, prod.p, prod.n, num);
  } else if (asign) {
    qAdd(qL(numerator), numSize, prod.p, prod.n, num);
    neg = true;
  } else {
    qSubAbs(prod.p, prod.n, qL(numerator), numSize, num, neg);
  }
  if (num.isZero())
    neg = false;
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = qDupLimbs(denominator, denomSize);
  qn.denomSize = denomSize;
  return qn;
}


CS_FORCE_INLINE csQNUMBER csQNUMBER::operator-(long a)
{
  csQNUMBER qn(csQRaw{});
  bool asign = a < 0;
  QLimbs mag, prod, num;
  mag.setU64(qAbsLong(a));
  qMul(qL(denominator), denomSize, mag.p, mag.n, prod);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (asign)
      qAdd(qL(numerator), numSize, prod.p, prod.n, num);
    else
      qSubAbs(qL(numerator), numSize, prod.p, prod.n, num, neg);
  } else if (asign) {
    qSubAbs(prod.p, prod.n, qL(numerator), numSize, num, neg);
  } else {
    qAdd(qL(numerator), numSize, prod.p, prod.n, num);
    neg = true;
  }
  if (num.isZero())
    neg = false;
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = qDupLimbs(denominator, denomSize);
  qn.denomSize = denomSize;
  return qn;
}


CS_FORCE_INLINE csQNUMBER csQNUMBER::operator*(long a)
{
  csQNUMBER qn(csQRaw{});
  QLimbs mag, num;
  mag.setU64(qAbsLong(a));
  qMul(qL(numerator), numSize, mag.p, mag.n, num);
  qn.sign = (!num.isZero() && (sign != (a < 0))) ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = qDupLimbs(denominator, denomSize);
  qn.denomSize = denomSize;
  return qn;
}


CS_FORCE_INLINE csQNUMBER csQNUMBER::operator/(long a)
{
  csQNUMBER qn(csQRaw{});
  QLimbs mag, den;
  mag.setU64(qAbsLong(a));
  qMul(qL(denominator), denomSize, mag.p, mag.n, den);
  qn.sign = (!qRawZero(numerator, numSize) && (sign != (a < 0))) ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = qDupLimbs(numerator, numSize);
  qn.numSize = numSize;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
}

CS_FORCE_INLINE double csQNUMBER::getDouble()
{
  double d = qLimbsToDouble(denominator, denomSize);
  if (d == 0)
    return 0;
  double r = qLimbsToDouble(numerator, numSize) / d;
  return sign ? -r : r;
}

CS_FORCE_INLINE char* csQNUMBER::quotient()
{
  char* ns = csLimbsToDec(numerator, numSize);
  char* ds = csLimbsToDec(denominator, denomSize);
  size_t nlen = strlen(ns), dlen = strlen(ds);
  char* resInt = 0;
  char* resDec = 0;
  long precicion = getRNumberPrecision();
  size_t resz = 0, risz = 0, rdsz = 0;
  char const* stat = makeDivisionR2(ns, ds, resInt, resDec, nlen, dlen, resz, risz, rdsz, (size_t)precicion);
  if (stat) {
    free(resInt);
    free(resDec);
    free(ns);
    free(ds);
    return newString(stat);
  }
  removeFrontZeros(resInt, risz);
  size_t t = risz + rdsz + 1;
  char* str = csAlloc<char>(t + 2);
  if (sign)
    sprintf(str, "-%s.%s", resInt, resDec);
  else
    sprintf(str, "%s.%s", resInt, resDec);
  free(resInt);
  free(resDec);
  free(ns);
  free(ds);
  return str;
}

CS_FORCE_INLINE csQNUMBER::operator csRNUMBER()
{
  char* ns = csLimbsToDec(numerator, numSize);
  char* ds = csLimbsToDec(denominator, denomSize);
  size_t nlen = strlen(ns);
  size_t dlen = strlen(ds);
  size_t aSize = nlen + RPRECISION;
  char* a = csAlloc<char>(aSize + 1);
  a[aSize] = '\0';
  for (size_t i = 0; i < nlen; i++)
    a[i] = ns[i];
  for (size_t i = nlen; i < aSize; i++)
    a[i] = '0';
  int rs = (int)nlen - (int)dlen + 1;
  size_t resSize = (rs > 1) ? (size_t)rs : 1;
  char* result = csAllocCharPtr(resSize, '0');
  result[resSize] = '\0';
  size_t bSize = dlen;
  const char* status = makeDivisionQ(a, ds, result, aSize, bSize, resSize);
  free(ns);
  if (status) {
    csRNUMBER rn("0");
    free(result);
    free(a);
    free(ds);
    return rn;
  }
  removeFrontZeros(result, resSize);
  csRNUMBER rn(result, -RPRECISION, sign);
  free(result);
  free(a);
  free(ds);
  return rn;
}


CS_FORCE_INLINE csQNUMBER CSARITHMETIC_API CSARITHMETIC::pow(csQNUMBER a, size_t p)
{
  csQNUMBER r("1");
  for(size_t i=0; i<p; i++)
  {
    r = r*a;
  }
  return r;
}

CS_FORCE_INLINE void csQNUMBER::copy(csQNUMBER a)
{
  if (numerator) {
    free(numerator);
    free(denominator);
    numerator = 0;
    denominator = 0;
  }
  sign = a.sign;
  if (a.numerator && a.numSize) {
    numerator = qDupLimbs(a.numerator, a.numSize);
    numSize = a.numSize;
  } else {
    numerator = qDupLimbs(0, 0);
    numSize = 1;
  }
  if (a.denominator && a.denomSize) {
    denominator = qDupLimbs(a.denominator, a.denomSize);
    denomSize = a.denomSize;
  } else {
    denominator = qLimbOne();
    denomSize = 1;
  }
}

CS_FORCE_INLINE bool csQNUMBER::isZero()
{
  return qRawZero(numerator, numSize);
}

CS_FORCE_INLINE bool csQNUMBER::isNonZero()
{
  return !isZero();
}

/* ---- src/csRNUMBER.cpp ---- */
#include <cstddef>
#include <cstdio>
#include <cstring>

using namespace __mem_man;
using namespace __ar_man;
using namespace CSARITHMETIC;

int RPRECISION = 20;

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::setRNumberPrecision(int precision)
{
  RPRECISION = precision;
}
CS_FORCE_INLINE int CSARITHMETIC_API CSARITHMETIC::getRNumberPrecision()
{
  return RPRECISION;
}

CS_FORCE_INLINE csRNUMBER::csRNUMBER(const char* _mantissa, int _exponent, bool _sign)
{
  set(_mantissa, _exponent, _sign);
}
CS_FORCE_INLINE csRNUMBER::csRNUMBER(unsigned long _mantissa, int _exponent, bool _sign)
{
  set(_mantissa, _exponent, _sign);
}
CS_FORCE_INLINE csRNUMBER::csRNUMBER(long _mantissa, int _exponent)
{
  set(_mantissa, _exponent);
}
CS_FORCE_INLINE csRNUMBER::csRNUMBER(bool evaluate, const char* number)
{
  set(evaluate, number);
}

CS_FORCE_INLINE csRNUMBER::~csRNUMBER()
{
}


CS_FORCE_INLINE void csRNUMBER::set(const char* _mantissa, int _exponent, bool _sign)
{
  QLimbs n;
  rFromDigits(_mantissa, n);
  rSave(*this, n);
  exponent = _exponent;
  sign = n.isZero() ? CS_POSITIVE_NUMBER : _sign;
  precision = 0;
}

CS_FORCE_INLINE void csRNUMBER::set(unsigned long _mantissa, int _exponent, bool _sign)
{
  QLimbs n;
  n.setU64((uint64_t)_mantissa);
  rSave(*this, n);
  exponent = _exponent;
  sign = n.isZero() ? CS_POSITIVE_NUMBER : _sign;
  precision = 0;
}

CS_FORCE_INLINE void csRNUMBER::set(long _mantissa, int _exponent)
{
  uint64_t mag = _mantissa < 0 ? (uint64_t)(-_mantissa) : (uint64_t)_mantissa;
  QLimbs n;
  n.setU64(mag);
  rSave(*this, n);
  exponent = _exponent;
  sign = _mantissa < 0 ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  precision = 0;
}

CS_FORCE_INLINE void csRNUMBER::set(bool evaluate, const char* _number)
{
  (void)evaluate;
  if (!_number)
    _number = "0";
  size_t len = strlen(_number);
  size_t start = 0;
  bool neg = 0;
  if (len && _number[0] == '-') {
    neg = 1;
    start = 1;
  } else if (len && _number[0] == '+') {
    start = 1;
  }
  size_t dot = len;
  size_t expPos = len;
  for (size_t i = start; i < len; ++i) {
    if (_number[i] == '.') {
      dot = i;
      break;
    }
  }
  size_t frac = dot < len ? dot + 1 : len;
  for (size_t i = frac; i < len; ++i) {
    if (_number[i] == 'e' || _number[i] == 'E') {
      expPos = i;
      break;
    }
  }
  size_t whole = dot - start;
  size_t fracN = expPos > frac ? expPos - frac : 0;
  size_t nd = whole + fracN;
  if (nd == 0)
    nd = 1;
  char* digs = (char*)malloc(nd + 1);
  size_t w = 0;
  if (whole == 0 && fracN == 0)
    digs[w++] = '0';
  for (size_t i = 0; i < whole; ++i) {
    char c = _number[start + i];
    digs[w++] = (c >= '0' && c <= '9') ? c : '0';
  }
  for (size_t i = 0; i < fracN; ++i) {
    char c = _number[frac + i];
    digs[w++] = (c >= '0' && c <= '9') ? c : '0';
  }
  digs[w] = 0;
  int exp = -(int)fracN;
  if (expPos < len && expPos + 1 < len)
    exp = (int)strtol(_number + expPos + 1, 0, 10) - (int)fracN;
  QLimbs n;
  rFromDigits(digs, n);
  free(digs);
  rSave(*this, n);
  exponent = exp;
  sign = n.isZero() ? CS_POSITIVE_NUMBER : neg;
  precision = 0;
}

CS_FORCE_INLINE void csRNUMBER::setAsTenPower(long power)
{
  QLimbs n;
  n.setOne();
  rSave(*this, n);
  exponent = (int)power;
  sign = CS_POSITIVE_NUMBER;
  precision = 0;
}

CS_FORCE_INLINE void csRNUMBER::init()
{
  mantissa=0;
}

CS_FORCE_INLINE csRNUMBER* CSARITHMETIC_API CSARITHMETIC::csRNUMBER_PTR_ALLOC(size_t nb)
{
  csRNUMBER *rn = csAlloc<csRNUMBER>(nb);
  for(size_t i=0; i<nb; i++)
  {
    rn[i].init();
  }
  return rn;
}

CS_FORCE_INLINE csRNUMBER* CSARITHMETIC_API CSARITHMETIC::csRNUMBER_PTR_ALLOC(size_t nb, csRNUMBER init)
{
  csRNUMBER *rn = csAlloc<csRNUMBER>(nb);
  for(size_t i=0; i<nb; i++)
  {
    rn[i].init();
    rn[i].set(init.mantSize,init.exponent,init.sign);
  }
  return rn;
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csRNUMBER_PTR_FREE(csRNUMBER*& rn, size_t nb)
{
  for(size_t i=0; i<nb; i++)
  {
    rn[i].clear();
  }
  free(rn);
}

CS_FORCE_INLINE void csRNUMBER::random(size_t nDigits, char digitMin, char digitMax, long exponent, bool sign)
{
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> distrib(digitMin,digitMax);
  char digit[2];
  char* str = csAlloc<char>(nDigits+1);
  sprintf(str,"");
  for(long i=0; i<nDigits; i++)
  {
    sprintf(digit,"%d", distrib(gen));
    strncat(str,digit,1);
  }
  strcat(str,"\0");
  set(str, exponent, sign);
  free(str);
}

CS_FORCE_INLINE void csRNUMBER::setPrecision(int size)
{
  if (exponent <= 0 && -exponent > size)
    return;
  int diff = exponent + size;
  if (diff > 0) {
    QLimbs n;
    rLoad(*this, n);
    rMulPow10(n, diff);
    rSave(*this, n);
  }
  exponent = -size;
}

CS_FORCE_INLINE size_t csRNUMBER::significantZerosCount(size_t initialPos)
{
  char* s = rExportDigits(*this);
  size_t n = strlen(s);
  long i = (long)n + exponent + (long)initialPos;
  size_t ans = 0;
  if (i < 0)
    ans = (size_t)(-i);
  else if (!(i == 1 && s[0] == '0'))
    ans = 0;
  else {
    for (; i < (long)n; ++i) {
      if (s[i] != '0') {
        ans = i == 0 ? 0 : (size_t)(i - 1);
        free(s);
        return ans;
      }
    }
    ans = (size_t)i;
  }
  free(s);
  return ans;
}

CS_FORCE_INLINE void addOne(char*&nb, size_t& size)
{
  long i = (long)size-1;
  while (i >= 0 && nb[i] == '9')
  {
    nb[i] = '0';
    i--;
  }
  if(i < 0)
  {
    size_t old = size;
    size += 1;
    csReallocString(&nb,old,size,'0');
    nb[0] = '1';
    return;
  }
  nb[i] += 1;
}

CS_FORCE_INLINE void csRNUMBER::forcePrecision(int size)
{
  char* s = rExportDigits(*this);
  size_t n = strlen(s);
  if (exponent <= 0) {
    long diff = exponent + size;
    if (diff < (long)n && (diff < 0 ? -diff : diff) < (long)n) {
      if (diff > 0) {
        char* g = (char*)malloc(n + (size_t)diff + 1);
        memcpy(g, s, n);
        memset(g + n, '0', (size_t)diff);
        g[n + (size_t)diff] = 0;
        free(s);
        s = g;
      } else if (diff < 0) {
        size_t keep = n + (size_t)diff;
        if (keep == 0) {
          s[0] = '0';
          s[1] = 0;
        } else {
          if (keep < n && s[keep] >= '5') {
            long i = (long)keep - 1;
            while (i >= 0 && s[i] == '9') {
              s[i] = '0';
              --i;
            }
            if (i < 0) {
              char* g = (char*)malloc(keep + 2);
              g[0] = '1';
              memcpy(g + 1, s, keep);
              g[keep + 1] = 0;
              free(s);
              s = g;
              keep += 1;
            } else {
              s[i] += 1;
              s[keep] = 0;
            }
          } else {
            s[keep] = 0;
          }
        }
      }
      exponent = -size;
    } else if (diff > (long)n) {
      char* g = (char*)malloc(n + (size_t)diff + 1);
      memcpy(g, s, n);
      memset(g + n, '0', (size_t)diff);
      g[n + diff] = 0;
      free(s);
      s = g;
      exponent = -size;
    } else {
      free(s);
      s = (char*)malloc(2);
      s[0] = '0';
      s[1] = 0;
      exponent = 0;
    }
  } else {
    size_t add = (size_t)exponent + (size_t)size;
    char* g = (char*)malloc(n + add + 1);
    memcpy(g, s, n);
    memset(g + n, '0', add);
    g[n + add] = 0;
    free(s);
    s = g;
    exponent = -size;
  }
  bool sg = sign;
  long pr = precision;
  int exp = exponent;
  QLimbs v;
  rFromDigits(s, v);
  free(s);
  rSave(*this, v);
  exponent = exp;
  sign = v.isZero() ? CS_POSITIVE_NUMBER : sg;
  precision = pr;
}

CS_FORCE_INLINE void csRNUMBER::reshape(size_t newMantissaSize)
{
  QLimbs n;
  rLoad(*this, n);
  int d = rDecDigits(n);
  if (newMantissaSize > (size_t)d)
    increaseShape(newMantissaSize - (size_t)d);
  else if (newMantissaSize < (size_t)d)
    decreaseShape((size_t)d - newMantissaSize);
}

CS_FORCE_INLINE void csRNUMBER::increaseShape(size_t size)
{
  if (!size)
    return;
  QLimbs n;
  rLoad(*this, n);
  rMulPow10(n, (int)size);
  rSave(*this, n);
  exponent -= (int)size;
}

CS_FORCE_INLINE void csRNUMBER::decreaseShape(size_t size)
{
  if (!size)
    return;
  QLimbs n;
  rLoad(*this, n);
  rDivPow10(n, (int)size);
  rSave(*this, n);
  exponent += (int)size;
  if (n.isZero())
    sign = CS_POSITIVE_NUMBER;
}

CS_FORCE_INLINE void csRNUMBER::shapeOutZeros()
{
  QLimbs n;
  rLoad(*this, n);
  int k = rTrim10(n);
  rSave(*this, n);
  exponent += k;
}

CS_FORCE_INLINE size_t csRNUMBER::getDigitNumber()
{
  QLimbs n;
  rLoad(*this, n);
  int d = rDecDigits(n);
  if (exponent >= 0)
    return (size_t)d + (size_t)exponent;
  int ae = exponent < 0 ? -exponent : exponent;
  return (size_t)(d > ae ? d : ae);
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::abs()
{
  csRNUMBER rn;
  QLimbs n;
  rLoad(*this, n);
  rSave(rn, n);
  rn.exponent = exponent;
  rn.sign = CS_POSITIVE_NUMBER;
  rn.precision = precision;
  return rn;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator+(long a)
{
  csRNUMBER rn(a);
  csRNUMBER b = (*this + rn);
  rn.clear();
  return b;
}
CS_FORCE_INLINE csRNUMBER csRNUMBER::operator+(csRNUMBER _a)
{
  csRNUMBER rn;
  QLimbs left, right, sum;
  rLoad(*this, left);
  rLoad(_a, right);
  int exp = exponent < _a.exponent ? exponent : _a.exponent;
  if (exponent > exp)
    rMulPow10(left, exponent - exp);
  if (_a.exponent > exp)
    rMulPow10(right, _a.exponent - exp);
  int outNeg = CS_POSITIVE_NUMBER;
  rCombine(left, sign ? 1 : 0, right, _a.sign ? 1 : 0, sum, outNeg);
  rSave(rn, sum);
  rn.sign = outNeg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  rn.exponent = exp;
  rn.precision = _a.precision > precision ? _a.precision : precision;
  return rn;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator-(long a)
{
  csRNUMBER rn(a);
  csRNUMBER b = (*this - rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator-(csRNUMBER _a)
{
  _a.sign = _a.sign ? CS_POSITIVE_NUMBER : CS_NEGATIVE_NUMBER;
  return *this + _a;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator*(long a)
{
  csRNUMBER rn(a);
  csRNUMBER b = (*this * rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator*(csRNUMBER _a)
{
  csRNUMBER rn;
  QLimbs left, right, prod;
  rLoad(*this, left);
  rLoad(_a, right);
  qMul(left.p, left.n, right.p, right.n, prod);
  rSave(rn, prod);
  rn.exponent = exponent + _a.exponent;
  if (prod.isZero())
    rn.sign = CS_POSITIVE_NUMBER;
  else if (sign == CS_POSITIVE_NUMBER)
    rn.sign = _a.sign == CS_NEGATIVE_NUMBER ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  else
    rn.sign = _a.sign == CS_NEGATIVE_NUMBER ? CS_POSITIVE_NUMBER : CS_NEGATIVE_NUMBER;
  rn.precision = _a.precision > precision ? _a.precision : precision;
  rn.setPrecision(RPRECISION + (int)rn.precision);
  return rn;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator/(long a)
{
  csRNUMBER rn(a);
  csRNUMBER b = (*this / rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator/(csRNUMBER _a)
{
  csRNUMBER rn;
  long precSize = RPRECISION + (_a.precision > precision ? _a.precision : precision);
  QLimbs left, right, quot, rem;
  rLoad(*this, left);
  rLoad(_a, right);
  long mag = (long)rDecDigits(right) + _a.exponent - ((long)rDecDigits(left) + exponent);
  if (mag > precSize) {
    rn.set("0", 0, 0);
    return rn;
  }
  int dropped = rTrim10(right);
  qDivMod(left.p, left.n, right.p, right.n, quot, rem);
  rSave(rn, quot);
  rn.exponent = exponent - (_a.exponent + dropped);
  if (quot.isZero())
    rn.sign = CS_POSITIVE_NUMBER;
  else if (sign == CS_POSITIVE_NUMBER)
    rn.sign = _a.sign == CS_NEGATIVE_NUMBER ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  else
    rn.sign = _a.sign == CS_NEGATIVE_NUMBER ? CS_POSITIVE_NUMBER : CS_NEGATIVE_NUMBER;
  rn.precision = _a.precision > precision ? _a.precision : precision;
  rn.setPrecision(RPRECISION + (int)rn.precision);
  return rn;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator-() const
{
  csRNUMBER rn;
  QLimbs n;
  rLoad(*this, n);
  rSave(rn, n);
  rn.exponent = exponent;
  rn.sign = n.isZero() ? CS_POSITIVE_NUMBER : (sign ? CS_POSITIVE_NUMBER : CS_NEGATIVE_NUMBER);
  rn.precision = precision;
  return rn;
}

CS_FORCE_INLINE void csRNUMBER::clear()
{
  if(mantissa)
  {
    free(mantissa);
    sign = 0;
    exponent = 0;
    mantSize = 0;
    precision = 0;
    mantissa = 0;
  }
}

CS_FORCE_INLINE void csRNUMBER::copy(csRNUMBER a)
{
  if (mantissa == a.mantissa)
    return;
  QLimbs n;
  rLoad(a, n);
  rSave(*this, n);
  sign = a.sign;
  exponent = a.exponent;
  precision = a.precision > precision ? a.precision : precision;
}

CS_FORCE_INLINE void csRNUMBER::operator=(csRNUMBER a)
{
  if (mantissa != a.mantissa) {
    QLimbs n;
    rLoad(a, n);
    rSave(*this, n);
    sign = a.sign;
    exponent = a.exponent;
    precision = a.precision > precision ? a.precision : precision;
  }
  a.mantissa = 0;
}

CS_FORCE_INLINE void csRNUMBER::operator=(long a)
{
  int exp = exponent;
  long pr = precision;
  uint64_t mag = a < 0 ? (uint64_t)(-a) : (uint64_t)a;
  QLimbs n;
  n.setU64(mag);
  bool sg = a < 0 ? CS_NEGATIVE_NUMBER : (a ? CS_POSITIVE_NUMBER : sign);
  if (!a)
    sg = sign;
  rSave(*this, n);
  exponent = exp;
  precision = pr;
  sign = n.isZero() ? CS_POSITIVE_NUMBER : sg;
}

CS_FORCE_INLINE void csRNUMBER::operator=(const char*a)
{
  size_t l = strlen(a);
  if(a && l)
  {
    if(mantissa)
    {
      free(mantissa);
    }
    
    bool eval = 0;
    size_t start = 0, i0 = 0;
    if(l > 2 && a[1] == ' ')
    {
      eval = (bool)a[0];
      i0 = 1;
    }
    
    for(int i=i0; i<l; i++)
    {
      if(a[i] != ' ' && a[i] != '\t')
      {
        start = i;
        break;
      }
    }
    
    csRNUMBER rn(eval, a + start);
    sign = rn.sign;
    mantSize = rn.mantSize;
    exponent = rn.exponent;
    mantissa = rn.mantissa;
    precision = rn.precision > precision ? rn.precision : precision;
  }
}

CS_FORCE_INLINE void csRNUMBER::assign(csRNUMBER a)
{
  if(mantissa)
  {
    free(mantissa);
  }

  sign = a.sign;
  mantSize = a.mantSize;
  exponent = a.exponent;
  mantissa = a.mantissa;
  precision = a.precision > precision ? a.precision : precision;


}
CS_FORCE_INLINE void csRNUMBER::set(size_t id, char digit)
{
  char* s = rExportDigits(*this);
  size_t n = strlen(s);
  char ch = (digit >= '0' && digit <= '9') ? digit : (char)('0' + (digit % 10));
  if (id >= n) {
    char* g = (char*)malloc(id + 2);
    memcpy(g, s, n);
    for (size_t i = n; i < id; ++i)
      g[i] = '0';
    g[id] = ch;
    g[id + 1] = 0;
    free(s);
    s = g;
  } else {
    s[id] = ch;
  }
  int exp = exponent;
  bool sg = sign;
  long pr = precision;
  QLimbs v;
  rFromDigits(s, v);
  free(s);
  rSave(*this, v);
  exponent = exp;
  sign = v.isZero() ? CS_POSITIVE_NUMBER : sg;
  precision = pr;
}
CS_FORCE_INLINE void csRNUMBER::setInt(size_t id, char digit)
{
  long l = exponent + (long)mantSize;

  if(exponent <= 0)
  {
    if(l < 0)
    {
      char* newMant = shiftRightCopy(mantissa, mantSize, 1-l);
      free(mantissa);
      mantissa = newMant;
      mantissa[0] = digit;
      mantSize -= l;
      return;
    }

    if(id >= l)
    {
      if(mantissa[0] == '0' && mantSize == 1)
      {
        mantissa[0] = digit;
        return;
      }
      if(mantSize == 1)
      {
        size_t s = mantSize+1+(long)id-l;
        csReallocString(&mantissa, mantSize, s, '0');
        mantissa[id] = digit;
        mantSize = s;
        return;
      }
      size_t s = mantSize+1+(long)id-l;
      csReallocString(&mantissa, mantSize, s, '0');
      mantissa[id] = digit;
      mantSize = s;

      return;
    }


    mantissa[id] = digit;
    return;
  }

  if(id >= mantSize)
  {
    //if(mantSize == 1 &&)

    size_t dl = id - mantSize;
    size_t l = mantSize + dl;
    csReallocString(&mantissa, mantSize, l, '0');
    mantSize = l;
    if(exponent >= dl)
      exponent -= dl;
    else exponent = 0;

    return;
  }

  mantissa[id] = digit;

}

CS_FORCE_INLINE csRNUMBER::operator csQNUMBER()
{
  char* digs = rExportDigits(*this);
  size_t n = strlen(digs);
  csQNUMBER qn;
  if (exponent >= 0) {
    char* num = (char*)malloc(n + (size_t)exponent + 1);
    memcpy(num, digs, n);
    memset(num + n, '0', (size_t)exponent);
    num[n + (size_t)exponent] = 0;
    qn.set(num, "1", sign);
    free(num);
  } else {
    size_t z = (size_t)(-exponent);
    char* den = (char*)malloc(z + 2);
    den[0] = '1';
    memset(den + 1, '0', z);
    den[z + 1] = 0;
    qn.set(digs, den, sign);
    free(den);
  }
  free(digs);
  return qn;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::getPrintFormat(csRNUMBER a) // a corriger car augmente une case vide a la fin de res, ajoutant sa taille reelle
{
  char* digs = rExportDigits(a);
  size_t n = strlen(digs);
  bool neg = a.sign && !(n == 1 && digs[0] == '0');
  char* ret = 0;
  if (a.exponent >= 0) {
    size_t extra = (size_t)a.exponent;
    size_t len = n + extra + (neg ? 1 : 0);
    ret = (char*)malloc(len + 1);
    char* w = ret;
    if (neg)
      *w++ = '-';
    memcpy(w, digs, n);
    w += n;
    memset(w, '0', extra);
    w[extra] = 0;
  } else {
    size_t exp = (size_t)(-a.exponent);
    if (n <= exp) {
      size_t zeros = exp - n;
      size_t len = (neg ? 1 : 0) + 2 + exp;
      ret = (char*)malloc(len + 1);
      char* w = ret;
      if (neg)
        *w++ = '-';
      *w++ = '0';
      *w++ = '.';
      memset(w, '0', zeros);
      w += zeros;
      memcpy(w, digs, n);
      w[n] = 0;
    } else {
      size_t whole = n - exp;
      size_t len = (neg ? 1 : 0) + whole + 1 + exp;
      ret = (char*)malloc(len + 1);
      char* w = ret;
      if (neg)
        *w++ = '-';
      memcpy(w, digs, whole);
      w += whole;
      *w++ = '.';
      memcpy(w, digs + whole, exp);
      w[exp] = 0;
    }
  }
  free(digs);
  return ret;
}

CS_FORCE_INLINE void csRNUMBER::print(const char*title)
{
  char*formated = getPrintFormat(*this);
  cout<<"\n"<<title<<formated<<"\n";
  free(formated);
}

CS_FORCE_INLINE void csRNUMBER::print2(const char*title)
{
  char* s = rExportDigits(*this);
  if (sign == CS_POSITIVE_NUMBER)
    cout << "\n " << title << s << "x10^" << exponent << "\n";
  else
    cout << "\n " << title << "-" << s << "x10^" << exponent << "\n";
  free(s);
}

CS_FORCE_INLINE bool clearAndReturn(csRNUMBER& a, csRNUMBER& b, bool cndResult)
{
  a.clear();
  b.clear();
  return cndResult;
}

CS_FORCE_INLINE bool csRNUMBER::operator!=(csRNUMBER a)
{
  return !(*this == a);
}

CS_FORCE_INLINE bool csRNUMBER::operator!=(long a)
{
  csRNUMBER rn(a);
  bool b = !(*this == rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csRNUMBER::operator==(long a)
{
  csRNUMBER rn(a);
  bool b = (*this == rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csRNUMBER::equalZero()
{
  QLimbs n;
  rLoad(*this, n);
  return n.isZero();
}

CS_FORCE_INLINE bool csRNUMBER::operator==(csRNUMBER _a)
{
  return rCmpValue(*this, _a) == 0;
}

CS_FORCE_INLINE bool csRNUMBER::operator<(long a)
{
  csRNUMBER rn(a);
  bool b = (*this < rn);
  rn.clear();
  return b;
}
CS_FORCE_INLINE bool csRNUMBER::operator<(csRNUMBER _a)
{
  return rCmpValue(*this, _a) < 0;
}

CS_FORCE_INLINE bool csRNUMBER::operator<=(long a)
{
  csRNUMBER rn(a);
  bool b = (*this <= rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csRNUMBER::operator<=(csRNUMBER _a)
{
  return rCmpValue(*this, _a) <= 0;
}

CS_FORCE_INLINE bool csRNUMBER::operator>(long a)
{
  csRNUMBER rn(a);
  bool b = (*this > rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csRNUMBER::operator>(csRNUMBER _a)
{
  return rCmpValue(*this, _a) > 0;
}

CS_FORCE_INLINE bool csRNUMBER::operator>=(long a)
{
  csRNUMBER rn(a);
  bool b = (*this >= rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csRNUMBER::operator>=(csRNUMBER _a)
{
  return rCmpValue(*this, _a) >= 0;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::getMantissaSection(size_t first, size_t last)
{
  csRNUMBER rn;
  char* s = rExportDigits(*this);
  size_t n = strlen(s);
  if (first > n)
    first = n;
  if (last > n)
    last = n;
  if (last < first)
    last = first;
  size_t m = last - first;
  char* slice = (char*)malloc(m + 1);
  if (m)
    memcpy(slice, s + first, m);
  slice[m] = 0;
  if (m == 0) {
    slice[0] = '0';
    slice[1] = 0;
  }
  QLimbs v;
  rFromDigits(slice, v);
  free(slice);
  free(s);
  rSave(rn, v);
  rn.exponent = exponent;
  rn.sign = v.isZero() ? CS_POSITIVE_NUMBER : sign;
  rn.precision = precision;
  return rn;
}

using namespace __mem_man;
using namespace __ar_man;
