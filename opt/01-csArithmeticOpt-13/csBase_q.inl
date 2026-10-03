// Limbs are in base 2^32. Decimal text exists only on input and on display.
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cmath>
#ifndef CS_BASE_Q
#define CS_BASE_Q (4294967296LL)
#endif
#ifndef CS_FORCEINLINE
#define CS_FORCEINLINE inline __attribute__((always_inline))
#endif

namespace {

enum { csLimbStack_q = 256 };

struct csLimbs_q {
  uint32_t small[csLimbStack_q];
  uint32_t* p;
  size_t n;
  size_t cap;
  bool heap;

/**
 * @brief Constructs the object.
 */
  CS_INLINE csLimbs_q() : n(1), cap(csLimbStack_q), heap(false) {
    p = small;
    small[0] = 0;
  }
  CS_FORCEINLINE ~csLimbs_q() {
    if (heap && p)
      free(p);
  }
  csLimbs_q(const csLimbs_q&) = delete;
  csLimbs_q& operator=(const csLimbs_q&) = delete;

/**
 * @brief Reserves the requested capacity.
 * @param need Parameter @p need.
 */
  CS_INLINE void reserve(size_t need) {
    if (need < 1)
      need = 1;
    if (need <= (size_t)csLimbStack_q) {
      if (heap) {
        if (n && n <= (size_t)csLimbStack_q)
          memcpy(small, p, n * sizeof(uint32_t));
        free(p);
        heap = false;
      }
      p = small;
      cap = csLimbStack_q;
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
/**
 * @brief Removes high-order zero limbs.
 */
  CS_INLINE void trim() {
    while (n > 1 && p[n - 1] == 0)
      --n;
  }
/**
 * @brief Sets the value to zero.
 */
  CS_INLINE void setZero() {
    reserve(1);
    p[0] = 0;
    n = 1;
  }
/**
 * @brief Sets the value to one.
 */
  CS_INLINE void setOne() {
    reserve(1);
    p[0] = 1;
    n = 1;
  }
/**
 * @brief Reports whether the value is zero.
 * @return True when the condition holds.
 */
  CS_INLINE bool isZero() const { return n == 0 || (n == 1 && p[0] == 0); }
/**
 * @brief Reports whether the value equals one.
 * @return True when the condition holds.
 */
  CS_INLINE bool isOne() const { return n == 1 && p[0] == 1; }
/**
 * @brief Copies limbs into this object.
 * @param s Limb array.
 * @param sn Parameter @p sn.
 */
  CS_FORCEINLINE void copyFrom(const uint32_t* s, size_t sn) {
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
/**
 * @brief Copies the value.
 * @param o Integer in limbs.
 */
  CS_FORCEINLINE void copy(const csLimbs_q& o) { copyFrom(o.p, o.n); }
/**
 * @brief Assigns an unsigned 64-bit integer.
 * @param v Parameter @p v.
 */
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
/**
 * @brief Transfers ownership of the limb memory to the caller.
 * @return Resulting text. The caller frees the memory.
 */
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

/**
 * @brief Compares two integers in base 2^32.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param b Second operand.
 * @param nb Number of elements.
 * @return Computed value.
 */
CS_INLINE int csCmpLimbs_q(const uint32_t* a, size_t na, const uint32_t* b, size_t nb) {
  if (na != nb)
    return na < nb ? -1 : 1;
  for (size_t i = na; i-- > 0; ) {
    if (a[i] != b[i])
      return a[i] < b[i] ? -1 : 1;
  }
  return 0;
}

/**
 * @brief Adds two integers.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param b Second operand.
 * @param nb Number of elements.
 * @param r Integer in limbs.
 */
CS_INLINE void csAdd_q(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, csLimbs_q& r) {
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

/**
 * @brief Subtracts the second integer from the first.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param b Second operand.
 * @param nb Number of elements.
 * @param r Integer in limbs.
 */
CS_INLINE void csSub_q(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, csLimbs_q& r) {
  r.reserve(na);
  int64_t borrow = 0;
  for (size_t i = 0; i < na; ++i) {
    int64_t cur = (int64_t)a[i] - (i < nb ? (int64_t)b[i] : 0) - borrow;
    if (cur < 0) {
      cur += CS_BASE_Q;
      borrow = 1;
    } else {
      borrow = 0;
    }
    r.p[i] = (uint32_t)cur;
  }
  r.n = na;
  r.trim();
}

/**
 * @brief Subtracts the absolute values and reports the sign of the result.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param b Second operand.
 * @param nb Number of elements.
 * @param r Integer in limbs.
 * @param neg Receives the sign of the result.
 */
CS_INLINE void csSubAbs_q(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, csLimbs_q& r, bool& neg) {
  int c = csCmpLimbs_q(a, na, b, nb);
  neg = c < 0;
  if (c == 0) {
    r.setZero();
    return;
  }
  if (c > 0)
    csSub_q(a, na, b, nb, r);
  else
    csSub_q(b, nb, a, na, r);
}

static int csKaraCutoff_q = 32;
static thread_local int csKaraOverflow_q = 0;

struct csArena_q {
  uint32_t* p;
  size_t used;
  size_t cap;
/**
 * @brief Constructs the object.
 * @param n Number of elements.
 */
  CS_INLINE csArena_q(size_t n) : used(0), cap(n) {
    p = (uint32_t*)malloc(n * sizeof(uint32_t));
  }
  CS_INLINE ~csArena_q() { free(p); }
/**
 * @brief Takes a buffer from the arena.
 * @param n Number of elements.
 * @return Computed value.
 */
  CS_INLINE uint32_t* take(size_t n) {
    if (n < 1) n = 1;
    if (used + n > cap) {
      csKaraOverflow_q = 1;
      return p;
    }
    uint32_t* r = p + used;
    used += n;
    return r;
  }
/**
 * @brief Saves the current arena position.
 * @return Computed value.
 */
  CS_INLINE size_t mark() const { return used; }
/**
 * @brief Restores the arena to the last mark.
 * @param m Number of evaluation points.
 */
  CS_INLINE void rewind(size_t m) { used = m; }
};

/**
 * @brief Returns the number of significant limbs.
 * @param a First operand.
 * @param n Number of elements.
 * @return Computed value.
 */
CS_INLINE size_t csSigLen_q(const uint32_t* a, size_t n) {
  while (n > 1 && a[n - 1] == 0)
    --n;
  if (n == 1 && a[0] == 0)
    return 0;
  return n;
}

/**
 * @brief Computes the schoolbook product of two integers.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param b Second operand.
 * @param nb Number of elements.
 * @param r Limb array.
 */
CS_INLINE void csMulSchoolRaw_q(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, uint32_t* r) {
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

/**
 * @brief Adds two integers into a buffer.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param b Second operand.
 * @param nb Number of elements.
 * @param r Limb array.
 * @param nr Parameter @p nr.
 */
CS_INLINE void csAddRaw_q(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, uint32_t* r, size_t& nr) {
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
  nr = csSigLen_q(r, i);
}

/**
 * @brief Subtracts an integer in place.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param acap Parameter @p acap.
 * @param b Second operand.
 * @param nb Number of elements.
 */
CS_INLINE void csSubRaw_q(uint32_t* a, size_t& na, size_t acap, const uint32_t* b, size_t nb) {
  if (nb == 0)
    return;
  size_t n = na > nb ? na : nb;
  if (n > acap) {
    csKaraOverflow_q = 1;
    return;
  }
  int borrow = 0;
  for (size_t i = 0; i < n; ++i) {
    __int128 cur = (i < na ? (__int128)a[i] : 0) - (i < nb ? (__int128)b[i] : 0) - borrow;
    if (cur < 0) {
      cur += CS_BASE_Q;
      borrow = 1;
    } else {
      borrow = 0;
    }
    a[i] = (uint32_t)cur;
  }
  if (borrow) {
    csKaraOverflow_q = 1;
    return;
  }
  na = csSigLen_q(a, n);
}

/**
 * @brief Adds an integer starting at a limb offset.
 * @param dst Destination of the operation.
 * @param dstLen Parameter @p dstLen.
 * @param src Source of the operation.
 * @param ns Parameter @p ns.
 * @param offset Parameter @p offset.
 */
CS_INLINE void csAddInto_q(uint32_t* dst, size_t dstLen, const uint32_t* src, size_t ns, size_t offset) {
  uint64_t carry = 0;
  size_t i = 0;
  while (i < ns || carry) {
    if (offset + i >= dstLen) {
      csKaraOverflow_q = 1;
      return;
    }
    uint64_t cur = carry + dst[offset + i] + (i < ns ? src[i] : 0);
    dst[offset + i] = (uint32_t)cur;
    carry = cur >> 32;
    ++i;
  }
}

static int csToomCutoff_q = 4096;

struct csSigned_q {
  uint32_t* p;
  size_t n;
  size_t cap;
  int neg;
};

/**
 * @brief Computes a recursive product: schoolbook, Karatsuba, Toom-3 or Fourier.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param b Second operand.
 * @param nb Number of elements.
 * @param r Limb array.
 * @param ar Parameter @p ar.
 */
CS_INLINE void csMulRec_q(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, uint32_t* r, csArena_q& ar);

/**
 * @brief Normalizes a signed integer used by Toom-Cook.
 * @param a First operand.
 */
CS_INLINE void csNormSigned_q(csSigned_q& a) {
  while (a.n > 1 && a.p[a.n - 1] == 0)
    --a.n;
  if (a.n == 0 || (a.n == 1 && a.p[0] == 0)) {
    a.n = 0;
    a.neg = 0;
  }
}

/**
 * @brief Reserves a signed-integer buffer.
 * @param ar Parameter @p ar.
 * @param cap Parameter @p cap.
 * @return Result.
 */
CS_INLINE csSigned_q csBufSigned_q(csArena_q& ar, size_t cap) {
  if (cap < 1)
    cap = 1;
  csSigned_q a;
  a.p = ar.take(cap);
  memset(a.p, 0, cap * sizeof(uint32_t));
  a.n = 0;
  a.cap = cap;
  a.neg = 0;
  return a;
}

/**
 * @brief Copies a signed integer.
 * @param d Parameter @p d.
 * @param s Parameter @p s.
 */
CS_INLINE void csCopyToSigned_q(csSigned_q& d, const csSigned_q& s) {
  if (!s.n) {
    d.n = 0;
    d.neg = 0;
    return;
  }
  memcpy(d.p, s.p, s.n * sizeof(uint32_t));
  d.n = s.n;
  d.neg = s.neg;
}

/**
 * @brief Adds two signed integers.
 * @param a First operand.
 * @param b Second operand.
 * @param r Remainder.
 */
CS_INLINE void csAddSigned_q(const csSigned_q& a, const csSigned_q& b, csSigned_q& r) {
  if (!a.n) {
    csCopyToSigned_q(r, b);
    return;
  }
  if (!b.n) {
    csCopyToSigned_q(r, a);
    return;
  }
  if (a.neg == b.neg) {
    size_t nr = 0;
    csAddRaw_q(a.p, a.n, b.p, b.n, r.p, nr);
    r.n = nr;
    r.neg = a.neg;
    csNormSigned_q(r);
    return;
  }
  int c = csCmpLimbs_q(a.p, a.n, b.p, b.n);
  if (c == 0) {
    r.n = 0;
    r.neg = 0;
    return;
  }
  const csSigned_q& hi = c > 0 ? a : b;
  const csSigned_q& lo = c > 0 ? b : a;
  memcpy(r.p, hi.p, hi.n * sizeof(uint32_t));
  size_t rn = hi.n;
  csSubRaw_q(r.p, rn, r.cap, lo.p, lo.n);
  r.n = rn;
  r.neg = hi.neg;
  csNormSigned_q(r);
}

/**
 * @brief Subtracts two signed integers.
 * @param a First operand.
 * @param b Second operand.
 * @param r Remainder.
 */
CS_INLINE void csSubSigned_q(const csSigned_q& a, csSigned_q b, csSigned_q& r) {
  if (b.n)
    b.neg = !b.neg;
  csAddSigned_q(a, b, r);
}

/**
 * @brief Shifts a signed integer to the left.
 * @param a First operand.
 * @param bits Number of bits.
 * @param r Remainder.
 */
CS_INLINE void csShlSigned_q(const csSigned_q& a, int bits, csSigned_q& r) {
  if (!a.n || bits <= 0) {
    csCopyToSigned_q(r, a);
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
  csNormSigned_q(r);
}

/**
 * @brief Loads an unsigned integer into a signed integer.
 * @param d Parameter @p d.
 * @param p Power or degree.
 * @param n Number of elements.
 */
CS_INLINE void csFromSigned_q(csSigned_q& d, const uint32_t* p, size_t n) {
  n = p ? csSigLen_q(p, n) : 0;
  if (!n) {
    d.n = 0;
    d.neg = 0;
    return;
  }
  memcpy(d.p, p, n * sizeof(uint32_t));
  d.n = n;
  d.neg = 0;
}

/**
 * @brief Computes the product of two signed integers.
 * @param a First operand.
 * @param b Second operand.
 * @param r Remainder.
 * @param ar Parameter @p ar.
 */
CS_INLINE void csMulSigned_q(const csSigned_q& a, const csSigned_q& b, csSigned_q& r, csArena_q& ar) {
  if (!a.n || !b.n) {
    r.n = 0;
    r.neg = 0;
    return;
  }
  memset(r.p, 0, (a.n + b.n) * sizeof(uint32_t));
  csMulRec_q(a.p, a.n, b.p, b.n, r.p, ar);
  r.n = csSigLen_q(r.p, a.n + b.n);
  r.neg = (a.neg != b.neg);
  if (!r.n)
    r.neg = 0;
}

/**
 * @brief Divides a signed integer by a small integer.
 * @param a First operand.
 * @param div Parameter @p div.
 */
CS_INLINE void csDivSmallSigned_q(csSigned_q& a, uint32_t div) {
  if (!a.n)
    return;
  uint64_t rem = 0;
  for (size_t i = a.n; i-- > 0; ) {
    uint64_t cur = (rem << 32) | a.p[i];
    a.p[i] = (uint32_t)(cur / div);
    rem = cur % div;
  }
  if (rem)
    csKaraOverflow_q = 1;
  csNormSigned_q(a);
}

/**
 * @brief Splits an integer into two parts.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param off Parameter @p off.
 * @param len Parameter @p len.
 * @param p Power or degree.
 * @param n Number of elements.
 */
CS_INLINE void csPart_q(const uint32_t* a, size_t na, size_t off, size_t len, const uint32_t*& p, size_t& n) {
  if (!a || off >= na || len == 0) {
    p = 0;
    n = 0;
    return;
  }
  size_t m = na - off;
  if (m > len)
    m = len;
  p = a + off;
  n = csSigLen_q(p, m);
}

/**
 * @brief Adds a signed integer at a limb offset.
 * @param dst Destination of the operation.
 * @param dstLen Parameter @p dstLen.
 * @param c Parameter @p c.
 * @param shift Shift in limbs.
 */
CS_INLINE void csAddShiftSigned_q(uint32_t* dst, size_t dstLen, const csSigned_q& c, size_t shift) {
  if (!c.n)
    return;
  if (!c.neg) {
    csAddInto_q(dst, dstLen, c.p, c.n, shift);
    return;
  }
  int borrow = 0;
  size_t i = 0;
  while (i < c.n || borrow) {
    if (shift + i >= dstLen) {
      csKaraOverflow_q = 1;
      return;
    }
    int64_t cur = (int64_t)dst[shift + i] - (i < c.n ? (int64_t)c.p[i] : 0) - borrow;
    if (cur < 0) {
      cur += CS_BASE_Q;
      borrow = 1;
    } else {
      borrow = 0;
    }
    dst[shift + i] = (uint32_t)cur;
    ++i;
  }
}

/**
 * @brief Computes a three-way Toom-Cook product.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param b Second operand.
 * @param nb Number of elements.
 * @param r Limb array.
 * @param ar Parameter @p ar.
 */
CS_INLINE void csToom3_q(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, uint32_t* r, csArena_q& ar) {
  size_t n = na > nb ? na : nb;
  size_t s = (n + 2) / 3;
  if (s < 1)
    s = 1;
  const uint32_t *a0p, *a1p, *a2p, *b0p, *b1p, *b2p;
  size_t a0n, a1n, a2n, b0n, b1n, b2n;
  csPart_q(a, na, 0, s, a0p, a0n);
  csPart_q(a, na, s, s, a1p, a1n);
  csPart_q(a, na, 2 * s, na, a2p, a2n);
  csPart_q(b, nb, 0, s, b0p, b0n);
  csPart_q(b, nb, s, s, b1p, b1n);
  csPart_q(b, nb, 2 * s, nb, b2p, b2n);

  size_t saved = ar.mark();
  size_t pc = s + 4;
  csSigned_q A0 = csBufSigned_q(ar, pc), A1 = csBufSigned_q(ar, pc), A2 = csBufSigned_q(ar, pc);
  csSigned_q B0 = csBufSigned_q(ar, pc), B1 = csBufSigned_q(ar, pc), B2 = csBufSigned_q(ar, pc);
  csFromSigned_q(A0, a0p, a0n);
  csFromSigned_q(A1, a1p, a1n);
  csFromSigned_q(A2, a2p, a2n);
  csFromSigned_q(B0, b0p, b0n);
  csFromSigned_q(B1, b1p, b1n);
  csFromSigned_q(B2, b2p, b2n);

  csSigned_q t1 = csBufSigned_q(ar, pc), t2 = csBufSigned_q(ar, pc), accP = csBufSigned_q(ar, pc);
  csSigned_q Ap = csBufSigned_q(ar, pc), Am = csBufSigned_q(ar, pc), A2e = csBufSigned_q(ar, pc);
  csSigned_q Bp = csBufSigned_q(ar, pc), Bm = csBufSigned_q(ar, pc), B2e = csBufSigned_q(ar, pc);

  csAddSigned_q(A0, A1, t1);
  csAddSigned_q(t1, A2, Ap);
  csSubSigned_q(A0, A1, t1);
  csAddSigned_q(t1, A2, Am);
  csShlSigned_q(A1, 1, t1);
  csShlSigned_q(A2, 2, t2);
  csAddSigned_q(A0, t1, accP);
  csAddSigned_q(accP, t2, A2e);

  csAddSigned_q(B0, B1, t1);
  csAddSigned_q(t1, B2, Bp);
  csSubSigned_q(B0, B1, t1);
  csAddSigned_q(t1, B2, Bm);
  csShlSigned_q(B1, 1, t1);
  csShlSigned_q(B2, 2, t2);
  csAddSigned_q(B0, t1, accP);
  csAddSigned_q(accP, t2, B2e);

  size_t mc = pc * 2 + 2;
  csSigned_q P0 = csBufSigned_q(ar, mc), P1 = csBufSigned_q(ar, mc), Pm = csBufSigned_q(ar, mc), P2 = csBufSigned_q(ar, mc), Pi = csBufSigned_q(ar, mc);
  csMulSigned_q(A0, B0, P0, ar);
  csMulSigned_q(Ap, Bp, P1, ar);
  csMulSigned_q(Am, Bm, Pm, ar);
  csMulSigned_q(A2e, B2e, P2, ar);
  csMulSigned_q(A2, B2, Pi, ar);

  csSigned_q c0 = csBufSigned_q(ar, mc), c1 = csBufSigned_q(ar, mc), c2 = csBufSigned_q(ar, mc), c3 = csBufSigned_q(ar, mc), c4 = csBufSigned_q(ar, mc);
  csSigned_q U = csBufSigned_q(ar, mc), V = csBufSigned_q(ar, mc), W = csBufSigned_q(ar, mc), X = csBufSigned_q(ar, mc), Y = csBufSigned_q(ar, mc);
  csCopyToSigned_q(c0, P0);
  csCopyToSigned_q(c4, Pi);

  csAddSigned_q(P1, Pm, U);
  csDivSmallSigned_q(U, 2);
  csSubSigned_q(U, c0, V);
  csSubSigned_q(V, c4, c2);

  csSubSigned_q(P1, Pm, U);
  csDivSmallSigned_q(U, 2);

  csShlSigned_q(c2, 2, V);
  csShlSigned_q(c4, 4, W);
  csSubSigned_q(P2, c0, X);
  csSubSigned_q(X, V, Y);
  csSubSigned_q(Y, W, X);
  csDivSmallSigned_q(X, 2);

  csSubSigned_q(X, U, c3);
  csDivSmallSigned_q(c3, 3);
  csSubSigned_q(U, c3, c1);

  size_t big = 6 * s + 8;
  if (big < na + nb)
    big = na + nb;
  uint32_t* acc = ar.take(big);
  memset(acc, 0, big * sizeof(uint32_t));
  csAddShiftSigned_q(acc, big, c0, 0);
  csAddShiftSigned_q(acc, big, c1, s);
  csAddShiftSigned_q(acc, big, c2, 2 * s);
  csAddShiftSigned_q(acc, big, c3, 3 * s);
  csAddShiftSigned_q(acc, big, c4, 4 * s);
  for (size_t i = na + nb; i < big; ++i) {
    if (acc[i])
      csKaraOverflow_q = 1;
  }
  memcpy(r, acc, (na + nb) * sizeof(uint32_t));
  ar.rewind(saved);
}

#include "csFft_q.inl"

/**
 * @brief Computes a recursive product: schoolbook, Karatsuba, Toom-3 or Fourier.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param b Second operand.
 * @param nb Number of elements.
 * @param r Limb array.
 * @param ar Parameter @p ar.
 */
CS_INLINE void csMulRec_q(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, uint32_t* r, csArena_q& ar) {
  if (na == 0 || nb == 0) {
    if (na + nb)
      memset(r, 0, (na + nb) * sizeof(uint32_t));
    return;
  }
  if (na < (size_t)csKaraCutoff_q || nb < (size_t)csKaraCutoff_q) {
    csMulSchoolRaw_q(a, na, b, nb, r);
    return;
  }
  if (na >= (size_t)csFftCutoff_q && nb >= (size_t)csFftCutoff_q && csFftFits_q(na, nb)) {
    csFftMul_q(a, na, b, nb, r);
    return;
  }
  if (na >= (size_t)csToomCutoff_q && nb >= (size_t)csToomCutoff_q) {
    csToom3_q(a, na, b, nb, r, ar);
    return;
  }
  size_t n = na > nb ? na : nb;
  size_t k = n >> 1;
  size_t a0n = csSigLen_q(a, na < k ? na : k);
  size_t b0n = csSigLen_q(b, nb < k ? nb : k);
  size_t a1n = na > k ? csSigLen_q(a + k, na - k) : 0;
  size_t b1n = nb > k ? csSigLen_q(b + k, nb - k) : 0;
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
    csMulRec_q(a, a0n, b, b0n, z0, ar);
    if (z0[z0cap])
      csKaraOverflow_q = 1;
    z0n = csSigLen_q(z0, z0cap);
  }

  uint32_t* z2 = 0;
  size_t z2n = 0;
  size_t z2cap = 0;
  if (a1n && b1n) {
    z2cap = a1n + b1n;
    z2 = ar.take(z2cap + 1);
    z2[z2cap] = 0;
    csMulRec_q(a1, a1n, b1, b1n, z2, ar);
    if (z2[z2cap])
      csKaraOverflow_q = 1;
    z2n = csSigLen_q(z2, z2cap);
  }

  uint32_t* sa = ar.take(k + 2);
  uint32_t* sb = ar.take(k + 2);
  size_t san = 0, sbn = 0;
  csAddRaw_q(a, a0n, a1n ? a1 : 0, a1n, sa, san);
  csAddRaw_q(b, b0n, b1n ? b1 : 0, b1n, sb, sbn);

  uint32_t* z1 = 0;
  size_t z1n = 0;
  size_t z1cap = 0;
  if (san && sbn) {
    z1cap = san + sbn;
    z1 = ar.take(z1cap + 1);
    z1[z1cap] = 0;
    csMulRec_q(sa, san, sb, sbn, z1, ar);
    if (z1[z1cap])
      csKaraOverflow_q = 1;
    z1n = csSigLen_q(z1, z1cap);
    if (z0n)
      csSubRaw_q(z1, z1n, z1cap, z0, z0n);
    if (z2n)
      csSubRaw_q(z1, z1n, z1cap, z2, z2n);
  }

  memset(r, 0, (na + nb) * sizeof(uint32_t));
  if (z0n)
    csAddInto_q(r, na + nb, z0, z0n, 0);
  if (z1n)
    csAddInto_q(r, na + nb, z1, z1n, k);
  if (z2n)
    csAddInto_q(r, na + nb, z2, z2n, k * 2);
  ar.rewind(saved);
}

/**
 * @brief Computes the product of two integers.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param b Second operand.
 * @param nb Number of elements.
 * @param r Integer in limbs.
 */
CS_INLINE void csMul_q(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, csLimbs_q& r) {
  if (!a || !b || na == 0 || nb == 0 || (na == 1 && a[0] == 0) || (nb == 1 && b[0] == 0)) {
    r.setZero();
    return;
  }
  r.reserve(na + nb + 1);
  r.p[na + nb] = 0;
  if (na >= (size_t)csFftCutoff_q && nb >= (size_t)csFftCutoff_q && csFftFits_q(na, nb))
    csFftMul_q(a, na, b, nb, r.p);
  else if (na < (size_t)csKaraCutoff_q || nb < (size_t)csKaraCutoff_q)
    csMulSchoolRaw_q(a, na, b, nb, r.p);
  else {
    
    csArena_q ar((na + nb) * 32 + 512);
    csMulRec_q(a, na, b, nb, r.p, ar);
  }
  if (r.p[na + nb] != 0)
    csKaraOverflow_q = 1;
  r.n = na + nb;
  r.trim();
}

/**
 * @brief Multiplies an integer by a single limb.
 * @param a First operand.
 * @param n Number of elements.
 * @param m Number of evaluation points.
 * @param dst Destination of the operation.
 * @param keep Parameter @p keep.
 */
CS_INLINE void csMulSmall_q(const uint32_t* a, size_t n, uint32_t m, uint32_t* dst, size_t keep) {
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

/**
 * @brief Computes the quotient and the remainder by Knuth division.
 * @param u0 Limb array.
 * @param un Parameter @p un.
 * @param v0 Limb array.
 * @param vn Parameter @p vn.
 * @param q Integer in limbs.
 * @param r Integer in limbs.
 */
CS_INLINE void csDivModKnuth_q(const uint32_t* u0, size_t un, const uint32_t* v0, size_t vn, csLimbs_q& q, csLimbs_q& r) {
  if (!v0 || vn == 0 || (vn == 1 && v0[0] == 0)) {
    q.setZero();
    r.setZero();
    return;
  }
  while (un > 1 && u0[un - 1] == 0)
    --un;
  while (vn > 1 && v0[vn - 1] == 0)
    --vn;
  if (csCmpLimbs_q(u0, un, v0, vn) < 0) {
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
  uint32_t d = (uint32_t)((uint64_t)CS_BASE_Q / ((uint64_t)v0[vn - 1] + 1));
  csLimbs_q u, v;
  u.reserve(un + 1);
  csMulSmall_q(u0, un, d, u.p, un + 1);
  u.n = un + 1;
  v.reserve(vn);
  csMulSmall_q(v0, vn, d, v.p, vn);
  v.n = vn;
  size_t qn = un - vn + 1;
  q.reserve(qn);
  memset(q.p, 0, qn * sizeof(uint32_t));
  q.n = qn;
  for (int j = (int)un - (int)vn; j >= 0; --j) {
    uint64_t num = ((uint64_t)u.p[j + vn] << 32) + u.p[j + vn - 1];
    uint64_t qhat, rhat;
    if (u.p[j + vn] >= v.p[vn - 1]) {
      qhat = CS_BASE_Q - 1;
      rhat = num - qhat * v.p[vn - 1];
    } else {
      qhat = num / v.p[vn - 1];
      rhat = num % v.p[vn - 1];
    }
    while (qhat >= (uint64_t)CS_BASE_Q || (rhat < (uint64_t)CS_BASE_Q && qhat * (uint64_t)v.p[vn - 2] > (rhat << 32) + u.p[j + vn - 2])) {
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
        cur += CS_BASE_Q;
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
      last += (int64_t)CS_BASE_Q + (int64_t)c;
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

static int csDivCutoff_q = 48;
static thread_local int csDivOverflow_q = 0;
static thread_local int csDivFallback_q = 0;

/**
 * @brief Compares an integer with another shifted by limbs.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param b Second operand.
 * @param nb Number of elements.
 * @param shift Shift in limbs.
 * @return Computed value.
 */
CS_INLINE int csCmpShifted_q(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, size_t shift) {
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


/**
 * @brief Subtracts a shifted integer.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param b Second operand.
 * @param nb Number of elements.
 * @param shift Shift in limbs.
 * @param dst Destination of the operation.
 * @param nd Parameter @p nd.
 */
CS_INLINE void csSubShifted_q(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, size_t shift, uint32_t* dst, size_t& nd) {
  int borrow = 0;
  size_t m = na > shift + nb ? na : shift + nb;
  for (size_t i = 0; i < m; ++i) {
    int64_t cur = (i < na ? (int64_t)a[i] : 0) - (i >= shift && i - shift < nb ? (int64_t)b[i - shift] : 0) - borrow;
    if (cur < 0) {
      cur += CS_BASE_Q;
      borrow = 1;
    } else {
      borrow = 0;
    }
    dst[i] = (uint32_t)cur;
  }
  if (borrow)
    csDivOverflow_q = 1;
  nd = csSigLen_q(dst, m);
}

/**
 * @brief Subtracts an integer from a shifted integer.
 * @param b Second operand.
 * @param nb Number of elements.
 * @param shift Shift in limbs.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param dst Destination of the operation.
 * @param nd Parameter @p nd.
 */
CS_INLINE void csSubFromShifted_q(const uint32_t* b, size_t nb, size_t shift, const uint32_t* a, size_t na, uint32_t* dst, size_t& nd) {
  int borrow = 0;
  size_t m = shift + nb;
  if (na > m)
    m = na;
  for (size_t i = 0; i < m; ++i) {
    int64_t bv = (i >= shift && i - shift < nb) ? (int64_t)b[i - shift] : 0;
    int64_t cur = bv - (i < na ? (int64_t)a[i] : 0) - borrow;
    if (cur < 0) {
      cur += CS_BASE_Q;
      borrow = 1;
    } else {
      borrow = 0;
    }
    dst[i] = (uint32_t)cur;
  }
  if (borrow)
    csDivOverflow_q = 1;
  nd = csSigLen_q(dst, m);
}

/**
 * @brief Decrements an integer by one.
 * @param a First operand.
 * @param n Number of elements.
 */
CS_INLINE void csDecLimb_q(uint32_t* a, size_t& n) {
  if (n == 0) {
    csDivOverflow_q = 1;
    return;
  }
  size_t i = 0;
  while (i < n && a[i] == 0) {
    a[i] = (uint32_t)(CS_BASE_Q - 1);
    ++i;
  }
  if (i >= n) {
    csDivOverflow_q = 1;
    return;
  }
  --a[i];
  n = csSigLen_q(a, n);
}

/**
 * @brief Computes a Knuth division into the supplied buffers.
 * @param u Limb array.
 * @param un Parameter @p un.
 * @param v Limb array.
 * @param vn Parameter @p vn.
 * @param Q Limb array.
 * @param qcap Parameter @p qcap.
 * @param qn Parameter @p qn.
 * @param R Limb array.
 * @param rcap Parameter @p rcap.
 * @param rn Parameter @p rn.
 */
CS_INLINE void csDivKnuthInto_q(const uint32_t* u, size_t un, const uint32_t* v, size_t vn, uint32_t* Q, size_t qcap, size_t& qn, uint32_t* R, size_t rcap, size_t& rn) {
  csLimbs_q qq, rr;
  csDivModKnuth_q(u, un, v, vn, qq, rr);
  if (qq.n > qcap || rr.n > rcap) {
    csDivOverflow_q = 1;
    qn = 0;
    rn = 0;
    return;
  }
  qn = csSigLen_q(qq.p, qq.n);
  rn = csSigLen_q(rr.p, rr.n);
  if (qn)
    memcpy(Q, qq.p, qn * sizeof(uint32_t));
  if (rn)
    memcpy(R, rr.p, rn * sizeof(uint32_t));
}

/**
 * @brief Computes a recursive division, quotient and remainder.
 * @param A Limb array.
 * @param aLen Parameter @p aLen.
 * @param B Limb array.
 * @param bLen Parameter @p bLen.
 * @param Q Limb array.
 * @param qcap Parameter @p qcap.
 * @param qn Parameter @p qn.
 * @param R Limb array.
 * @param rcap Parameter @p rcap.
 * @param rn Parameter @p rn.
 * @param ar Parameter @p ar.
 */
CS_INLINE void csDivRec_q(const uint32_t* A, size_t aLen, const uint32_t* B, size_t bLen, uint32_t* Q, size_t qcap, size_t& qn, uint32_t* R, size_t rcap, size_t& rn, csArena_q& ar);

/**
 * @brief Divides a dividend much longer than the divisor.
 * @param A Limb array.
 * @param aLen Parameter @p aLen.
 * @param B Limb array.
 * @param n Number of elements.
 * @param Q Limb array.
 * @param qcap Parameter @p qcap.
 * @param qn Parameter @p qn.
 * @param R Limb array.
 * @param rcap Parameter @p rcap.
 * @param rn Parameter @p rn.
 * @param ar Parameter @p ar.
 */
CS_INLINE void csDivUnbal_q(const uint32_t* A, size_t aLen, const uint32_t* B, size_t n, uint32_t* Q, size_t qcap, size_t& qn, uint32_t* R, size_t rcap, size_t& rn, csArena_q& ar) {
  size_t m0 = aLen - n;
  size_t m = m0;
  size_t saved = ar.mark();
  uint32_t* cur = ar.take(aLen + 2);
  memset(cur, 0, (aLen + 2) * sizeof(uint32_t));
  memcpy(cur, A, aLen * sizeof(uint32_t));
  size_t cn = aLen;
  size_t csAccCap_q = m0 + n + 8;
  uint32_t* Qacc = ar.take(csAccCap_q);
  memset(Qacc, 0, csAccCap_q * sizeof(uint32_t));
  size_t csAccN_q = 0;
  while (m > n && !csDivOverflow_q) {
    size_t drop = m - n;
    size_t topN = cn > drop ? csSigLen_q(cur + drop, cn - drop) : 0;
    size_t step = ar.mark();
    size_t qcap2 = n + 4;
    size_t rcap2 = n + 2;
    uint32_t* q = ar.take(qcap2);
    uint32_t* r = ar.take(rcap2);
    size_t qq = 0, rr = 0;
    csDivRec_q(topN ? cur + drop : cur, topN, B, n, q, qcap2, qq, r, rcap2, rr, ar);
    if (csAccN_q) {
      if (csAccN_q + n > csAccCap_q) {
        csDivOverflow_q = 1;
        break;
      }
      memmove(Qacc + n, Qacc, csAccN_q * sizeof(uint32_t));
      memset(Qacc, 0, n * sizeof(uint32_t));
      csAccN_q += n;
    }
    if (qq)
      csAddInto_q(Qacc, csAccCap_q, q, qq, 0);
    csAccN_q = csSigLen_q(Qacc, csAccCap_q);
    uint32_t* nxt = ar.take(drop + n + 2);
    memset(nxt, 0, (drop + n + 2) * sizeof(uint32_t));
    size_t lowN = cn < drop ? cn : drop;
    if (lowN)
      memcpy(nxt, cur, lowN * sizeof(uint32_t));
    if (rr)
      memcpy(nxt + drop, r, rr * sizeof(uint32_t));
    size_t nn = csSigLen_q(nxt, drop + rr);
    memset(cur, 0, (aLen + 2) * sizeof(uint32_t));
    if (nn)
      memcpy(cur, nxt, nn * sizeof(uint32_t));
    cn = nn;
    ar.rewind(step);
    m -= n;
  }
  if (!csDivOverflow_q) {
    size_t qcap2 = m + 4;
    size_t rcap2 = n + 2;
    uint32_t* q = ar.take(qcap2);
    uint32_t* r = ar.take(rcap2);
    size_t qq = 0, rr = 0;
    csDivRec_q(cur, csSigLen_q(cur, cn), B, n, q, qcap2, qq, r, rcap2, rr, ar);
    if (csAccN_q && m) {
      if (csAccN_q + m > csAccCap_q)
        csDivOverflow_q = 1;
      else {
        memmove(Qacc + m, Qacc, csAccN_q * sizeof(uint32_t));
        memset(Qacc, 0, m * sizeof(uint32_t));
        csAccN_q += m;
      }
    }
    if (qq && !csDivOverflow_q)
      csAddInto_q(Qacc, csAccCap_q, q, qq, 0);
    csAccN_q = csSigLen_q(Qacc, csAccCap_q);
    if (csAccN_q > qcap || rr > rcap)
      csDivOverflow_q = 1;
    else {
      qn = csAccN_q;
      if (qn)
        memcpy(Q, Qacc, qn * sizeof(uint32_t));
      rn = rr;
      if (rn)
        memcpy(R, r, rn * sizeof(uint32_t));
    }
  }
  ar.rewind(saved);
}

/**
 * @brief Computes a recursive division, quotient and remainder.
 * @param A Limb array.
 * @param aLen Parameter @p aLen.
 * @param B Limb array.
 * @param bLen Parameter @p bLen.
 * @param Q Limb array.
 * @param qcap Parameter @p qcap.
 * @param qn Parameter @p qn.
 * @param R Limb array.
 * @param rcap Parameter @p rcap.
 * @param rn Parameter @p rn.
 * @param ar Parameter @p ar.
 */
CS_INLINE void csDivRec_q(const uint32_t* A, size_t aLen, const uint32_t* B, size_t bLen, uint32_t* Q, size_t qcap, size_t& qn, uint32_t* R, size_t rcap, size_t& rn, csArena_q& ar) {
  size_t nA = csSigLen_q(A, aLen);
  size_t n = csSigLen_q(B, bLen);
  qn = 0;
  rn = 0;
  if (n == 0 || (n == 1 && B[0] == 0)) {
    csDivOverflow_q = 1;
    return;
  }
  if (nA < n) {
    if (nA > rcap) {
      csDivOverflow_q = 1;
      return;
    }
    rn = nA;
    if (rn)
      memcpy(R, A, rn * sizeof(uint32_t));
    return;
  }
  size_t m = nA - n;
  if (n < (size_t)csDivCutoff_q || m + 1 < (size_t)csDivCutoff_q || m < 2) {
    csDivKnuthInto_q(A, nA, B, n, Q, qcap, qn, R, rcap, rn);
    return;
  }
  if (m > n) {
    csDivUnbal_q(A, nA, B, n, Q, qcap, qn, R, rcap, rn, ar);
    return;
  }
  size_t k = m >> 1;
  const uint32_t* B1 = B + k;
  size_t nB1 = n - k;
  size_t nB0 = csSigLen_q(B, k);
  size_t saved = ar.mark();
  size_t shift2 = k * 2;
  size_t nAhi = nA > shift2 ? nA - shift2 : 0;
  const uint32_t* Ahi = nAhi ? A + shift2 : A;
  size_t q1cap = (m - k) + 4;
  size_t r1cap = nB1 + 2;
  uint32_t* Q1 = ar.take(q1cap);
  uint32_t* R1 = ar.take(r1cap);
  size_t q1n = 0, r1n = 0;
  csDivRec_q(Ahi, nAhi, B1, nB1, Q1, q1cap, q1n, R1, r1cap, r1n, ar);
  size_t lowN = nA < shift2 ? nA : shift2;
  size_t concCap = shift2 + r1n + 2;
  uint32_t* conc = ar.take(concCap);
  memset(conc, 0, concCap * sizeof(uint32_t));
  if (lowN)
    memcpy(conc, A, lowN * sizeof(uint32_t));
  if (r1n)
    memcpy(conc + shift2, R1, r1n * sizeof(uint32_t));
  size_t concN = csSigLen_q(conc, shift2 + r1n);
  csLimbs_q prod;
  if (q1n && nB0)
    csMul_q(Q1, q1n, B, nB0, prod);
  else
    prod.setZero();
  size_t prodN = (prod.n == 1 && prod.p[0] == 0) ? 0 : prod.n;
  size_t apCap = concCap + prodN + k + 4;
  uint32_t* Ap = ar.take(apCap);
  size_t apN = 0;
  int neg = 0;
  if (csCmpShifted_q(conc, concN, prod.p, prodN, k) >= 0)
    csSubShifted_q(conc, concN, prod.p, prodN, k, Ap, apN);
  else {
    csSubFromShifted_q(prod.p, prodN, k, conc, concN, Ap, apN);
    neg = 1;
  }
  int fixes = 0;
  while (neg && !csDivOverflow_q) {
    if (++fixes > 6) {
      csDivOverflow_q = 1;
      break;
    }
    csDecLimb_q(Q1, q1n);
    uint32_t* tmp = ar.take(apCap + n + k + 2);
    if (csCmpShifted_q(Ap, apN, B, n, k) <= 0) {
      csSubFromShifted_q(B, n, k, Ap, apN, tmp, apN);
      if (apN)
        memcpy(Ap, tmp, apN * sizeof(uint32_t));
      neg = 0;
    } else {
      size_t nn = 0;
      csSubShifted_q(Ap, apN, B, n, k, tmp, nn);
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
  if (!csDivOverflow_q)
    csDivRec_q(Amid, nMid, B1, nB1, Q0, q0cap, q0n, R0, r0cap, r0n, ar);
  size_t low2 = apN < k ? apN : k;
  size_t conc2cap = k + r0n + 2;
  uint32_t* conc2 = ar.take(conc2cap);
  memset(conc2, 0, conc2cap * sizeof(uint32_t));
  if (low2)
    memcpy(conc2, Ap, low2 * sizeof(uint32_t));
  if (r0n)
    memcpy(conc2 + k, R0, r0n * sizeof(uint32_t));
  size_t conc2N = csSigLen_q(conc2, k + r0n);
  csLimbs_q prod0;
  if (q0n && nB0)
    csMul_q(Q0, q0n, B, nB0, prod0);
  else
    prod0.setZero();
  size_t p0n = (prod0.n == 1 && prod0.p[0] == 0) ? 0 : prod0.n;
  uint32_t* App = ar.take(conc2cap + p0n + 4);
  size_t appN = 0;
  int neg2 = 0;
  if (csCmpShifted_q(conc2, conc2N, prod0.p, p0n, 0) >= 0)
    csSubShifted_q(conc2, conc2N, prod0.p, p0n, 0, App, appN);
  else {
    csSubFromShifted_q(prod0.p, p0n, 0, conc2, conc2N, App, appN);
    neg2 = 1;
  }
  fixes = 0;
  while (neg2 && !csDivOverflow_q) {
    if (++fixes > 6) {
      csDivOverflow_q = 1;
      break;
    }
    csDecLimb_q(Q0, q0n);
    uint32_t* tmp = ar.take(appN + n + 4);
    if (csCmpShifted_q(App, appN, B, n, 0) <= 0) {
      csSubFromShifted_q(B, n, 0, App, appN, tmp, appN);
      if (appN)
        memcpy(App, tmp, appN * sizeof(uint32_t));
      neg2 = 0;
    } else {
      size_t nn = 0;
      csSubShifted_q(App, appN, B, n, 0, tmp, nn);
      if (nn)
        memcpy(App, tmp, nn * sizeof(uint32_t));
      appN = nn;
    }
  }
  if (csDivOverflow_q) {
    ar.rewind(saved);
    return;
  }
  if (k + q1n + 2 > qcap || appN > rcap) {
    csDivOverflow_q = 1;
    ar.rewind(saved);
    return;
  }
  memset(Q, 0, qcap * sizeof(uint32_t));
  if (q0n)
    csAddInto_q(Q, qcap, Q0, q0n, 0);
  if (q1n)
    csAddInto_q(Q, qcap, Q1, q1n, k);
  qn = csSigLen_q(Q, qcap);
  rn = appN;
  if (rn)
    memcpy(R, App, rn * sizeof(uint32_t));
  if (rn && csCmpLimbs_q(R, rn, B, n) >= 0)
    csDivOverflow_q = 1;
  ar.rewind(saved);
}

#include "csNewton_q.inl"

/**
 * @brief Computes the quotient and the remainder of two integers.
 * @param u0 Limb array.
 * @param un Parameter @p un.
 * @param v0 Limb array.
 * @param vn Parameter @p vn.
 * @param q Integer in limbs.
 * @param r Integer in limbs.
 */
CS_INLINE void csDivMod_q(const uint32_t* u0, size_t un, const uint32_t* v0, size_t vn, csLimbs_q& q, csLimbs_q& r) {
  if (!v0 || vn == 0 || (vn == 1 && v0[0] == 0)) {
    q.setZero();
    r.setZero();
    return;
  }
  while (un > 1 && u0[un - 1] == 0)
    --un;
  while (vn > 1 && v0[vn - 1] == 0)
    --vn;
  if (csCmpLimbs_q(u0, un, v0, vn) < 0) {
    q.setZero();
    r.copyFrom(u0, un);
    return;
  }
  if (vn >= (size_t)csNewtonCutoff_q && un > vn && (un - vn) >= (size_t)csNewtonCutoff_q) {
    if (csDivNewton_q(u0, un, v0, vn, q, r))
      return;
    ++csDivFallback_q;
    csDivModKnuth_q(u0, un, v0, vn, q, r);
    return;
  }
  if (vn == 1 || vn < (size_t)csDivCutoff_q || un - vn < (size_t)csDivCutoff_q) {
    csDivModKnuth_q(u0, un, v0, vn, q, r);
    return;
  }
  uint32_t d = (uint32_t)((uint64_t)CS_BASE_Q / ((uint64_t)v0[vn - 1] + 1));
  csLimbs_q u, v;
  u.reserve(un + 1);
  csMulSmall_q(u0, un, d, u.p, un + 1);
  u.n = csSigLen_q(u.p, un + 1);
  v.reserve(vn + 1);
  csMulSmall_q(v0, vn, d, v.p, vn);
  v.n = csSigLen_q(v.p, vn);
  size_t qcap = u.n - v.n + 4;
  size_t rcap = v.n + 2;
  int prevKara = csKaraOverflow_q;
  csKaraOverflow_q = 0;
  csDivOverflow_q = 0;
  int ok = 0;
  {
    
    csArena_q ar((u.n + v.n) * 64 + 256);
    uint32_t* Qb = ar.take(qcap);
    uint32_t* Rb = ar.take(rcap);
    size_t qn = 0, rn = 0;
    csDivRec_q(u.p, u.n, v.p, v.n, Qb, qcap, qn, Rb, rcap, rn, ar);
    if (csKaraOverflow_q)
      csDivOverflow_q = 1;
    if (!csDivOverflow_q) {
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
        csDivOverflow_q = 1;
      else
        ok = 1;
    }
  }
  csKaraOverflow_q = prevKara;
  if (!ok) {
    ++csDivFallback_q;
    csDivOverflow_q = 0;
    csDivModKnuth_q(u0, un, v0, vn, q, r);
  }
}

/**
 * @brief Computes the greatest common divisor of two integers.
 * @param a0 Original text of the first operand.
 * @param na Number of limbs of the first operand.
 * @param b0 Original text of the second operand.
 * @param nb Number of elements.
 * @param g Integer in limbs.
 */
CS_INLINE void csGcd_q(const uint32_t* a0, size_t na, const uint32_t* b0, size_t nb, csLimbs_q& g) {
  if ((na == 1 && a0 && a0[0] == 1) || (nb == 1 && b0 && b0[0] == 1)) {
    g.setOne();
    return;
  }
  csLimbs_q a, b, q, r;
  a.copyFrom(a0, na);
  b.copyFrom(b0, nb);
  while (!b.isZero()) {
    csDivMod_q(a.p, a.n, b.p, b.n, q, r);
    a.copy(b);
    b.copy(r);
  }
  g.copy(a);
}

/**
 * @brief Reduces a fraction by the greatest common divisor.
 * @param num Integer in limbs.
 * @param den Integer in limbs.
 */
CS_INLINE void csNormalize_q(csLimbs_q& num, csLimbs_q& den) {
  if (num.isZero()) {
    den.setOne();
    return;
  }
  if (num.isOne() || den.isOne())
    return;
  csLimbs_q g, q, r;
  csGcd_q(num.p, num.n, den.p, den.n, g);
  if (g.isOne())
    return;
  csDivMod_q(num.p, num.n, g.p, g.n, q, r);
  num.copy(q);
  csDivMod_q(den.p, den.n, g.p, g.n, q, r);
  den.copy(q);
}

/**
 * @brief Converts a decimal text to limbs.
 * @param s Parameter @p s.
 * @param out Parameter @p out.
 * @param n Number of elements.
 */
CS_INLINE void csFromDec_q(const char* s, char*& out, size_t& n) {
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

/**
 * @brief Copies a block of limbs.
 * @param s Parameter @p s.
 * @param n Number of elements.
 * @return Resulting text. The caller frees the memory.
 */
CS_INLINE char* csDupLimbs_q(const char* s, size_t n) {
  if (!s || n == 0) {
    uint32_t* p = (uint32_t*)malloc(sizeof(uint32_t));
    p[0] = 0;
    return (char*)p;
  }
  uint32_t* p = (uint32_t*)malloc(n * sizeof(uint32_t));
  memcpy(p, s, n * sizeof(uint32_t));
  return (char*)p;
}

/**
 * @brief Creates the limb of value 1.
 * @return Resulting text. The caller frees the memory.
 */
CS_INLINE char* csLimbOne_q() {
  uint32_t* p = (uint32_t*)malloc(sizeof(uint32_t));
  p[0] = 1;
  return (char*)p;
}

/**
 * @brief Returns the absolute value of a long integer.
 * @param v Parameter @p v.
 * @return Computed value.
 */
CS_INLINE uint64_t csAbsLong_q(long v) {
  if (v >= 0)
    return (uint64_t)v;
  return (uint64_t)0 - (uint64_t)v;
}

/**
 * @brief Reads a block of limbs back from its memory.
 * @param p Power or degree.
 * @return Computed value.
 */
CS_INLINE const uint32_t* csPtr_q(const char* p) { return (const uint32_t*)p; }

/**
 * @brief Converts an integer to a double, approximately.
 * @param raw Parameter @p raw.
 * @param n Number of elements.
 * @return Approximation as a double.
 */
CS_INLINE double csLimbsToDouble_q(const char* raw, size_t n) {
  if (!raw || n == 0)
    return 0;
  const uint32_t* p = (const uint32_t*)raw;
  double v = 0;
  for (size_t i = n; i-- > 0; )
    v = v * (double)CS_BASE_Q + (double)p[i];
  return v;
}

#ifndef CS_TEST_Q
/**
 * @brief Reports whether a numerator is zero.
 * @param p Power or degree.
 * @param n Number of elements.
 * @return True when the condition holds.
 */
CS_INLINE bool csRawZero_q(const char* p, size_t n) {
  if (!p || n == 0)
    return true;
  const uint32_t* a = (const uint32_t*)p;
  return n == 1 && a[0] == 0;
}

/**
 * @brief Compares two rationals by absolute value.
 * @param a First operand.
 * @param b Second operand.
 * @return Computed value.
 */
CS_INLINE int csCmpAbs_q(const CSARITHMETIC::csRational& a, const CSARITHMETIC::csRational& b) {
  csLimbs_q n1, n2;
  csMul_q(csPtr_q(a.numerator), a.numSize, csPtr_q(b.denominator), b.denomSize, n1);
  csMul_q(csPtr_q(a.denominator), a.denomSize, csPtr_q(b.numerator), b.numSize, n2);
  
  return csCmpLimbs_q(n1.p, n1.n, n2.p, n2.n);
}

/**
 * @brief Compares two signed rationals.
 * @param a First operand.
 * @param b Second operand.
 * @return Computed value.
 */
CS_INLINE int csCmpSigned_q(const CSARITHMETIC::csRational& a, const CSARITHMETIC::csRational& b) {
  bool az = csRawZero_q(a.numerator, a.numSize);
  bool bz = csRawZero_q(b.numerator, b.numSize);
  if (az && bz)
    return 0;
  if (az)
    return b.sign ? 1 : -1;
  if (bz)
    return a.sign ? -1 : 1;
  if (a.sign != b.sign)
    return a.sign ? -1 : 1;
  int c = csCmpAbs_q(a, b);
  return a.sign ? -c : c;
}

/**
 * @brief Cuts a decimal string down to the requested number of digits.
 * @param s Parameter @p s.
 * @param keep Parameter @p keep.
 */
CS_INLINE void csChopDec_q(char* s, size_t keep) {
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

/**
 * @brief Reduces a fraction under a digit size.
 * @param numerator Decimal text of the numerator.
 * @param numSize Parameter @p numSize.
 * @param denominator Decimal text of the denominator.
 * @param denomSize Parameter @p denomSize.
 * @param sizeCondition Parameter @p sizeCondition.
 */
CS_INLINE void csReducePair_q(char*& numerator, size_t& numSize, char*& denominator, size_t& denomSize, size_t sizeCondition) {
  char* ns = CSARITHMETIC::csLimbsToDec(numerator, numSize);
  char* ds = CSARITHMETIC::csLimbsToDec(denominator, denomSize);
  size_t nd = strlen(ns), dd = strlen(ds);
  size_t maxd = nd > dd ? nd : dd;
  size_t mind = nd < dd ? nd : dd;
  if (maxd > sizeCondition && mind > maxd - sizeCondition) {
    size_t diff = maxd - sizeCondition;
    csChopDec_q(ns, nd > dd ? sizeCondition : nd - diff);
    csChopDec_q(ds, dd > nd ? sizeCondition : dd - diff);
    free(numerator);
    free(denominator);
    csFromDec_q(ns, numerator, numSize);
    csFromDec_q(ds, denominator, denomSize);
  }
  free(ns);
  free(ds);
}
#endif

#ifndef CS_TEST_Q
/**
 * @brief Loads the digits of a mantissa.
 * @param s Parameter @p s.
 * @param o Integer in limbs.
 */
CS_INLINE void csFromDigits_r(const char* s, csLimbs_q& o) {
  char* raw = 0;
  size_t n = 0;
  csFromDec_q(s && s[0] ? s : "0", raw, n);
  o.copyFrom((const uint32_t*)raw, n);
  free(raw);
}

/**
 * @brief Multiplies an integer by a power of ten.
 * @param a First operand.
 * @param k Parameter @p k.
 */
CS_INLINE void csMulPow10_r(csLimbs_q& a, int k);

/**
 * @brief Loads a real's mantissa into limbs.
 * @param a First operand.
 * @param o Integer in limbs.
 */
CS_INLINE void csLoad_r(const CSARITHMETIC::csReal& a, csLimbs_q& o) {
  if (!a.mantissa || a.mantSize == 0) {
    o.setZero();
    return;
  }
  o.copyFrom((const uint32_t*)a.mantissa, a.mantSize);
}

/**
 * @brief Stores limbs into a real's mantissa.
 * @param a First operand.
 * @param v Integer in limbs.
 */
CS_INLINE void csSave_r(CSARITHMETIC::csReal& a, const csLimbs_q& v) {
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

/**
 * @brief Exports the decimal mantissa of a real.
 * @param a First operand.
 * @return Resulting text. The caller frees the memory.
 */
CS_INLINE char* csExportDigits_r(const CSARITHMETIC::csReal& a) {
  if (!a.mantissa || a.mantSize == 0)
    return CSARITHMETIC::csLimbsToDec(0, 0);
  return CSARITHMETIC::csLimbsToDec(a.mantissa, a.mantSize);
}

/**
 * @brief Counts the decimal digits of an integer.
 * @param a First operand.
 * @return Computed value.
 */
CS_INLINE int csDecDigits_r(const csLimbs_q& a) {
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

/**
 * @brief Divides an integer by a power of ten.
 * @param a First operand.
 * @param k Parameter @p k.
 */
CS_INLINE void csPow10Limbs_q(int k, csLimbs_q& out) {
  if (k <= 0) {
    out.setOne();
    return;
  }
  if (k <= 9) {
    uint32_t scale = 1;
    for (int i = 0; i < k; ++i)
      scale *= 10u;
    out.setU64(scale);
    return;
  }
  csLimbs_q base;
  base.setU64(10);
  out.setOne();
  while (k > 0) {
    if (k & 1) {
      if (out.isOne())
        out.copy(base);
      else {
        csLimbs_q prod;
        csMul_q(out.p, out.n, base.p, base.n, prod);
        out.copy(prod);
      }
    }
    k >>= 1;
    if (k > 0) {
      csLimbs_q squared;
      csMul_q(base.p, base.n, base.p, base.n, squared);
      base.copy(squared);
    }
  }
}

CS_INLINE void csDivPow10_r(csLimbs_q& a, int k) {
  if (k <= 0 || a.isZero())
    return;
  if (k <= 36) {
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
    return;
  }
  csLimbs_q pow10;
  csPow10Limbs_q(k, pow10);
  if (csCmpLimbs_q(a.p, a.n, pow10.p, pow10.n) < 0) {
    a.setZero();
    return;
  }
  csLimbs_q quot, rem;
  csDivMod_q(a.p, a.n, pow10.p, pow10.n, quot, rem);
  a.copy(quot);
}

/**
 * @brief Compares two reals.
 * @param a First operand.
 * @param b Second operand.
 * @return Computed value.
 */
CS_INLINE int csCmpValue_r(const CSARITHMETIC::csReal& a, const CSARITHMETIC::csReal& b) {
  csLimbs_q A, B;
  csLoad_r(a, A);
  csLoad_r(b, B);
  if (A.isZero() && B.isZero())
    return 0;
  if (A.isZero())
    return b.sign ? 1 : -1;
  if (B.isZero())
    return a.sign ? -1 : 1;
  int ea = a.exponent, eb = b.exponent;
  int e = ea < eb ? ea : eb;
  if (ea > e)
    csMulPow10_r(A, ea - e);
  if (eb > e)
    csMulPow10_r(B, eb - e);
  int c = csCmpLimbs_q(A.p, A.n, B.p, B.n);
  if (!c)
    return 0;
  if (a.sign != b.sign)
    return a.sign ? -1 : 1;
  return a.sign ? -c : c;
}

/**
 * @brief Multiplies an integer by a power of ten.
 * @param a First operand.
 * @param k Parameter @p k.
 */
CS_INLINE void csMulPow10_r(csLimbs_q& a, int k) {
  if (k <= 0 || a.isZero())
    return;
  if (k <= 36) {
    while (k > 0) {
      int take = k > 9 ? 9 : k;
      uint32_t scale = 1;
      for (int i = 0; i < take; ++i)
        scale *= 10u;
      csLimbs_q t;
      t.reserve(a.n + 1);
      csMulSmall_q(a.p, a.n, scale, t.p, a.n + 1);
      t.n = a.n + 1;
      t.trim();
      a.copy(t);
      k -= take;
    }
    return;
  }
  csLimbs_q pow10, prod;
  csPow10Limbs_q(k, pow10);
  csMul_q(a.p, a.n, pow10.p, pow10.n, prod);
  a.copy(prod);
}

/**
 * @brief Removes low-order factors of ten.
 * @param a First operand.
 * @return Computed value.
 */
CS_INLINE int csTrim10_r(csLimbs_q& a) {
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


/**
 * @brief Adds or subtracts two signed values.
 * @param left Integer in limbs.
 * @param leftNeg Parameter @p leftNeg.
 * @param right Integer in limbs.
 * @param rightNeg Parameter @p rightNeg.
 * @param out Integer in limbs.
 * @param outNeg Parameter @p outNeg.
 */
CS_INLINE void csCombine_r(const csLimbs_q& left, int leftNeg, const csLimbs_q& right, int rightNeg, csLimbs_q& out, int& outNeg) {
  if (!leftNeg) {
    if (rightNeg) {
      bool neg = false;
      csSubAbs_q(left.p, left.n, right.p, right.n, out, neg);
      outNeg = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
    } else {
      csAdd_q(left.p, left.n, right.p, right.n, out);
      outNeg = CS_POSITIVE_NUMBER;
    }
  } else if (rightNeg) {
    csAdd_q(left.p, left.n, right.p, right.n, out);
    outNeg = CS_NEGATIVE_NUMBER;
  } else {
    bool neg = false;
    csSubAbs_q(right.p, right.n, left.p, left.n, out, neg);
    outNeg = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  }
  if (out.isZero())
    outNeg = CS_POSITIVE_NUMBER;
}
#endif

}
