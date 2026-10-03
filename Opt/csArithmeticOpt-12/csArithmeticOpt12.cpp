#include "csArithmeticOpt12.h"

#ifdef CS_STATIC_LIB
#undef CS_FORCE_INLINE
#undef CS_INLINE
#undef CS_FORCEINLINE
#define CS_FORCE_INLINE __attribute__((used))
#define CS_INLINE __attribute__((used))
#define CS_FORCEINLINE __attribute__((used))
#endif

// Limbs are in base 2^32. Decimal text exists only on input and on display.
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cmath>
#ifndef CS_BASE_Q
#define CS_BASE_Q (4294967296LL)
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
  CS_INLINE ~csLimbs_q() {
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
/**
 * @brief Copies the value.
 * @param o Integer in limbs.
 */
  CS_INLINE void copy(const csLimbs_q& o) { copyFrom(o.p, o.n); }
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
static int csKaraOverflow_q = 0;

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
static int csDivOverflow_q = 0;
static int csDivFallback_q = 0;

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
    numerator = 0;
    free(denominator);
    denominator = 0;
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
CS_INLINE void csDivPow10_r(csLimbs_q& a, int k) {
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


enum { CS_STACK_MAX = 2048 };

struct csScratch {
  char small[CS_STACK_MAX + 1];
  char* p;
  bool heap;
/**
 * @brief Constructs the object.
 */
  CS_FORCE_INLINE csScratch() {
    p = 0;
    heap = false;
  }
/**
 * @brief Allocates a memory block.
 * @param n Number of elements.
 * @param fill Parameter @p fill.
 */
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
/**
 * @brief Copies limbs into this object.
 * @param s Parameter @p s.
 * @param n Number of elements.
 */
  CS_FORCE_INLINE void copyFrom(const char* s, size_t n) {
    alloc(n, 0);
    for (size_t i = 0; i < n; ++i)
      p[i] = s[i];
    p[n] = '\0';
  }
/**
 * @brief Releases a memory block.
 */
  CS_FORCE_INLINE void release() {
    if (heap && p)
      free(p);
    p = 0;
    heap = false;
  }
  CS_FORCE_INLINE ~csScratch() { release(); }
};

/**
 * @brief Aligns an address to the requested boundary.
 * @param src Source of the operation.
 * @param srcSize Parameter @p srcSize.
 * @param dst Destination of the operation.
 * @param opSize Number of aligned digits.
 */
CS_FORCE_INLINE static void csPlaceAligned(const char* src, size_t srcSize, char* dst, size_t opSize) {
  size_t delta = opSize - srcSize;
  for (size_t i = 0; i < srcSize; ++i)
    dst[delta + i] = src[i];
  dst[opSize] = '\0';
}

/**
 * @brief Moves an address back to the alignment boundary.
 * @param a0 Original text of the first operand.
 * @param aSize Number of digits of the first text.
 * @param b0 Original text of the second operand.
 * @param bSize Number of digits of the second text.
 * @param a First operand.
 * @param b Second operand.
 * @param opSize Number of aligned digits.
 * @return True when the condition holds.
 */
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

CS_FORCE_INLINE CSARITHMETIC::csRational::csRational(CSARITHMETIC::csRaw_q) {
  numerator = 0;
  denominator = 0;
  numSize = 0;
  denomSize = 0;
  sign = CS_POSITIVE_NUMBER;
}

CS_FORCE_INLINE csRational::csRational(const csRational& a)
  : numerator(0), denominator(0), sign(a.sign), numSize(0), denomSize(0)
{
  if (a.numerator) {
    size_t n = a.numSize ? a.numSize : 1;
    numerator = csDupLimbs_q(a.numerator, n);
    numSize = n;
  }
  if (a.denominator) {
    size_t n = a.denomSize ? a.denomSize : 1;
    denominator = csDupLimbs_q(a.denominator, n);
    denomSize = n;
  }
}

CS_FORCE_INLINE csRational::csRational(csRational&& a) noexcept
  : numerator(a.numerator), denominator(a.denominator), sign(a.sign), numSize(a.numSize), denomSize(a.denomSize)
{
  a.numerator = 0;
  a.denominator = 0;
  a.numSize = 0;
  a.denomSize = 0;
}

CS_FORCE_INLINE csRational::~csRational()
{
  if (numerator)
    free(numerator);
  if (denominator)
    free(denominator);
}

/**
 * @brief Fills the alignment area with zeros.
 * @param remain Receives the remainder of the division.
 * @param remSize Parameter @p remSize.
 */
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

/**
 * @brief Runs one step of the greatest common divisor.
 * @param a First operand.
 * @param b Second operand.
 * @param remain Receives the remainder of the division.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param remSize Parameter @p remSize.
 * @param ia Parameter @p ia.
 * @param ib Parameter @p ib.
 * @param ir Parameter @p ir.
 * @param cap Parameter @p cap.
 */
CS_FORCE_INLINE static void csGcdModStep(char*& a, char*& b, char*& remain,
                        size_t& aSize, size_t& bSize, size_t& remSize,
                        csScratch buf[3], int& ia, int& ib, int& ir, size_t cap) {
  remain = buf[ir].p;
  remSize = cap;
  CSARITHMETIC::remainderDecimal(a, b, remain, aSize, bSize, remSize);
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
/**
 * @brief Constructs the object.
 */
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
/**
 * @brief Constructs the object.
 */
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
/**
 * @brief Constructs the object.
 */
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
/**
 * @brief Constructs the object.
 */
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


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::buildAdditionTable()
{
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::addDecimal(char*a, char*b, char*&result, size_t opSize, size_t& resSize)
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

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::addDecimal(char*a,char*b)
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
  addDecimal(aBuf.p, bBuf.p, result, opSize, resSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::addDecimal(char*a,char*b, size_t&resSize)
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
  addDecimal(aBuf.p, bBuf.p, result, opSize, resSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::addDecimal(const char*a,const char*b)
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
  addDecimal(aBuf.p, bBuf.p, result, opSize, resSize);
  return result;
}

/* ---- src/csSubstraction.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::buildSubtractionTable()
{
}


CS_FORCE_INLINE csBIDIGITS CSARITHMETIC_API CSARITHMETIC::subtractionStep(int i)
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

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::subtractDecimal(char*a, char*b, char*&result, size_t opSize)
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

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::subtractDecimal(char*a,char*b)
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
  subtractDecimal(pa, pb, result, opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::subtractDecimal(char*a,char*b, bool& sign)
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
  subtractDecimal(pa, pb, result, opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::subtractDecimal(char*a,char*b, size_t aSize, size_t bSize, bool& sign)
{
  size_t opSize = aSize > bSize ? aSize : bSize;
  csScratch aBuf, bBuf;
  aBuf.alloc(opSize, '0');
  bBuf.alloc(opSize, '0');
  char* pa = aBuf.p;
  char* pb = bBuf.p;
  sign = csAlignSub(a, aSize, b, bSize, pa, pb, opSize);
  char* result = csAllocCharPtr(opSize, '0');
  subtractDecimal(pa, pb, result, opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::subtractDecimal(char*a,char*b, size_t aSize, size_t bSize, size_t& resSize, bool& sign)
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
  subtractDecimal(pa, pb, result, resSize);
  return result;
}


CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::subtractDecimal(const char*a,const char*b)
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
  subtractDecimal(pa, pb, result, opSize);
  return result;
}

/* ---- src/csMultiplication.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::buildMultiplicationTable()
{
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::multiplyByDigit(char*a, uchar b, char*&result, size_t aSize, size_t resSize)
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

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::multiplyDigitBase2(char*a, uchar b, char*&result, size_t aSize, size_t resSize
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

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::addShiftedProduct(char*a, char*b, char*&result, size_t opSize, size_t resSize)
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

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::multiplyDecimal(char*a, char*b, char*&result, char*& tmpResult,
 size_t aSize, size_t bSize, size_t resSize)
{
  
  size_t m=bSize-1, n = aSize + 1;

  csBIDIGITS prevCarry;
  size_t m1=aSize-1, n1=resSize-1;

  for(size_t i=m,j=0; j<bSize; j++, i-=1)
  {
    
    multiplyDigitBase2(a, b[i], tmpResult, aSize, resSize,
                            prevCarry, m1, n1);
    shiftLeft(tmpResult, resSize, j);
    addShiftedProduct(result, tmpResult, result, n+j, resSize);

  }
  
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::multiplyDecimal(char*a, char*b)
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

  multiplyDecimal(a,b,result,tmpResult,aSize,bSize,opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::multiplyDecimal(char*a, char*b, size_t aSize, size_t bSize)
{
  size_t opSize = aSize + bSize;
  size_t sz = opSize + 1;
  char*result = csAllocCharPtr(opSize, sz, '0');
  csScratch tmpBuf;
  tmpBuf.alloc(opSize, '0');
  char*tmpResult = tmpBuf.p;

  multiplyDecimal(a,b,result,tmpResult,aSize,bSize,opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::multiplyDecimal(char*a, char*b, size_t aSize, size_t bSize, size_t& resSize)
{
  resSize = aSize + bSize;
  size_t sz = resSize + 1;
  char*result = csAllocCharPtr(resSize, sz, '0');
  csScratch tmpBuf;
  tmpBuf.alloc(resSize, '0');
  char*tmpResult = tmpBuf.p;
  
  multiplyDecimal(a,b,result,tmpResult,aSize,bSize,resSize);
   
  return result;
}


CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::multiplyDecimal(const char*a, const char*b)
{
  
  return multiplyDecimal((char*)a,(char*)b);
}

/* ---- src/csDivision.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::buildDivisionTable()
{
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::subtractPartialQuotient(char*a, char*b, char*&result, size_t opSize, size_t frontOffset)
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


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::multiplyDigitForDivision(char*a, uchar b, char*&result, size_t aSize, size_t resSize)
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
CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::multiplyDigitPairForDivision(char*a, uchar b, char*&result, size_t aSize, size_t resSize)
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


CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::divideDecimal(char*_a, char* _b, char*&result,
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
          quotientDigit = (divTable[digitPairIndex(a[skipZerosLen], a[skipZerosLen+1])][digitPairIndex(b[0],b[1])]-1);

        if(quotientDigit == 48) quotientDigit = 49;
        
        multiplyDigitForDivision(b, quotientDigit, tmpRes, tmpResSizeForSub, tmpResSizeForSub);
        subtractPartialQuotient(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);

        while(isaGreaterEqual(a,b,tmpResSizeForSub,skipZerosLen))
        {
           subtractPartialQuotient(a, b, a, tmpResSizeForSub, skipZerosLen);
           quotientDigit += 1;
        }
        
      }
      else
      {
        if(bSize == 1)
          quotientDigit = divTable[digitPairIndex(a[skipZerosLen], a[skipZerosLen+1])][b[0]]-2;
        else
          quotientDigit = (divTable[digitTripleIndex(a[skipZerosLen], a[skipZerosLen+1], a[skipZerosLen+2])]
                                   [digitPairIndex(b[0],b[1])]-1);

        if(quotientDigit <= 48) quotientDigit = 49;
        if(quotientDigit > 57) quotientDigit = 57;
      
        multiplyDigitPairForDivision(b, quotientDigit, tmpRes, bSize, tmpResSizeForSub);
        subtractPartialQuotient(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);

        while(isaGreaterEqual(a,bCpy,tmpResSizeForSub,skipZerosLen))
        {
           subtractPartialQuotient(a, bCpy, a, tmpResSizeForSub, skipZerosLen);
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

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::divideDecimal(char*_a, char* _b, char*&result, char*&remain,
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
          quotientDigit = (divTable[digitPairIndex(a[skipZerosLen], a[skipZerosLen+1])][digitPairIndex(b[0],b[1])]-1);

        if(quotientDigit == 48) quotientDigit = 49;


        multiplyDigitForDivision(b, quotientDigit, tmpRes, tmpResSizeForSub, tmpResSizeForSub);
        subtractPartialQuotient(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);

        while(isaGreaterEqual(a,b,tmpResSizeForSub,skipZerosLen))
        {
           subtractPartialQuotient(a, b, a, tmpResSizeForSub, skipZerosLen);
           quotientDigit += 1;
        }
        
      }
      else
      {
        
      //convert the first and second digits to a number, to find the quotient with the first digit of b

        if(bSize == 1)
          quotientDigit = divTable[digitPairIndex(a[skipZerosLen], a[skipZerosLen+1])][b[0]]-2;
        else
          quotientDigit = (divTable[digitTripleIndex(a[skipZerosLen], a[skipZerosLen+1], a[skipZerosLen+2])]
                                 [digitPairIndex(b[0],b[1])]-1);

        if(quotientDigit <= 48) quotientDigit = 49;
        if(quotientDigit > 57) quotientDigit = 57;
        
        multiplyDigitPairForDivision(b, quotientDigit, tmpRes, bSize, tmpResSizeForSub);
        subtractPartialQuotient(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);

        while(isaGreaterEqual(a,bCpy,tmpResSizeForSub,skipZerosLen))
        {
           subtractPartialQuotient(a, bCpy, a, tmpResSizeForSub, skipZerosLen);
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

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::divideDecimal(char*a, char* b, char*&remain)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  int rs;
  size_t resSize = (rs=(aSize-bSize+1))>1?rs:1;
  size_t remSize = bSize;

  remain = csAllocCharPtr(remSize, '0');

  char*quotient = csAllocCharPtr(resSize, '0');

  divideDecimal(a, b, quotient, remain, aSize, bSize, resSize, remSize);

  return quotient;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::divideDecimal(char*a, char* b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  int rs;
  size_t resSize = (rs=(aSize-bSize+1))>1?rs:1;
  size_t remSize = bSize;

  char*quotient = csAllocCharPtr(resSize, '0');

  divideDecimal(a, b, quotient, aSize, bSize, resSize);

  return quotient;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::divideDecimal(char*a, char* b, size_t aSize, size_t bSize, size_t& resSize)
{
  int rs;
  resSize = (rs=(aSize-bSize+1))>1?rs:1;
  size_t remSize = bSize, remSize1 = remSize+1;

  char*quotient = csAllocCharPtr(resSize, '0');
  
  divideDecimal(a, b, quotient, aSize, bSize, resSize);

  return quotient;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::divideDecimal(const char*_a, const char* _b, char*&remain)
{

  size_t aSize = strlen(_a);
  size_t bSize = strlen(_b);
  char*a = filledString(_a,aSize);
  char*b = filledString(_b,bSize);

  size_t rs, resSize = (rs=(aSize-bSize+1))>1?rs:1;
  
  size_t remSize = bSize;

  remain = csAllocCharPtr(remSize, '0');

  char*quotient = csAllocCharPtr(resSize, '0');

  divideDecimal(a, b, quotient, remain, aSize, bSize, resSize, remSize);
  free(a);
  free(b);
  return quotient;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::divideDecimal(const char*_a, const char* _b)
{

  size_t aSize = strlen(_a);
  size_t bSize = strlen(_b);
  char*a = filledString(_a,aSize);
  char*b = filledString(_b,bSize);
  

  size_t rs, resSize = (rs=(aSize-bSize+1))>1?rs:1;
  
  size_t remSize = bSize, remSize1 = remSize+1;

  char*quotient = csAllocCharPtr(resSize, '0');

  divideDecimal(a, b, quotient, aSize, bSize, resSize);
  free(a);
  free(b);
  return quotient;
}

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::divideWithFraction(char*_a, char* _b, char*&resInt, char*&resDec, char*&remain,
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

  const char* status = divideDecimal(a, _b, resInt, remain, aSize, bSize, resSize, remSize);


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

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::divideWithScale(char*_a, char* _b, char*&resInt, char*&resDec, char*&remain,
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

  const char* status = divideDecimal(a, _b, resInt, remain, aSize, bSize, resSize, remSize);


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

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::divideWithScale(char*_a, char* _b, char*&resInt, char*&resDec,
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

  const char* status = divideDecimal(a, _b, resInt, aSize, bSize, resSize);


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

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::divideWithFraction(char*a, char* b, char*&resInt, char*&resDec, char*&remain, size_t nDecimals)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t remSize=0, resSize=0, resIntSize, resDecSize;
  
  return divideWithFraction(a, b, resInt, resDec, remain,
                    aSize, bSize, resSize, resIntSize, resDecSize, remSize, nDecimals);

}

/* ---- src/csModulus.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;


CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::remainderDecimal(char*a, char* b, char*&remain,
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
          quotientDigit = (divTable[digitPairIndex(a[skipZerosLen], a[skipZerosLen+1])][digitPairIndex(b[0],b[1])]-1);

        if(quotientDigit == 48) quotientDigit = 49;

  
        multiplyDigitForDivision(b, quotientDigit, tmpRes, tmpResSizeForSub, tmpResSizeForSub);
        subtractPartialQuotient(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);
        
        while(isaGreaterEqual(a,b,tmpResSizeForSub,skipZerosLen))
          subtractPartialQuotient(a, b, a, tmpResSizeForSub, skipZerosLen);

      }
      else
      {
      //convert the first and second digits to a number, to find the quotient with the first digit of b
        if(bSize == 1)
          quotientDigit = divTable[digitPairIndex(a[skipZerosLen], a[skipZerosLen+1])][b[0]]-2;
        else
          quotientDigit = (divTable[digitTripleIndex(a[skipZerosLen], a[skipZerosLen+1], a[skipZerosLen+2])]
                                 [digitPairIndex(b[0],b[1])]-1);

        if(quotientDigit <= 48) quotientDigit = 49;
        if(quotientDigit > 57) quotientDigit = 57;
        
        multiplyDigitPairForDivision(b, quotientDigit, tmpRes, bSize, tmpResSizeForSub);
        subtractPartialQuotient(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);
        while(isaGreaterEqual(a,bCpy,tmpResSizeForSub,skipZerosLen))
          subtractPartialQuotient(a, bCpy, a, tmpResSizeForSub, skipZerosLen);


      }
    }

  }

  
  return 0;
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::remainderDecimal(char*a, char* b, char*&remain)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t remSize = 0;
  remSize = bSize;
  remain = csAllocCharPtr(remSize, '0');
  remainderDecimal(a, b, remain, aSize, bSize, remSize);
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::remainderDecimal(char*a, char* b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t remSize = 0;
  remSize = bSize;
  char* remain = csAllocCharPtr(remSize, '0');
  remainderDecimal(a, b, remain, aSize, bSize, remSize);
  return remain;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::remainderDecimal(const char*a, const char* b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  char* _a = newString(a, aSize);
  char* _b = newString(b, bSize);
  size_t remSize = 0;
  char* remain = 0;
  remSize = bSize;
  remain = csAllocCharPtr(remSize, '0');
  remainderDecimal(_a, _b, remain, aSize, bSize, remSize);
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


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::printDigits(char*nb, const char*separator)
{
  cout<<nb<<separator;
}

CS_FORCE_INLINE csBIDIGITS** CSARITHMETIC_API CSARITHMETIC::packDigitPairs(int** opTable)
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


CS_FORCE_INLINE uchar CSARITHMETIC_API CSARITHMETIC::digitPairIndex(uchar tens, uchar units)
{
  return (tens-48)*10+units;
}

CS_FORCE_INLINE uchar CSARITHMETIC_API CSARITHMETIC::digitTripleIndex(int cents, uchar tens, uchar units)
{
  
  return uchar((cents-48)*100+(tens-48)*10+units);
}

//unsigned uchar

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::printDigitTable(char*opName)
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


CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::gcd(char*_a, char*_b, size_t aSize, size_t bSize, size_t& gcdSize)
{
  if ((aSize == 1 && _a[0] == '1') || (bSize == 1 && _b[0] == '1'))
  {
    gcdSize = 1;
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
  gcdSize = srcSize;
  char* out = csAlloc<char>(srcSize + 1);
  for (size_t i = 0; i < srcSize; ++i)
    out[i] = srcDigits[i];
  out[srcSize] = '\0';
  return out;
}

CS_FORCE_INLINE char*CSARITHMETIC_API CSARITHMETIC::gcd(char*a, char*b,size_t& gcdSize)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  char* res = gcd(a,b,aSize,bSize,gcdSize);
  return res;
}

CS_FORCE_INLINE char*CSARITHMETIC_API CSARITHMETIC::gcd(const char*a, const char*b,size_t& gcdSize)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  char* _a = newString(a, aSize);
  char* _b = newString(b, bSize);
  char * g = gcd(_a,_b,aSize,bSize,gcdSize);
  free(_a);
  free(_b);
  return g;
}

/**
 * @brief Returns the high part of a split real.
 * @param _a First operand.
 * @param sr Real.
 * @return Resulting real.
 */
static long csFloorHalf(long mag)
{
  return mag >= 0 ? mag / 2 : (mag - 1) / 2;
}

static long csRealMagnitude(const csReal& a)
{
  csLimbs_q n;
  csLoad_r(a, n);
  if (n.isZero())
    return 0;
  return (long)csDecDigits_r(n) - 1 + a.exponent;
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csSortMinR(csReal*& rn, size_t size)
{
  size_t s1 = size-1;
  csReal tmp;
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

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csSortMin_CoordsByRNumber(csFCOORDS*& fc, csReal*& rn, size_t size)
{
  size_t s1 = size-1;
  csReal tmp;
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
CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csSortMax_CoordsByRNumber(csFCOORDS*& fc, csReal*& rn, size_t size)
{
  size_t s1 = size-1;
  csReal tmp;
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

/* ---- src/csRational.cpp ---- */
#include <cstddef>
#include <cstdio>
#include <cstring>

using namespace __mem_man;
using namespace __ar_man;
using namespace CSARITHMETIC;

extern int RPRECISION;

CS_FORCE_INLINE csRational::csRational(const char* _numerator, const char* _denominator, bool _sign)
{
  csFromDec_q(_numerator, numerator, numSize);
  csFromDec_q(_denominator, denominator, denomSize);
  sign = _sign;
}

CS_FORCE_INLINE csRational::csRational(size_t num, size_t denom, bool _sign)
{
  sign = _sign;
  csLimbs_q n, d;
  n.setU64((uint64_t)num);
  d.setU64((uint64_t)denom);
  numerator = n.leak();
  numSize = n.n;
  denominator = d.leak();
  denomSize = d.n;
}

CS_FORCE_INLINE void csRational::init()
{
  numerator=0;
  denominator=0;
}

CS_FORCE_INLINE csRational* CSARITHMETIC_API CSARITHMETIC::csPtrAlloc_q(size_t nb)
{
  csRational *qn = csAlloc<csRational>(nb);
  for(size_t i=0; i<nb; i++)
  {
    qn[i].init();
  }
  return qn;
}

CS_FORCE_INLINE csRational* CSARITHMETIC_API CSARITHMETIC::csPtrAlloc_q(size_t nb, csRational init)
{
  csRational *qn = csAlloc<csRational>(nb);
  for (size_t i = 0; i < nb; i++) {
    qn[i].sign = init.sign;
    if (init.numerator && init.numSize) {
      qn[i].numerator = csDupLimbs_q(init.numerator, init.numSize);
      qn[i].numSize = init.numSize;
    } else {
      qn[i].numerator = csDupLimbs_q(0, 0);
      qn[i].numSize = 1;
    }
    if (init.denominator && init.denomSize) {
      qn[i].denominator = csDupLimbs_q(init.denominator, init.denomSize);
      qn[i].denomSize = init.denomSize;
    } else {
      qn[i].denominator = csLimbOne_q();
      qn[i].denomSize = 1;
    }
  }
  return qn;
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csPtrFree_q(csRational*& qn, size_t nb)
{
  for(size_t i=0; i<nb; i++)
  {
    qn[i].clear();
  }
  free(qn);
}


CS_FORCE_INLINE void csRational::set(const char* _numerator, const char* _denominator, bool _sign)
{
  if (numerator) {
    free(numerator);
    numerator = 0;
    free(denominator);
    denominator = 0;
  }
  csFromDec_q(_numerator, numerator, numSize);
  csFromDec_q(_denominator, denominator, denomSize);
  sign = _sign;
}

CS_FORCE_INLINE void csRational::set(size_t num, size_t denom, bool _sign)
{
  if (numerator) {
    free(numerator);
    numerator = 0;
    free(denominator);
    denominator = 0;
  }
  sign = _sign;
  csLimbs_q n, d;
  n.setU64((uint64_t)num);
  d.setU64((uint64_t)denom);
  numerator = n.leak();
  numSize = n.n;
  denominator = d.leak();
  denomSize = d.n;
}

CS_FORCE_INLINE void csRational::assignValue(long num, size_t denom)
{
  if (numerator) {
    free(numerator);
    numerator = 0;
    free(denominator);
    denominator = 0;
  }
  sign = num < 0 ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  csLimbs_q n, d;
  n.setU64(csAbsLong_q(num));
  d.setU64((uint64_t)denom);
  numerator = n.leak();
  numSize = n.n;
  denominator = d.leak();
  denomSize = d.n;
}

CS_FORCE_INLINE size_t csRational::maxLimbCount()
{
  return numSize>denomSize?numSize:denomSize;
}

CS_FORCE_INLINE void csRational::reduce(size_t sizeCondition)
{
  csReducePair_q(numerator, numSize, denominator, denomSize, sizeCondition);
}

/**
 * @brief Reduces the value to the requested size.
 * @param a First operand.
 * @param sizeCondition Parameter @p sizeCondition.
 * @return Resulting rational.
 */
CS_FORCE_INLINE csRational csReduce(csRational a, size_t sizeCondition)
{
  char* n = csDupLimbs_q(a.numerator, a.numSize);
  char* d = csDupLimbs_q(a.denominator, a.denomSize);
  size_t ns = a.numSize ? a.numSize : 1;
  size_t ds = a.denomSize ? a.denomSize : 1;
  csReducePair_q(n, ns, d, ds, sizeCondition);
  if (a.numerator)
    free(a.numerator);
  if (a.denominator)
    free(a.denominator);
  a.numerator = n;
  a.denominator = d;
  a.numSize = ns;
  a.denomSize = ds;
  return a;
}

CS_FORCE_INLINE bool csRational::differAbsolute(const csRational& a)
{
  return !equalAbsolute(a);
}

CS_FORCE_INLINE bool csRational::operator==(const csRational& a)
{
  
  return csCmpSigned_q(*this, a) == 0;
}

CS_FORCE_INLINE bool csRational::equalAbsolute(const csRational& a)
{
  
  return csCmpAbs_q(*this, a) == 0;
}

CS_FORCE_INLINE bool csRational::operator!=(const csRational& a)
{
  return !(*this == a);
}

CS_FORCE_INLINE bool csRational::operator>(const csRational& a)
{
  
  return csCmpSigned_q(*this, a) > 0;
}

CS_FORCE_INLINE bool csRational::greaterAbsolute(const csRational& a)
{
  
  return csCmpAbs_q(*this, a) > 0;
}

CS_FORCE_INLINE bool csRational::operator>=(const csRational& a)
{
  
  return csCmpSigned_q(*this, a) >= 0;
}

CS_FORCE_INLINE bool csRational::greaterOrEqualAbsolute(const csRational& a)
{
  
  return csCmpAbs_q(*this, a) >= 0;
}

CS_FORCE_INLINE bool csRational::operator<(const csRational& a)
{
  
  return csCmpSigned_q(*this, a) < 0;
}

CS_FORCE_INLINE bool csRational::lessAbsolute(const csRational& a)
{
  
  return csCmpAbs_q(*this, a) < 0;
}

CS_FORCE_INLINE bool csRational::operator<=(const csRational& a)
{
  
  return csCmpSigned_q(*this, a) <= 0;
}

CS_FORCE_INLINE bool csRational::lessOrEqualAbsolute(const csRational& a)
{
  
  return csCmpAbs_q(*this, a) <= 0;
}

CS_FORCE_INLINE void csRational::print(const char*title)
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

CS_FORCE_INLINE void csRational::clear(const source_location loc)
{
  if(numerator)
  {
    free(numerator);
    numerator = 0;
    free(denominator);
    denominator = 0;
    numerator = 0;
    denominator = 0;
    numSize = 0;
    denomSize = 0;

  }
}

CS_FORCE_INLINE csRational csRational::operator+(const csRational& a)
{
  
  csRational qn(csRaw_q{});
  csLimbs_q n1, n2, den, num;
  csMul_q(csPtr_q(numerator), numSize, csPtr_q(a.denominator), a.denomSize, n1);
  csMul_q(csPtr_q(denominator), denomSize, csPtr_q(a.numerator), a.numSize, n2);
  csMul_q(csPtr_q(denominator), denomSize, csPtr_q(a.denominator), a.denomSize, den);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (a.sign == CS_NEGATIVE_NUMBER)
      csSubAbs_q(n1.p, n1.n, n2.p, n2.n, num, neg);
    else
      csAdd_q(n1.p, n1.n, n2.p, n2.n, num);
  } else if (a.sign == CS_NEGATIVE_NUMBER) {
    csAdd_q(n1.p, n1.n, n2.p, n2.n, num);
    neg = true;
  } else {
    csSubAbs_q(n2.p, n2.n, n1.p, n1.n, num, neg);
  }
  if (num.isZero())
    neg = false;
  csNormalize_q(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
}

CS_FORCE_INLINE csRational csRational::operator-(const csRational& a)
{
  
  csRational qn(csRaw_q{});
  csLimbs_q n1, n2, den, num;
  csMul_q(csPtr_q(numerator), numSize, csPtr_q(a.denominator), a.denomSize, n1);
  csMul_q(csPtr_q(denominator), denomSize, csPtr_q(a.numerator), a.numSize, n2);
  csMul_q(csPtr_q(denominator), denomSize, csPtr_q(a.denominator), a.denomSize, den);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (a.sign == CS_NEGATIVE_NUMBER)
      csAdd_q(n1.p, n1.n, n2.p, n2.n, num);
    else
      csSubAbs_q(n1.p, n1.n, n2.p, n2.n, num, neg);
  } else if (a.sign == CS_NEGATIVE_NUMBER) {
    csSubAbs_q(n2.p, n2.n, n1.p, n1.n, num, neg);
  } else {
    csAdd_q(n1.p, n1.n, n2.p, n2.n, num);
    neg = true;
  }
  if (num.isZero())
    neg = false;
  csNormalize_q(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
}

CS_FORCE_INLINE csRational csRational::operator*(const csRational& a)
{
  
  csRational qn(csRaw_q{});
  csLimbs_q num, den;
  csMul_q(csPtr_q(numerator), numSize, csPtr_q(a.numerator), a.numSize, num);
  csMul_q(csPtr_q(denominator), denomSize, csPtr_q(a.denominator), a.denomSize, den);
  bool neg = !num.isZero() && (sign != a.sign);
  csNormalize_q(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
}

CS_FORCE_INLINE csRational csRational::operator/(const csRational& a)
{
  
  csRational qn(csRaw_q{});
  csLimbs_q num, den;
  csMul_q(csPtr_q(numerator), numSize, csPtr_q(a.denominator), a.denomSize, num);
  csMul_q(csPtr_q(denominator), denomSize, csPtr_q(a.numerator), a.numSize, den);
  bool neg = !num.isZero() && (sign != a.sign);
  csNormalize_q(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
}

CS_FORCE_INLINE csRational& csRational::operator=(csRational a)
{
  char* n = numerator;
  numerator = a.numerator;
  a.numerator = n;
  char* d = denominator;
  denominator = a.denominator;
  a.denominator = d;
  bool s = sign;
  sign = a.sign;
  a.sign = s;
  size_t ns = numSize;
  numSize = a.numSize;
  a.numSize = ns;
  size_t ds = denomSize;
  denomSize = a.denomSize;
  a.denomSize = ds;
  return *this;
}

CS_FORCE_INLINE csRational& csRational::operator=(long a)
{
  if (numerator) {
    free(numerator);
    numerator = 0;
    free(denominator);
    denominator = 0;
    numerator = 0;
  }
  sign = a < 0 ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  csLimbs_q n;
  n.setU64(csAbsLong_q(a));
  numerator = n.leak();
  numSize = n.n;
  denomSize = 1;
  denominator = csLimbOne_q();
  return *this;
}



CS_FORCE_INLINE csRational csRational::operator+(long a)
{
  
  csRational qn(csRaw_q{});
  bool asign = a < 0;
  csLimbs_q mag, prod, num;
  mag.setU64(csAbsLong_q(a));
  csMul_q(csPtr_q(denominator), denomSize, mag.p, mag.n, prod);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (asign)
      csSubAbs_q(csPtr_q(numerator), numSize, prod.p, prod.n, num, neg);
    else
      csAdd_q(csPtr_q(numerator), numSize, prod.p, prod.n, num);
  } else if (asign) {
    csAdd_q(csPtr_q(numerator), numSize, prod.p, prod.n, num);
    neg = true;
  } else {
    csSubAbs_q(prod.p, prod.n, csPtr_q(numerator), numSize, num, neg);
  }
  if (num.isZero())
    neg = false;
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = csDupLimbs_q(denominator, denomSize);
  qn.denomSize = denomSize;
  return qn;
}


CS_FORCE_INLINE csRational csRational::operator-(long a)
{
  
  csRational qn(csRaw_q{});
  bool asign = a < 0;
  csLimbs_q mag, prod, num;
  mag.setU64(csAbsLong_q(a));
  csMul_q(csPtr_q(denominator), denomSize, mag.p, mag.n, prod);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (asign)
      csAdd_q(csPtr_q(numerator), numSize, prod.p, prod.n, num);
    else
      csSubAbs_q(csPtr_q(numerator), numSize, prod.p, prod.n, num, neg);
  } else if (asign) {
    csSubAbs_q(prod.p, prod.n, csPtr_q(numerator), numSize, num, neg);
  } else {
    csAdd_q(csPtr_q(numerator), numSize, prod.p, prod.n, num);
    neg = true;
  }
  if (num.isZero())
    neg = false;
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = csDupLimbs_q(denominator, denomSize);
  qn.denomSize = denomSize;
  return qn;
}


CS_FORCE_INLINE csRational csRational::operator*(long a)
{
  
  csRational qn(csRaw_q{});
  csLimbs_q mag, num;
  mag.setU64(csAbsLong_q(a));
  csMul_q(csPtr_q(numerator), numSize, mag.p, mag.n, num);
  qn.sign = (!num.isZero() && (sign != (a < 0))) ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = csDupLimbs_q(denominator, denomSize);
  qn.denomSize = denomSize;
  return qn;
}


CS_FORCE_INLINE csRational csRational::operator/(long a)
{
  
  csRational qn(csRaw_q{});
  csLimbs_q mag, den;
  mag.setU64(csAbsLong_q(a));
  csMul_q(csPtr_q(denominator), denomSize, mag.p, mag.n, den);
  qn.sign = (!csRawZero_q(numerator, numSize) && (sign != (a < 0))) ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = csDupLimbs_q(numerator, numSize);
  qn.numSize = numSize;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
}

CS_FORCE_INLINE double csRational::getDouble()
{
  double d = csLimbsToDouble_q(denominator, denomSize);
  if (d == 0)
    return 0;
  double r = csLimbsToDouble_q(numerator, numSize) / d;
  return sign ? -r : r;
}

CS_FORCE_INLINE char* csRational::decimalExpansion()
{
  char* ns = csLimbsToDec(numerator, numSize);
  char* ds = csLimbsToDec(denominator, denomSize);
  size_t nlen = strlen(ns), dlen = strlen(ds);
  char* resInt = 0;
  char* resDec = 0;
  long precicion = realPrecision();
  size_t resz = 0, risz = 0, rdsz = 0;
  char const* stat = divideWithScale(ns, ds, resInt, resDec, nlen, dlen, resz, risz, rdsz, (size_t)precicion);
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

/**
 * @brief Real number as a mantissa and an exponent.
 */
CS_FORCE_INLINE csRational::operator csReal()
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
  const char* status = divideDecimal(a, ds, result, aSize, bSize, resSize);
  free(ns);
  if (status) {
    
    csReal rn("0");
    free(result);
    free(a);
    free(ds);
    return rn;
  }
  removeFrontZeros(result, resSize);
  
  csReal rn(result, -RPRECISION, sign);
  free(result);
  free(a);
  free(ds);
  return rn;
}


CS_FORCE_INLINE csRational CSARITHMETIC_API CSARITHMETIC::pow(csRational a, size_t p)
{
  
  csRational r("1");
  for(size_t i=0; i<p; i++)
  {
    r = r*a;
  }
  return r;
}

CS_FORCE_INLINE void csRational::copy(const csRational& a)
{
  if (this == &a)
    return;
  if (numerator) {
    free(numerator);
    numerator = 0;
    free(denominator);
    denominator = 0;
    numerator = 0;
    denominator = 0;
  }
  sign = a.sign;
  if (a.numerator && a.numSize) {
    numerator = csDupLimbs_q(a.numerator, a.numSize);
    numSize = a.numSize;
  } else {
    numerator = csDupLimbs_q(0, 0);
    numSize = 1;
  }
  if (a.denominator && a.denomSize) {
    denominator = csDupLimbs_q(a.denominator, a.denomSize);
    denomSize = a.denomSize;
  } else {
    denominator = csLimbOne_q();
    denomSize = 1;
  }
}

CS_FORCE_INLINE bool csRational::isZero()
{
  
  return csRawZero_q(numerator, numSize);
}

CS_FORCE_INLINE bool csRational::isNonZero()
{
  return !isZero();
}

/* ---- src/csReal.cpp ---- */
#include <cstddef>
#include <cstdio>
#include <cstring>

using namespace __mem_man;
using namespace __ar_man;
using namespace CSARITHMETIC;

int RPRECISION = 20;

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::setRealPrecision(int precision)
{
  RPRECISION = precision;
}
CS_FORCE_INLINE int CSARITHMETIC_API CSARITHMETIC::realPrecision()
{
  return RPRECISION;
}

static char* csBuildRepeatedMantissa(const char* src, int repeat, int pos)
{
  if (!src || !src[0] || repeat < 1) {
    char* z = (char*)malloc(2);
    z[0] = '0';
    z[1] = 0;
    return z;
  }
  size_t len = strlen(src);
  size_t sec0 = 0;
  size_t secN = len;
  if (pos > 0) {
    secN = (size_t)pos;
    if (secN > len)
      secN = len;
  } else if (pos < 0) {
    size_t fromEnd = (size_t)(-pos);
    if (fromEnd > len)
      fromEnd = len;
    sec0 = len - fromEnd;
  }
  size_t secLen = secN - sec0;
  if (repeat == 1 || secLen == 0) {
    char* s = (char*)malloc(len + 1);
    memcpy(s, src, len + 1);
    return s;
  }
  size_t head = sec0;
  size_t tail = len - secN;
  size_t n = head + secLen * (size_t)repeat + tail;
  char* rep = (char*)malloc(n + 1);
  if (head)
    memcpy(rep, src, head);
  for (int i = 0; i < repeat; ++i)
    memcpy(rep + head + (size_t)i * secLen, src + sec0, secLen);
  if (tail)
    memcpy(rep + head + secLen * (size_t)repeat, src + secN, tail);
  rep[n] = 0;
  return rep;
}

static char* csBuildSliceRepeatedMantissa(const char* src, int repeat, int length, int index)
{
  if (!src || !src[0] || repeat < 1) {
    char* z = (char*)malloc(2);
    z[0] = '0';
    z[1] = 0;
    return z;
  }
  size_t len = strlen(src);
  size_t sec0 = 0;
  if (index > 0)
    sec0 = (size_t)index;
  if (sec0 > len)
    sec0 = len;
  size_t secLen = 0;
  if (length > 0)
    secLen = (size_t)length;
  if (sec0 + secLen > len)
    secLen = len - sec0;
  if (repeat == 1 || secLen == 0) {
    char* s = (char*)malloc(len + 1);
    memcpy(s, src, len + 1);
    return s;
  }
  size_t head = sec0;
  size_t tail = len - (sec0 + secLen);
  size_t n = head + secLen * (size_t)repeat + tail;
  char* rep = (char*)malloc(n + 1);
  if (head)
    memcpy(rep, src, head);
  for (int i = 0; i < repeat; ++i)
    memcpy(rep + head + (size_t)i * secLen, src + sec0, secLen);
  if (tail)
    memcpy(rep + head + secLen * (size_t)repeat, src + sec0 + secLen, tail);
  rep[n] = 0;
  return rep;
}

CS_FORCE_INLINE csReal::csReal(const char* _mantissa, int _exponent, bool _sign)
{
  set(_mantissa, _exponent, _sign);
}
CS_FORCE_INLINE csReal::csReal(const char* _mantissa, int _exponent, bool _sign, int repeat)
{
  set(_mantissa, _exponent, _sign, repeat);
}
CS_FORCE_INLINE csReal::csReal(const char* _mantissa, int _exponent, bool _sign, int repeat, int pos)
{
  set(_mantissa, _exponent, _sign, repeat, pos);
}
CS_FORCE_INLINE csReal::csReal(const char* _mantissa, int _exponent, bool _sign, int repeat, int length, int index)
{
  set(_mantissa, _exponent, _sign, repeat, length, index);
}
CS_FORCE_INLINE csReal::csReal(unsigned long _mantissa, int _exponent, bool _sign)
{
  set(_mantissa, _exponent, _sign);
}
CS_FORCE_INLINE csReal::csReal(long _mantissa, int _exponent)
{
  set(_mantissa, _exponent);
}
CS_FORCE_INLINE csReal::csReal(bool evaluate, const char* number)
{
  set(evaluate, number);
}
CS_FORCE_INLINE csReal::csReal(double value)
{
  if (!std::isfinite(value)) {
    set("0", 0, 0);
    return;
  }
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.17g", value);
  set(true, buf);
}

CS_FORCE_INLINE csReal::~csReal()
{
  if (mantissa)
    free(mantissa);
}

CS_FORCE_INLINE csReal::csReal(const csReal& a)
  : mantissa(0), exponent(a.exponent), sign(a.sign), precision(a.precision), mantSize(0)
{
  if (a.mantissa) {
    size_t n = a.mantSize ? a.mantSize : 1;
    mantissa = csDupLimbs_q(a.mantissa, n);
    mantSize = n;
  }
}

CS_FORCE_INLINE csReal::csReal(csReal&& a) noexcept
  : mantissa(a.mantissa), exponent(a.exponent), sign(a.sign), precision(a.precision), mantSize(a.mantSize)
{
  a.mantissa = 0;
  a.mantSize = 0;
}


CS_FORCE_INLINE void csReal::set(const char* _mantissa, int _exponent, bool _sign)
{
  csLimbs_q n;
  csFromDigits_r(_mantissa, n);
  csSave_r(*this, n);
  exponent = _exponent;
  sign = n.isZero() ? CS_POSITIVE_NUMBER : _sign;
  precision = 0;
}
CS_FORCE_INLINE void csReal::set(const char* _mantissa, int _exponent, bool _sign, int repeat)
{
  set(_mantissa, _exponent, _sign, repeat, 0);
}
CS_FORCE_INLINE void csReal::set(const char* _mantissa, int _exponent, bool _sign, int repeat, int pos)
{
  char* rep = csBuildRepeatedMantissa(_mantissa, repeat, pos);
  set(rep, _exponent, _sign);
  free(rep);
}
CS_FORCE_INLINE void csReal::set(const char* _mantissa, int _exponent, bool _sign, int repeat, int length, int index)
{
  char* rep = csBuildSliceRepeatedMantissa(_mantissa, repeat, length, index);
  set(rep, _exponent, _sign);
  free(rep);
}

CS_FORCE_INLINE void csReal::set(unsigned long _mantissa, int _exponent, bool _sign)
{
  csLimbs_q n;
  n.setU64((uint64_t)_mantissa);
  csSave_r(*this, n);
  exponent = _exponent;
  sign = n.isZero() ? CS_POSITIVE_NUMBER : _sign;
  precision = 0;
}

CS_FORCE_INLINE void csReal::set(long _mantissa, int _exponent)
{
  uint64_t mag = _mantissa < 0 ? (uint64_t)(-_mantissa) : (uint64_t)_mantissa;
  csLimbs_q n;
  n.setU64(mag);
  csSave_r(*this, n);
  exponent = _exponent;
  sign = _mantissa < 0 ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  precision = 0;
}

CS_FORCE_INLINE void csReal::set(bool evaluate, const char* _number)
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
  size_t expPos = len;
  for (size_t i = start; i < len; ++i) {
    if (_number[i] == 'e' || _number[i] == 'E') {
      expPos = i;
      break;
    }
  }
  size_t dot = expPos;
  for (size_t i = start; i < expPos; ++i) {
    if (_number[i] == '.') {
      dot = i;
      break;
    }
  }
  size_t frac = dot < expPos ? dot + 1 : expPos;
  size_t whole = dot > start ? dot - start : 0;
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
  csLimbs_q n;
  csFromDigits_r(digs, n);
  free(digs);
  csSave_r(*this, n);
  exponent = exp;
  sign = n.isZero() ? CS_POSITIVE_NUMBER : neg;
  precision = 0;
}

CS_FORCE_INLINE void csReal::setPowerOfTen(long power)
{
  csLimbs_q n;
  n.setOne();
  csSave_r(*this, n);
  exponent = (int)power;
  sign = CS_POSITIVE_NUMBER;
  precision = 0;
}

CS_FORCE_INLINE void csReal::init()
{
  mantissa=0;
}

CS_FORCE_INLINE csReal* CSARITHMETIC_API CSARITHMETIC::csPtrAlloc_r(size_t nb)
{
  csReal *rn = csAlloc<csReal>(nb);
  for(size_t i=0; i<nb; i++)
  {
    rn[i].init();
  }
  return rn;
}

CS_FORCE_INLINE csReal* CSARITHMETIC_API CSARITHMETIC::csPtrAlloc_r(size_t nb, csReal init)
{
  csReal *rn = csAlloc<csReal>(nb);
  for(size_t i=0; i<nb; i++)
  {
    rn[i].init();
    rn[i].set(init.mantSize,init.exponent,init.sign);
  }
  return rn;
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csPtrFree_r(csReal*& rn, size_t nb)
{
  for(size_t i=0; i<nb; i++)
  {
    rn[i].clear();
  }
  free(rn);
}

CS_FORCE_INLINE void csReal::random(size_t nDigits, char digitMin, char digitMax, long exponent, bool sign)
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

CS_FORCE_INLINE void csReal::setPrecision(int size)
{
  if (exponent <= 0 && -exponent > size)
    return;
  int diff = exponent + size;
  if (diff > 0) {
    csLimbs_q n;
    csLoad_r(*this, n);
    csMulPow10_r(n, diff);
    csSave_r(*this, n);
  }
  exponent = -size;
}

CS_FORCE_INLINE size_t csReal::significantZeroCount(size_t initialPos)
{
  char* s = csExportDigits_r(*this);
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

/**
 * @brief Adds one to the number.
 * @param nb Number of elements.
 * @param size Requested size.
 */
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

CS_FORCE_INLINE void csReal::setDecimalPlaces(int size)
{
  char* s = csExportDigits_r(*this);
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
  csLimbs_q v;
  csFromDigits_r(s, v);
  free(s);
  csSave_r(*this, v);
  exponent = exp;
  sign = v.isZero() ? CS_POSITIVE_NUMBER : sg;
  precision = pr;
}

CS_FORCE_INLINE void csReal::resizeMantissa(size_t newMantissaSize)
{
  csLimbs_q n;
  csLoad_r(*this, n);
  int d = csDecDigits_r(n);
  if (newMantissaSize > (size_t)d)
    extendMantissa(newMantissaSize - (size_t)d);
  else if (newMantissaSize < (size_t)d)
    shortenMantissa((size_t)d - newMantissaSize);
}

CS_FORCE_INLINE void csReal::extendMantissa(size_t size)
{
  if (!size)
    return;
  csLimbs_q n;
  csLoad_r(*this, n);
  csMulPow10_r(n, (int)size);
  csSave_r(*this, n);
  exponent -= (int)size;
}

CS_FORCE_INLINE void csReal::shortenMantissa(size_t size)
{
  if (!size)
    return;
  csLimbs_q n;
  csLoad_r(*this, n);
  csDivPow10_r(n, (int)size);
  csSave_r(*this, n);
  exponent += (int)size;
  if (n.isZero())
    sign = CS_POSITIVE_NUMBER;
}

CS_FORCE_INLINE void csReal::trimTrailingZeros()
{
  csLimbs_q n;
  csLoad_r(*this, n);
  int k = csTrim10_r(n);
  csSave_r(*this, n);
  exponent += k;
}

CS_FORCE_INLINE size_t csReal::digitCount()
{
  csLimbs_q n;
  csLoad_r(*this, n);
  int d = csDecDigits_r(n);
  if (exponent >= 0)
    return (size_t)d + (size_t)exponent;
  int ae = exponent < 0 ? -exponent : exponent;
  return (size_t)(d > ae ? d : ae);
}

CS_FORCE_INLINE csReal csReal::abs()
{
  csReal rn;
  csLimbs_q n;
  csLoad_r(*this, n);
  csSave_r(rn, n);
  rn.exponent = exponent;
  rn.sign = CS_POSITIVE_NUMBER;
  rn.precision = precision;
  return rn;
}

CS_FORCE_INLINE csReal csReal::operator+(long a)
{
  
  csReal rn(a);
  csReal b = (*this + rn);
  rn.clear();
  return b;
}
CS_FORCE_INLINE csReal csReal::operator+(const csReal& _a)
{
  csReal rn;
  csLimbs_q left, right, sum;
  csLoad_r(*this, left);
  csLoad_r(_a, right);
  int exp = exponent < _a.exponent ? exponent : _a.exponent;
  if (exponent > exp)
    csMulPow10_r(left, exponent - exp);
  if (_a.exponent > exp)
    csMulPow10_r(right, _a.exponent - exp);
  int outNeg = CS_POSITIVE_NUMBER;
  csCombine_r(left, sign ? 1 : 0, right, _a.sign ? 1 : 0, sum, outNeg);
  csSave_r(rn, sum);
  rn.sign = outNeg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  rn.exponent = exp;
  rn.precision = _a.precision > precision ? _a.precision : precision;
  return rn;
}

CS_FORCE_INLINE csReal csReal::operator-(long a)
{
  
  csReal rn(a);
  csReal b = (*this - rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE csReal csReal::operator-(const csReal& _a)
{
  csReal rn;
  csLimbs_q left, right, sum;
  csLoad_r(*this, left);
  csLoad_r(_a, right);
  int exp = exponent < _a.exponent ? exponent : _a.exponent;
  if (exponent > exp)
    csMulPow10_r(left, exponent - exp);
  if (_a.exponent > exp)
    csMulPow10_r(right, _a.exponent - exp);
  int outNeg = CS_POSITIVE_NUMBER;
  int rightSign = _a.sign ? 0 : 1;
  csCombine_r(left, sign ? 1 : 0, right, rightSign, sum, outNeg);
  csSave_r(rn, sum);
  rn.sign = outNeg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  rn.exponent = exp;
  rn.precision = _a.precision > precision ? _a.precision : precision;
  return rn;
}

CS_FORCE_INLINE csReal csReal::operator*(long a)
{
  
  csReal rn(a);
  csReal b = (*this * rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE csReal csReal::operator*(double a)
{
  csReal rn(a);
  csReal b = (*this * rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE csReal csReal::operator/(double a)
{
  csReal rn(a);
  csReal b = (*this / rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE csReal csReal::operator+(double a)
{
  csReal rn(a);
  csReal b = (*this + rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE csReal csReal::operator-(double a)
{
  csReal rn(a);
  csReal b = (*this - rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE csReal csReal::operator*(const csReal& _a)
{
  csReal rn;
  csLimbs_q left, right, prod;
  csLoad_r(*this, left);
  csLoad_r(_a, right);
  csMul_q(left.p, left.n, right.p, right.n, prod);
  csSave_r(rn, prod);
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

CS_FORCE_INLINE csReal csReal::operator/(long a)
{
  
  csReal rn(a);
  csReal b = (*this / rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE csReal csReal::integerQuotient(long n)
{
  csReal x = integer();
  csReal q = x / n;
  if (q.exponent < 0)
    q.shortenMantissa((size_t)(-q.exponent));
  return q;
}

CS_FORCE_INLINE csReal csReal::integer()
{
  csReal x;
  x = *this;
  if (x.exponent > 0)
    x.extendMantissa((size_t)x.exponent);
  else if (x.exponent < 0)
    x.shortenMantissa((size_t)(-x.exponent));
  return x;
}

CS_FORCE_INLINE csReal csReal::operator/(const csReal& _a)
{
  csReal rn;
  long precSize = RPRECISION + (_a.precision > precision ? _a.precision : precision);
  csLimbs_q left, right, quot, rem;
  csLoad_r(*this, left);
  csLoad_r(_a, right);
  long mag = (long)csDecDigits_r(right) + _a.exponent - ((long)csDecDigits_r(left) + exponent);
  if (mag > precSize) {
    rn.set("0", 0, 0);
    return rn;
  }
  int dropped = csTrim10_r(right);
  int scale = (int)precSize + exponent - (_a.exponent + dropped);
  if (scale > 0)
    csMulPow10_r(left, scale);
  csDivMod_q(left.p, left.n, right.p, right.n, quot, rem);
  csSave_r(rn, quot);
  rn.exponent = scale > 0 ? -(int)precSize : exponent - (_a.exponent + dropped);
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

CS_FORCE_INLINE csReal csReal::operator-() const
{
  csReal rn;
  csLimbs_q n;
  csLoad_r(*this, n);
  csSave_r(rn, n);
  rn.exponent = exponent;
  rn.sign = n.isZero() ? CS_POSITIVE_NUMBER : (sign ? CS_POSITIVE_NUMBER : CS_NEGATIVE_NUMBER);
  rn.precision = precision;
  return rn;
}

CS_FORCE_INLINE void csReal::clear()
{
  if(mantissa)
  {
    free(mantissa);
    mantissa = 0;
    sign = 0;
    exponent = 0;
    mantSize = 0;
    precision = 0;
    mantissa = 0;
  }
}

CS_FORCE_INLINE void csReal::copy(const csReal& a)
{
  if (this == &a || mantissa == a.mantissa)
    return;
  csLimbs_q n;
  csLoad_r(a, n);
  csSave_r(*this, n);
  sign = a.sign;
  exponent = a.exponent;
  precision = a.precision > precision ? a.precision : precision;
}

CS_FORCE_INLINE csReal& csReal::operator=(csReal a)
{
  char* m = mantissa;
  mantissa = a.mantissa;
  a.mantissa = m;
  int e = exponent;
  exponent = a.exponent;
  a.exponent = e;
  bool s = sign;
  sign = a.sign;
  a.sign = s;
  long p = precision;
  precision = a.precision;
  a.precision = p;
  size_t z = mantSize;
  mantSize = a.mantSize;
  a.mantSize = z;
  return *this;
}

CS_FORCE_INLINE void csReal::operator=(double a)
{
  csReal rn(a);
  *this = rn;
  rn.clear();
}

CS_FORCE_INLINE void csReal::operator=(long a)
{
  int exp = exponent;
  long pr = precision;
  uint64_t mag = a < 0 ? (uint64_t)(-a) : (uint64_t)a;
  csLimbs_q n;
  n.setU64(mag);
  bool sg = a < 0 ? CS_NEGATIVE_NUMBER : (a ? CS_POSITIVE_NUMBER : sign);
  if (!a)
    sg = sign;
  csSave_r(*this, n);
  exponent = exp;
  precision = pr;
  sign = n.isZero() ? CS_POSITIVE_NUMBER : sg;
}

CS_FORCE_INLINE void csReal::operator=(const char*a)
{
  size_t l = strlen(a);
  if(a && l)
  {
    if(mantissa)
    {
      free(mantissa);
      mantissa = 0;
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
    
    
    csReal rn(eval, a + start);
    sign = rn.sign;
    mantSize = rn.mantSize;
    exponent = rn.exponent;
    mantissa = rn.mantissa;
    precision = rn.precision > precision ? rn.precision : precision;
  }
}

CS_FORCE_INLINE void csReal::copyFrom(const csReal& a)
{
  *this = a;
}
CS_FORCE_INLINE void csReal::setDigit(size_t id, char digit)
{
  char* s = csExportDigits_r(*this);
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
  csLimbs_q v;
  csFromDigits_r(s, v);
  free(s);
  csSave_r(*this, v);
  exponent = exp;
  sign = v.isZero() ? CS_POSITIVE_NUMBER : sg;
  precision = pr;
}
CS_FORCE_INLINE void csReal::setIntegerDigit(size_t id, char digit)
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

/**
 * @brief Exact rational, numerator and denominator in limbs.
 */
CS_FORCE_INLINE csReal::operator csRational()
{
  char* digs = csExportDigits_r(*this);
  size_t n = strlen(digs);
  csRational qn;
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

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::formatReal(const csReal& a) // a corriger car augmente une case vide a la fin de res, ajoutant sa taille reelle
{
  char* digs = csExportDigits_r(a);
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

CS_FORCE_INLINE void csReal::print(const char*title)
{
  char*formated = formatReal(*this);
  cout<<"\n"<<title<<formated<<"\n";
  free(formated);
}

CS_FORCE_INLINE void csReal::printScientific(const char*title)
{
  char* s = csExportDigits_r(*this);
  if (sign == CS_POSITIVE_NUMBER)
    cout << "\n " << title << s << "x10^" << exponent << "\n";
  else
    cout << "\n " << title << "-" << s << "x10^" << exponent << "\n";
  free(s);
}

/**
 * @brief Releases the buffers and returns the result.
 * @param a First operand.
 * @param b Second operand.
 * @param cndResult Parameter @p cndResult.
 * @return True when the condition holds.
 */
CS_FORCE_INLINE bool releaseAndReturn(csReal& a, csReal& b, bool cndResult)
{
  a.clear();
  b.clear();
  return cndResult;
}

CS_FORCE_INLINE bool csReal::operator!=(const csReal& a)
{
  return !(*this == a);
}

CS_FORCE_INLINE bool csReal::operator!=(long a)
{
  
  csReal rn(a);
  bool b = !(*this == rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csReal::operator==(long a)
{
  
  csReal rn(a);
  bool b = (*this == rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csReal::isZero()
{
  csLimbs_q n;
  csLoad_r(*this, n);
  return n.isZero();
}

CS_FORCE_INLINE bool csReal::operator==(const csReal& _a)
{
  
  return csCmpValue_r(*this, _a) == 0;
}

CS_FORCE_INLINE bool csReal::operator<(long a)
{
  
  csReal rn(a);
  bool b = (*this < rn);
  rn.clear();
  return b;
}
CS_FORCE_INLINE bool csReal::operator<(const csReal& _a)
{
  
  return csCmpValue_r(*this, _a) < 0;
}

CS_FORCE_INLINE bool csReal::operator<=(long a)
{
  
  csReal rn(a);
  bool b = (*this <= rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csReal::operator<=(const csReal& _a)
{
  
  return csCmpValue_r(*this, _a) <= 0;
}

CS_FORCE_INLINE bool csReal::operator>(long a)
{
  
  csReal rn(a);
  bool b = (*this > rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csReal::operator>(const csReal& _a)
{
  
  return csCmpValue_r(*this, _a) > 0;
}

CS_FORCE_INLINE bool csReal::operator>=(long a)
{
  
  csReal rn(a);
  bool b = (*this >= rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csReal::operator>=(const csReal& _a)
{
  
  return csCmpValue_r(*this, _a) >= 0;
}

CS_FORCE_INLINE csReal csReal::mantissaSection(size_t first, size_t last)
{
  csReal rn;
  char* s = csExportDigits_r(*this);
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
  csLimbs_q v;
  csFromDigits_r(slice, v);
  free(slice);
  free(s);
  csSave_r(rn, v);
  rn.exponent = exponent;
  rn.sign = v.isZero() ? CS_POSITIVE_NUMBER : sign;
  rn.precision = precision;
  return rn;
}

using namespace __mem_man;
using namespace __ar_man;


static void csLoadMagnitude_z(const csInteger& number, csLimbs_q& magnitude)
{
  if (!number.extended && number.count == 0)
    magnitude.setU64(number.value);
  else if (number.count == 0 || (number.extended && !number.limbs))
    magnitude.setZero();
  else
    magnitude.copyFrom(number.extended ? (const uint32_t*)number.limbs : number.stack, number.count);
}

static void csStoreMagnitude_z(csInteger& number, const csLimbs_q& magnitude, bool negative)
{
  uint32_t none = 0;
  const uint32_t* digits = magnitude.isZero() ? &none : magnitude.p;
  size_t limbCount = magnitude.isZero() ? 0 : magnitude.n;
  csStoreRaw_z(number, digits, limbCount, negative && !magnitude.isZero());
}

static int csCompareMagnitude_z(const csInteger& left, const csInteger& right)
{
  if (!left.extended && left.count == 0 && !right.extended && right.count == 0)
  {
    if (left.value < right.value)
      return -1;
    if (left.value > right.value)
      return 1;
    return 0;
  }
  uint32_t leftWord[2], rightWord[2];
  const uint32_t* leftDigits = 0;
  const uint32_t* rightDigits = 0;
  size_t leftCount = 0, rightCount = 0;
  csMaterialize_z(left, leftDigits, leftCount, leftWord);
  csMaterialize_z(right, rightDigits, rightCount, rightWord);
  return csCmpMagPtr_z(leftDigits, leftCount, rightDigits, rightCount);
}

static int csCompareSigned_z(const csInteger& left, const csInteger& right)
{
  if (left.isZero() && right.isZero())
    return 0;
  if (left.negative != right.negative)
    return left.negative ? -1 : 1;
  int magnitude = csCompareMagnitude_z(left, right);
  return left.negative ? -magnitude : magnitude;
}

CS_FORCE_INLINE csInteger::csInteger(long long number)
{
  set(number);
}

CS_FORCE_INLINE csInteger::csInteger(const char* digits)
{
  set(digits);
}

CS_FORCE_INLINE void csInteger::copy(const csInteger& other)
{
  *this = other;
}

CS_FORCE_INLINE void csInteger::set(long long number)
{
  if (number >= 0)
  {
    set((unsigned long long)number, false);
    return;
  }
  unsigned long long magnitude = (unsigned long long)(-(number + 1)) + 1ull;
  set(magnitude, true);
}

CS_FORCE_INLINE void csInteger::set(unsigned long long number, bool negativeSign)
{
  clear();
  value = number;
  negative = negativeSign && number != 0;
}

CS_FORCE_INLINE void csInteger::set(const char* digits)
{
  clear();
  if (!digits)
    return;
  while (*digits == ' ' || *digits == '\t')
    ++digits;
  bool negativeSign = false;
  if (*digits == '+' || *digits == '-')
  {
    negativeSign = *digits == '-';
    ++digits;
  }
  char* raw = 0;
  size_t rawCount = 0;
  csFromDec_q(digits, raw, rawCount);
  csLimbs_q magnitude;
  magnitude.copyFrom(raw ? (const uint32_t*)raw : 0, rawCount);
  if (raw)
    free(raw);
  csStoreMagnitude_z(*this, magnitude, negativeSign);
}

CS_FORCE_INLINE void csInteger::assignMagnitude(const uint32_t* digits, size_t limbCount, bool negativeSign)
{
  if (!digits)
    limbCount = 0;
  while (limbCount > 0 && digits[limbCount - 1] == 0)
    --limbCount;
  csLimbs_q magnitude;
  if (limbCount == 0)
    magnitude.setZero();
  else
    magnitude.copyFrom(digits, limbCount);
  csStoreMagnitude_z(*this, magnitude, negativeSign && !magnitude.isZero());
}

CS_FORCE_INLINE csInteger& csInteger::widenAdd(const csInteger& other)
{
  csLimbs_q left, right, sum;
  csLoadMagnitude_z(*this, left);
  csLoadMagnitude_z(other, right);
  bool negativeSign = false;
  if (negative == other.negative)
  {
    csAdd_q(left.p, left.n, right.p, right.n, sum);
    negativeSign = negative;
  }
  else
  {
    bool flipped = false;
    csSubAbs_q(left.p, left.n, right.p, right.n, sum, flipped);
    negativeSign = sum.isZero() ? false : (flipped ? other.negative : negative);
  }
  csStoreMagnitude_z(*this, sum, negativeSign);
  return *this;
}

CS_FORCE_INLINE csInteger& csInteger::widenMul(const csInteger& other)
{
  if (isZero() || other.isZero())
  {
    clear();
    return *this;
  }
  bool negativeSign = negative != other.negative;
  csLimbs_q left, right, product;
  csLoadMagnitude_z(*this, left);
  csLoadMagnitude_z(other, right);
  csMul_q(left.p, left.n, right.p, right.n, product);
  csStoreMagnitude_z(*this, product, negativeSign);
  return *this;
}

CS_FORCE_INLINE csInteger csInteger::quotientAndRemainder(const csInteger& divisor, csInteger& remainder) const
{
  csInteger quotient;
  if (divisor.isZero())
  {
    remainder.clear();
    return quotient;
  }
  if (!extended && count == 0 && !divisor.extended && divisor.count == 0)
  {
    uint64_t quot = divisor.value ? value / divisor.value : 0;
    uint64_t rest = divisor.value ? value % divisor.value : 0;
    quotient.set(quot, negative != divisor.negative);
    remainder.set(rest, negative);
    return quotient;
  }
  csLimbs_q left, right, quot, rest;
  csLoadMagnitude_z(*this, left);
  csLoadMagnitude_z(divisor, right);
  csDivMod_q(left.p, left.n, right.p, right.n, quot, rest);
  csStoreMagnitude_z(quotient, quot, negative != divisor.negative);
  csStoreMagnitude_z(remainder, rest, negative);
  return quotient;
}

CS_FORCE_INLINE csInteger csInteger::operator/(const csInteger& other) const
{
  csInteger remainder;
  return quotientAndRemainder(other, remainder);
}

CS_FORCE_INLINE csInteger csInteger::operator%(const csInteger& other) const
{
  csInteger remainder;
  quotientAndRemainder(other, remainder);
  return remainder;
}

CS_FORCE_INLINE csInteger& csInteger::operator=(long long number)
{
  set(number);
  return *this;
}

CS_FORCE_INLINE bool csInteger::operator==(const csInteger& other) const
{
  return csCompareSigned_z(*this, other) == 0;
}

CS_FORCE_INLINE bool csInteger::operator!=(const csInteger& other) const
{
  return csCompareSigned_z(*this, other) != 0;
}

CS_FORCE_INLINE bool csInteger::operator<(const csInteger& other) const
{
  return csCompareSigned_z(*this, other) < 0;
}

CS_FORCE_INLINE bool csInteger::operator>(const csInteger& other) const
{
  return csCompareSigned_z(*this, other) > 0;
}

CS_FORCE_INLINE bool csInteger::operator<=(const csInteger& other) const
{
  return csCompareSigned_z(*this, other) <= 0;
}

CS_FORCE_INLINE bool csInteger::operator>=(const csInteger& other) const
{
  return csCompareSigned_z(*this, other) >= 0;
}

CS_FORCE_INLINE csInteger csInteger::power(size_t exponent) const
{
  csInteger result(1);
  csInteger base(*this);
  while (exponent)
  {
    if (exponent & 1u)
      result *= base;
    exponent >>= 1;
    if (exponent)
      base *= base;
  }
  return result;
}

CS_FORCE_INLINE csRational csInteger::toRational() const
{
  csRational result(csRaw_q{});
  csLimbs_q magnitude;
  csLoadMagnitude_z(*this, magnitude);
  result.numerator = magnitude.leak();
  result.numSize = magnitude.n ? magnitude.n : 1;
  result.denominator = csLimbOne_q();
  result.denomSize = 1;
  result.sign = isZero() ? CS_POSITIVE_NUMBER : (negative ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER);
  return result;
}

CS_FORCE_INLINE char* csInteger::toDecimal() const
{
  csLimbs_q magnitude;
  csLoadMagnitude_z(*this, magnitude);
  char* text = csLimbsToDec(magnitude.p, magnitude.n);
  if (!negative || isZero())
    return text;
  size_t length = strlen(text);
  char* signedText = (char*)malloc(length + 2);
  signedText[0] = '-';
  memcpy(signedText + 1, text, length + 1);
  free(text);
  return signedText;
}

CS_FORCE_INLINE void csInteger::print(const char* title) const
{
  char* text = toDecimal();
  cout << "\n" << title << text << "\n";
  free(text);
}
