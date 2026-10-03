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

#ifndef CSARITHMETIC_OPT6_H
#define CSARITHMETIC_OPT6_H

#include <iostream>

namespace __mem_man
{
template<class T> CS_FORCE_INLINE T* csAlloc(size_t n)
{
  T*t = (T*)malloc(n*sizeof(T));
  return t;
}

template<class T> CS_FORCE_INLINE T* csAlloc(size_t n, T init)
{
  T*t = (T*)malloc(n*sizeof(T));
  for(size_t i=0; i<n; i++)
  {
    t[i] = init;
  }
  return t;
}

template<class T> CS_FORCE_INLINE T* csAlloc2(size_t n, T init)
{
  T*t = (T*)malloc(n*sizeof(T));
  for(size_t i=0; i<n; i++)
  {
    t[i] = init;
  }
  return t;
}

char* CSARITHMETIC_API csAllocCharPtr(size_t n, size_t n1, char init);
char* CSARITHMETIC_API csAllocCharPtr(size_t n, char init);

void CSARITHMETIC_API csReallocString(char** str, size_t size);
void CSARITHMETIC_API csReallocString(char** str, size_t size, size_t newSize, char cFill);

char* CSARITHMETIC_API newString(char*cstr, size_t size);
char* CSARITHMETIC_API newString(const char*cstr, size_t size);
char* CSARITHMETIC_API newString(const char*cstr, size_t begin, size_t end);
char* CSARITHMETIC_API newString(const char*cstr);
char* intToString(int nb, size_t& sz);
char* uLongToString(size_t nb, size_t& sz);

};

namespace __ar_man
{
void shiftRight(char*& nb, size_t size, size_t nShift);
char* shiftRightCopy(char* nb, size_t nbSize, size_t nbCopySize);
void shiftLeft(char*& nb, size_t size, size_t nShift);
void shiftLeftCopy(char* nb, char*&nbCopy, size_t nbSize, size_t nbCopyPos);
void skipZeros(char* a, size_t aSize, size_t& skipLen);
void skipZeros2(char* a, size_t aSize, size_t& skipLen, size_t&incr);
bool isaGreater(char* a,char* b,size_t opSize, size_t ibegin);
bool isaGreaterEqual(char* a,char* b,size_t opSize, size_t ibegin);
bool isaGreater2(char* a,char* b,size_t aSize,size_t bSize, size_t ibegin);
bool isaGreaterEqual2(char* a,char* b,size_t aSize, size_t bSize, size_t ibegin);

int getReadySub(char*& a,char*& b, size_t&opSize);
bool getReadySub2(const char* a0,const char* b0, char*& a, char*& b, size_t& opSize);
bool getReadySub3(const char* a0,const char* b0, char*& a, char*& b, size_t aSize, size_t bSize, size_t&opSize);

void getReady(char*& a,char*& b, size_t& opSize);
void getReady2(const char* a0,const char* b0, char*& a, char*& b, size_t& opSize);

bool removeFrontZeros(char*& a, size_t& size);
void removeLeft(char*& nb, size_t& size, size_t remLen);
void removeRight(char*& nb, size_t& size, size_t remLen);
void addFrontZeros(char*& a, size_t& size, size_t nZeros);
void fillString(char*& str, const char* cstr, size_t size);
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
#include <cxxabi.h>
#include <experimental/source_location>
using std::experimental::source_location;


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

#define CS_Q_BASE 10000000000000000000ULL
#define CS_Q_DIGS 19
CS_INLINE static char* csLimbsToDec(const void* raw, size_t n)
{
  const unsigned long long* p = (const unsigned long long*)raw;
  if (!p || n == 0 || (n == 1 && p[0] == 0)) {
    char* z = (char*)malloc(2);
    z[0] = '0';
    z[1] = 0;
    return z;
  }
  char* buf = (char*)malloc(n * (size_t)CS_Q_DIGS + 1);
  char* w = buf;
  w += sprintf(w, "%llu", p[n - 1]);
  for (size_t i = n - 1; i-- > 0; )
    w += sprintf(w, "%019llu", p[i]);
  return buf;
}


  typedef struct csQNUMBER csQNUMBER;
  typedef struct csRNUMBER csRNUMBER;
  char* getPrintFormat(csRNUMBER a);
  void setRNumberPrecision(int precision);
  int getRNumberPrecision();


  struct csQRaw {};

  struct CSARITHMETIC_API csQNUMBER
  {
    char* numerator=0;
    char* denominator=0;
    bool sign = CS_POSITIVE_NUMBER;
    size_t numSize;
    size_t denomSize;

