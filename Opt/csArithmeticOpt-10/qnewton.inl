// Integer division by a Newton reciprocal.
// A reciprocal of 2^(2n) / v is computed by doubling the precision,
// then the quotient is formed by blocks. A final correction of a few units
// makes the quotient exact. Otherwise the caller falls back to Knuth.

static int qNewtonCutoff = 100000;

CS_FORCEINLINE int qLimbBits(uint32_t x) {
  if (x == 0)
    return 0;
  return 32 - __builtin_clz(x);
}

CS_FORCEINLINE size_t qBitLen(const uint32_t* a, size_t n) {
  if (!a || n == 0 || (n == 1 && a[0] == 0))
    return 0;
  while (n > 1 && a[n - 1] == 0)
    --n;
  return (n - 1) * 32 + (size_t)qLimbBits(a[n - 1]);
}

CS_FORCEINLINE void qShlBits(const uint32_t* a, size_t na, size_t bits, QLimbs& r) {
  if (!a || na == 0 || (na == 1 && a[0] == 0)) {
    r.setZero();
    return;
  }
  while (na > 1 && a[na - 1] == 0)
    --na;
  size_t limb = bits >> 5;
  unsigned b = (unsigned)(bits & 31);
  size_t words = na + limb + 1;
  r.reserve(words);
  memset(r.p, 0, words * sizeof(uint32_t));
  if (b == 0) {
    memcpy(r.p + limb, a, na * sizeof(uint32_t));
    r.n = na + limb;
  } else {
    uint32_t carry = 0;
    for (size_t i = 0; i < na; ++i) {
      uint32_t cur = a[i];
      r.p[i + limb] = (cur << b) | carry;
      carry = cur >> (32 - b);
    }
    r.p[na + limb] = carry;
    r.n = na + limb + (carry ? 1 : 0);
  }
  r.trim();
}

CS_FORCEINLINE void qShrBits(const uint32_t* a, size_t na, size_t bits, QLimbs& r) {
  if (!a || na == 0 || (na == 1 && a[0] == 0)) {
    r.setZero();
    return;
  }
  size_t limb = bits >> 5;
  unsigned b = (unsigned)(bits & 31);
  if (limb >= na) {
    r.setZero();
    return;
  }
  size_t n = na - limb;
  r.reserve(n);
  if (b == 0) {
    memcpy(r.p, a + limb, n * sizeof(uint32_t));
    r.n = n;
  } else {
    for (size_t i = 0; i < n - 1; ++i)
      r.p[i] = (a[i + limb] >> b) | (a[i + limb + 1] << (32 - b));
    r.p[n - 1] = a[na - 1] >> b;
    r.n = n;
  }
  r.trim();
}

CS_INLINE int qHasLowBits(const uint32_t* a, size_t na, size_t bits) {
  size_t limb = bits >> 5;
  unsigned b = (unsigned)(bits & 31);
  size_t m = limb < na ? limb : na;
  for (size_t i = 0; i < m; ++i) {
    if (a[i])
      return 1;
  }
  if (b && limb < na && (a[limb] & ((1u << b) - 1)))
    return 1;
  return 0;
}

CS_INLINE void qMaskLow(QLimbs& a, size_t bits) {
  size_t limb = bits >> 5;
  unsigned b = (unsigned)(bits & 31);
  if (limb >= a.n) {
    return;
  }
  if (b == 0) {
    a.n = limb ? limb : 1;
    if (limb == 0)
      a.p[0] = 0;
  } else {
    a.p[limb] &= (1u << b) - 1;
    a.n = limb + 1;
  }
  a.trim();
}

CS_FORCEINLINE void qIncLimb(QLimbs& a) {
  size_t i = 0;
  while (1) {
    if (i >= a.n) {
      a.reserve(i + 1);
      a.p[i] = 1;
      a.n = i + 1;
      return;
    }
    if (a.p[i] != 0xFFFFFFFFu) {
      a.p[i]++;
      return;
    }
    a.p[i] = 0;
    ++i;
  }
}

CS_FORCEINLINE void qDecLimb(QLimbs& a) {
  size_t i = 0;
  while (i < a.n && a.p[i] == 0) {
    a.p[i] = 0xFFFFFFFFu;
    ++i;
  }
  if (i < a.n)
    a.p[i]--;
  a.trim();
}

