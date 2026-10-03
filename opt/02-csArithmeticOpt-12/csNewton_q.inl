// Integer division by a Newton reciprocal.
// A reciprocal of 2^(2n) / v is computed by doubling the precision,
// then the quotient is formed by blocks. A final correction of a few units
// makes the quotient exact. Otherwise the caller falls back to Knuth.

static int csNewtonCutoff_q = 100000;

/**
 * @brief Returns the number of significant bits of a limb.
 * @param x Abscissa or input value.
 * @return Computed value.
 */
CS_FORCEINLINE int csLimbBits_q(uint32_t x) {
  if (x == 0)
    return 0;
  return 32 - __builtin_clz(x);
}

/**
 * @brief Returns the number of bits of an integer.
 * @param a First operand.
 * @param n Number of elements.
 * @return Computed value.
 */
CS_FORCEINLINE size_t csBitLen_q(const uint32_t* a, size_t n) {
  if (!a || n == 0 || (n == 1 && a[0] == 0))
    return 0;
  while (n > 1 && a[n - 1] == 0)
    --n;
  return (n - 1) * 32 + (size_t)csLimbBits_q(a[n - 1]);
}

/**
 * @brief Shifts an integer left by a number of bits.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param bits Number of bits.
 * @param r Integer in limbs.
 */