    csQNUMBER(const char* numerator="0", const char* denominator="1", bool sign=0);
    csQNUMBER(size_t num, size_t denom=1, bool sign=0);
    csQNUMBER(csQRaw);
    void init();
    void set(const char* numerator, const char* denominator, bool sign);
    void set(size_t num, size_t denom=1, bool sign=0);
    void setl(long num, size_t denom=1);
    void reduce(size_t precision);
    csQNUMBER operator+(csQNUMBER a);
    csQNUMBER operator-(csQNUMBER a);
    csQNUMBER operator*(csQNUMBER a);
    csQNUMBER operator/(csQNUMBER a);
    csQNUMBER operator+(long a);
    csQNUMBER operator-(long a);
    csQNUMBER operator*(long a);
    csQNUMBER operator/(long a);
    csQNUMBER& operator=(csQNUMBER a);
    csQNUMBER& operator=(long a);
    bool operator==(csQNUMBER a);
    bool isAbsEqual(csQNUMBER a);
    bool operator!=(csQNUMBER a);
    bool isAbsDifferent(csQNUMBER a);
    bool operator>(csQNUMBER a);
    bool isAbsGreater(csQNUMBER a);
    bool operator<(csQNUMBER a);
    bool isAbsLess(csQNUMBER a);
    bool operator>=(csQNUMBER a);
    bool isAbsGreaterEqual(csQNUMBER a);
    bool operator<=(csQNUMBER a);
    bool isAbsLessEqual(csQNUMBER a);

    operator csRNUMBER();
    CS_FORCE_INLINE friend std::ostream& operator<<(std::ostream& os, const csQNUMBER& a)
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
    bool isZero();
    bool isNonZero();
    void copy(csQNUMBER a);
    double getDouble();
    size_t maxSize();
    char* quotient();
    void print(const char*title);
    void clear(const source_location loc = source_location::current());
  };
  csQNUMBER* csQNUMBER_PTR_ALLOC(size_t nb);
  csQNUMBER* csQNUMBER_PTR_ALLOC(size_t nb, csQNUMBER init);
  void csQNUMBER_PTR_FREE(csQNUMBER*& qn, size_t size);
  csRNUMBER* csRNUMBER_PTR_ALLOC(size_t nb);
  csRNUMBER* csRNUMBER_PTR_ALLOC(size_t nb, csRNUMBER init);
  void csRNUMBER_PTR_FREE(csRNUMBER*& rn, size_t size);

  struct CSARITHMETIC_API csRNUMBER
  {
    char* mantissa=0;
    int exponent=0;
    bool sign = CS_POSITIVE_NUMBER;
    long precision=0;
    size_t mantSize;
    csRNUMBER(const char* mantissa="0", int exponent=0, bool sign=0);
    csRNUMBER(unsigned long mantissa, int exponent=0, bool sign=0);
    csRNUMBER(long mantissa, int exponent=0);
    csRNUMBER(bool evaluate, const char* number);
    void init();
    ~csRNUMBER();
    void set(const char* mantissa, int exponent, bool sign);
    void set(unsigned long mantissa, int exponent, bool sign);
    void set(long mantissa, int exponent);
    void set(bool evaluate, const char* number);
    void setPrecision(int precision);
    void forcePrecision(int size);
    void increaseShape(size_t size);
    void decreaseShape(size_t size);
    void reshape(size_t newMantissaSize);
    void shapeOutZeros();
    void reduce(size_t size);
    size_t getDigitNumber();
    csRNUMBER abs();
    csRNUMBER operator+(csRNUMBER a);
    csRNUMBER operator-(csRNUMBER a);
    csRNUMBER operator*(csRNUMBER a);
    csRNUMBER operator/(csRNUMBER a);
    csRNUMBER operator+(long a);
    csRNUMBER operator-(long a);
    csRNUMBER operator*(long a);
    csRNUMBER operator/(long a);
    csRNUMBER operator-() const;
    void operator=(csRNUMBER a);
    void operator=(long a);
    void operator=(const char* a);
    bool operator==(csRNUMBER a);
    bool operator!=(csRNUMBER a);
    bool operator>(csRNUMBER a);
    bool operator<(csRNUMBER a);
    bool operator>=(csRNUMBER a);
    bool operator<=(csRNUMBER a);
    bool operator==(long a);
    bool operator!=(long a);
    bool operator>(long a);
    bool operator>=(long a);
    bool operator<(long a);
    bool operator<=(long a);

    operator csQNUMBER();
    CS_FORCE_INLINE friend std::ostream& operator<<(std::ostream& os, const csRNUMBER& a)
    {
      char*formated = getPrintFormat(a);
      cout<<formated;
      free(formated);
      return os;
    }
    void copy(csRNUMBER a);
    bool equalZero();
    void print(const char* title);
    void print2(const char* title);
    csRNUMBER getMantissaSection(size_t first, size_t last);
    void assign(csRNUMBER a);
    void set(size_t id, char digit);
    void setInt(size_t id, char digit);
    void setDec(size_t id, char digit);
    void setAsTenPower(long power);
    void random(size_t nDigits, char digitMin=0, char digitMax=9, long exponent=0, bool sign=0);
    size_t significantZerosCount(size_t initialPos=0);
    void clear();
  };
  void init();
  void printNumber(char* nb, const char* seperator=" ");
  uchar bicharIndex(uchar tens, uchar units);
  uchar tricharIndex(int cents, uchar tens, uchar units);
  csBIDIGITS substractionTransform(int i);