CS_INLINE void qRecip2n(const QLimbs& a, size_t nbits, QLimbs& g) {
  if (nbits <= 96) {
    QLimbs one, num, qq, rr;
    one.reserve(1);
    one.p[0] = 1;
    one.n = 1;
    qShlBits(one.p, one.n, 2 * nbits, num);
    qDivModKnuth(num.p, num.n, a.p, a.n, qq, rr);
    g.copy(qq);
    return;
  }
  size_t h = (nbits + 1) / 2;
  size_t s = nbits - h;
  QLimbs ahi;
  qShrBits(a.p, a.n, s, ahi);
  size_t hb = qBitLen(ahi.p, ahi.n);
  QLimbs x;
  qRecip2n(ahi, hb, x);
  ptrdiff_t sh = (ptrdiff_t)(2 * nbits - s - 2 * hb);
  QLimbs approx;
  if (sh >= 0)
    qShlBits(x.p, x.n, (size_t)sh, approx);
  else
    qShrBits(x.p, x.n, (size_t)(-sh), approx);
  QLimbs ag, gg, quot, two;
  qMul(a.p, a.n, approx.p, approx.n, ag);
  qMul(approx.p, approx.n, ag.p, ag.n, gg);
  qShrBits(gg.p, gg.n, 2 * nbits, quot);
  if (qHasLowBits(gg.p, gg.n, 2 * nbits))
    qIncLimb(quot);
  qShlBits(approx.p, approx.n, 1, two);
  if (qCmpLimbs(two.p, two.n, quot.p, quot.n) < 0) {
    qKaraOverflow = 1;
    g.setZero();
    return;
  }
  qSub(two.p, two.n, quot.p, quot.n, g);
}

CS_INLINE int qDivNewton(const uint32_t* u0, size_t un, const uint32_t* v0, size_t vn, QLimbs& q, QLimbs& r) {
  size_t nb = qBitLen(v0, vn);
  if (nb == 0) {
    q.setZero();
    r.setZero();
    return 0;
  }
  QLimbs v;
  v.copyFrom(v0, vn);
  QLimbs g;
  int prev = qKaraOverflow;
  qKaraOverflow = 0;
  qRecip2n(v, nb, g);
  if (qKaraOverflow) {
    qKaraOverflow = prev;
    return 0;
  }
  size_t ub = qBitLen(u0, un);
  size_t blocks = (ub + nb - 1) / nb;
  if (blocks < 1)
    blocks = 1;
  size_t top = blocks * nb;
  QLimbs rem, quot;
  rem.setZero();
  quot.setZero();
  int guardMax = 64;
  for (size_t k = blocks; k-- > 0; ) {
    size_t pos = k * nb;
    QLimbs shifted, chunk, cur, prod, qhat, qv;
    qShrBits(u0, un, pos, shifted);
    qMaskLow(shifted, nb);
    chunk.copy(shifted);
    qShlBits(rem.p, rem.n, nb, cur);
    QLimbs curSum;
    qAdd(cur.p, cur.n, chunk.p, chunk.n, curSum);
    qMul(curSum.p, curSum.n, g.p, g.n, prod);
    qShrBits(prod.p, prod.n, 2 * nb, qhat);
    qMul(qhat.p, qhat.n, v.p, v.n, qv);
    int guard = 0;
    while (guard < guardMax) {
      int c = qCmpLimbs(qv.p, qv.n, curSum.p, curSum.n);
      if (c == 0) {
        rem.setZero();
        break;
      }
      if (c < 0) {
        qSub(curSum.p, curSum.n, qv.p, qv.n, rem);
        if (qCmpLimbs(rem.p, rem.n, v.p, v.n) < 0)
          break;
        qIncLimb(qhat);
        QLimbs qv2;
        qAdd(qv.p, qv.n, v.p, v.n, qv2);
        qv.copy(qv2);
      } else {
        if (qhat.isZero()) {
          qKaraOverflow = 1;
          break;
        }
        qDecLimb(qhat);
        if (qCmpLimbs(qv.p, qv.n, v.p, v.n) < 0) {
          qKaraOverflow = 1;
          break;
        }
        QLimbs qv2;
        qSub(qv.p, qv.n, v.p, v.n, qv2);
        qv.copy(qv2);
      }
      ++guard;
    }
    if (guard >= guardMax || qKaraOverflow) {
      qKaraOverflow = prev;
      return 0;
    }
    QLimbs shiftedQ;
    qShlBits(quot.p, quot.n, nb, shiftedQ);
    qAdd(shiftedQ.p, shiftedQ.n, qhat.p, qhat.n, quot);
    (void)pos;
    (void)top;
  }
  if (qKaraOverflow) {
    qKaraOverflow = prev;
    return 0;
  }
  q.copy(quot);
  r.copy(rem);
  qKaraOverflow = prev;
  return 1;
}
