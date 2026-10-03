// Limbs are in base 10^9. Decimal text exists only on input and on display.
#include <cstdint>
#include <cstring>
#include <cstdlib>
#ifndef CS_Q_BASE
#define CS_Q_BASE 1000000000u
#define CS_Q_DIGS 9
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
    uint32_t t[3];
    size_t k = 0;
    while (v) {
      t[k++] = (uint32_t)(v % CS_Q_BASE);
      v /= CS_Q_BASE;
    }
    copyFrom(t, k);
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
    r.p[i] = (uint32_t)(cur % CS_Q_BASE);
    carry = cur / CS_Q_BASE;
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
      r[i + j] = (uint32_t)(cur % CS_Q_BASE);
      carry = cur / CS_Q_BASE;
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
    r[i] = (uint32_t)(cur % CS_Q_BASE);
    carry = cur / CS_Q_BASE;
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
    dst[offset + i] = (uint32_t)(cur % CS_Q_BASE);
    carry = cur / CS_Q_BASE;
    ++i;
  }
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
    QArena ar((na + nb) * 8 + 64);
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
    dst[i] = (uint32_t)(cur % CS_Q_BASE);
    carry = cur / CS_Q_BASE;
  }
  if (n < keep)
    dst[n] = (uint32_t)carry;
  for (size_t i = n + 1; i < keep; ++i)
    dst[i] = 0;
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
  if (vn == 1) {
    uint32_t vd = v0[0];
    q.reserve(un);
    uint64_t rem = 0;
    for (size_t i = un; i-- > 0; ) {
      uint64_t cur = rem * CS_Q_BASE + u0[i];
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
  uint32_t d = (uint32_t)(CS_Q_BASE / (v0[vn - 1] + 1));
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
    uint64_t num = (uint64_t)u.p[j + vn] * CS_Q_BASE + u.p[j + vn - 1];
    uint64_t qhat, rhat;
    if (u.p[j + vn] >= v.p[vn - 1]) {
      qhat = CS_Q_BASE - 1;
      rhat = num - qhat * v.p[vn - 1];
    } else {
      qhat = num / v.p[vn - 1];
      rhat = num % v.p[vn - 1];
    }
    while (qhat >= CS_Q_BASE || (rhat < CS_Q_BASE && qhat * (uint64_t)v.p[vn - 2] > rhat * CS_Q_BASE + u.p[j + vn - 2])) {
      --qhat;
      rhat += v.p[vn - 1];
    }
    uint64_t carry = 0;
    int borrow = 0;
    for (size_t i = 0; i < vn; ++i) {
      uint64_t prod = qhat * (uint64_t)v.p[i] + carry;
      carry = prod / CS_Q_BASE;
      uint32_t digit = (uint32_t)(prod % CS_Q_BASE);
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
        u.p[j + i] = (uint32_t)(s % CS_Q_BASE);
        c = s / CS_Q_BASE;
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
    uint64_t cur = rem * CS_Q_BASE + u.p[i];
    r.p[i] = (uint32_t)(cur / d);
    rem = cur % d;
  }
  r.n = vn;
  r.trim();
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
  size_t cap = (digits + CS_Q_DIGS - 1) / CS_Q_DIGS;
  uint32_t* p = (uint32_t*)malloc(cap * sizeof(uint32_t));
  memset(p, 0, cap * sizeof(uint32_t));
  size_t limb = 0;
  uint32_t acc = 0, mul = 1;
  int cnt = 0;
  for (size_t k = len; k-- > i; ) {
    acc += (uint32_t)(s[k] - '0') * mul;
    mul *= 10;
    ++cnt;
    if (cnt == CS_Q_DIGS) {
      p[limb++] = acc;
      acc = 0;
      mul = 1;
      cnt = 0;
    }
  }
  if (cnt)
    p[limb++] = acc;
  out = (char*)p;
  n = limb ? limb : 1;
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

}
