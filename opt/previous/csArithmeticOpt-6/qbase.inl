// Membres en base 10^19. Le produit de deux membres tient dans 128 bits.
#include <cstdint>
#include <cstring>
#include <cstdlib>
#ifndef CS_Q_BASE
#define CS_Q_BASE 10000000000000000000ULL
#define CS_Q_DIGS 19
#endif

namespace {

enum { QLIMB_STACK = 256 };

using QLimb = uint64_t;
using QWide = unsigned __int128;

struct QLimbs {
  QLimb small[QLIMB_STACK];
  QLimb* p;
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
          memcpy(small, p, n * sizeof(QLimb));
        free(p);
        heap = false;
      }
      p = small;
      cap = QLIMB_STACK;
      return;
    }
    if (heap && cap >= need)
      return;
    QLimb* np = (QLimb*)malloc(need * sizeof(QLimb));
    if (p && n)
      memcpy(np, p, n * sizeof(QLimb));
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
  CS_INLINE void copyFrom(const QLimb* s, size_t sn) {
    if (!s || sn == 0) {
      setZero();
      return;
    }
    while (sn > 1 && s[sn - 1] == 0)
      --sn;
    reserve(sn);
    memcpy(p, s, sn * sizeof(QLimb));
    n = sn;
  }
  CS_INLINE void copy(const QLimbs& o) { copyFrom(o.p, o.n); }
  CS_INLINE void setU64(uint64_t v) {
    if (v == 0) {
      setZero();
      return;
    }
    QLimb t[3];
    size_t k = 0;
    while (v) {
      t[k++] = (QLimb)(v % CS_Q_BASE);
      v /= CS_Q_BASE;
    }
    copyFrom(t, k);
  }
  CS_INLINE char* leak() const {
    size_t m = n ? n : 1;
    QLimb* o = (QLimb*)malloc(m * sizeof(QLimb));
    if (!p || n == 0)
      o[0] = 0;
    else
      memcpy(o, p, n * sizeof(QLimb));
    return (char*)o;
  }
};

CS_INLINE int qCmpLimbs(const QLimb* a, size_t na, const QLimb* b, size_t nb) {
  if (na != nb)
    return na < nb ? -1 : 1;
  for (size_t i = na; i-- > 0; ) {
    if (a[i] != b[i])
      return a[i] < b[i] ? -1 : 1;
  }
  return 0;
}

CS_INLINE void qAdd(const QLimb* a, size_t na, const QLimb* b, size_t nb, QLimbs& r) {
  size_t m = na > nb ? na : nb;
  r.reserve(m + 1);
  QWide carry = 0;
  for (size_t i = 0; i < m; ++i) {
    QWide cur = carry;
    if (i < na)
      cur += a[i];
    if (i < nb)
      cur += b[i];
    r.p[i] = (QLimb)(cur % CS_Q_BASE);
    carry = cur / CS_Q_BASE;
  }
  if (carry) {
    r.p[m] = (QLimb)carry;
    r.n = m + 1;
  } else {
    r.n = m;
  }
  r.trim();
}

CS_INLINE void qSub(const QLimb* a, size_t na, const QLimb* b, size_t nb, QLimbs& r) {
  r.reserve(na);
  int borrow = 0;
  for (size_t i = 0; i < na; ++i) {
    __int128 cur = (__int128)a[i] - (i < nb ? (__int128)b[i] : 0) - borrow;
    if (cur < 0) {
      cur += CS_Q_BASE;
      borrow = 1;
    } else {
      borrow = 0;
    }
    r.p[i] = (QLimb)cur;
  }
  r.n = na;
  r.trim();
}

