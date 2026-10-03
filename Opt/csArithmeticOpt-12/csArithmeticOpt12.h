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

#ifndef CSARITHMETIC_OPT12_H
#define CSARITHMETIC_OPT12_H

#include <iostream>
#include <cstdint>
#include <cstring>
#include <cstdlib>

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
  char* formatReal(const csReal& a);
/**
 * @brief Sets the precision used for reals.
 * @param precision Requested precision, in digits.
 */
  void setRealPrecision(int precision);
/**
 * @brief Returns the precision used for reals.
 * @return Computed value.
 */
  int realPrecision();


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
      assignValue((long)num, denom);
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
    void assignValue(long num, size_t denom=1);
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
    bool equalAbsolute(const csRational& a);
    bool operator!=(const csRational& a);
/**
 * @brief Reports whether the absolute values differ.
 * @param a First operand.
 * @return True when the condition holds.
 */
    bool differAbsolute(const csRational& a);
    bool operator>(const csRational& a);
/**
 * @brief Reports whether the absolute value is greater.
 * @param a First operand.
 * @return True when the condition holds.
 */
    bool greaterAbsolute(const csRational& a);
    bool operator<(const csRational& a);
/**
 * @brief Reports whether the absolute value is less.
 * @param a First operand.
 * @return True when the condition holds.
 */
    bool lessAbsolute(const csRational& a);
    bool operator>=(const csRational& a);
/**
 * @brief Reports whether the absolute value is greater than or equal.
 * @param a First operand.
 * @return True when the condition holds.
 */
    bool greaterOrEqualAbsolute(const csRational& a);
    bool operator<=(const csRational& a);
/**
 * @brief Reports whether the absolute value is less than or equal.
 * @param a First operand.
 * @return True when the condition holds.
 */
    bool lessOrEqualAbsolute(const csRational& a);

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
 * @brief Returns the stored sign, without testing for zero.
 * @return True when the negative flag is set.
 */
    bool negativeSign() const { return sign == CS_NEGATIVE_NUMBER; }
/**
 * @brief Reports whether the value is strictly negative.
 * @return True when the sign is negative and the value is not zero.
 */
    bool isNegative() { return negativeSign() && !isZero(); }
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
    size_t maxLimbCount();
/**
 * @brief Returns the decimal expansion of the rational.
 * @return Resulting text. The caller frees the memory.
 */
    char* decimalExpansion();
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

    [[deprecated("use assignValue")]]
    void setl(long num, size_t denom = 1) { assignValue(num, denom); }
    [[deprecated("use equalAbsolute")]]
    bool isAbsEqual(const csRational& a) { return equalAbsolute(a); }
    [[deprecated("use differAbsolute")]]
    bool isAbsDifferent(const csRational& a) { return differAbsolute(a); }
    [[deprecated("use greaterAbsolute")]]
    bool isAbsGreater(const csRational& a) { return greaterAbsolute(a); }
    [[deprecated("use lessAbsolute")]]
    bool isAbsLess(const csRational& a) { return lessAbsolute(a); }
    [[deprecated("use greaterOrEqualAbsolute")]]
    bool isAbsGreaterEqual(const csRational& a) { return greaterOrEqualAbsolute(a); }
    [[deprecated("use lessOrEqualAbsolute")]]
    bool isAbsLessEqual(const csRational& a) { return lessOrEqualAbsolute(a); }
    [[deprecated("use maxLimbCount")]]
    size_t maxSize() { return maxLimbCount(); }
    [[deprecated("use decimalExpansion")]]
    char* quotient() { return decimalExpansion(); }
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

enum { csIntegerStackLimbs = 32 };

