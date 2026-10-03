#pragma once

#if defined(CSARITHMETIC_STATIC)
  #define CSARITHMETIC_API
#elif defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_CSARITHMETIC_DLL
    #define CSARITHMETIC_API __declspec(dllexport)
  #else
    #define CSARITHMETIC_API __declspec(dllimport)
  #endif
#else
  #ifdef BUILDING_CSARITHMETIC_DLL
    #define CSARITHMETIC_API __attribute__ ((visibility ("default")))
  #else
    #define CSARITHMETIC_API
  #endif
#endif

/* Applies to the definitions. Declarations without a body stay callable from a DLL. */
#define CS_FORCE_INLINE inline __attribute__((always_inline))
#define CS_INLINE inline
#ifndef CS_FORCEINLINE
#define CS_FORCEINLINE CS_FORCE_INLINE
#endif

#ifndef CSARITHMETIC_OPT11_H
#define CSARITHMETIC_OPT11_H

#include <iostream>

namespace __mem_man
{
/**
 * @brief Allocates a memory block.
 * @param n Number of elements.
 * @return Result.
 */
template<class T> CS_FORCE_INLINE T* csAlloc(size_t n)
{
  T*t = (T*)malloc(n*sizeof(T));
  return t;
}

/**
 * @brief Allocates a memory block.
 * @param n Number of elements.
 * @param init Initial fill character.
 * @return Result.
 */
template<class T> CS_FORCE_INLINE T* csAlloc(size_t n, T init)
{
  T*t = (T*)malloc(n*sizeof(T));
  for(size_t i=0; i<n; i++)
  {
    t[i] = init;
  }
  return t;
}

/**
 * @brief Allocates a two-dimensional memory block.
 * @param n Number of elements.
 * @param init Initial fill character.
 * @return Result.
 */
template<class T> CS_FORCE_INLINE T* csAlloc2(size_t n, T init)
{
  T*t = (T*)malloc(n*sizeof(T));
  for(size_t i=0; i<n; i++)
  {
    t[i] = init;
  }
  return t;
}

/**
 * @brief Allocates a character array.
 * @param n Number of elements.
 * @param n1 Size of the second dimension.
 * @param init Initial fill character.
 * @return Resulting text. The caller frees the memory.
 */
char* CSARITHMETIC_API csAllocCharPtr(size_t n, size_t n1, char init);
/**
 * @brief Allocates a character array.
 * @param n Number of elements.
 * @param init Initial fill character.
 * @return Resulting text. The caller frees the memory.
 */
char* CSARITHMETIC_API csAllocCharPtr(size_t n, char init);

/**
 * @brief Reallocates a character string.
 * @param str Text to modify.
 * @param size Requested size.
 * @return Result.
 */
void CSARITHMETIC_API csReallocString(char** str, size_t size);
/**
 * @brief Reallocates a character string.
 * @param str Text to modify.
 * @param size Requested size.
 * @param newSize New size.
 * @param cFill Fill character.
 * @return Result.
 */
void CSARITHMETIC_API csReallocString(char** str, size_t size, size_t newSize, char cFill);

/**
 * @brief Copies a text into a new buffer.
 * @param cstr Source text.
 * @param size Requested size.
 * @return Resulting text. The caller frees the memory.
 */
char* CSARITHMETIC_API newString(char*cstr, size_t size);
/**
 * @brief Copies a text into a new buffer.
 * @param cstr Source text.
 * @param size Requested size.
 * @return Resulting text. The caller frees the memory.
 */
char* CSARITHMETIC_API newString(const char*cstr, size_t size);
/**
 * @brief Copies a text into a new buffer.
 * @param cstr Source text.
 * @param begin Start index, inclusive.
 * @param end End index, exclusive.
 * @return Resulting text. The caller frees the memory.
 */
char* CSARITHMETIC_API newString(const char*cstr, size_t begin, size_t end);
/**
 * @brief Copies a text into a new buffer.
 * @param cstr Source text.
 * @return Resulting text. The caller frees the memory.
 */
char* CSARITHMETIC_API newString(const char*cstr);
/**
 * @brief Converts a signed integer to text.
 * @param nb Number of elements.
 * @param sz Receives the size of the produced text.
 * @return Resulting text. The caller frees the memory.
 */
char* intToString(int nb, size_t& sz);
/**
 * @brief Converts an unsigned integer to text.
 * @param nb Number of elements.
 * @param sz Receives the size of the produced text.
 * @return Resulting text. The caller frees the memory.
 */
char* uLongToString(size_t nb, size_t& sz);

};