CS_INLINE void qSubAbs(const QLimb* a, size_t na, const QLimb* b, size_t nb, QLimbs& r, bool& neg) {
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

CS_INLINE void qMul(const QLimb* a, size_t na, const QLimb* b, size_t nb, QLimbs& r) {
  if (!a || !b || na == 0 || nb == 0 || (na == 1 && a[0] == 0) || (nb == 1 && b[0] == 0)) {
    r.setZero();
    return;
  }
  r.reserve(na + nb);
  r.n = na + nb;
  memset(r.p, 0, r.n * sizeof(QLimb));
  for (size_t i = 0; i < na; ++i) {
    QWide carry = 0;
    for (size_t j = 0; j < nb; ++j) {
      QWide cur = (QWide)r.p[i + j] + (QWide)a[i] * b[j] + carry;
      r.p[i + j] = (QLimb)(cur % CS_Q_BASE);
      carry = cur / CS_Q_BASE;
    }
    r.p[i + nb] = (QLimb)carry;
  }
  r.trim();
}

CS_INLINE void qMulSmall(const QLimb* a, size_t n, QLimb m, QLimb* dst, size_t keep) {
  QWide carry = 0;
  for (size_t i = 0; i < n; ++i) {
    QWide cur = (QWide)a[i] * m + carry;
    dst[i] = (QLimb)(cur % CS_Q_BASE);
    carry = cur / CS_Q_BASE;
  }
  if (n < keep)
    dst[n] = (QLimb)carry;
  for (size_t i = n + 1; i < keep; ++i)
    dst[i] = 0;
}

CS_INLINE void qDivMod(const QLimb* u0, size_t un, const QLimb* v0, size_t vn, QLimbs& q, QLimbs& r) {
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
    QLimb vd = v0[0];
    q.reserve(un);
    QLimb rem = 0;
    for (size_t i = un; i-- > 0; ) {
      QWide cur = (QWide)rem * CS_Q_BASE + u0[i];
      q.p[i] = (QLimb)(cur / vd);
      rem = (QLimb)(cur % vd);
    }
    q.n = un;
    q.trim();
    r.reserve(1);
    r.p[0] = rem;
    r.n = 1;
    return;
  }
  QLimb d = (QLimb)(CS_Q_BASE / (v0[vn - 1] + 1));
  QLimbs u, v;
  u.reserve(un + 1);
  qMulSmall(u0, un, d, u.p, un + 1);
  u.n = un + 1;
  v.reserve(vn);
  qMulSmall(v0, vn, d, v.p, vn);
  v.n = vn;
  size_t qn = un - vn + 1;
  q.reserve(qn);
  memset(q.p, 0, qn * sizeof(QLimb));
  q.n = qn;
  for (int j = (int)un - (int)vn; j >= 0; --j) {
    QWide num = (QWide)u.p[j + vn] * CS_Q_BASE + u.p[j + vn - 1];
    QWide qhat, rhat;
    if (u.p[j + vn] >= v.p[vn - 1]) {
      qhat = (QWide)CS_Q_BASE - 1;
      rhat = num - qhat * v.p[vn - 1];
    } else {
      qhat = num / v.p[vn - 1];
      rhat = num % v.p[vn - 1];
    }
    while (qhat >= CS_Q_BASE || (rhat < CS_Q_BASE && qhat * (QWide)v.p[vn - 2] > rhat * CS_Q_BASE + u.p[j + vn - 2])) {
      --qhat;
      rhat += v.p[vn - 1];
    }
    QWide carry = 0;
    int borrow = 0;
    for (size_t i = 0; i < vn; ++i) {
      QWide prod = qhat * (QWide)v.p[i] + carry;
      carry = prod / CS_Q_BASE;
      QLimb digit = (QLimb)(prod % CS_Q_BASE);
      __int128 cur = (__int128)u.p[j + i] - (__int128)digit - borrow;
      if (cur < 0) {
        cur += CS_Q_BASE;
        borrow = 1;
      } else {
        borrow = 0;
      }
      u.p[j + i] = (QLimb)cur;
    }
    __int128 last = (__int128)u.p[j + vn] - (__int128)carry - borrow;
    if (last < 0) {
      --qhat;
      QWide c = 0;
      for (size_t i = 0; i < vn; ++i) {
        QWide s = (QWide)u.p[j + i] + v.p[i] + c;
        u.p[j + i] = (QLimb)(s % CS_Q_BASE);
        c = s / CS_Q_BASE;
      }
      last += (__int128)CS_Q_BASE + (__int128)c;
    }
    u.p[j + vn] = (QLimb)last;
    q.p[j] = (QLimb)qhat;
  }
  q.trim();
  r.reserve(vn);
  QLimb rem = 0;
  for (size_t i = vn; i-- > 0; ) {
    QWide cur = (QWide)rem * CS_Q_BASE + u.p[i];
    r.p[i] = (QLimb)(cur / d);
    rem = (QLimb)(cur % d);
  }
  r.n = vn;
  r.trim();
}

CS_INLINE void qGcd(const QLimb* a0, size_t na, const QLimb* b0, size_t nb, QLimbs& g) {
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
    QLimb* p = (QLimb*)malloc(sizeof(QLimb));
    p[0] = 0;
    out = (char*)p;
    n = 1;
    return;
  }
  size_t digits = len - i;
  size_t cap = (digits + CS_Q_DIGS - 1) / CS_Q_DIGS;
  QLimb* p = (QLimb*)malloc(cap * sizeof(QLimb));
  memset(p, 0, cap * sizeof(QLimb));
  size_t limb = 0;
  uint64_t acc = 0, mul = 1;
  int cnt = 0;
  for (size_t k = len; k-- > i; ) {
    acc += (uint64_t)(s[k] - '0') * mul;
    mul *= 10;
    ++cnt;
    if (cnt == CS_Q_DIGS) {
      p[limb++] = (QLimb)acc;
      acc = 0;
      mul = 1;
      cnt = 0;
    }
  }
  if (cnt)
    p[limb++] = (QLimb)acc;
  out = (char*)p;
  n = limb ? limb : 1;
}

CS_INLINE char* qDupLimbs(const char* s, size_t n) {
  if (!s || n == 0) {
    QLimb* p = (QLimb*)malloc(sizeof(QLimb));
    p[0] = 0;
    return (char*)p;
  }
  QLimb* p = (QLimb*)malloc(n * sizeof(QLimb));
  memcpy(p, s, n * sizeof(QLimb));
  return (char*)p;
}

CS_INLINE char* qLimbOne() {
  QLimb* p = (QLimb*)malloc(sizeof(QLimb));
  p[0] = 1;
  return (char*)p;
}

CS_INLINE uint64_t qAbsLong(long v) {
  if (v >= 0)
    return (uint64_t)v;
  return (uint64_t)0 - (uint64_t)v;
}

CS_INLINE const QLimb* qL(const char* p) { return (const QLimb*)p; }

CS_INLINE double qLimbsToDouble(const char* raw, size_t n) {
  if (!raw || n == 0)
    return 0;
  const QLimb* p = (const QLimb*)raw;
  double v = 0;
  for (size_t i = n; i-- > 0; )
    v = v * (double)CS_Q_BASE + (double)p[i];
  return v;
}

#ifndef QBASE_TEST
CS_INLINE bool qRawZero(const char* p, size_t n) {
  if (!p || n == 0)
    return true;
  const QLimb* a = (const QLimb*)p;
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