CS_FORCEINLINE void csShlBits_q(const uint32_t* a, size_t na, size_t bits, csLimbs_q& r) {
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

/**
 * @brief Shifts an integer right by a number of bits.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param bits Number of bits.
 * @param r Integer in limbs.
 */
CS_FORCEINLINE void csShrBits_q(const uint32_t* a, size_t na, size_t bits, csLimbs_q& r) {
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

/**
 * @brief Reports whether any low-order bits are non-zero.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param bits Number of bits.
 * @return Computed value.
 */
CS_INLINE int csHasLowBits_q(const uint32_t* a, size_t na, size_t bits) {
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

/**
 * @brief Keeps only the requested low-order bits.
 * @param a First operand.
 * @param bits Number of bits.
 */
CS_INLINE void csMaskLow_q(csLimbs_q& a, size_t bits) {
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

/**
 * @brief Increments an integer by one.
 * @param a First operand.
 */
CS_FORCEINLINE void csIncLimb_q(csLimbs_q& a) {
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

/**
 * @brief Decrements an integer by one.
 * @param a First operand.
 */
CS_FORCEINLINE void csDecLimb_q(csLimbs_q& a) {
  size_t i = 0;
  while (i < a.n && a.p[i] == 0) {
    a.p[i] = 0xFFFFFFFFu;
    ++i;
  }
  if (i < a.n)
    a.p[i]--;
  a.trim();
}

/**
 * @brief Computes an integer reciprocal by Newton's method.
 * @param a First operand.
 * @param nbits Parameter @p nbits.
 * @param g Integer in limbs.
 */
CS_INLINE void csRecip2n_q(const csLimbs_q& a, size_t nbits, csLimbs_q& g) {
  if (nbits <= 96) {
    csLimbs_q one, num, qq, rr;
    one.reserve(1);
    one.p[0] = 1;
    one.n = 1;
    csShlBits_q(one.p, one.n, 2 * nbits, num);
    csDivModKnuth_q(num.p, num.n, a.p, a.n, qq, rr);
    g.copy(qq);
    return;
  }
  size_t h = (nbits + 1) / 2;
  size_t s = nbits - h;
  csLimbs_q ahi;
  csShrBits_q(a.p, a.n, s, ahi);
  size_t hb = csBitLen_q(ahi.p, ahi.n);
  csLimbs_q x;
  csRecip2n_q(ahi, hb, x);
  ptrdiff_t sh = (ptrdiff_t)(2 * nbits - s - 2 * hb);
  csLimbs_q approx;
  if (sh >= 0)
    csShlBits_q(x.p, x.n, (size_t)sh, approx);
  else
    csShrBits_q(x.p, x.n, (size_t)(-sh), approx);
  csLimbs_q ag, gg, quot, two;
  csMul_q(a.p, a.n, approx.p, approx.n, ag);
  csMul_q(approx.p, approx.n, ag.p, ag.n, gg);
  csShrBits_q(gg.p, gg.n, 2 * nbits, quot);
  if (csHasLowBits_q(gg.p, gg.n, 2 * nbits))
    csIncLimb_q(quot);
  csShlBits_q(approx.p, approx.n, 1, two);
  if (csCmpLimbs_q(two.p, two.n, quot.p, quot.n) < 0) {
    csKaraOverflow_q = 1;
    g.setZero();
    return;
  }
  csSub_q(two.p, two.n, quot.p, quot.n, g);
}

/**
 * @brief Divides two integers by a Newton reciprocal.
 * @param u0 Limb array.
 * @param un Parameter @p un.
 * @param v0 Limb array.
 * @param vn Parameter @p vn.
 * @param q Integer in limbs.
 * @param r Integer in limbs.
 * @return Computed value.
 */
CS_INLINE int csDivNewton_q(const uint32_t* u0, size_t un, const uint32_t* v0, size_t vn, csLimbs_q& q, csLimbs_q& r) {
  size_t nb = csBitLen_q(v0, vn);
  if (nb == 0) {
    q.setZero();
    r.setZero();
    return 0;
  }
  csLimbs_q v;
  v.copyFrom(v0, vn);
  csLimbs_q g;
  int prev = csKaraOverflow_q;
  csKaraOverflow_q = 0;
  csRecip2n_q(v, nb, g);
  if (csKaraOverflow_q) {
    csKaraOverflow_q = prev;
    return 0;
  }
  size_t ub = csBitLen_q(u0, un);
  size_t blocks = (ub + nb - 1) / nb;
  if (blocks < 1)
    blocks = 1;
  size_t top = blocks * nb;
  csLimbs_q rem, quot;
  rem.setZero();
  quot.setZero();
  int guardMax = 64;
  for (size_t k = blocks; k-- > 0; ) {
    size_t pos = k * nb;
    csLimbs_q shifted, chunk, cur, prod, qhat, qv;
    csShrBits_q(u0, un, pos, shifted);
    csMaskLow_q(shifted, nb);
    chunk.copy(shifted);
    csShlBits_q(rem.p, rem.n, nb, cur);
    csLimbs_q curSum;
    csAdd_q(cur.p, cur.n, chunk.p, chunk.n, curSum);
    csMul_q(curSum.p, curSum.n, g.p, g.n, prod);
    csShrBits_q(prod.p, prod.n, 2 * nb, qhat);
    csMul_q(qhat.p, qhat.n, v.p, v.n, qv);
    int guard = 0;
    while (guard < guardMax) {
      int c = csCmpLimbs_q(qv.p, qv.n, curSum.p, curSum.n);
      if (c == 0) {
        rem.setZero();
        break;
      }
      if (c < 0) {
        csSub_q(curSum.p, curSum.n, qv.p, qv.n, rem);
        if (csCmpLimbs_q(rem.p, rem.n, v.p, v.n) < 0)
          break;
        csIncLimb_q(qhat);
        csLimbs_q qv2;
        csAdd_q(qv.p, qv.n, v.p, v.n, qv2);
        qv.copy(qv2);
      } else {
        if (qhat.isZero()) {
          csKaraOverflow_q = 1;
          break;
        }
        csDecLimb_q(qhat);
        if (csCmpLimbs_q(qv.p, qv.n, v.p, v.n) < 0) {
          csKaraOverflow_q = 1;
          break;
        }
        csLimbs_q qv2;
        csSub_q(qv.p, qv.n, v.p, v.n, qv2);
        qv.copy(qv2);
      }
      ++guard;
    }
    if (guard >= guardMax || csKaraOverflow_q) {
      csKaraOverflow_q = prev;
      return 0;
    }
    csLimbs_q shiftedQ;
    csShlBits_q(quot.p, quot.n, nb, shiftedQ);
    csAdd_q(shiftedQ.p, shiftedQ.n, qhat.p, qhat.n, quot);
    (void)pos;
    (void)top;
  }
  if (csKaraOverflow_q) {
    csKaraOverflow_q = prev;
    return 0;
  }
  q.copy(quot);
  r.copy(rem);
  csKaraOverflow_q = prev;
  return 1;
}