namespace __ar_man
{
/**
 * @brief Shifts the digits of a text to the right.
 * @param nb Number of elements.
 * @param size Requested size.
 * @param nShift Number of shift positions.
 */
void shiftRight(char*& nb, size_t size, size_t nShift);
/**
 * @brief Returns a copy of the text shifted to the right.
 * @param nb Number of elements.
 * @param nbSize Size of the number.
 * @param nbCopySize Size of the copy.
 * @return Resulting text. The caller frees the memory.
 */
char* shiftRightCopy(char* nb, size_t nbSize, size_t nbCopySize);
/**
 * @brief Shifts the digits of a text to the left.
 * @param nb Number of elements.
 * @param size Requested size.
 * @param nShift Number of shift positions.
 */
void shiftLeft(char*& nb, size_t size, size_t nShift);
/**
 * @brief Copies a text while shifting it to the left.
 * @param nb Number of elements.
 * @param nbCopy Produced copy.
 * @param nbSize Size of the number.
 * @param nbCopyPos Write position in the copy.
 */
void shiftLeftCopy(char* nb, char*&nbCopy, size_t nbSize, size_t nbCopyPos);
/**
 * @brief Skips the leading zeros of a text.
 * @param a First operand.
 * @param aSize Number of digits of the first text.
 * @param skipLen Number of skipped zeros.
 */
void skipZeros(char* a, size_t aSize, size_t& skipLen);
/**
 * @brief Skips the leading zeros and measures the step.
 * @param a First operand.
 * @param aSize Number of digits of the first text.
 * @param skipLen Number of skipped zeros.
 * @param incr Step associated with the skipped zeros.
 */
void skipZeros2(char* a, size_t aSize, size_t& skipLen, size_t&incr);
/**
 * @brief Reports whether the first text is greater than the second.
 * @param a First operand.
 * @param b Second operand.
 * @param opSize Number of aligned digits.
 * @param ibegin Index of the first digit compared.
 * @return True when the condition holds.
 */
bool isaGreater(char* a,char* b,size_t opSize, size_t ibegin);
/**
 * @brief Reports whether the first text is greater than or equal to the second.
 * @param a First operand.
 * @param b Second operand.
 * @param opSize Number of aligned digits.
 * @param ibegin Index of the first digit compared.
 * @return True when the condition holds.
 */
bool isaGreaterEqual(char* a,char* b,size_t opSize, size_t ibegin);
/**
 * @brief Reports whether the first text is greater than the second, at different sizes.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param ibegin Index of the first digit compared.
 * @return True when the condition holds.
 */
bool isaGreater2(char* a,char* b,size_t aSize,size_t bSize, size_t ibegin);
/**
 * @brief Reports whether the first text is greater than or equal to the second, at different sizes.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param ibegin Index of the first digit compared.
 * @return True when the condition holds.
 */
bool isaGreaterEqual2(char* a,char* b,size_t aSize, size_t bSize, size_t ibegin);

/**
 * @brief Prepares two texts for a subtraction.
 * @param a First operand.
 * @param b Second operand.
 * @param opSize Number of aligned digits.
 * @return Computed value.
 */
int getReadySub(char*& a,char*& b, size_t&opSize);
/**
 * @brief Prepares two source texts for a subtraction.
 * @param a0 Original text of the first operand.
 * @param b0 Original text of the second operand.
 * @param a First operand.
 * @param b Second operand.
 * @param opSize Number of aligned digits.
 * @return True when the condition holds.
 */
bool getReadySub2(const char* a0,const char* b0, char*& a, char*& b, size_t& opSize);
/**
 * @brief Prepares two texts of different sizes for a subtraction.
 * @param a0 Original text of the first operand.
 * @param b0 Original text of the second operand.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param opSize Number of aligned digits.
 * @return True when the condition holds.
 */
bool getReadySub3(const char* a0,const char* b0, char*& a, char*& b, size_t aSize, size_t bSize, size_t&opSize);

/**
 * @brief Aligns two texts before an operation.
 * @param a First operand.
 * @param b Second operand.
 * @param opSize Number of aligned digits.
 */
void getReady(char*& a,char*& b, size_t& opSize);
/**
 * @brief Aligns two source texts before an operation.
 * @param a0 Original text of the first operand.
 * @param b0 Original text of the second operand.
 * @param a First operand.
 * @param b Second operand.
 * @param opSize Number of aligned digits.
 */
void getReady2(const char* a0,const char* b0, char*& a, char*& b, size_t& opSize);

/**
 * @brief Removes leading zeros from a text.
 * @param a First operand.
 * @param size Requested size.
 * @return True when the condition holds.
 */
bool removeFrontZeros(char*& a, size_t& size);
/**
 * @brief Removes characters from the left of a text.
 * @param nb Number of elements.
 * @param size Requested size.
 * @param remLen Number of characters removed.
 */
void removeLeft(char*& nb, size_t& size, size_t remLen);
/**
 * @brief Removes characters from the right of a text.
 * @param nb Number of elements.
 * @param size Requested size.
 * @param remLen Number of characters removed.
 */
void removeRight(char*& nb, size_t& size, size_t remLen);
/**
 * @brief Adds leading zeros to a text.
 * @param a First operand.
 * @param size Requested size.
 * @param nZeros Number of zeros added.
 */
void addFrontZeros(char*& a, size_t& size, size_t nZeros);
/**
 * @brief Copies a text into an already allocated buffer.
 * @param str Text to modify.
 * @param cstr Source text.
 * @param size Requested size.
 */
void fillString(char*& str, const char* cstr, size_t size);
/**
 * @brief Returns a text padded to the requested size.
 * @param cstr Source text.
 * @param size Requested size.
 * @return Resulting text. The caller frees the memory.
 */
char* filledString(const char* cstr, size_t size);
}

#include <cstddef>
#include <math.h>
#include <time.h>
#include <random>
#include <iostream>
#include <string.h>
#include <complex>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <numeric>
#include <type_traits>
#include <cxxabi.h>
struct source_location {
  static source_location current() { return {}; }
};


#define CS_POSITIVE_NUMBER 0
#define CS_NEGATIVE_NUMBER 1

typedef unsigned char uchar;

using namespace std;


typedef struct
{
  uchar tens, units;
}csBIDIGITS;

typedef struct csFCOORDS
{
  long i, j;
};

