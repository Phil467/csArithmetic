#pragma once

#if defined _WIN32 || defined __CYGWIN__
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

#ifndef CSARITHMETIC_H_INCLUDED
#define CSARITHMETIC_H_INCLUDED

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
#include <cxxabi.h>//abi
//#include <bits/stdc++.h>//auto
#include <experimental/source_location>
using std::experimental::source_location;

//#include "csPARALLEL.h"
#include "csArithUtils.h"

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
  typedef struct csQNUMBER csQNUMBER;
  typedef struct csRNUMBER csRNUMBER;
  char* getPrintFormat(csRNUMBER a);
  void setRNumberPrecision(int precision);
  int getRNumberPrecision();


  struct CSARITHMETIC_API csQNUMBER
  {
    char* numerator=0;
    char* denominator=0;
    bool sign = CS_POSITIVE_NUMBER;
    size_t numSize;
    size_t denomSize;

    csQNUMBER(const char* numerator="0", const char* denominator="1", bool sign=0);
    csQNUMBER(size_t num, size_t denom=1, bool sign=0);
    void init();
    //~csQNUMBER();
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
    friend std::ostream& operator<<(std::ostream& os, const csQNUMBER& a)
    {
      if(a.sign)
        os << "-"<< a.numerator << " / " << a.denominator;
      else
        os << a.numerator << " / " << a.denominator;
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
    //void clear();
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
    friend std::ostream& operator<<(std::ostream& os, const csRNUMBER& a)
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

  char* findGcd(char* a);

  csRNUMBER csSqrt(csRNUMBER n , int precision);
  csRNUMBER csSqrt_Prec(csRNUMBER n , int precision);
  csQNUMBER csSqrtQ_Prec(csQNUMBER n, int precision);

  csRNUMBER csSqrt0_Prec(csRNUMBER C2, csRNUMBER a, csRNUMBER b, int precision);
  csRNUMBER cs10PSqrt_Prec(long power, long precision);
  csRNUMBER cs10PSqrt_Prec(csRNUMBER rt10, long power);

  csQNUMBER csSqrtQ(csQNUMBER n, int precision);
  csRNUMBER BSqrtR_Prec(csRNUMBER C2, int precision);
  csRNUMBER NRSqrt(csRNUMBER n);
  csRNUMBER NRSqrtR(csRNUMBER _C2, long prec, size_t nIter=10);
  csRNUMBER NRSqrtR_Bissec(csRNUMBER _C2, long prec, size_t nIter=10);
  csRNUMBER NRSqrtR0(csRNUMBER _C2, csRNUMBER x0, long prec, size_t nIter=10);
  csQNUMBER NRSqrtQ(csQNUMBER n, long precision, size_t nIter=10);
  csQNUMBER nsr(csQNUMBER a);
  csQNUMBER nsr2(csQNUMBER a);

  csRNUMBER aplr(csRNUMBER a);

  csRNUMBER* csSortMinR(csRNUMBER* rn, size_t size);
  void csSortMinR(csRNUMBER*& rn, size_t size);
  void csSortMin_CoordsByRNumber(csFCOORDS*& fc, csRNUMBER*& rn, size_t size);
  void csSortMax_CoordsByRNumber(csFCOORDS*& fc, csRNUMBER*& rn, size_t size);
  }

CSARITHMETIC_API CSARITHMETIC::csQNUMBER csReduce(CSARITHMETIC_API CSARITHMETIC::csQNUMBER a, size_t sizeCondition);

#endif