  void makeMultiplicationBase2(char* a, uchar b, char*& result, size_t aSize, size_t resSize,csBIDIGITS& prevCarry, size_t m, size_t n);
  void makeAdditionForMul(char* a, char* b, char*& result, size_t opSize, size_t resSize);
  void makeSubstractionForDiv(char* a, char* b, char*& result, size_t opSize, size_t frontOffset);
  void makeMultiplicationForDivBase1(char* a, uchar b, char*& result, size_t aSize, size_t resSize);
  void makeMultiplicationForDivBase2(char* a, uchar b, char*& result, size_t aSize, size_t resSize);

  csBIDIGITS** toBIDIGITS(int** opTable);
  void makeMultiplicationTable();
  void createAdditionTable();
  void makeDivisionTable();
  void makeSubstractionTable();
  void printTable(char* opName);

  void makeAddition(char* a, char* b, char*& result, size_t opSize, size_t& resSize);
  char* makeAddition(char* a,char* b);
  char* makeAddition(char* a,char* b, size_t& resSize);
  char* makeAddition(const char* a,const char* b);


  void makeSubstraction(char* a, char* b, char*& result, size_t opSize);
  char* makeSubstraction(char* a,char* b);
  char* makeSubstraction(char* a,char* b, bool& sign);
  char* makeSubstraction(char* a,char* b, size_t aSize, size_t bSize);
  char* makeSubstraction(char* a,char* b, size_t aSize, size_t bSize, bool&sign);
  char* makeSubstraction(const char* a,const char*b);
  char* makeSubstraction(char* a,char* b, size_t aSize, size_t bSize, size_t& resSize, bool& sign);

  void makeMultiplicationBase(char* a, uchar b, char*& result, size_t aSize, size_t resSize);
  void makeMultiplication(char* a, char* b, char*& result, char*& tmpResult,
                          size_t aSize, size_t bSize, size_t resSize);
  char* makeMultiplication(char* a, char* b);
  char* makeMultiplication(char* a, char* b, size_t aSize, size_t bSize);
  char* makeMultiplication(const char* a, const char* b);
  char* makeMultiplication(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize);

  char* makeDivisionQ(char*a, char* b, char*& remain);
  char* makeDivisionQ(char*a, char* b);
  char const* makeDivisionQ(char* a, char* b, char*& result, char*& remain,
        size_t& aSize, size_t& bSize, size_t& resSize, size_t& remSize);
  char* makeDivisionQ(const char* a, const char* b, char*&remain);
  char* makeDivisionQ(const char* a, const char* b);
  char* makeDivisionQ(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize);


  char const* makeDivisionR(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain,
                    size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals);
  char const* makeDivisionR2(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain,
                    size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals);
  char const* makeDivisionR2(char*_a, char* _b, char*& resInt, char*& resDec,
                    size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t nDecimals);
  char const* makeDivisionQ(char*a, char* b, char*& result,
        size_t& aSize, size_t& bSize, size_t& resSize);
  char const* makeDivisionR(char* a, char* b, char*& resInt, char*& resDec, char*& remain, size_t nDecimals);

  char const* makeModulusQ(char*a, char* b, char*& remain,
        size_t& aSize, size_t& bSize, size_t& remSize);
  void makeModulusQ(char*a, char* b, char*&remain);
  char* makeModulusQ(char*a, char* b);
  char* makeModulusQ(const char*a, const char* b);

  csQNUMBER pow(csQNUMBER a, size_t p);

  char* gcd(const char* a, const char* b, size_t& gdcSize);
  char* gcd(char* a, char* b, size_t& gdcSize);
  char* gcd(char* a, char* b, size_t aSize, size_t bSize, size_t& gdcSize);

  csRNUMBER* csSortMinR(csRNUMBER* rn, size_t size);
  void csSortMinR(csRNUMBER*& rn, size_t size);
  void csSortMin_CoordsByRNumber(csFCOORDS*& fc, csRNUMBER*& rn, size_t size);
  void csSortMax_CoordsByRNumber(csFCOORDS*& fc, csRNUMBER*& rn, size_t size);
  }

CSARITHMETIC_API CSARITHMETIC::csQNUMBER csReduce(CSARITHMETIC_API CSARITHMETIC::csQNUMBER a, size_t sizeCondition);

#include <thread>
using namespace CSARITHMETIC;

#endif
