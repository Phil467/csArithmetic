// Multiplication by a modular Fourier transform.
// Each 32-bit limb is split into two 16-bit digits.
// The convolution product fits in a single modulus, then the carry
// rebuilds base 2^32. Below the threshold, Karatsuba or Toom-3 stay in place.

static const uint64_t QFFT_P = 2305843010555871233ULL;
static const uint64_t QFFT_NINV = 504403159607672831ULL;
static const uint64_t QFFT_R2 = 104689827854ULL;
static const uint64_t QFFT_R1 = 2305842999818452985ULL;
static const uint64_t QFFT_ROOT = 3ULL;
static const int QFFT_MAX_LOG = 28;

static int qFftCutoff = 1024;

CS_INLINE uint64_t qModMul(uint64_t a, uint64_t b) {
  unsigned __int128 t = (unsigned __int128)a * b;
  return (uint64_t)(t % QFFT_P);
}

CS_INLINE uint64_t qMontMul(uint64_t a, uint64_t b) {
  unsigned __int128 t = (unsigned __int128)a * b;
  uint64_t m = (uint64_t)t * QFFT_NINV;
  unsigned __int128 sum = t + (unsigned __int128)m * QFFT_P;
  uint64_t r = (uint64_t)(sum >> 64);
  if (r >= QFFT_P)
    r -= QFFT_P;
  return r;
}

CS_INLINE uint64_t qPowMod(uint64_t a, uint64_t e) {
  uint64_t r = 1;
  while (e) {
    if (e & 1)
      r = qModMul(r, a);
    a = qModMul(a, a);
    e >>= 1;
  }
  return r;
}

CS_INLINE size_t qNextPow2(size_t n) {
  size_t p = 1;
  while (p < n) {
    if (p > ((size_t)1 << (QFFT_MAX_LOG - 1)))
      return 0;
    p <<= 1;
  }
  return p;
}

CS_INLINE int qFftFits(size_t na, size_t nb) {
  if (na > ((size_t)1 << (QFFT_MAX_LOG - 1)) || nb > ((size_t)1 << (QFFT_MAX_LOG - 1)))
    return 0;
  return qNextPow2((na + nb) * 2) != 0;
}

CS_INLINE void qNtt(uint64_t* a, size_t n, int inverse) {
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
    uint64_t wlen = qPowMod(QFFT_ROOT, (QFFT_P - 1) / len);
    if (inverse)
      wlen = qPowMod(wlen, QFFT_P - 2);
    wlen = qMontMul(wlen, QFFT_R2);
    size_t half = len >> 1;
    for (size_t i = 0; i < n; i += len) {
      uint64_t w = QFFT_R1;
      for (size_t j = 0; j < half; ++j) {
        uint64_t u = a[i + j];
        uint64_t v = qMontMul(a[i + j + half], w);
        uint64_t sum = u + v;
        if (sum >= QFFT_P)
          sum -= QFFT_P;
        uint64_t dif = u + QFFT_P - v;
        if (dif >= QFFT_P)
          dif -= QFFT_P;
        a[i + j] = sum;
        a[i + j + half] = dif;
        w = qMontMul(w, wlen);
      }
    }
  }
}

CS_INLINE void qFftMul(const uint32_t* a, size_t na, const uint32_t* b, size_t nb, uint32_t* r) {
  size_t need = (na + nb) * 2;
  size_t L = qNextPow2(need);
  uint64_t* A = (uint64_t*)malloc(L * sizeof(uint64_t));
  uint64_t* B = (uint64_t*)malloc(L * sizeof(uint64_t));
  if (!A || !B || L == 0) {
    free(A);
    free(B);
    qKaraOverflow = 1;
    memset(r, 0, (na + nb) * sizeof(uint32_t));
    return;
  }
  memset(A, 0, L * sizeof(uint64_t));
  memset(B, 0, L * sizeof(uint64_t));
  for (size_t i = 0; i < na; ++i) {
    A[2 * i] = qMontMul((uint64_t)(a[i] & 0xFFFFu), QFFT_R2);
    A[2 * i + 1] = qMontMul((uint64_t)(a[i] >> 16), QFFT_R2);
  }
  for (size_t i = 0; i < nb; ++i) {
    B[2 * i] = qMontMul((uint64_t)(b[i] & 0xFFFFu), QFFT_R2);
    B[2 * i + 1] = qMontMul((uint64_t)(b[i] >> 16), QFFT_R2);
  }
  qNtt(A, L, 0);
  qNtt(B, L, 0);
  for (size_t i = 0; i < L; ++i)
    A[i] = qMontMul(A[i], B[i]);
  qNtt(A, L, 1);
  uint64_t ninv = qPowMod(L % QFFT_P, QFFT_P - 2);
  uint64_t carry = 0;
  uint32_t* dig = (uint32_t*)malloc(L * sizeof(uint32_t));
  if (!dig) {
    free(A);
    free(B);
    qKaraOverflow = 1;
    memset(r, 0, (na + nb) * sizeof(uint32_t));
    return;
  }
  for (size_t i = 0; i < L; ++i) {
    uint64_t v = qMontMul(A[i], ninv) + carry;
    dig[i] = (uint32_t)(v & 0xFFFFu);
    carry = v >> 16;
  }
  free(A);
  free(B);
  if (carry) {
    free(dig);
    qKaraOverflow = 1;
    memset(r, 0, (na + nb) * sizeof(uint32_t));
    return;
  }
  for (size_t i = need; i < L; ++i) {
    if (dig[i]) {
      free(dig);
      qKaraOverflow = 1;
      memset(r, 0, (na + nb) * sizeof(uint32_t));
      return;
    }
  }
  if ((na + nb) * 2 > L) {
    free(dig);
    qKaraOverflow = 1;
    memset(r, 0, (na + nb) * sizeof(uint32_t));
    return;
  }
  for (size_t i = 0; i < na + nb; ++i)
    r[i] = dig[2 * i] | (dig[2 * i + 1] << 16);
  free(dig);
}