namespace CSARITHMETIC
{

#define CS_BASE_Q (4294967296LL)
/**
 * @brief Converts limbs to decimal text.
 * @param raw Parameter @p raw.
 * @param n Number of elements.
 * @return Resulting text. The caller frees the memory.
 */
CS_INLINE static char* csLimbsToDec(const void* raw, size_t n)
{
  const uint32_t* src = (const uint32_t*)raw;
  if (!src || n == 0 || (n == 1 && src[0] == 0)) {
    char* z = (char*)malloc(2);
    z[0] = '0';
    z[1] = 0;
    return z;
  }
  uint32_t* a = (uint32_t*)malloc(n * sizeof(uint32_t));
  memcpy(a, src, n * sizeof(uint32_t));
  size_t nd = n;
  size_t maxc = n * 10 + 4;
  uint32_t* chunks = (uint32_t*)malloc(maxc * sizeof(uint32_t));
  size_t nc = 0;
  while (nd > 1 || (nd == 1 && a[0] != 0)) {
    uint64_t rem = 0;
    for (size_t i = nd; i-- > 0; ) {
      uint64_t cur = (rem << 32) | a[i];
      a[i] = (uint32_t)(cur / 1000000000ull);
      rem = cur % 1000000000ull;
    }
    if (nc >= maxc)
      break;
    chunks[nc++] = (uint32_t)rem;
    while (nd > 1 && a[nd - 1] == 0)
      --nd;
    if (nd == 1 && a[0] == 0)
      break;
  }
  free(a);
  if (nc == 0) {
    free(chunks);
    char* z = (char*)malloc(2);
    z[0] = '0';
    z[1] = 0;
    return z;
  }
  char* buf = (char*)malloc(nc * 9 + 2);
  char* w = buf;
  w += sprintf(w, "%u", chunks[nc - 1]);
  for (size_t i = nc - 1; i-- > 0; )
    w += sprintf(w, "%09u", chunks[i]);
  free(chunks);
  return buf;
}


  typedef struct csRational csRational;
  typedef struct csReal csReal;
/**
 * @brief Prepares the display text of a real.
 * @param a First operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* getPrintFormat(const csReal& a);
/**
 * @brief Sets the precision used for reals.
 * @param precision Requested precision, in digits.
 */
  void setRNumberPrecision(int precision);
/**
 * @brief Returns the precision used for reals.
 * @return Computed value.
 */
  int getRNumberPrecision();


  struct csRaw_q {};

  struct CSARITHMETIC_API csRational
  {
    char* numerator=0;
    char* denominator=0;
    bool sign = CS_POSITIVE_NUMBER;
    size_t numSize;
    size_t denomSize;