/**
 * @brief Exact integer. One machine word while the value fits, then 32 internal limbs, then the heap.
 */
  struct CSARITHMETIC_API csInteger
  {
    uint64_t value = 0;
    uint32_t stack[csIntegerStackLimbs];
    char* limbs = 0;
    size_t count = 0;
    bool negative = false;
    bool extended = false;

    csInteger();
    csInteger(long long number);
    csInteger(const char* digits);
    csInteger(const csInteger& other);
    csInteger(csInteger&& other) noexcept;
    ~csInteger();

/**
 * @brief Sets the integer to zero and releases the limbs.
 */
    void clear();
/**
 * @brief Copies the value.
 * @param other Source integer.
 */
    void copy(const csInteger& other);
/**
 * @brief Assigns a signed 64-bit integer.
 * @param number Signed value.
 */
    void set(long long number);
/**
 * @brief Assigns an unsigned 64-bit integer.
 * @param number Absolute value.
 * @param negativeSign True when the result must be negative.
 */
    void set(unsigned long long number, bool negativeSign = false);
/**
 * @brief Assigns a decimal text.
 * @param digits Digits, with an optional sign.
 */
    void set(const char* digits);
/**
 * @brief Assigns a magnitude in base 2^32, little-endian.
 * @param digits Limbs. May be null.
 * @param limbCount Number of limbs.
 * @param negativeSign True when the result must be negative.
 */
    void assignMagnitude(const uint32_t* digits, size_t limbCount, bool negativeSign);
/**
 * @brief Remainder in [0, mod), sign included.
 * @param mod Modulus.
 * @return Remainder.
 */
    uint32_t modulo(uint32_t mod) const;
/**
 * @brief Number of bits of the absolute value.
 * @return Number of bits.
 */
    size_t bitCount() const;

    csInteger operator+(const csInteger& other) const;
    csInteger operator-(const csInteger& other) const;
    csInteger operator*(const csInteger& other) const;
    csInteger operator/(const csInteger& other) const;
    csInteger operator%(const csInteger& other) const;
    csInteger operator-() const;
    csInteger& operator=(const csInteger& other);
    csInteger& operator=(long long number);
    csInteger& operator+=(const csInteger& other);
    csInteger& operator*=(const csInteger& other);

    bool operator==(const csInteger& other) const;
    bool operator!=(const csInteger& other) const;
    bool operator<(const csInteger& other) const;
    bool operator>(const csInteger& other) const;
    bool operator<=(const csInteger& other) const;
    bool operator>=(const csInteger& other) const;

/**
 * @brief Reports whether the value is zero.
 * @return True when the value is zero.
 */
    bool isZero() const;
/**
 * @brief Returns the stored sign, without testing for zero.
 * @return True when the negative flag is set.
 */
    bool negativeSign() const;
/**
 * @brief Reports whether the value is negative.
 * @return True when the value is strictly negative.
 */
    bool isNegative() const;
/**
 * @brief Returns the absolute value.
 * @return Non-negative integer.
 */
    csInteger absolute() const;
/**
 * @brief Quotient and remainder of Euclidean division truncated toward zero.
 * @param divisor Divisor.
 * @param remainder Receives the remainder, with the sign of the dividend.
 * @return Quotient.
 */
    csInteger quotientAndRemainder(const csInteger& divisor, csInteger& remainder) const;
/**
 * @brief Raises the integer to a non-negative integer power.
 * @param exponent Exponent.
 * @return Resulting integer. Zero to the power zero is one.
 */
    csInteger power(size_t exponent) const;
/**
 * @brief Converts the integer to a rational with denominator one.
 * @return Resulting rational.
 */
    csRational toRational() const;
/**
 * @brief Returns the decimal digits, sign included.
 * @return Resulting text. The caller frees the memory.
 */
    char* toDecimal() const;
/**
 * @brief Prints the value.
 * @param title Title printed before the value.
 */
    void print(const char* title) const;

    CS_FORCE_INLINE friend std::ostream& operator<<(std::ostream& os, const csInteger& number)
    {
      char* text = number.toDecimal();
      os << text;
      free(text);
      return os;
    }

    csInteger& widenAdd(const csInteger& other);
    csInteger& widenMul(const csInteger& other);
  };

  inline void csMaterialize_z(const csInteger& number, const uint32_t*& digits, size_t& limbCount, uint32_t word[2])
  {
    if (!number.extended && number.count == 0)
    {
      word[0] = (uint32_t)number.value;
      word[1] = (uint32_t)(number.value >> 32);
      limbCount = number.value == 0 ? 0 : (word[1] ? 2 : 1);
      digits = word;
      return;
    }
    limbCount = number.count;
    digits = number.extended ? (const uint32_t*)number.limbs : number.stack;
  }

  inline int csCmpMagPtr_z(const uint32_t* left, size_t leftCount, const uint32_t* right, size_t rightCount)
  {
    while (leftCount > 0 && left[leftCount - 1] == 0)
      --leftCount;
    while (rightCount > 0 && right[rightCount - 1] == 0)
      --rightCount;
    if (leftCount != rightCount)
      return leftCount < rightCount ? -1 : 1;
    for (size_t i = leftCount; i-- > 0; )
    {
      if (left[i] != right[i])
        return left[i] < right[i] ? -1 : 1;
    }
    return 0;
  }

  inline void csStoreRaw_z(csInteger& number, const uint32_t* digits, size_t limbCount, bool negativeSign)
  {
    while (limbCount > 0 && digits[limbCount - 1] == 0)
      --limbCount;
    if (number.extended && number.limbs)
      free(number.limbs);
    number.limbs = 0;
    number.extended = false;
    if (limbCount <= 2)
    {
      uint64_t word = 0;
      if (limbCount >= 1)
        word = digits[0];
      if (limbCount == 2)
        word |= (uint64_t)digits[1] << 32;
      number.value = word;
      number.count = 0;
      number.negative = negativeSign && word != 0;
      return;
    }
    if (limbCount <= (size_t)csIntegerStackLimbs)
    {
      memcpy(number.stack, digits, limbCount * sizeof(uint32_t));
      number.count = limbCount;
      number.value = 0;
      number.negative = negativeSign;
      return;
    }
    number.limbs = (char*)malloc(limbCount * sizeof(uint32_t));
    memcpy(number.limbs, digits, limbCount * sizeof(uint32_t));
    number.count = limbCount;
    number.value = 0;
    number.negative = negativeSign;
    number.extended = true;
  }

  inline csInteger& csAddWide_z(csInteger& self, const csInteger& other)
  {
    if (self.extended || other.extended)
      return self.widenAdd(other);
    uint32_t leftWord[2], rightWord[2];
    const uint32_t* leftDigits = 0;
    const uint32_t* rightDigits = 0;
    size_t leftCount = 0, rightCount = 0;
    csMaterialize_z(self, leftDigits, leftCount, leftWord);
    csMaterialize_z(other, rightDigits, rightCount, rightWord);
    uint32_t raw[csIntegerStackLimbs + 1];
    size_t rawCount = 0;
    bool negativeSign = false;
    if (self.negative == other.negative)
    {
      size_t n = leftCount > rightCount ? leftCount : rightCount;
      uint64_t carry = 0;
      for (size_t i = 0; i < n; ++i)
      {
        uint64_t sum = carry;
        if (i < leftCount)
          sum += leftDigits[i];
        if (i < rightCount)
          sum += rightDigits[i];
        raw[i] = (uint32_t)sum;
        carry = sum >> 32;
      }
      rawCount = n;
      if (carry)
        raw[rawCount++] = (uint32_t)carry;
      negativeSign = self.negative;
    }
    else
    {
      int order = csCmpMagPtr_z(leftDigits, leftCount, rightDigits, rightCount);
      if (order == 0)
      {
        csStoreRaw_z(self, raw, 0, false);
        return self;
      }
      const uint32_t* high = order > 0 ? leftDigits : rightDigits;
      const uint32_t* low = order > 0 ? rightDigits : leftDigits;
      size_t highCount = order > 0 ? leftCount : rightCount;
      size_t lowCount = order > 0 ? rightCount : leftCount;
      int64_t borrow = 0;
      for (size_t i = 0; i < highCount; ++i)
      {
        int64_t diff = (int64_t)high[i] - borrow - (i < lowCount ? (int64_t)low[i] : 0);
        if (diff < 0)
        {
          diff += 4294967296LL;
          borrow = 1;
        }
        else
          borrow = 0;
        raw[i] = (uint32_t)diff;
      }
      rawCount = highCount;
      negativeSign = order > 0 ? self.negative : other.negative;
    }
    csStoreRaw_z(self, raw, rawCount, negativeSign);
    return self;
  }

  inline csInteger& csMulWide_z(csInteger& self, const csInteger& other)
  {
    if (self.isZero() || other.isZero())
    {
      self.clear();
      return self;
    }
    bool negativeSign = self.negative != other.negative;
    if (self.extended || other.extended)
      return self.widenMul(other);
    uint32_t leftWord[2], rightWord[2];
    const uint32_t* leftDigits = 0;
    const uint32_t* rightDigits = 0;
    size_t leftCount = 0, rightCount = 0;
    csMaterialize_z(self, leftDigits, leftCount, leftWord);
    csMaterialize_z(other, rightDigits, rightCount, rightWord);
    if (leftCount == 0 || rightCount == 0)
    {
      self.clear();
      return self;
    }
    if (leftCount + rightCount > (size_t)csIntegerStackLimbs)
      return self.widenMul(other);
    uint32_t raw[csIntegerStackLimbs];
    memset(raw, 0, (leftCount + rightCount) * sizeof(uint32_t));
    for (size_t i = 0; i < leftCount; ++i)
    {
      uint64_t carry = 0;
      for (size_t j = 0; j < rightCount; ++j)
      {
        uint64_t sum = (uint64_t)raw[i + j] + (uint64_t)leftDigits[i] * rightDigits[j] + carry;
        raw[i + j] = (uint32_t)sum;
        carry = sum >> 32;
      }
      raw[i + rightCount] = (uint32_t)carry;
    }
    csStoreRaw_z(self, raw, leftCount + rightCount, negativeSign);
    return self;
  }

  inline csInteger::csInteger()
    : value(0), limbs(0), count(0), negative(false), extended(false) {}

  inline csInteger::csInteger(const csInteger& other)
    : value(other.value), limbs(0), count(0), negative(other.negative), extended(false)
  {
    if (other.extended && other.limbs && other.count)
    {
      limbs = (char*)malloc(other.count * sizeof(uint32_t));
      memcpy(limbs, other.limbs, other.count * sizeof(uint32_t));
      count = other.count;
      extended = true;
      value = 0;
    }
    else
    {
      count = other.count;
      if (count)
        memcpy(stack, other.stack, count * sizeof(uint32_t));
    }
  }

  inline csInteger::csInteger(csInteger&& other) noexcept
    : value(other.value), limbs(other.extended ? other.limbs : 0), count(other.count), negative(other.negative), extended(other.extended)
  {
    if (!extended && count)
      memcpy(stack, other.stack, count * sizeof(uint32_t));
    other.value = 0;
    other.limbs = 0;
    other.count = 0;
    other.negative = false;
    other.extended = false;
  }

  inline csInteger::~csInteger()
  {
    if (extended && limbs)
      free(limbs);
  }

  inline void csInteger::clear()
  {
    if (extended && limbs)
      free(limbs);
    value = 0;
    limbs = 0;
    count = 0;
    negative = false;
    extended = false;
  }

  inline bool csInteger::isZero() const
  {
    if (!extended && count == 0)
      return value == 0;
    const uint32_t* digits = extended ? (const uint32_t*)limbs : stack;
    if (!digits || count == 0)
      return true;
    for (size_t i = 0; i < count; ++i)
    {
      if (digits[i] != 0)
        return false;
    }
    return true;
  }

  inline bool csInteger::negativeSign() const
  {
    return negative;
  }

  inline bool csInteger::isNegative() const
  {
    return negative && !isZero();
  }

  inline csInteger csInteger::absolute() const
  {
    csInteger result(*this);
    result.negative = false;
    return result;
  }

  inline csInteger& csInteger::operator=(const csInteger& other)
  {
    if (this == &other)
      return *this;
    if (extended && limbs)
      free(limbs);
    limbs = 0;
    extended = false;
    negative = other.negative;
    if (other.extended && other.limbs && other.count)
    {
      limbs = (char*)malloc(other.count * sizeof(uint32_t));
      memcpy(limbs, other.limbs, other.count * sizeof(uint32_t));
      count = other.count;
      value = 0;
      extended = true;
      return *this;
    }
    value = other.value;
    count = other.count;
    if (count)
      memcpy(stack, other.stack, count * sizeof(uint32_t));
    return *this;
  }

  inline csInteger& csInteger::operator+=(const csInteger& other)
  {
    if (!extended && count == 0 && !other.extended && other.count == 0)
    {
      if (negative == other.negative)
      {
        unsigned __int128 sum = (unsigned __int128)value + other.value;
        if (sum <= (unsigned __int128)~uint64_t(0))
        {
          value = (uint64_t)sum;
          negative = negative && value != 0;
          return *this;
        }
      }
      else if (value >= other.value)
      {
        value -= other.value;
        negative = negative && value != 0;
        return *this;
      }
      else
      {
        value = other.value - value;
        negative = other.negative && value != 0;
        return *this;
      }
    }
    return csAddWide_z(*this, other);
  }

  inline csInteger& csInteger::operator*=(const csInteger& other)
  {
    if (!extended && count == 0 && !other.extended && other.count == 0)
    {
      unsigned __int128 product = (unsigned __int128)value * other.value;
      if (product <= (unsigned __int128)~uint64_t(0))
      {
        bool negativeSign = negative != other.negative;
        value = (uint64_t)product;
        negative = negativeSign && value != 0;
        return *this;
      }
    }
    return csMulWide_z(*this, other);
  }

  inline csInteger csInteger::operator+(const csInteger& other) const
  {
    csInteger result(*this);
    result += other;
    return result;
  }

  inline csInteger csInteger::operator-(const csInteger& other) const
  {
    csInteger opposite(other);
    if (!opposite.isZero())
      opposite.negative = !opposite.negative;
    return *this + opposite;
  }

  inline csInteger csInteger::operator*(const csInteger& other) const
  {
    csInteger result(*this);
    result *= other;
    return result;
  }

  inline csInteger csInteger::operator-() const
  {
    csInteger result(*this);
    if (!result.isZero())
      result.negative = !result.negative;
    return result;
  }

  inline uint32_t csInteger::modulo(uint32_t mod) const
  {
    if (mod < 2)
      return 0;
    uint32_t residue;
    if (!extended && count == 0)
      residue = (uint32_t)(value % mod);
    else
    {
      const uint32_t* digits = extended ? (const uint32_t*)limbs : stack;
      uint64_t acc = 0;
      for (size_t i = count; i-- > 0; )
        acc = ((acc << 32) | (digits ? digits[i] : 0)) % mod;
      residue = (uint32_t)acc;
    }
    if (negative && residue)
      residue = mod - residue;
    return residue;
  }

  inline size_t csInteger::bitCount() const
  {
    if (!extended && count == 0)
    {
      if (value == 0)
        return 0;
      return (size_t)(64 - __builtin_clzll(value));
    }
    const uint32_t* digits = extended ? (const uint32_t*)limbs : stack;
    size_t n = count;
    if (!digits)
      return 0;
    while (n > 0 && digits[n - 1] == 0)
      --n;
    if (n == 0)
      return 0;
    return n * 32 - (size_t)__builtin_clz(digits[n - 1]);
  }

  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE csInteger operator+(T n, const csInteger& number) { return number + csInteger((long long)n); }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE csInteger operator-(T n, const csInteger& number) { return csInteger((long long)n) - number; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE csInteger operator*(T n, const csInteger& number) { return number * csInteger((long long)n); }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE csInteger operator/(T n, const csInteger& number) { return csInteger((long long)n) / number; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE csInteger operator%(T n, const csInteger& number) { return csInteger((long long)n) % number; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator==(T n, const csInteger& number) { return number == csInteger((long long)n); }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator!=(T n, const csInteger& number) { return number != csInteger((long long)n); }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator<(T n, const csInteger& number) { return csInteger((long long)n) < number; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator>(T n, const csInteger& number) { return csInteger((long long)n) > number; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator<=(T n, const csInteger& number) { return csInteger((long long)n) <= number; }
  template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
  CS_FORCE_INLINE bool operator>=(T n, const csInteger& number) { return csInteger((long long)n) >= number; }

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
    void setDecimalPlaces(int size);
/**
 * @brief Increases the number of mantissa digits.
 * @param size Requested size.
 */
    void extendMantissa(size_t size);
/**
 * @brief Reduces the number of mantissa digits.
 * @param size Requested size.
 */
    void shortenMantissa(size_t size);
/**
 * @brief Reshapes the mantissa to the requested form.
 * @param newMantissaSize Parameter @p newMantissaSize.
 */
    void resizeMantissa(size_t newMantissaSize);
/**
 * @brief Removes cosmetic zeros from the mantissa.
 */
    void trimTrailingZeros();
/**
 * @brief Reduces the value to the requested size.
 * @param size Requested size.
 */
    void reduce(size_t size);
/**
 * @brief Returns the number of mantissa digits.
 * @return Computed value.
 */
    size_t digitCount();
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
      char*formated = formatReal(a);
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
    bool isZero();
/**
 * @brief Returns the stored sign, without testing for zero.
 * @return True when the negative flag is set.
 */
    bool negativeSign() const { return sign == CS_NEGATIVE_NUMBER; }
/**
 * @brief Reports whether the value is strictly negative.
 * @return True when the sign is negative and the value is not zero.
 */
    bool isNegative() { return negativeSign() && !isZero(); }
/**
 * @brief Prints the value.
 * @param title Title printed before the values.
 */
    void print(const char* title);
/**
 * @brief Prints the value with an extended format.
 * @param title Title printed before the values.
 */
    void printScientific(const char* title);
/**
 * @brief Returns a section of the mantissa.
 * @param first First index, inclusive.
 * @param last Last index, exclusive.
 * @return Resulting real.
 */
    csReal mantissaSection(size_t first, size_t last);
/**
 * @brief Assigns another real to this object.
 * @param a First operand.
 */
    void copyFrom(const csReal& a);
/**
 * @brief Assigns a new value.
 * @param id Index of the element.
 * @param digit Digit to write.
 */
    void setDigit(size_t id, char digit);
/**
 * @brief Assigns the integer part.
 * @param id Index of the element.
 * @param digit Digit to write.
 */
    void setIntegerDigit(size_t id, char digit);
/**
 * @brief Assigns the fractional part.
 * @param id Index of the element.
 * @param digit Digit to write.
 */
    void setFractionalDigit(size_t id, char digit);
/**
 * @brief Assigns a power of ten.
 * @param power Power of ten.
 */
    void setPowerOfTen(long power);
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
    size_t significantZeroCount(size_t initialPos=0);
/**
 * @brief Releases the memory held by the object.
 */
    void clear();

    [[deprecated("use setDecimalPlaces")]]
    void forcePrecision(int size) { setDecimalPlaces(size); }
    [[deprecated("use extendMantissa")]]
    void increaseShape(size_t size) { extendMantissa(size); }
    [[deprecated("use shortenMantissa")]]
    void decreaseShape(size_t size) { shortenMantissa(size); }
    [[deprecated("use resizeMantissa")]]
    void reshape(size_t newMantissaSize) { resizeMantissa(newMantissaSize); }
    [[deprecated("use trimTrailingZeros")]]
    void shapeOutZeros() { trimTrailingZeros(); }
    [[deprecated("use digitCount")]]
    size_t getDigitNumber() { return digitCount(); }
    [[deprecated("use isZero")]]
    bool equalZero() { return isZero(); }
    [[deprecated("use printScientific")]]
    void print2(const char* title) { printScientific(title); }
    [[deprecated("use mantissaSection")]]
    csReal getMantissaSection(size_t first, size_t last) { return mantissaSection(first, last); }
    [[deprecated("use copyFrom")]]
    void assign(const csReal& a) { copyFrom(a); }
    [[deprecated("use setDigit")]]
    void set(size_t id, char digit) { setDigit(id, digit); }
    [[deprecated("use setIntegerDigit")]]
    void setInt(size_t id, char digit) { setIntegerDigit(id, digit); }
    [[deprecated("use setFractionalDigit")]]
    void setDec(size_t id, char digit) { setFractionalDigit(id, digit); }
    [[deprecated("use setPowerOfTen")]]
    void setAsTenPower(long power) { setPowerOfTen(power); }
    [[deprecated("use significantZeroCount")]]
    size_t significantZerosCount(size_t initialPos = 0) { return significantZeroCount(initialPos); }
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
 * @param separator Parameter @p separator.
 */
  void printDigits(char* nb, const char* separator=" ");
/**
 * @brief Returns the index of a digit pair.
 * @param tens Parameter @p tens.
 * @param units Parameter @p units.
 * @return Result.
 */
  uchar digitPairIndex(uchar tens, uchar units);
/**
 * @brief Returns the index of a digit triple.
 * @param cents Parameter @p cents.
 * @param tens Parameter @p tens.
 * @param units Parameter @p units.
 * @return Result.
 */
  uchar digitTripleIndex(int cents, uchar tens, uchar units);
/**
 * @brief Transforms the digits of a subtraction.
 * @param i Parameter @p i.
 * @return Result.
 */
  csBIDIGITS subtractionStep(int i);

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
  void multiplyDigitBase2(char* a, uchar b, char*& result, size_t aSize, size_t resSize,csBIDIGITS& prevCarry, size_t m, size_t n);
/**
 * @brief Adds two texts during a multiplication.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param opSize Number of aligned digits.
 * @param resSize Parameter @p resSize.
 */
  void addShiftedProduct(char* a, char* b, char*& result, size_t opSize, size_t resSize);
/**
 * @brief Subtracts two texts during a division.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param opSize Number of aligned digits.
 * @param frontOffset Parameter @p frontOffset.
 */
  void subtractPartialQuotient(char* a, char* b, char*& result, size_t opSize, size_t frontOffset);
/**
 * @brief Multiplies during a division, in the first base.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param aSize Number of digits of the first text.
 * @param resSize Parameter @p resSize.
 */
  void multiplyDigitForDivision(char* a, uchar b, char*& result, size_t aSize, size_t resSize);
/**
 * @brief Multiplies during a division, in the second base.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param aSize Number of digits of the first text.
 * @param resSize Parameter @p resSize.
 */
  void multiplyDigitPairForDivision(char* a, uchar b, char*& result, size_t aSize, size_t resSize);

/**
 * @brief Groups decimal digits into pairs.
 * @param opTable Parameter @p opTable.
 * @return Result.
 */
  csBIDIGITS** packDigitPairs(int** opTable);
/**
 * @brief Builds the multiplication table.
 */
  void buildMultiplicationTable();
/**
 * @brief Builds the addition table.
 */
  void buildAdditionTable();
/**
 * @brief Builds the division table.
 */
  void buildDivisionTable();
/**
 * @brief Builds the subtraction table.
 */
  void buildSubtractionTable();
/**
 * @brief Prints a digit table.
 * @param opName Parameter @p opName.
 */
  void printDigitTable(char* opName);

/**
 * @brief Adds two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param opSize Number of aligned digits.
 * @param resSize Parameter @p resSize.
 */
  void addDecimal(char* a, char* b, char*& result, size_t opSize, size_t& resSize);
/**
 * @brief Adds two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* addDecimal(char* a,char* b);
/**
 * @brief Adds two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param resSize Parameter @p resSize.
 * @return Resulting text. The caller frees the memory.
 */
  char* addDecimal(char* a,char* b, size_t& resSize);
/**
 * @brief Adds two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* addDecimal(const char* a,const char* b);


/**
 * @brief Subtracts two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param opSize Number of aligned digits.
 */
  void subtractDecimal(char* a, char* b, char*& result, size_t opSize);
/**
 * @brief Subtracts two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* subtractDecimal(char* a,char* b);
/**
 * @brief Subtracts two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param sign Sign. Zero when the number is positive.
 * @return Resulting text. The caller frees the memory.
 */
  char* subtractDecimal(char* a,char* b, bool& sign);
/**
 * @brief Subtracts two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @return Resulting text. The caller frees the memory.
 */
  char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize);
/**
 * @brief Subtracts two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param sign Sign. Zero when the number is positive.
 * @return Resulting text. The caller frees the memory.
 */
  char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize, bool&sign);
/**
 * @brief Subtracts two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* subtractDecimal(const char* a,const char*b);
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
  char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize, size_t& resSize, bool& sign);

/**
 * @brief Computes a product in the current base.
 * @param a First operand.
 * @param b Second operand.
 * @param result Parameter @p result.
 * @param aSize Number of digits of the first text.
 * @param resSize Parameter @p resSize.
 */
  void multiplyByDigit(char* a, uchar b, char*& result, size_t aSize, size_t resSize);
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
  void multiplyDecimal(char* a, char* b, char*& result, char*& tmpResult,
                          size_t aSize, size_t bSize, size_t resSize);
/**
 * @brief Multiplies two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* multiplyDecimal(char* a, char* b);
/**
 * @brief Multiplies two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @return Resulting text. The caller frees the memory.
 */
  char* multiplyDecimal(char* a, char* b, size_t aSize, size_t bSize);
/**
 * @brief Multiplies two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* multiplyDecimal(const char* a, const char* b);
/**
 * @brief Multiplies two decimal texts.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param resSize Parameter @p resSize.
 * @return Resulting text. The caller frees the memory.
 */
  char* multiplyDecimal(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize);

/**
 * @brief Divides two decimal texts and returns the quotient.
 * @param a First operand.
 * @param b Second operand.
 * @param remain Receives the remainder of the division.
 * @return Resulting text. The caller frees the memory.
 */
  char* divideDecimal(char*a, char* b, char*& remain);
/**
 * @brief Divides two decimal texts and returns the quotient.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* divideDecimal(char*a, char* b);
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
  char const* divideDecimal(char* a, char* b, char*& result, char*& remain,
        size_t& aSize, size_t& bSize, size_t& resSize, size_t& remSize);
/**
 * @brief Divides two decimal texts and returns the quotient.
 * @param a First operand.
 * @param b Second operand.
 * @param remain Receives the remainder of the division.
 * @return Resulting text. The caller frees the memory.
 */
  char* divideDecimal(const char* a, const char* b, char*&remain);
/**
 * @brief Divides two decimal texts and returns the quotient.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* divideDecimal(const char* a, const char* b);
/**
 * @brief Divides two decimal texts and returns the quotient.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param resSize Parameter @p resSize.
 * @return Resulting text. The caller frees the memory.
 */
  char* divideDecimal(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize);


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
  char const* divideWithFraction(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain,
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
  char const* divideWithScale(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain,
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
  char const* divideWithScale(char*_a, char* _b, char*& resInt, char*& resDec,
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
  char const* divideDecimal(char*a, char* b, char*& result,
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
  char const* divideWithFraction(char* a, char* b, char*& resInt, char*& resDec, char*& remain, size_t nDecimals);

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
  char const* remainderDecimal(char*a, char* b, char*& remain,
        size_t& aSize, size_t& bSize, size_t& remSize);
/**
 * @brief Computes the remainder of a rational division.
 * @param a First operand.
 * @param b Second operand.
 * @param remain Receives the remainder of the division.
 */
  void remainderDecimal(char*a, char* b, char*&remain);
/**
 * @brief Computes the remainder of a rational division.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* remainderDecimal(char*a, char* b);
/**
 * @brief Computes the remainder of a rational division.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting text. The caller frees the memory.
 */
  char* remainderDecimal(const char*a, const char* b);

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
 * @param gcdSize Parameter @p gcdSize.
 * @return Resulting text. The caller frees the memory.
 */
  char* gcd(const char* a, const char* b, size_t& gcdSize);
/**
 * @brief Computes the greatest common divisor.
 * @param a First operand.
 * @param b Second operand.
 * @param gcdSize Parameter @p gcdSize.
 * @return Resulting text. The caller frees the memory.
 */
  char* gcd(char* a, char* b, size_t& gcdSize);
/**
 * @brief Computes the greatest common divisor.
 * @param a First operand.
 * @param b Second operand.
 * @param aSize Number of digits of the first text.
 * @param bSize Number of digits of the second text.
 * @param gcdSize Parameter @p gcdSize.
 * @return Resulting text. The caller frees the memory.
 */
  char* gcd(char* a, char* b, size_t aSize, size_t bSize, size_t& gcdSize);

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

[[deprecated("use formatReal")]]
inline char* getPrintFormat(const csReal& a) { return formatReal(a); }
[[deprecated("use setRealPrecision")]]
inline void setRNumberPrecision(int precision) { setRealPrecision(precision); }
[[deprecated("use realPrecision")]]
inline int getRNumberPrecision() { return realPrecision(); }
[[deprecated("use printDigits")]]
inline void printNumber(char* nb, const char* seperator = " ") { printDigits(nb, seperator); }
[[deprecated("use digitPairIndex")]]
inline uchar bicharIndex(uchar tens, uchar units) { return digitPairIndex(tens, units); }
[[deprecated("use digitTripleIndex")]]
inline uchar tricharIndex(int cents, uchar tens, uchar units) { return digitTripleIndex(cents, tens, units); }
[[deprecated("use subtractionStep")]]
inline csBIDIGITS substractionTransform(int i) { return subtractionStep(i); }
[[deprecated("use multiplyDigitBase2")]]
inline void makeMultiplicationBase2(char* a, uchar b, char*& result, size_t aSize, size_t resSize, csBIDIGITS& prevCarry, size_t m, size_t n) { multiplyDigitBase2(a, b, result, aSize, resSize, prevCarry, m, n); }
[[deprecated("use addShiftedProduct")]]
inline void makeAdditionForMul(char* a, char* b, char*& result, size_t opSize, size_t resSize) { addShiftedProduct(a, b, result, opSize, resSize); }
[[deprecated("use subtractPartialQuotient")]]
inline void makeSubstractionForDiv(char* a, char* b, char*& result, size_t opSize, size_t frontOffset) { subtractPartialQuotient(a, b, result, opSize, frontOffset); }
[[deprecated("use multiplyDigitForDivision")]]
inline void makeMultiplicationForDivBase1(char* a, uchar b, char*& result, size_t aSize, size_t resSize) { multiplyDigitForDivision(a, b, result, aSize, resSize); }
[[deprecated("use multiplyDigitPairForDivision")]]
inline void makeMultiplicationForDivBase2(char* a, uchar b, char*& result, size_t aSize, size_t resSize) { multiplyDigitPairForDivision(a, b, result, aSize, resSize); }
[[deprecated("use packDigitPairs")]]
inline csBIDIGITS** toBIDIGITS(int** opTable) { return packDigitPairs(opTable); }
[[deprecated("use buildMultiplicationTable")]]
inline void makeMultiplicationTable() { buildMultiplicationTable(); }
[[deprecated("use buildAdditionTable")]]
inline void createAdditionTable() { buildAdditionTable(); }
[[deprecated("use buildDivisionTable")]]
inline void makeDivisionTable() { buildDivisionTable(); }
[[deprecated("use buildSubtractionTable")]]
inline void makeSubstractionTable() { buildSubtractionTable(); }
[[deprecated("use printDigitTable")]]
inline void printTable(char* opName) { printDigitTable(opName); }
[[deprecated("use addDecimal")]]
inline void makeAddition(char* a, char* b, char*& result, size_t opSize, size_t& resSize) { addDecimal(a, b, result, opSize, resSize); }
[[deprecated("use addDecimal")]]
inline char* makeAddition(char* a, char* b) { return addDecimal(a, b); }
[[deprecated("use addDecimal")]]
inline char* makeAddition(char* a, char* b, size_t& resSize) { return addDecimal(a, b, resSize); }
[[deprecated("use addDecimal")]]
inline char* makeAddition(const char* a, const char* b) { return addDecimal(a, b); }
[[deprecated("use subtractDecimal")]]
inline void makeSubstraction(char* a, char* b, char*& result, size_t opSize) { subtractDecimal(a, b, result, opSize); }
[[deprecated("use subtractDecimal")]]
inline char* makeSubstraction(char* a, char* b) { return subtractDecimal(a, b); }
[[deprecated("use subtractDecimal")]]
inline char* makeSubstraction(char* a, char* b, bool& sign) { return subtractDecimal(a, b, sign); }
[[deprecated("use subtractDecimal")]]
inline char* makeSubstraction(char* a, char* b, size_t aSize, size_t bSize) { return subtractDecimal(a, b, aSize, bSize); }
[[deprecated("use subtractDecimal")]]
inline char* makeSubstraction(char* a, char* b, size_t aSize, size_t bSize, bool& sign) { return subtractDecimal(a, b, aSize, bSize, sign); }
[[deprecated("use subtractDecimal")]]
inline char* makeSubstraction(const char* a, const char* b) { return subtractDecimal(a, b); }
[[deprecated("use subtractDecimal")]]
inline char* makeSubstraction(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize, bool& sign) { return subtractDecimal(a, b, aSize, bSize, resSize, sign); }
[[deprecated("use multiplyByDigit")]]
inline void makeMultiplicationBase(char* a, uchar b, char*& result, size_t aSize, size_t resSize) { multiplyByDigit(a, b, result, aSize, resSize); }
[[deprecated("use multiplyDecimal")]]
inline void makeMultiplication(char* a, char* b, char*& result, char*& tmpResult, size_t aSize, size_t bSize, size_t resSize) { multiplyDecimal(a, b, result, tmpResult, aSize, bSize, resSize); }
[[deprecated("use multiplyDecimal")]]
inline char* makeMultiplication(char* a, char* b) { return multiplyDecimal(a, b); }
[[deprecated("use multiplyDecimal")]]
inline char* makeMultiplication(char* a, char* b, size_t aSize, size_t bSize) { return multiplyDecimal(a, b, aSize, bSize); }
[[deprecated("use multiplyDecimal")]]
inline char* makeMultiplication(const char* a, const char* b) { return multiplyDecimal(a, b); }
[[deprecated("use multiplyDecimal")]]
inline char* makeMultiplication(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize) { return multiplyDecimal(a, b, aSize, bSize, resSize); }
[[deprecated("use divideDecimal")]]
inline char* makeDivisionQ(char* a, char* b, char*& remain) { return divideDecimal(a, b, remain); }
[[deprecated("use divideDecimal")]]
inline char* makeDivisionQ(char* a, char* b) { return divideDecimal(a, b); }
[[deprecated("use divideDecimal")]]
inline char const* makeDivisionQ(char* a, char* b, char*& result, char*& remain, size_t& aSize, size_t& bSize, size_t& resSize, size_t& remSize) { return divideDecimal(a, b, result, remain, aSize, bSize, resSize, remSize); }
[[deprecated("use divideDecimal")]]
inline char* makeDivisionQ(const char* a, const char* b, char*& remain) { return divideDecimal(a, b, remain); }
[[deprecated("use divideDecimal")]]
inline char* makeDivisionQ(const char* a, const char* b) { return divideDecimal(a, b); }
[[deprecated("use divideDecimal")]]
inline char* makeDivisionQ(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize) { return divideDecimal(a, b, aSize, bSize, resSize); }
[[deprecated("use divideDecimal")]]
inline char const* makeDivisionQ(char* a, char* b, char*& result, size_t& aSize, size_t& bSize, size_t& resSize) { return divideDecimal(a, b, result, aSize, bSize, resSize); }
[[deprecated("use divideWithFraction")]]
inline char const* makeDivisionR(char* _a, char* _b, char*& resInt, char*& resDec, char*& remain, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals) { return divideWithFraction(_a, _b, resInt, resDec, remain, _aSize, bSize, resSize, resIntSize, resDecSize, remSize, nDecimals); }
[[deprecated("use divideWithScale")]]
inline char const* makeDivisionR2(char* _a, char* _b, char*& resInt, char*& resDec, char*& remain, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals) { return divideWithScale(_a, _b, resInt, resDec, remain, _aSize, bSize, resSize, resIntSize, resDecSize, remSize, nDecimals); }
[[deprecated("use divideWithScale")]]
inline char const* makeDivisionR2(char* _a, char* _b, char*& resInt, char*& resDec, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t nDecimals) { return divideWithScale(_a, _b, resInt, resDec, _aSize, bSize, resSize, resIntSize, resDecSize, nDecimals); }
[[deprecated("use divideWithFraction")]]
inline char const* makeDivisionR(char* a, char* b, char*& resInt, char*& resDec, char*& remain, size_t nDecimals) { return divideWithFraction(a, b, resInt, resDec, remain, nDecimals); }
[[deprecated("use remainderDecimal")]]
inline char const* makeModulusQ(char* a, char* b, char*& remain, size_t& aSize, size_t& bSize, size_t& remSize) { return remainderDecimal(a, b, remain, aSize, bSize, remSize); }
[[deprecated("use remainderDecimal")]]
inline void makeModulusQ(char* a, char* b, char*& remain) { remainderDecimal(a, b, remain); }
[[deprecated("use remainderDecimal")]]
inline char* makeModulusQ(char* a, char* b) { return remainderDecimal(a, b); }
[[deprecated("use remainderDecimal")]]
inline char* makeModulusQ(const char* a, const char* b) { return remainderDecimal(a, b); }

#endif
