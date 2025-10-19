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

#ifndef CSARITHUTILS_H
#define CSARITHUTILS_H

#include <iostream>

namespace __mem_man
{
template<class T> T* csAlloc(size_t n)
{
  T*t = (T*)malloc(n*sizeof(T));
  return t;
}

template<class T> T* csAlloc(size_t n, T init)
{
  T*t = (T*)malloc(n*sizeof(T));
  for(size_t i=0; i<n; i++)
  {
    t[i] = init;
  }
  return t;
}

template<class T> T* csAlloc2(size_t n, T init)
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



#endif // CSARITHUTILS_H
