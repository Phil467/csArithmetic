// Multiplication by a modular Fourier transform.
// Each 32-bit limb is split into two 16-bit digits.
// The convolution product fits in a single modulus, then the carry
// rebuilds base 2^32. Below the threshold, Karatsuba or Toom-3 stay in place.

static const uint64_t csFftPrime_q = 2305843010555871233ULL;
static const uint64_t csFftNinv_q = 504403159607672831ULL;
static const uint64_t csFftR2_q = 104689827854ULL;
static const uint64_t csFftR1_q = 2305842999818452985ULL;
static const uint64_t csFftRoot_q = 3ULL;
static const int csFftMaxLog_q = 28;

static int csFftCutoff_q = 1024;

/**
 * @brief Computes a product in the Fourier-transform modulus.
 * @param a First operand.
 * @param b Second operand.
 * @return Computed value.
 */
CS_INLINE uint64_t csModMul_q(uint64_t a, uint64_t b) {
  unsigned __int128 t = (unsigned __int128)a * b;
  return (uint64_t)(t % csFftPrime_q);
}

/**
 * @brief Computes a Montgomery product in this modulus.
 * @param a First operand.
 * @param b Second operand.
 * @return Computed value.
 */
CS_INLINE uint64_t csMontMul_q(uint64_t a, uint64_t b) {
  unsigned __int128 t = (unsigned __int128)a * b;
  uint64_t m = (uint64_t)t * csFftNinv_q;
  unsigned __int128 sum = t + (unsigned __int128)m * csFftPrime_q;
  uint64_t r = (uint64_t)(sum >> 64);
  if (r >= csFftPrime_q)
    r -= csFftPrime_q;
  return r;
}

/**
 * @brief Computes a power in the Fourier modulus.
 * @param a First operand.
 * @param e Parameter @p e.
 * @return Computed value.
 */
CS_INLINE uint64_t csPowMod_q(uint64_t a, uint64_t e) {
  uint64_t r = 1;
  while (e) {
    if (e & 1)
      r = csModMul_q(r, a);
    a = csModMul_q(a, a);
    e >>= 1;
  }
  return r;
}

/**
 * @brief Returns the smallest power of two at least equal to @p n.
 * @param n Number of elements.
 * @return Computed value.
 */
CS_INLINE size_t csNextPow2_q(size_t n) {
  size_t p = 1;
  while (p < n) {
    if (p > ((size_t)1 << (csFftMaxLog_q - 1)))
      return 0;
    p <<= 1;
  }
  return p;
}

/**
 * @brief Reports whether the product fits in the Fourier modulus.
 * @param na Number of limbs of the first operand.
 * @param nb Number of elements.
 * @return Computed value.
 */
CS_INLINE int csFftFits_q(size_t na, size_t nb) {
  if (na > ((size_t)1 << (csFftMaxLog_q - 1)) || nb > ((size_t)1 << (csFftMaxLog_q - 1)))
    return 0;
  
  return csNextPow2_q((na + nb) * 2) != 0;
}

/**
 * @brief Computes the modular Fourier transform, forward or inverse.
 * @param a First operand.
 * @param n Number of elements.
 * @param inverse Inverts the transform when the value is non-zero.
 */