    csRational(const char* numerator="0", const char* denominator="1", bool sign=0);
    csRational(size_t num, size_t denom=1, bool sign=0);
/**
 * @brief Builds a rational from a signed integer.
 * @param num Signed integer of the numerator.
 * @param denom Positive denominator.
 */
    template<class T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type = 0>
    csRational(T num, size_t denom=1) : csRational(csRaw_q{})
    {
      setl((long)num, denom);
    }
    csRational(csRaw_q);
    csRational(const csRational& a);
    csRational(csRational&& a) noexcept;
/**
 * @brief Initializes the object or the library tables.
 */
    void init();
    ~csRational();
/**
 * @brief Assigns a new value.
 * @param numerator Decimal text of the numerator.
 * @param denominator Decimal text of the denominator.
 * @param sign Sign. Zero when the number is positive.
 */
    void set(const char* numerator, const char* denominator, bool sign);
/**
 * @brief Assigns a new value.
 * @param num Parameter @p num.
 * @param denom Parameter @p denom.
 * @param sign Sign. Zero when the number is positive.
 */
    void set(size_t num, size_t denom=1, bool sign=0);
/**
 * @brief Assigns a long integer.
 * @param num Parameter @p num.
 * @param denom Parameter @p denom.
 */
    void setl(long num, size_t denom=1);
/**
 * @brief Reduces the value to the requested size.
 * @param precision Requested precision, in digits.
 */
    void reduce(size_t precision);
    csRational operator+(const csRational& a);
    csRational operator-(const csRational& a);
    csRational operator*(const csRational& a);
    csRational operator/(const csRational& a);
    csRational operator+(long a);
    csRational operator-(long a);
    csRational operator*(long a);
    csRational operator/(long a);
    csRational& operator=(csRational a);
    csRational& operator=(long a);
    bool operator==(const csRational& a);
/**
 * @brief Reports whether the absolute values are equal.
 * @param a First operand.
 * @return True when the condition holds.
 */
    bool isAbsEqual(const csRational& a);
    bool operator!=(const csRational& a);
/**
 * @brief Reports whether the absolute values differ.
 * @param a First operand.
 * @return True when the condition holds.
 */
    bool isAbsDifferent(const csRational& a);
    bool operator>(const csRational& a);
/**
 * @brief Reports whether the absolute value is greater.
 * @param a First operand.
 * @return True when the condition holds.
 */
    bool isAbsGreater(const csRational& a);
    bool operator<(const csRational& a);
/**
 * @brief Reports whether the absolute value is less.
 * @param a First operand.
 * @return True when the condition holds.
 */
    bool isAbsLess(const csRational& a);
    bool operator>=(const csRational& a);
/**
 * @brief Reports whether the absolute value is greater than or equal.
 * @param a First operand.
 * @return True when the condition holds.
 */
    bool isAbsGreaterEqual(const csRational& a);
    bool operator<=(const csRational& a);
/**
 * @brief Reports whether the absolute value is less than or equal.
 * @param a First operand.
 * @return True when the condition holds.
 */
    bool isAbsLessEqual(const csRational& a);

/**
 * @brief Real number as a mantissa and an exponent.
 */
    operator csReal();
    CS_FORCE_INLINE friend std::ostream& operator<<(std::ostream& os, const csRational& a)
    {
      char* ns = csLimbsToDec(a.numerator, a.numSize);
      char* ds = csLimbsToDec(a.denominator, a.denomSize);
      if(a.sign)
        os << "-" << ns << " / " << ds;
      else
        os << ns << " / " << ds;
      free(ns);
      free(ds);
      return os;
    }
/**
 * @brief Reports whether the value is zero.
 * @return True when the condition holds.
 */
    bool isZero();
/**
 * @brief Reports whether the value is non-zero.
 * @return True when the condition holds.
 */
    bool isNonZero();
/**
 * @brief Copies the value.
 * @param a First operand.
 */
    void copy(const csRational& a);
/**
 * @brief Returns an approximation as a double.
 * @return Approximation as a double.
 */
    double getDouble();
/**
 * @brief Returns the larger of the two sizes.
 * @return Computed value.
 */
    size_t maxSize();
/**
 * @brief Returns the real quotient of the rational.
 * @return Resulting text. The caller frees the memory.
 */
    char* quotient();
/**
 * @brief Prints the value.
 * @param title Title printed before the values.
 */
    void print(const char*title);
/**
 * @brief Releases the memory held by the object.
 * @param loc Call site, for diagnostics.
 */
    void clear(const source_location loc = source_location::current());
  };

  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE csRational operator+(T n, csRational a) { return a + n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE csRational operator-(T n, csRational a) { return csRational((long)n) - a; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE csRational operator*(T n, csRational a) { return a * n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE csRational operator/(T n, csRational a) { return csRational((long)n) / a; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator==(T n, csRational a) { return a == n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator!=(T n, csRational a) { return a != n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator<(T n, csRational a) { return a > n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator>(T n, csRational a) { return a < n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator<=(T n, csRational a) { return a >= n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator>=(T n, csRational a) { return a <= n; }

/**
 * @brief Allocates an array of rationals.
 * @param nb Number of elements.
 * @return Pointer to the resulting rational or array.
 */
  csRational* csPtrAlloc_q(size_t nb);
/**
 * @brief Allocates an array of rationals.
 * @param nb Number of elements.
 * @param init Initial fill character.
 * @return Pointer to the resulting rational or array.
 */
  csRational* csPtrAlloc_q(size_t nb, csRational init);
/**
 * @brief Releases an array of rationals.
 * @param qn Array of rationals.
 * @param size Requested size.
 */
  void csPtrFree_q(csRational*& qn, size_t size);
/**
 * @brief Allocates an array of reals.
 * @param nb Number of elements.
 * @return Pointer to the resulting real or array.
 */
  csReal* csPtrAlloc_r(size_t nb);
/**
 * @brief Allocates an array of reals.
 * @param nb Number of elements.
 * @param init Initial fill character.
 * @return Pointer to the resulting real or array.
 */
  csReal* csPtrAlloc_r(size_t nb, csReal init);
/**
 * @brief Releases an array of reals.
 * @param rn Array of reals.
 * @param size Requested size.
 */
  void csPtrFree_r(csReal*& rn, size_t size);

  struct CSARITHMETIC_API csReal
  {
    char* mantissa=0;
    int exponent=0;
    bool sign = CS_POSITIVE_NUMBER;
    long precision=0;
    size_t mantSize;
    csReal(const char* mantissa="0", int exponent=0, bool sign=0);
/**
 * @brief Builds a real by repeating the mantissa.
 * @param mantissa Decimal text of the mantissa.
 * @param exponent Decimal exponent.
 * @param sign Sign. Zero when the number is positive.
 * @param repeat Number of times the mantissa is repeated. A value below 1 yields zero.
 */
    csReal(const char* mantissa, int exponent, bool sign, int repeat);
/**
 * @brief Builds a real by repeating a section of the mantissa.
 * @param mantissa Decimal text of the mantissa.
 * @param exponent Decimal exponent.
 * @param sign Sign. Zero when the number is positive.
 * @param repeat Number of times the section is repeated. A value below 1 yields zero.
 * @param pos Repeated section. Positive: from the first character through the pos-th, then the rest. Negative: from the pos-th character from the end through the last character, placed at the end.
 */
    csReal(const char* mantissa, int exponent, bool sign, int repeat, int pos);
/**
 * @brief Builds a real by repeating a slice of the mantissa.
 * @param mantissa Decimal text of the mantissa.
 * @param exponent Decimal exponent.
 * @param sign Sign. Zero when the number is positive.
 * @param repeat Number of times the slice is repeated. A value below 1 yields zero.
 * @param length Length of the slice, in characters.
 * @param index Start index of the slice, from zero.
 */
    csReal(const char* mantissa, int exponent, bool sign, int repeat, int length, int index);
    csReal(unsigned long mantissa, int exponent=0, bool sign=0);
    csReal(long mantissa, int exponent=0);
/**
 * @brief Builds a real from a double.
 * @param value Value to convert to decimal.
 */
    csReal(double value);
    csReal(bool evaluate, const char* number);
    csReal(const csReal& a);
    csReal(csReal&& a) noexcept;
/**
 * @brief Initializes the object or the library tables.
 */
    void init();
    ~csReal();
/**
 * @brief Assigns a new value.
 * @param mantissa Decimal text of the mantissa.
 * @param exponent Decimal exponent.
 * @param sign Sign. Zero when the number is positive.
 */
    void set(const char* mantissa, int exponent, bool sign);
/**
 * @brief Assigns a repeated mantissa to an existing real.
 * @param mantissa Decimal text of the mantissa.
 * @param exponent Decimal exponent.
 * @param sign Sign. Zero when the number is positive.
 * @param repeat Number of times the mantissa is repeated. A value below 1 yields zero.
 */
    void set(const char* mantissa, int exponent, bool sign, int repeat);
/**
 * @brief Assigns a repeated section of the mantissa to an existing real.
 * @param mantissa Decimal text of the mantissa.
 * @param exponent Decimal exponent.
 * @param sign Sign. Zero when the number is positive.
 * @param repeat Number of times the section is repeated. A value below 1 yields zero.
 * @param pos Repeated section. Positive: from the first character through the pos-th, then the rest. Negative: from the pos-th character from the end through the last character, placed at the end.
 */
    void set(const char* mantissa, int exponent, bool sign, int repeat, int pos);
/**
 * @brief Assigns a repeated slice of the mantissa to an existing real.
 * @param mantissa Decimal text of the mantissa.
 * @param exponent Decimal exponent.
 * @param sign Sign. Zero when the number is positive.
 * @param repeat Number of times the slice is repeated. A value below 1 yields zero.
 * @param length Length of the slice, in characters.
 * @param index Start index of the slice, from zero.
 */
    void set(const char* mantissa, int exponent, bool sign, int repeat, int length, int index);
/**
 * @brief Assigns a new value.
 * @param mantissa Decimal text of the mantissa.
 * @param exponent Decimal exponent.
 * @param sign Sign. Zero when the number is positive.
 */
    void set(unsigned long mantissa, int exponent, bool sign);
/**
 * @brief Assigns a new value.
 * @param mantissa Decimal text of the mantissa.
 * @param exponent Decimal exponent.
 */
    void set(long mantissa, int exponent);
/**
 * @brief Assigns a new value.
 * @param evaluate Interprets the text as a number.
 * @param number Text of the number.
 */
    void set(bool evaluate, const char* number);
/**
 * @brief Sets the precision of this real.
 * @param precision Requested precision, in digits.
 */
    void setPrecision(int precision);
/**
 * @brief Forces the number of mantissa digits.
 * @param size Requested size.
 */
    void forcePrecision(int size);
/**
 * @brief Increases the number of mantissa digits.
 * @param size Requested size.
 */
    void increaseShape(size_t size);
/**
 * @brief Reduces the number of mantissa digits.
 * @param size Requested size.
 */
    void decreaseShape(size_t size);
/**
 * @brief Reshapes the mantissa to the requested form.
 * @param newMantissaSize Parameter @p newMantissaSize.
 */
    void reshape(size_t newMantissaSize);
/**
 * @brief Removes cosmetic zeros from the mantissa.
 */
    void shapeOutZeros();
/**
 * @brief Reduces the value to the requested size.
 * @param size Requested size.
 */
    void reduce(size_t size);
/**
 * @brief Returns the number of mantissa digits.
 * @return Computed value.
 */
    size_t getDigitNumber();
/**
 * @brief Returns the absolute value.
 * @return Resulting real.
 */
    csReal abs();
    csReal operator+(const csReal& a);
    csReal operator-(const csReal& a);
    csReal operator*(const csReal& a);
    csReal operator/(const csReal& a);
    csReal operator+(long a);
    csReal operator-(long a);
    csReal operator*(long a);
/**
 * @brief Multiplies by a double, including values between 0 and 1.
 * @param a Decimal factor.
 * @return Resulting real.
 */
    csReal operator*(double a);
    csReal operator/(long a);
    csReal operator/(double a);
    csReal operator+(double a);
    csReal operator-(double a);
    template<class T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, long>::value, int>::type = 0>
    csReal operator+(T a) { return (*this) + (long)a; }
    template<class T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, long>::value, int>::type = 0>
    csReal operator-(T a) { return (*this) - (long)a; }
    template<class T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, long>::value, int>::type = 0>
    csReal operator*(T a) { return (*this) * (long)a; }
    template<class T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, long>::value, int>::type = 0>
    csReal operator/(T a) { return (*this) / (long)a; }
/**
 * @brief Returns the integer quotient of division by @p n, truncated toward zero.
 * @param n Integer divisor.
 * @return Integer quotient, with a zero exponent.
 */
    csReal integerQuotient(long n);
/**
 * @brief Returns the integer part, truncated toward zero.
 * @return Integer part, with a zero exponent.
 */
    csReal integer();
    csReal operator-() const;
    csReal& operator=(csReal a);
    void operator=(long a);
    void operator=(double a);
    template<class T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, long>::value, int>::type = 0>
    void operator=(T a) { *this = (long)a; }
    void operator=(const char* a);
    bool operator==(const csReal& a);
    bool operator!=(const csReal& a);
    bool operator>(const csReal& a);
    bool operator<(const csReal& a);
    bool operator>=(const csReal& a);
    bool operator<=(const csReal& a);
    bool operator==(long a);
    bool operator!=(long a);
    bool operator>(long a);
    bool operator>=(long a);
    bool operator<(long a);
    bool operator<=(long a);
    template<class T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
    bool operator==(T a) { return *this == csReal((double)a); }
    template<class T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
    bool operator!=(T a) { return *this != csReal((double)a); }
    template<class T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
    bool operator>(T a) { return *this > csReal((double)a); }
    template<class T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
    bool operator>=(T a) { return *this >= csReal((double)a); }
    template<class T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
    bool operator<(T a) { return *this < csReal((double)a); }
    template<class T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
    bool operator<=(T a) { return *this <= csReal((double)a); }

/**
 * @brief Exact rational, numerator and denominator in limbs.
 */
    operator csRational();
    CS_FORCE_INLINE friend std::ostream& operator<<(std::ostream& os, const csReal& a)
    {
      char*formated = getPrintFormat(a);
      cout<<formated;
      free(formated);
      return os;
    }
/**
 * @brief Copies the value.
 * @param a First operand.
 */
    void copy(const csReal& a);
/**
 * @brief Sets the real to zero.
 * @return True when the condition holds.
 */
    bool equalZero();
/**
 * @brief Prints the value.
 * @param title Title printed before the values.
 */
    void print(const char* title);
/**
 * @brief Prints the value with an extended format.
 * @param title Title printed before the values.
 */
    void print2(const char* title);
/**
 * @brief Returns a section of the mantissa.
 * @param first First index, inclusive.
 * @param last Last index, exclusive.
 * @return Resulting real.
 */
    csReal getMantissaSection(size_t first, size_t last);
/**
 * @brief Assigns another real to this object.
 * @param a First operand.
 */
    void assign(const csReal& a);
/**
 * @brief Assigns a new value.
 * @param id Index of the element.
 * @param digit Digit to write.
 */
    void set(size_t id, char digit);
/**
 * @brief Assigns the integer part.
 * @param id Index of the element.
 * @param digit Digit to write.
 */
    void setInt(size_t id, char digit);
/**
 * @brief Assigns the fractional part.
 * @param id Index of the element.
 * @param digit Digit to write.
 */
    void setDec(size_t id, char digit);
/**
 * @brief Assigns a power of ten.
 * @param power Power of ten.
 */
    void setAsTenPower(long power);
/**
 * @brief Draws a random value.
 * @param nDigits Number of digits.
 * @param digitMin Parameter @p digitMin.
 * @param digitMax Parameter @p digitMax.
 * @param exponent Decimal exponent.
 * @param sign Sign. Zero when the number is positive.
 */
    void random(size_t nDigits, char digitMin=0, char digitMax=9, long exponent=0, bool sign=0);
/**
 * @brief Counts the significant zeros of the mantissa.
 * @param initialPos Parameter @p initialPos.
 * @return Computed value.
 */
    size_t significantZerosCount(size_t initialPos=0);
/**
 * @brief Releases the memory held by the object.
 */
    void clear();
  };

  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE csReal operator+(T n, csReal a) { return a + n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE csReal operator-(T n, csReal a) { return csReal((long)n) - a; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE csReal operator*(T n, csReal a) { return a * n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE csReal operator/(T n, csReal a) { return csReal((long)n) / a; }
  CS_FORCE_INLINE csReal operator+(double n, csReal a) { return a + n; }
  CS_FORCE_INLINE csReal operator-(double n, csReal a) { return csReal(n) - a; }
  CS_FORCE_INLINE csReal operator*(double n, csReal a) { return a * n; }
  CS_FORCE_INLINE csReal operator/(double n, csReal a) { return csReal(n) / a; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator==(T n, csReal a) { return a == n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator!=(T n, csReal a) { return a != n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator<(T n, csReal a) { return a > n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator>(T n, csReal a) { return a < n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator<=(T n, csReal a) { return a >= n; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator>=(T n, csReal a) { return a <= n; }
  template<class T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator==(T n, csReal a) { return a == n; }
  template<class T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator!=(T n, csReal a) { return a != n; }
  template<class T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator<(T n, csReal a) { return a > n; }
  template<class T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator>(T n, csReal a) { return a < n; }
  template<class T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator<=(T n, csReal a) { return a >= n; }
  template<class T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator>=(T n, csReal a) { return a <= n; }

/**
 * @brief Initializes the object or the library tables.
 */
  void init();
/**
 * @brief Prints an array of digits.
 * @param nb Number of elements.
 * @param seperator Parameter @p seperator.
 */
  void printNumber(char* nb, const char* seperator=" ");
/**
 * @brief Returns the index of a digit pair.
 * @param tens Parameter @p tens.
 * @param units Parameter @p units.
 * @return Result.
 */
  uchar bicharIndex(uchar tens, uchar units);
/**
 * @brief Returns the index of a digit triple.
 * @param cents Parameter @p cents.
 * @param tens Parameter @p tens.
 * @param units Parameter @p units.
 * @return Result.
 */
  uchar tricharIndex(int cents, uchar tens, uchar units);
/**
 * @brief Transforms the digits of a subtraction.
 * @param i Parameter @p i.
 * @return Result.
 */
  csBIDIGITS substractionTransform(int i);

/**
 * @brief Computes a decimal product in the secondary base.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param aSize Number of digits of the first text.
 * @param resSize Parameter @p resSize.
 * @param prevCarry Parameter @p prevCarry.
 * @param m Number of evaluation points.
 * @param n Number of elements.
 */
  void makeMultiplicationBase2(char* a, uchar b, char*& result, size_t aSize, size_t resSize,csBIDIGITS& prevCarry, size_t m, size_t n);
/**
 * @brief Adds two texts during a multiplication.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param opSize Number of aligned digits.
 * @param resSize Parameter @p resSize.
 */
  void makeAdditionForMul(char* a, char* b, char*& result, size_t opSize, size_t resSize);
/**
 * @brief Subtracts two texts during a division.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param opSize Number of aligned digits.
 * @param frontOffset Parameter @p frontOffset.
 */
  void makeSubstractionForDiv(char* a, char* b, char*& result, size_t opSize, size_t frontOffset);
/**
 * @brief Multiplies during a division, in the first base.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param aSize Number of digits of the first text.
 * @param resSize Parameter @p resSize.
 */
  void makeMultiplicationForDivBase1(char* a, uchar b, char*& result, size_t aSize, size_t resSize);
/**
 * @brief Multiplies during a division, in the second base.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param aSize Number of digits of the first text.
 * @param resSize Parameter @p resSize.
 */
  void makeMultiplicationForDivBase2(char* a, uchar b, char*& result, size_t aSize, size_t resSize);

/**
 * @brief Groups decimal digits into pairs.
 * @param opTable Parameter @p opTable.
 * @return Result.
 */
  csBIDIGITS** toBIDIGITS(int** opTable);
/**
 * @brief Builds the multiplication table.
 */
  void makeMultiplicationTable();
/**
 * @brief Builds the addition table.
 */
  void createAdditionTable();
/**
 * @brief Builds the division table.
 */
  void makeDivisionTable();
/**
 * @brief Builds the subtraction table.
 */
  void makeSubstractionTable();
/**
 * @brief Prints a digit table.
 * @param opName Parameter @p opName.
 */
  void printTable(char* opName);

/**
 * @brief Adds two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param opSize Number of aligned digits.
 * @param resSize Parameter @p resSize.
 */
  void makeAddition(char* a, char* b, char*& result, size_t opSize, size_t& resSize);
/**
 * @brief Adds two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeAddition(char* a,char* b);
/**
 * @brief Adds two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param resSize Parameter @p resSize.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeAddition(char* a,char* b, size_t& resSize);
/**
 * @brief Adds two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeAddition(const char* a,const char* b);


/**
 * @brief Subtracts two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param opSize Number of aligned digits.
 */
  void makeSubstraction(char* a, char* b, char*& result, size_t opSize);
/**
 * @brief Subtracts two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeSubstraction(char* a,char* b);
/**
 * @brief Subtracts two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param sign Sign. Zero when the number is positive.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeSubstraction(char* a,char* b, bool& sign);
/**
 * @brief Subtracts two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeSubstraction(char* a,char* b, size_t aSize, size_t bSize);
/**
 * @brief Subtracts two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param sign Sign. Zero when the number is positive.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeSubstraction(char* a,char* b, size_t aSize, size_t bSize, bool&sign);
/**
 * @brief Subtracts two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeSubstraction(const char* a,const char*b);
/**
 * @brief Subtracts two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param resSize Parameter @p resSize.
 * @param sign Sign. Zero when the number is positive.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeSubstraction(char* a,char* b, size_t aSize, size_t bSize, size_t& resSize, bool& sign);

/**
 * @brief Computes a product in the current base.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param aSize Number of digits of the first text.
 * @param resSize Parameter @p resSize.
 */
  void makeMultiplicationBase(char* a, uchar b, char*& result, size_t aSize, size_t resSize);
/**
 * @brief Multiplies two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param tmpResult Parameter @p tmpResult.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param resSize Parameter @p resSize.
 */
  void makeMultiplication(char* a, char* b, char*& result, char*& tmpResult,
                          size_t aSize, size_t bSize, size_t resSize);
/**
 * @brief Multiplies two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeMultiplication(char* a, char* b);
/**
 * @brief Multiplies two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeMultiplication(char* a, char* b, size_t aSize, size_t bSize);
/**
 * @brief Multiplies two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeMultiplication(const char* a, const char* b);
/**
 * @brief Multiplies two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param resSize Parameter @p resSize.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeMultiplication(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize);

/**
 * @brief Divides two decimal texts and returns the quotient.
 * @param a First operand.
 * @param b Second operand.
 * @param remain Receives the remainder of the division.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeDivisionQ(char*a, char* b, char*& remain);
/**
 * @brief Divides two decimal texts and returns the quotient.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeDivisionQ(char*a, char* b);
/**
 * @brief Divides two decimal texts and returns the quotient.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param remain Receives the remainder of the division.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param resSize Parameter @p resSize.
 * @param remSize Parameter @p remSize.
 * @return Resulting text. The caller frees the memory.
 */
  char const* makeDivisionQ(char* a, char* b, char*& result, char*& remain,
        size_t& aSize, size_t& bSize, size_t& resSize, size_t& remSize);
/**
 * @brief Divides two decimal texts and returns the quotient.
 * @param a First operand.
 * @param b Second operand.
 * @param remain Receives the remainder of the division.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeDivisionQ(const char* a, const char* b, char*&remain);
/**
 * @brief Divides two decimal texts and returns the quotient.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeDivisionQ(const char* a, const char* b);
/**
 * @brief Divides two decimal texts and returns the quotient.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param resSize Parameter @p resSize.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeDivisionQ(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize);


/**
 * @brief Divides two decimal texts and keeps a fractional part.
 * @param _a First operand.
 * @param _b Second operand.
 * @param resInt Parameter @p resInt.
 * @param resDec Parameter @p resDec.
 * @param remain Receives the remainder of the division.
 * @param _aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param resSize Parameter @p resSize.
 * @param resIntSize Parameter @p resIntSize.
 * @param resDecSize Parameter @p resDecSize.
 * @param remSize Parameter @p remSize.
 * @param nDecimals Parameter @p nDecimals.
 * @return Resulting text. The caller frees the memory.
 */
  char const* makeDivisionR(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain,
                    size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals);
/**
 * @brief Computes a real division, second variant.
 * @param _a First operand.
 * @param _b Second operand.
 * @param resInt Parameter @p resInt.
 * @param resDec Parameter @p resDec.
 * @param remain Receives the remainder of the division.
 * @param _aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param resSize Parameter @p resSize.
 * @param resIntSize Parameter @p resIntSize.
 * @param resDecSize Parameter @p resDecSize.
 * @param remSize Parameter @p remSize.
 * @param nDecimals Parameter @p nDecimals.
 * @return Resulting text. The caller frees the memory.
 */
  char const* makeDivisionR2(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain,
                    size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals);
/**
 * @brief Computes a real division, second variant.
 * @param _a First operand.
 * @param _b Second operand.
 * @param resInt Parameter @p resInt.
 * @param resDec Parameter @p resDec.
 * @param _aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param resSize Parameter @p resSize.
 * @param resIntSize Parameter @p resIntSize.
 * @param resDecSize Parameter @p resDecSize.
 * @param nDecimals Parameter @p nDecimals.
 * @return Resulting text. The caller frees the memory.
 */
  char const* makeDivisionR2(char*_a, char* _b, char*& resInt, char*& resDec,
                    size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t nDecimals);
/**
 * @brief Divides two decimal texts and returns the quotient.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param resSize Parameter @p resSize.
 * @return Resulting text. The caller frees the memory.
 */
  char const* makeDivisionQ(char*a, char* b, char*& result,
        size_t& aSize, size_t& bSize, size_t& resSize);
/**
 * @brief Divides two decimal texts and keeps a fractional part.
 * @param a First operand.
 * @param b Second operand.
 * @param resInt Parameter @p resInt.
 * @param resDec Parameter @p resDec.
 * @param remain Receives the remainder of the division.
 * @param nDecimals Parameter @p nDecimals.
 * @return Resulting text. The caller frees the memory.
 */
  char const* makeDivisionR(char* a, char* b, char*& resInt, char*& resDec, char*& remain, size_t nDecimals);

/**
 * @brief Computes the remainder of a rational division.
 * @param a First operand.
 * @param b Second operand.
 * @param remain Receives the remainder of the division.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param remSize Parameter @p remSize.
 * @return Resulting text. The caller frees the memory.
 */
  char const* makeModulusQ(char*a, char* b, char*& remain,
        size_t& aSize, size_t& bSize, size_t& remSize);
/**
 * @brief Computes the remainder of a rational division.
 * @param a First operand.
 * @param b Second operand.
 * @param remain Receives the remainder of the division.
 */
  void makeModulusQ(char*a, char* b, char*&remain);
/**
 * @brief Computes the remainder of a rational division.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeModulusQ(char*a, char* b);
/**
 * @brief Computes the remainder of a rational division.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* makeModulusQ(const char*a, const char* b);

/**
 * @brief Raises a rational to an integer power.
 * @param a First operand.
 * @param p Power or degree.
 * @return Resulting rational.
 */
  csRational pow(csRational a, size_t p);

/**
 * @brief Computes the greatest common divisor.
 * @param a First operand.
 * @param b Second operand.
 * @param gdcSize Parameter @p gdcSize.
 * @return Resulting text. The caller frees the memory.
 */
  char* gcd(const char* a, const char* b, size_t& gdcSize);
/**
 * @brief Computes the greatest common divisor.
 * @param a First operand.
 * @param b Second operand.
 * @param gdcSize Parameter @p gdcSize.
 * @return Resulting text. The caller frees the memory.
 */
  char* gcd(char* a, char* b, size_t& gdcSize);
/**
 * @brief Computes the greatest common divisor.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param gdcSize Parameter @p gdcSize.
 * @return Resulting text. The caller frees the memory.
 */
  char* gcd(char* a, char* b, size_t aSize, size_t bSize, size_t& gdcSize);

/**
 * @brief Sorts reals into increasing order.
 * @param rn Array of reals.
 * @param size Requested size.
 * @return Pointer to the resulting real or array.
 */
  csReal* csSortMinR(csReal* rn, size_t size);
/**
 * @brief Sorts reals into increasing order.
 * @param rn Array of reals.
 * @param size Requested size.
 */
  void csSortMinR(csReal*& rn, size_t size);
/**
 * @brief Sorts points by increasing real.
 * @param fc Parameter @p fc.
 * @param rn Array of reals.
 * @param size Requested size.
 */
  void csSortMin_CoordsByRNumber(csFCOORDS*& fc, csReal*& rn, size_t size);
/**
 * @brief Sorts points by decreasing real.
 * @param fc Parameter @p fc.
 * @param rn Array of reals.
 * @param size Requested size.
 */
  void csSortMax_CoordsByRNumber(csFCOORDS*& fc, csReal*& rn, size_t size);
  }

/**
 * @brief Reduces the value to the requested size.
 * @param a First operand.
 * @param sizeCondition Parameter @p sizeCondition.
 * @return Resulting rational.
 */
CSARITHMETIC_API CSARITHMETIC::csRational csReduce(CSARITHMETIC_API CSARITHMETIC::csRational a, size_t sizeCondition);

#include <thread>
using namespace CSARITHMETIC;

#endif