CS_INLINE void csNtt_q(uint64_t* a, size_t n, int inverse) {
  for (size_t i = 1, j = 0; i < n; ++i) {
    size_t bit = n >> 1;
    for (; j & bit; bit >>= 1)
      j ^= bit;
    j ^= bit;
    if (i < j) {
      uint64_t tmp = a[i];
      a[i] = a[j];
      a[j] = tmp;
    }
  }
  for (size_t len = 2; len <= n; len <<= 1) {
    uint64_t wlen = csPowMod_q(csFftRoot_q, (csFftPrime_q - 1) / len);
    if (inverse)
      wlen = csPowMod_q(wlen, csFftPrime_q - 2);
    wlen = csMontMul_q(wlen, csFftR2_q);
    size_t half = len >> 1;
    for (size_t i = 0; i < n; i += len) {
      uint64_t w = csFftR1_q;
      for (size_t j = 0; j < half; ++j) {
        uint64_t u = a[i + j];
        uint64_t v = csMontMul_q(a[i + j + half], w);
        uint64_t sum = u + v;
        if (sum >= csFftPrime_q)
          sum -= csFftPrime_q;
        uint64_t dif = u + csFftPrime_q - v;
        if (dif >= csFftPrime_q)
          dif -= csFftPrime_q;
        a[i + j] = sum;
        a[i + j + half] = dif;
        w = csMontMul_q(w, wlen);
      }
    }
  }
}

/**
 * @brief Computes the product of two integers by a Fourier transform.
 * @param a First operand.
 * @param na Number of limbs of the first operand.
 * @param b Second operand.
 * @param nb Number of elements.
 * @param r Limb array.
 */
CS_INLINE void csFftMul_q(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, uint32_t* r) {
  size_t need = (na + nb) * 2;
  size_t L = csNextPow2_q(need);
  uint64_t* A = (uint64_t*)malloc(L * sizeof(uint64_t));
  uint64_t* B = (uint64_t*)malloc(L * sizeof(uint64_t));
  if (!A || !B || L == 0) {
    free(A);
    free(B);
    csKaraOverflow_q = 1;
    memset(r, 0, (na + nb) * sizeof(uint32_t));
    return;
  }
  memset(A, 0, L * sizeof(uint64_t));
  memset(B, 0, L * sizeof(uint64_t));
  for (size_t i = 0; i < na; ++i) {
    A[2 * i] = csMontMul_q((uint64_t)(a[i] & 0xFFFFu), csFftR2_q);
    A[2 * i + 1] = csMontMul_q((uint64_t)(a[i] >> 16), csFftR2_q);
  }
  for (size_t i = 0; i < nb; ++i) {
    B[2 * i] = csMontMul_q((uint64_t)(b[i] & 0xFFFFu), csFftR2_q);
    B[2 * i + 1] = csMontMul_q((uint64_t)(b[i] >> 16), csFftR2_q);
  }
  csNtt_q(A, L, 0);
  csNtt_q(B, L, 0);
  for (size_t i = 0; i < L; ++i)
    A[i] = csMontMul_q(A[i], B[i]);
  csNtt_q(A, L, 1);
  uint64_t ninv = csPowMod_q(L % csFftPrime_q, csFftPrime_q - 2);
  uint64_t carry = 0;
  uint32_t* dig = (uint32_t*)malloc(L * sizeof(uint32_t));
  if (!dig) {
    free(A);
    free(B);
    csKaraOverflow_q = 1;
    memset(r, 0, (na + nb) * sizeof(uint32_t));
    return;
  }
  for (size_t i = 0; i < L; ++i) {
    uint64_t v = csMontMul_q(A[i], ninv) + carry;
    dig[i] = (uint32_t)(v & 0xFFFFu);
    carry = v >> 16;
  }
  free(A);
  free(B);
  if (carry) {
    free(dig);
    csKaraOverflow_q = 1;
    memset(r, 0, (na + nb) * sizeof(uint32_t));
    return;
  }
  for (size_t i = need; i < L; ++i) {
    if (dig[i]) {
      free(dig);
      csKaraOverflow_q = 1;
      memset(r, 0, (na + nb) * sizeof(uint32_t));
      return;
    }
  }
  if ((na + nb) * 2 > L) {
    free(dig);
    csKaraOverflow_q = 1;
    memset(r, 0, (na + nb) * sizeof(uint32_t));
    return;
  }
  for (size_t i = 0; i < na + nb; ++i)
    r[i] = dig[2 * i] | (dig[2 * i + 1] << 16);
  free(dig);
}
