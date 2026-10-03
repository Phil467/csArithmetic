#include "csArithmeticOpt4.h"


enum { CS_STACK_MAX = 2048 };

struct csScratch {
  char small[CS_STACK_MAX + 1];
  char* p;
  bool heap;
  CS_FORCE_INLINE csScratch() {
    p = 0;
    heap = false;
  }
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
  CS_FORCE_INLINE void copyFrom(const char* s, size_t n) {
    alloc(n, 0);
    for (size_t i = 0; i < n; ++i)
      p[i] = s[i];
    p[n] = '\0';
  }
  CS_FORCE_INLINE void release() {
    if (heap && p)
      free(p);
    p = 0;
    heap = false;
  }
  CS_FORCE_INLINE ~csScratch() { release(); }
};

CS_FORCE_INLINE static void csPlaceAligned(const char* src, size_t srcSize, char* dst, size_t opSize) {
  size_t delta = opSize - srcSize;
  for (size_t i = 0; i < srcSize; ++i)
    dst[delta + i] = src[i];
  dst[opSize] = '\0';
}

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

CS_FORCE_INLINE CSARITHMETIC::csQNUMBER::csQNUMBER(CSARITHMETIC::csQRaw) {
  numerator = 0;
  denominator = 0;
  numSize = 0;
  denomSize = 0;
  sign = CS_POSITIVE_NUMBER;
}

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

CS_FORCE_INLINE static void csGcdModStep(char*& a, char*& b, char*& remain,
                        size_t& aSize, size_t& bSize, size_t& remSize,
                        csScratch buf[3], int& ia, int& ib, int& ir, size_t cap) {
  remain = buf[ir].p;
  remSize = cap;
  CSARITHMETIC::makeModulusQ(a, b, remain, aSize, bSize, remSize);
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
  char* src = nb + nShift;
  char* dst = nb;
  char* end = nb + size;
  while (src < end)
    *dst++ = *src++;
  char* z = nb + (size - nShift);
  char* zend = nb + size;
  while (z < zend)
    *z++ = '0';
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


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::createAdditionTable()
{
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeAddition(char*a, char*b, char*&result, size_t opSize, size_t& resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  char* pa = a + opSize - 1;
  char* pb = b + opSize - 1;
  char* pr = result + opSize;
  for (size_t left = opSize; left > 0; --left)
  {
    csBIDIGITS bd1 = addTable[int(*pa)][int(*pb)];
    csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
    prevCarry = addTable[bd1.tens][bd2.tens];
    *pr = bd2.units;
    --pa;
    --pb;
    --pr;
  }
  result[0] = prevCarry.units;
  removeFrontZeros(result, resSize);
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeAddition(char*a,char*b)
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
  makeAddition(aBuf.p, bBuf.p, result, opSize, resSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeAddition(char*a,char*b, size_t&resSize)
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
  makeAddition(aBuf.p, bBuf.p, result, opSize, resSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeAddition(const char*a,const char*b)
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
  makeAddition(aBuf.p, bBuf.p, result, opSize, resSize);
  return result;
}

/* ---- src/csSubstraction.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeSubstractionTable()
{
}


CS_FORCE_INLINE csBIDIGITS CSARITHMETIC_API CSARITHMETIC::substractionTransform(int i)
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

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a, char*b, char*&result, size_t opSize)
{
  csBIDIGITS prevCarry={'0','0'};
  char* pa = a + opSize - 1;
  char* pb = b + opSize - 1;
  char* pr = result + opSize - 1;
  for (size_t left = opSize; left > 0; --left)
  {
    prevCarry = subTable[int(*pa)][prevCarry.tens];
    csBIDIGITS bd = subTable[prevCarry.units][*pb];
    prevCarry.tens = prevCarry.tens > bd.tens ? prevCarry.tens : bd.tens;
    *pr = bd.units;
    --pa;
    --pb;
    --pr;
  }
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a,char*b)
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
  makeSubstraction(pa, pb, result, opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a,char*b, bool& sign)
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
  makeSubstraction(pa, pb, result, opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a,char*b, size_t aSize, size_t bSize, bool& sign)
{
  size_t opSize = aSize > bSize ? aSize : bSize;
  csScratch aBuf, bBuf;
  aBuf.alloc(opSize, '0');
  bBuf.alloc(opSize, '0');
  char* pa = aBuf.p;
  char* pb = bBuf.p;
  sign = csAlignSub(a, aSize, b, bSize, pa, pb, opSize);
  char* result = csAllocCharPtr(opSize, '0');
  makeSubstraction(pa, pb, result, opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a,char*b, size_t aSize, size_t bSize, size_t& resSize, bool& sign)
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
  makeSubstraction(pa, pb, result, resSize);
  return result;
}


CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(const char*a,const char*b)
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
  makeSubstraction(pa, pb, result, opSize);
  return result;
}

/* ---- src/csMultiplication.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationTable()
{
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationBase(char*a, uchar b, char*&result, size_t aSize, size_t resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  if (aSize)
  {
    char* pa = a + aSize - 1;
    char* pr = result + resSize - 1;
    for (size_t left = aSize; left > 0; --left)
    {
      csBIDIGITS bd1 = mulTable[int(*pa)][b];
      csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
      prevCarry = addTable[bd1.tens][bd2.tens];
      *pr = bd2.units;
      --pa;
      --pr;
    }
    *pr = prevCarry.units;
  }
  else if (resSize)
    result[resSize - 1] = prevCarry.units;
  free(result);
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationBase2(char*a, uchar b, char*&result, size_t aSize, size_t resSize
    ,csBIDIGITS& prevCarry, size_t m, size_t n)
{
  prevCarry={'0','0'};
  if (aSize)
  {
    char* pa = a + m;
    char* pr = result + n;
    for (size_t left = aSize; left > 0; --left)
    {
      csBIDIGITS bd1 = mulTable[int(*pa)][b];
      csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
      prevCarry = addTable[bd1.tens][bd2.tens];
      *pr = bd2.units;
      --pa;
      --pr;
    }
    *pr = prevCarry.units;
  }
  else
    result[n] = prevCarry.units;
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeAdditionForMul(char*a, char*b, char*&result, size_t opSize, size_t resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  if (opSize)
  {
    char* pa = a + resSize - 1;
    char* pb = b + resSize - 1;
    char* pr = result + resSize - 1;
    for (size_t left = opSize; left > 0; --left)
    {
      csBIDIGITS bd1 = addTable[int(*pa)][int(*pb)];
      csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
      prevCarry = addTable[bd1.tens][bd2.tens];
      *pr = bd2.units;
      --pa;
      --pb;
      --pr;
    }
  }
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeMultiplication(char*a, char*b, char*&result, char*& tmpResult,
 size_t aSize, size_t bSize, size_t resSize)
{
  size_t n = aSize + 1;
  csBIDIGITS prevCarry;
  size_t m1 = aSize - 1;
  size_t n1 = resSize - 1;
  char* pb = b + bSize;
  for (size_t j = 0; j < bSize; ++j)
  {
    --pb;
    makeMultiplicationBase2(a, (uchar)*pb, tmpResult, aSize, resSize, prevCarry, m1, n1);
    shiftLeft(tmpResult, resSize, j);
    makeAdditionForMul(result, tmpResult, result, n + j, resSize);
  }
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeMultiplication(char*a, char*b)
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

  makeMultiplication(a,b,result,tmpResult,aSize,bSize,opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeMultiplication(char*a, char*b, size_t aSize, size_t bSize)
{
  size_t opSize = aSize + bSize;
  size_t sz = opSize + 1;
  char*result = csAllocCharPtr(opSize, sz, '0');
  csScratch tmpBuf;
  tmpBuf.alloc(opSize, '0');
  char*tmpResult = tmpBuf.p;

  makeMultiplication(a,b,result,tmpResult,aSize,bSize,opSize);
  return result;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeMultiplication(char*a, char*b, size_t aSize, size_t bSize, size_t& resSize)
{
  resSize = aSize + bSize;
  size_t sz = resSize + 1;
  char*result = csAllocCharPtr(resSize, sz, '0');
  csScratch tmpBuf;
  tmpBuf.alloc(resSize, '0');
  char*tmpResult = tmpBuf.p;
  
  makeMultiplication(a,b,result,tmpResult,aSize,bSize,resSize);
   
  return result;
}


CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeMultiplication(const char*a, const char*b)
{
  return makeMultiplication((char*)a,(char*)b);
}

/* ---- src/csDivision.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeDivisionTable()
{
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeSubstractionForDiv(char*a, char*b, char*&result, size_t opSize, size_t frontOffset)
{
  csBIDIGITS prevCarry={'0','0'};
  char* pa = a + (opSize - 1 + frontOffset);
  char* pr = result + (opSize - 1 + frontOffset);
  char* pb = b + (opSize - 1);
  for (size_t left = opSize; left > 0; --left)
  {
    prevCarry = subTable[int(*pa)][prevCarry.tens];
    csBIDIGITS bd = subTable[prevCarry.units][*pb];
    prevCarry.tens = prevCarry.tens > bd.tens ? prevCarry.tens : bd.tens;
    *pr = bd.units;
    --pa;
    --pr;
    --pb;
  }
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationForDivBase1(char*a, uchar b, char*&result, size_t aSize, size_t resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  if (aSize)
  {
    char* pa = a + aSize - 1;
    char* pr = result + resSize - 1;
    for (size_t left = aSize; left > 0; --left)
    {
      csBIDIGITS bd1 = mulTable[int(*pa)][b];
      csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
      prevCarry = addTable[bd1.tens][bd2.tens];
      *pr = bd2.units;
      --pa;
      --pr;
    }
  }
}
CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationForDivBase2(char*a, uchar b, char*&result, size_t aSize, size_t resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  if (aSize)
  {
    char* pa = a + aSize - 1;
    char* pr = result + resSize - 1;
    for (size_t left = aSize; left > 0; --left)
    {
      csBIDIGITS bd1 = mulTable[int(*pa)][b];
      csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
      prevCarry = addTable[bd1.tens][bd2.tens];
      *pr = bd2.units;
      --pa;
      --pr;
    }
    *pr = prevCarry.units;
  }
  else if (resSize)
    result[resSize - 1] = prevCarry.units;
}


CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*_a, char* _b, char*&result,
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
          quotientDigit = (divTable[bicharIndex(a[skipZerosLen], a[skipZerosLen+1])][bicharIndex(b[0],b[1])]-1);

        if(quotientDigit == 48) quotientDigit = 49;
        
        makeMultiplicationForDivBase1(b, quotientDigit, tmpRes, tmpResSizeForSub, tmpResSizeForSub);
        makeSubstractionForDiv(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);

        while(isaGreaterEqual(a,b,tmpResSizeForSub,skipZerosLen))
        {
           makeSubstractionForDiv(a, b, a, tmpResSizeForSub, skipZerosLen);
           quotientDigit += 1;
        }
        
      }
      else
      {
        if(bSize == 1)
          quotientDigit = divTable[bicharIndex(a[skipZerosLen], a[skipZerosLen+1])][b[0]]-2;
        else
          quotientDigit = (divTable[tricharIndex(a[skipZerosLen], a[skipZerosLen+1], a[skipZerosLen+2])]
                                   [bicharIndex(b[0],b[1])]-1);

        if(quotientDigit <= 48) quotientDigit = 49;
        if(quotientDigit > 57) quotientDigit = 57;
      
        makeMultiplicationForDivBase2(b, quotientDigit, tmpRes, bSize, tmpResSizeForSub);
        makeSubstractionForDiv(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);

        while(isaGreaterEqual(a,bCpy,tmpResSizeForSub,skipZerosLen))
        {
           makeSubstractionForDiv(a, bCpy, a, tmpResSizeForSub, skipZerosLen);
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

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*_a, char* _b, char*&result, char*&remain,
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
          quotientDigit = (divTable[bicharIndex(a[skipZerosLen], a[skipZerosLen+1])][bicharIndex(b[0],b[1])]-1);

        if(quotientDigit == 48) quotientDigit = 49;


        makeMultiplicationForDivBase1(b, quotientDigit, tmpRes, tmpResSizeForSub, tmpResSizeForSub);
        makeSubstractionForDiv(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);

        while(isaGreaterEqual(a,b,tmpResSizeForSub,skipZerosLen))
        {
           makeSubstractionForDiv(a, b, a, tmpResSizeForSub, skipZerosLen);
           quotientDigit += 1;
        }
        
      }
      else
      {
        
      //convert the first and second digits to a number, to find the quotient with the first digit of b

        if(bSize == 1)
          quotientDigit = divTable[bicharIndex(a[skipZerosLen], a[skipZerosLen+1])][b[0]]-2;
        else
          quotientDigit = (divTable[tricharIndex(a[skipZerosLen], a[skipZerosLen+1], a[skipZerosLen+2])]
                                 [bicharIndex(b[0],b[1])]-1);

        if(quotientDigit <= 48) quotientDigit = 49;
        if(quotientDigit > 57) quotientDigit = 57;
        
        makeMultiplicationForDivBase2(b, quotientDigit, tmpRes, bSize, tmpResSizeForSub);
        makeSubstractionForDiv(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);

        while(isaGreaterEqual(a,bCpy,tmpResSizeForSub,skipZerosLen))
        {
           makeSubstractionForDiv(a, bCpy, a, tmpResSizeForSub, skipZerosLen);
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

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*a, char* b, char*&remain)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  int rs;
  size_t resSize = (rs=(aSize-bSize+1))>1?rs:1;
  size_t remSize = bSize;

  remain = csAllocCharPtr(remSize, '0');

  char*quotient = csAllocCharPtr(resSize, '0');

  makeDivisionQ(a, b, quotient, remain, aSize, bSize, resSize, remSize);

  return quotient;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*a, char* b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  int rs;
  size_t resSize = (rs=(aSize-bSize+1))>1?rs:1;
  size_t remSize = bSize;

  char*quotient = csAllocCharPtr(resSize, '0');

  makeDivisionQ(a, b, quotient, aSize, bSize, resSize);

  return quotient;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*a, char* b, size_t aSize, size_t bSize, size_t& resSize)
{
  int rs;
  resSize = (rs=(aSize-bSize+1))>1?rs:1;
  size_t remSize = bSize, remSize1 = remSize+1;

  char*quotient = csAllocCharPtr(resSize, '0');
  
  makeDivisionQ(a, b, quotient, aSize, bSize, resSize);

  return quotient;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(const char*_a, const char* _b, char*&remain)
{

  size_t aSize = strlen(_a);
  size_t bSize = strlen(_b);
  char*a = filledString(_a,aSize);
  char*b = filledString(_b,bSize);

  size_t rs, resSize = (rs=(aSize-bSize+1))>1?rs:1;
  
  size_t remSize = bSize;

  remain = csAllocCharPtr(remSize, '0');

  char*quotient = csAllocCharPtr(resSize, '0');

  makeDivisionQ(a, b, quotient, remain, aSize, bSize, resSize, remSize);
  free(a);
  free(b);
  return quotient;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(const char*_a, const char* _b)
{

  size_t aSize = strlen(_a);
  size_t bSize = strlen(_b);
  char*a = filledString(_a,aSize);
  char*b = filledString(_b,bSize);
  

  size_t rs, resSize = (rs=(aSize-bSize+1))>1?rs:1;
  
  size_t remSize = bSize, remSize1 = remSize+1;

  char*quotient = csAllocCharPtr(resSize, '0');

  makeDivisionQ(a, b, quotient, aSize, bSize, resSize);
  free(a);
  free(b);
  return quotient;
}

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionR(char*_a, char* _b, char*&resInt, char*&resDec, char*&remain,
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

  const char* status = makeDivisionQ(a, _b, resInt, remain, aSize, bSize, resSize, remSize);


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

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionR2(char*_a, char* _b, char*&resInt, char*&resDec, char*&remain,
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

  const char* status = makeDivisionQ(a, _b, resInt, remain, aSize, bSize, resSize, remSize);


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

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionR2(char*_a, char* _b, char*&resInt, char*&resDec,
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

  const char* status = makeDivisionQ(a, _b, resInt, aSize, bSize, resSize);


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

CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionR(char*a, char* b, char*&resInt, char*&resDec, char*&remain, size_t nDecimals)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t remSize=0, resSize=0, resIntSize, resDecSize;
  return makeDivisionR(a, b, resInt, resDec, remain,
                    aSize, bSize, resSize, resIntSize, resDecSize, remSize, nDecimals);

}

/* ---- src/csModulus.cpp ---- */
using namespace __mem_man;
using namespace __ar_man;


CS_FORCE_INLINE char const* CSARITHMETIC_API CSARITHMETIC::makeModulusQ(char*a, char* b, char*&remain,
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
          quotientDigit = (divTable[bicharIndex(a[skipZerosLen], a[skipZerosLen+1])][bicharIndex(b[0],b[1])]-1);

        if(quotientDigit == 48) quotientDigit = 49;

  
        makeMultiplicationForDivBase1(b, quotientDigit, tmpRes, tmpResSizeForSub, tmpResSizeForSub);
        makeSubstractionForDiv(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);
        
        while(isaGreaterEqual(a,b,tmpResSizeForSub,skipZerosLen))
          makeSubstractionForDiv(a, b, a, tmpResSizeForSub, skipZerosLen);

      }
      else
      {
      //convert the first and second digits to a number, to find the quotient with the first digit of b
        if(bSize == 1)
          quotientDigit = divTable[bicharIndex(a[skipZerosLen], a[skipZerosLen+1])][b[0]]-2;
        else
          quotientDigit = (divTable[tricharIndex(a[skipZerosLen], a[skipZerosLen+1], a[skipZerosLen+2])]
                                 [bicharIndex(b[0],b[1])]-1);

        if(quotientDigit <= 48) quotientDigit = 49;
        if(quotientDigit > 57) quotientDigit = 57;
        
        makeMultiplicationForDivBase2(b, quotientDigit, tmpRes, bSize, tmpResSizeForSub);
        makeSubstractionForDiv(a, tmpRes, a, tmpResSizeForSub, skipZerosLen);
        while(isaGreaterEqual(a,bCpy,tmpResSizeForSub,skipZerosLen))
          makeSubstractionForDiv(a, bCpy, a, tmpResSizeForSub, skipZerosLen);


      }
    }

  }

  
  return 0;
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::makeModulusQ(char*a, char* b, char*&remain)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t remSize = 0;
  remSize = bSize;
  remain = csAllocCharPtr(remSize, '0');
  makeModulusQ(a, b, remain, aSize, bSize, remSize);
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeModulusQ(char*a, char* b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t remSize = 0;
  remSize = bSize;
  char* remain = csAllocCharPtr(remSize, '0');
  makeModulusQ(a, b, remain, aSize, bSize, remSize);
  return remain;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::makeModulusQ(const char*a, const char* b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  char* _a = newString(a, aSize);
  char* _b = newString(b, bSize);
  size_t remSize = 0;
  char* remain = 0;
  remSize = bSize;
  remain = csAllocCharPtr(remSize, '0');
  makeModulusQ(_a, _b, remain, aSize, bSize, remSize);
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


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::printNumber(char*nb, const char*seperator)
{
  cout<<nb<<seperator;
}

CS_FORCE_INLINE csBIDIGITS** CSARITHMETIC_API CSARITHMETIC::toBIDIGITS(int** opTable)
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


CS_FORCE_INLINE uchar CSARITHMETIC_API CSARITHMETIC::bicharIndex(uchar tens, uchar units)
{
  return (tens-48)*10+units;
}

CS_FORCE_INLINE uchar CSARITHMETIC_API CSARITHMETIC::tricharIndex(int cents, uchar tens, uchar units)
{
  return uchar((cents-48)*100+(tens-48)*10+units);
}

//unsigned uchar

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::printTable(char*opName)
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


CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::gcd(char*_a, char*_b, size_t aSize, size_t bSize, size_t& gdcSize)
{
  if ((aSize == 1 && _a[0] == '1') || (bSize == 1 && _b[0] == '1'))
  {
    gdcSize = 1;
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
  gdcSize = srcSize;
  char* out = csAlloc<char>(srcSize + 1);
  for (size_t i = 0; i < srcSize; ++i)
    out[i] = srcDigits[i];
  out[srcSize] = '\0';
  return out;
}

CS_FORCE_INLINE char*CSARITHMETIC_API CSARITHMETIC::gcd(char*a, char*b,size_t& gdcSize)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  char* res = gcd(a,b,aSize,bSize,gdcSize);
  return res;
}

CS_FORCE_INLINE char*CSARITHMETIC_API CSARITHMETIC::gcd(const char*a, const char*b,size_t& gdcSize)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  char* _a = newString(a, aSize);
  char* _b = newString(b, bSize);
  char * g = gcd(_a,_b,aSize,bSize,gdcSize);
  free(_a);
  free(_b);
  return g;
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csSortMinR(csRNUMBER*& rn, size_t size)
{
  size_t s1 = size-1;
  csRNUMBER tmp;
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

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csSortMin_CoordsByRNumber(csFCOORDS*& fc, csRNUMBER*& rn, size_t size)
{
  size_t s1 = size-1;
  csRNUMBER tmp;
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
CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csSortMax_CoordsByRNumber(csFCOORDS*& fc, csRNUMBER*& rn, size_t size)
{
  size_t s1 = size-1;
  csRNUMBER tmp;
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

/* ---- src/csQNUMBER.cpp ---- */
#include <cstddef>
#include <cstdio>
#include <cstring>

using namespace __mem_man;
using namespace __ar_man;
using namespace CSARITHMETIC;

extern int RPRECISION;

CS_FORCE_INLINE csQNUMBER::csQNUMBER(const char* _numerator, const char* _denominator, bool _sign)
{
  numSize = strlen(_numerator);
  denomSize = strlen(_denominator);

  numerator = newString(_numerator, numSize);
  denominator = newString(_denominator, denomSize);
  sign = _sign;
}

CS_FORCE_INLINE csQNUMBER::csQNUMBER(size_t num, size_t denom, bool _sign)
{
  sign = _sign;
  numerator = uLongToString(num, numSize);
  denominator = uLongToString(denom, denomSize);
}

CS_FORCE_INLINE void csQNUMBER::init()
{
  numerator=0;
  denominator=0;
}

CS_FORCE_INLINE csQNUMBER* CSARITHMETIC_API CSARITHMETIC::csQNUMBER_PTR_ALLOC(size_t nb)
{
  csQNUMBER *qn = csAlloc<csQNUMBER>(nb);
  for(size_t i=0; i<nb; i++)
  {
    qn[i].init();
  }
  return qn;
}

CS_FORCE_INLINE csQNUMBER* CSARITHMETIC_API CSARITHMETIC::csQNUMBER_PTR_ALLOC(size_t nb, csQNUMBER init)
{
  csQNUMBER *qn = csAlloc<csQNUMBER>(nb);
  for(size_t i=0; i<nb; i++)
  {
    qn[i].init();
    qn[i].set(init.numerator,init.denominator,init.sign);
  }
  return qn;
}

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csQNUMBER_PTR_FREE(csQNUMBER*& qn, size_t nb)
{
  for(size_t i=0; i<nb; i++)
  {
    qn[i].clear();
  }
  free(qn);
}


CS_FORCE_INLINE void csQNUMBER::set(const char* _numerator, const char* _denominator, bool _sign)
{
  if(numerator)
  {
    free(numerator);
    free(denominator);
  }
  numSize = strlen(_numerator);
  denomSize = strlen(_denominator);

  numerator = newString(_numerator, numSize);
  denominator = newString(_denominator, denomSize);
  sign = _sign;
}

CS_FORCE_INLINE void csQNUMBER::set(size_t num, size_t denom, bool _sign)
{
  if(numerator)
  {
    free(numerator);
    free(denominator);
  }
  sign = _sign;
  numerator = uLongToString(num, numSize);
  denominator = uLongToString(denom, denomSize);
}

CS_FORCE_INLINE void csQNUMBER::setl(long num, size_t denom)
{
  if(numerator)
  {
    free(numerator);
    free(denominator);
  }

  long l = abs(num);
  if(num == 0) sign = 0;
  else sign = (num)/l==-1 ? 1 : 0;

  numerator = uLongToString(l, numSize);
  denominator = uLongToString(denom, denomSize);
}

CS_FORCE_INLINE size_t csQNUMBER::maxSize()
{
  return numSize>denomSize?numSize:denomSize;
}

CS_FORCE_INLINE void csQNUMBER::reduce(size_t sizeCondition)
{
  char**max, **min;
  size_t* minsz, *maxsz;
  if(denomSize > numSize)
  {
    max = &denominator;
    min = &numerator;
    minsz = &numSize;
    maxsz = &denomSize;
  }
  else
  {
    min = &denominator;
    max = &numerator;
    maxsz = &numSize;
    minsz = &denomSize;
  }

  if(*maxsz > sizeCondition)
  {
    size_t diff = *maxsz - sizeCondition;
    if(*minsz > diff)
    {
      *maxsz = sizeCondition;
      *max = (char*)realloc(*max,sizeCondition+1);
      max[0][sizeCondition] = '\0';

      *minsz -= diff;
      size_t t = *minsz;
      *min = (char*)realloc(*min,t+1);
      min[0][t] = '\0';

    }
  }
}

CS_FORCE_INLINE csQNUMBER csReduce(csQNUMBER a, size_t sizeCondition)
{
  char**max, **min;
  size_t* minsz, *maxsz;
  if(a.denomSize > a.numSize)
  {
    max = &a.denominator;
    min = &a.numerator;
    minsz = &a.numSize;
    maxsz = &a.denomSize;
  }
  else
  {
    min = &a.denominator;
    max = &a.numerator;
    maxsz = &a.numSize;
    minsz = &a.denomSize;
  }

  if(*maxsz > sizeCondition)
  {
    size_t diff = *maxsz - sizeCondition;
    if(*minsz > diff)
    {
      *maxsz = sizeCondition;
      *max = (char*)realloc(*max,sizeCondition+1);
      max[0][sizeCondition] = '\0';

      *minsz -= diff;
      size_t t = *minsz;
      *min = (char*)realloc(*min,t+1);
      min[0][t] = '\0';

    }
  }
  return a;
}

CS_FORCE_INLINE bool csQNUMBER::isAbsDifferent(csQNUMBER a)
{
  return !isAbsEqual(a);
}

CS_FORCE_INLINE bool csQNUMBER::operator==(csQNUMBER a)
{
  size_t n1Size, n2Size;
  char*num1 = makeMultiplication(numerator, a.denominator, numSize, a.denomSize, n1Size);
  char*num2 = makeMultiplication(denominator, a.numerator, denomSize, a.numSize, n2Size);
  
  removeFrontZeros(num1,n1Size);
  removeFrontZeros(num2,n2Size);

  if(n1Size != n2Size)
  {
    free(num1);
    free(num2);
    return 0;
  }
  else
  {
    free(num1);
    free(num2);
    if(sign != a.sign)
      return 0;
  }

  for(size_t i=0; i<n1Size; i++)
    {
      if(num1[i]<num2[i] || num1[i]>num2[i])
      {
        free(num1);
        free(num2);
        return false;
      }
    }

  free(num1);
  free(num2);

  if(a.sign == sign)
    return 1;
  return 0;
}

CS_FORCE_INLINE bool csQNUMBER::isAbsEqual(csQNUMBER a)
{
  size_t n1Size, n2Size;
  char*num1 = makeMultiplication(numerator, a.denominator, numSize, a.denomSize, n1Size);
  char*num2 = makeMultiplication(denominator, a.numerator, denomSize, a.numSize, n2Size);
  
  removeFrontZeros(num1,n1Size);
  removeFrontZeros(num2,n2Size);

  if(n1Size != n2Size)
  {
    free(num1);
    free(num2);
    return 0;
  }
  else
  {
    free(num1);
    free(num2);
    if(a.sign == 1)
      return 0;
  }

  for(size_t i=0; i<n1Size; i++)
    {
      if(num1[i] != num2[i])
      {
        free(num1);
        free(num2);
        return false;
      }
    }

  free(num1);
  free(num2);
  return 1;
}

CS_FORCE_INLINE bool csQNUMBER::operator!=(csQNUMBER a)
{
  return !(*this == a);
}

CS_FORCE_INLINE bool csQNUMBER::operator>(csQNUMBER a)
{
  size_t n1Size, n2Size;
  char*num1 = makeMultiplication(numerator, a.denominator, numSize, a.denomSize, n1Size);
  char*num2 = makeMultiplication(denominator, a.numerator, denomSize, a.numSize, n2Size);
  
  removeFrontZeros(num1,n1Size);
  removeFrontZeros(num2,n2Size);

  if(n1Size < n2Size)
  {
    free(num1);
    free(num2);
    if(a.sign == 1)
      return 1;
    else
      return 0;
  }
  else if(n1Size > n2Size)
  {
    free(num1);
    free(num2);
    if(sign == 1)
      return 0;
    else
      return 1;
  }
  else
  {
    free(num1);
    free(num2);
    if(sign == 0 && a.sign == 1)
      return 1;
  }

  for(size_t i=0; i<n1Size; i++)
    {
      if(num1[i]<num2[i])
      {
        free(num1);
        free(num2);
        if(a.sign == 1)
          return 1;
        else
          return 0;
      }
      else if(num1[i]>num2[i])
      {
        free(num1);
        free(num2);
        if(sign == 1)
          return 0;
        else
          return 1;
      }
    }

  free(num1);
  free(num2);
  if(sign == 0)
  {
    if(a.sign == 0)
      return 0;
    else
      return true;
  }

}

CS_FORCE_INLINE bool csQNUMBER::isAbsGreater(csQNUMBER a)
{
  size_t n1Size, n2Size;
  char*num1 = makeMultiplication(numerator, a.denominator, numSize, a.denomSize, n1Size);
  char*num2 = makeMultiplication(denominator, a.numerator, denomSize, a.numSize, n2Size);
  
  removeFrontZeros(num1,n1Size);
  removeFrontZeros(num2,n2Size);

  if(n1Size < n2Size)
  {
    free(num1);
    free(num2);
    return 0;
  }
  else if(n1Size > n2Size)
  {
    free(num1);
    free(num2);
    return 1;
  }
  else
  {
    free(num1);
    free(num2);
    if(a.sign == 1)
      return 1;
  }

  for(size_t i=0; i<n1Size; i++)
    {
      if(num1[i]<num2[i])
      {
        free(num1);
        free(num2);
        return false;
      }
      else if(num1[i]>num2[i])
      {
        free(num1);
        free(num2);
        return 1;
      }
    }

  free(num1);
  free(num2);
  return 0;
}

CS_FORCE_INLINE bool csQNUMBER::operator>=(csQNUMBER a)
{
  size_t n1Size, n2Size;
  char*num1 = makeMultiplication(numerator, a.denominator, numSize, a.denomSize, n1Size);
  char*num2 = makeMultiplication(denominator, a.numerator, denomSize, a.numSize, n2Size);
  
  removeFrontZeros(num1,n1Size);
  removeFrontZeros(num2,n2Size);

  if(n1Size < n2Size)
  {
    free(num1);
    free(num2);
    if(a.sign == 1)
    {
      return 1;
    }
    else
      return 0;
  }
  else if(n1Size > n2Size)
  {
    free(num1);
    free(num2);
    if(sign == 1)
    {
      return 0;
    }
    else
      return 1;
  }
  else
  {
    free(num1);
    free(num2);
    if(sign == 1 && a.sign == 0)
      return 0;
  }

  for(size_t i=0; i<n1Size; i++)
    {
      if(num1[i]<num2[i])
      {
        free(num1);
        free(num2);
        if(sign == 1)
        {
          if(a.sign == 1)
            return 1;
          else
            return false;
        }
        else
          return 0;
      }
      else if(num1[i]>num2[i])
      {
        free(num1);
        free(num2);
        if(sign == 0)
          return 1;
        else
          return false;
      }
    }

  free(num1);
  free(num2);
  if(a.sign == sign)
    return 1;
  else if(a.sign == 1 && sign == 0)
    return 1;

  return 0;
}

CS_FORCE_INLINE bool csQNUMBER::isAbsGreaterEqual(csQNUMBER a)
{
  size_t n1Size, n2Size;
  char*num1 = makeMultiplication(numerator, a.denominator, numSize, a.denomSize, n1Size);
  char*num2 = makeMultiplication(denominator, a.numerator, denomSize, a.numSize, n2Size);
  
  removeFrontZeros(num1,n1Size);
  removeFrontZeros(num2,n2Size);

  if(n1Size < n2Size)
  {
    free(num1);
    free(num2);
    return 0;
  }
  else if(n1Size > n2Size)
  {
    free(num1);
    free(num2);
    return 1;
  }
  else
  {
    free(num1);
    free(num2);
    if(a.sign == 1)
      return 1;
  }

  for(size_t i=0; i<n1Size; i++)
    {
      if(num1[i]<num2[i])
      {
        free(num1);
        free(num2);
        return false;
      }
      else if(num1[i]>num2[i])
      {
        free(num1);
        free(num2);
        return 1;
      }
    }

  free(num1);
  free(num2);
  return 1;
}

CS_FORCE_INLINE bool csQNUMBER::operator<(csQNUMBER a)
{
  size_t n1Size, n2Size;
  char*num1 = makeMultiplication(numerator, a.denominator, numSize, a.denomSize, n1Size);
  char*num2 = makeMultiplication(denominator, a.numerator, denomSize, a.numSize, n2Size);
  
  removeFrontZeros(num1,n1Size);
  removeFrontZeros(num2,n2Size);

  if(n1Size > n2Size)
  {
    free(num1);
    free(num2);
    if(sign == 1)
      return 1;
    else
      return 0;
  }
  else if(n1Size < n2Size)
  {
    free(num1);
    free(num2);
    if(a.sign == 1)
      return 0;
    else
      return 1;
  }
  else
  {
    free(num1);
    free(num2);
    if(sign == 1 && a.sign == 0)
      return 1;
  }

  for(size_t i=0; i<n1Size; i++)
    {
      if(num1[i]>num2[i])
      {
        free(num1);
        free(num2);
        if(sign == 1)
          return 1;
        else
          return 0;
      }
      else if(num1[i]<num2[i])
      {
        free(num1);
        free(num2);
        if(a.sign == 1)
          return 0;
        else
          return 1;
      }
    }
  free(num1);
  free(num2);
  if(sign == 1)
  {
    if(a.sign == 0)
      return 1;
    else
      return 0;
  }
  return 0;
}

CS_FORCE_INLINE bool csQNUMBER::isAbsLess(csQNUMBER a)
{
  size_t n1Size, n2Size;
  char*num1 = makeMultiplication(numerator, a.denominator, numSize, a.denomSize, n1Size);
  char*num2 = makeMultiplication(denominator, a.numerator, denomSize, a.numSize, n2Size);
  removeFrontZeros(num1,n1Size);
  removeFrontZeros(num2,n2Size);

  if(n1Size > n2Size)
  {
    free(num1);
    free(num2);
   // if(a.sign < sign || (a.sign == 1 && sign == 1))
    //else
      return 0;
  }
  else if(n1Size < n2Size)
  {
    free(num1);
    free(num2);
    //if(sign < a.sign || (a.sign == 1 && sign == 1))
    //else
      return 1;
  }
  else
  {
    free(num1);
    free(num2);
    if(a.sign == 1)
      return 0;
  }

  for(size_t i=0; i<n1Size; i++)
    {
      if(num1[i]>num2[i])
      {
        free(num1);
        free(num2);
        return false;
      }
      else if(num1[i]<num2[i])
      {
        free(num1);
        free(num2);
        return 1;
      }
    }
  free(num1);
  free(num2);
  return 0;
}

CS_FORCE_INLINE bool csQNUMBER::operator<=(csQNUMBER a)
{
  size_t n1Size, n2Size;
  char*num1 = makeMultiplication(numerator, a.denominator, numSize, a.denomSize, n1Size);
  char*num2 = makeMultiplication(denominator, a.numerator, denomSize, a.numSize, n2Size);
  removeFrontZeros(num1,n1Size);
  removeFrontZeros(num2,n2Size);

  if(n1Size > n2Size)
  {
    free(num1);
    free(num2);
    if(sign == 1)
      return 1;
    else
      return 0;
    return 0;
  }
  else if(n1Size < n2Size)
  {
    free(num1);
    free(num2);
    if(a.sign == 1)
      return 0;
    else
      return 1;
    return 1;
  }
  else
  {
    free(num1);
    free(num2);
    if(sign == 1 && a.sign == 0)
      return 1;
  }

  for(size_t i=0; i<n1Size; i++)
    {
      if(num1[i]>num2[i])
      {
        free(num1);
        free(num2);
        if(sign == 1)
          return 1;
        else
          return 0;
      }
      else if(num1[i]<num2[i])
      {
        free(num1);
        free(num2);
        if(a.sign == 1)
          return 0;
        else
          return 1;
      }
    }
  free(num1);
  free(num2);
  if(a.sign == sign)
    return 1;
  else if(a.sign == 0 && sign == 1)
    return 1;
  return 0;
}

CS_FORCE_INLINE bool csQNUMBER::isAbsLessEqual(csQNUMBER a)
{
  size_t n1Size, n2Size;
  char*num1 = makeMultiplication(numerator, a.denominator, numSize, a.denomSize, n1Size);
  char*num2 = makeMultiplication(denominator, a.numerator, denomSize, a.numSize, n2Size);
  
  removeFrontZeros(num1,n1Size);
  removeFrontZeros(num2,n2Size);

  if(n1Size > n2Size)
  {
    free(num1);
    free(num2);
    return 0;
  }
  else if(n1Size < n2Size)
  {
    free(num1);
    free(num2);
    return 1;
  }

  for(size_t i=0; i<n1Size; i++)
    {
      if(num1[i]>num2[i])
      {
        free(num1);
        free(num2);
        return false;
      }
      else if(num1[i]<num2[i])
      {
        free(num1);
        free(num2);
        return 1;
      }
    }
  free(num1);
  free(num2);

  return 1;
}

CS_FORCE_INLINE void csQNUMBER::print(const char*title)
{
  if(sign == CS_POSITIVE_NUMBER)
  cout<<"\n "<<title<<numerator<<" / "<<denominator<<"\n";
  else
  cout<<"\n "<<title<<"-"<<numerator<<" / "<<denominator<<"\n";
}

CS_FORCE_INLINE void csQNUMBER::clear(const source_location loc)
{
  if(numerator)
  {
    free(numerator);
    free(denominator);
    numerator = 0;
    denominator = 0;
    numSize = 0;
    denomSize = 0;

  }
}

CS_FORCE_INLINE csQNUMBER csQNUMBER::operator+(csQNUMBER a)
{
  csQNUMBER qn(csQRaw{});
  size_t n1Size, n2Size, dsz, nsz;
  char*num1 = makeMultiplication(numerator, a.denominator, numSize, a.denomSize, n1Size);
  char*num2 = makeMultiplication(denominator, a.numerator, denomSize, a.numSize, n2Size);
  char*denom = makeMultiplication(denominator, a.denominator, denomSize, a.denomSize, dsz);

  removeFrontZeros(num1,n1Size);
  removeFrontZeros(num2,n2Size);
  removeFrontZeros(denom,dsz);

  char *num=0;
  if(sign == CS_POSITIVE_NUMBER)
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      num = makeSubstraction(num1,num2,n1Size, n2Size, nsz, qn.sign);
    }
    else
    {
      num = makeAddition(num1,num2,nsz);
    }
  }
  else
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      num = makeAddition(num1,num2,nsz);
      qn.sign = CS_NEGATIVE_NUMBER;
    }
    else
    {
      num = makeSubstraction(num2,num1,n2Size, n1Size, nsz,qn.sign);
    }
  }
  removeFrontZeros(num,nsz);
  removeFrontZeros(denom,dsz);
  size_t gsz;
  char* rgcd = gcd(num,denom,nsz,dsz,gsz);

  if(gsz == 1 && rgcd[0] == '1')
  {
      qn.numerator = num;
      qn.denominator = denom;
      qn.numSize = nsz;
      qn.denomSize = dsz;
  }
  else
  {
    size_t ns, ds;

    qn.numerator = makeDivisionQ(num,rgcd, nsz,gsz, ns);

    qn.denominator = makeDivisionQ(denom,rgcd, dsz, gsz, ds);
    removeFrontZeros(qn.numerator,ns);
    removeFrontZeros(qn.denominator,ds);
    qn.numSize = ns;
    qn.denomSize = ds;
    free(num);
    free(denom);
  }

  free(rgcd);
  free(num1);
  free(num2);
  return qn;
}

CS_FORCE_INLINE csQNUMBER csQNUMBER::operator-(csQNUMBER a)
{
  csQNUMBER qn(csQRaw{});

  size_t n1Size, n2Size, dsz, nsz;
  char*num1 = makeMultiplication(numerator, a.denominator, numSize, a.denomSize, n1Size);
  char*num2 = makeMultiplication(denominator, a.numerator, denomSize, a.numSize, n2Size);
  char*denom = makeMultiplication(denominator, a.denominator, denomSize, a.denomSize, dsz);
  removeFrontZeros(num1,n1Size);
  removeFrontZeros(num2,n2Size);
  removeFrontZeros(denom,dsz);

  char *num;
  if(sign == CS_POSITIVE_NUMBER)
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      num = makeAddition(num1,num2,nsz);
    }
    else
    {
      num = makeSubstraction(num1,num2,n1Size, n2Size, nsz, qn.sign);
    }
  }
  else
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      num = makeSubstraction(num2,num1,n2Size, n1Size, nsz,qn.sign);
    }
    else
    {
      num = makeAddition(num1,num2,nsz);
      qn.sign = CS_NEGATIVE_NUMBER;
     }
  }
  removeFrontZeros(num,nsz);
  removeFrontZeros(denom,dsz);

  size_t gsz;
  char* rgcd = gcd(num,denom,nsz,dsz,gsz);

  if(gsz == 1 && rgcd[0] == '1')
  {
      qn.numerator = num;
      qn.denominator = denom;
      qn.numSize = nsz;
      qn.denomSize = dsz;
  }
  else
  {
    size_t ns, ds;

    qn.numerator = makeDivisionQ(num,rgcd, nsz,gsz, ns);

    qn.denominator = makeDivisionQ(denom,rgcd, dsz, gsz, ds);
    removeFrontZeros(qn.numerator,ns);
    removeFrontZeros(qn.denominator,ds);
    qn.numSize = ns;
    qn.denomSize = ds;
    free(num);
    free(denom);
  }

  free(rgcd);
  free(num1);
  free(num2);
  return qn;
}

CS_FORCE_INLINE csQNUMBER csQNUMBER::operator*(csQNUMBER a)
{
  csQNUMBER qn(csQRaw{});
  size_t adsz = a.denomSize, ansz = a.numSize;

  char*num = makeMultiplication(numerator, a.numerator,  numSize, ansz, qn.numSize);
  char*denom = makeMultiplication(denominator, a.denominator, denomSize, adsz, qn.denomSize);

  if(sign == CS_POSITIVE_NUMBER)
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      qn.sign = CS_NEGATIVE_NUMBER;
    }
  }
  else
  {
    if(a.sign == CS_POSITIVE_NUMBER)
    {
      qn.sign = CS_NEGATIVE_NUMBER;
    }
  }
  removeFrontZeros(num,qn.numSize);
  removeFrontZeros(denom,qn.denomSize);
  
  size_t gsz;
  char* rgcd = gcd(num,denom,qn.numSize,qn.denomSize,gsz);
  
  if(gsz == 1 && rgcd[0] == '1')
  {
    qn.numerator = num;
    qn.denominator = denom;
  }
  else
  {
    size_t ns, ds;

    qn.numerator = makeDivisionQ(num,rgcd, qn.numSize,gsz, ns);
    qn.denominator = makeDivisionQ(denom,rgcd, qn.denomSize, gsz, ds);
    removeFrontZeros(qn.numerator,ns);
    removeFrontZeros(qn.denominator,ds);
    qn.numSize = ns;
    qn.denomSize = ds;
    free(num);
    free(denom);
  }

  free(rgcd);
  return qn;
}

CS_FORCE_INLINE csQNUMBER csQNUMBER::operator/(csQNUMBER a)
{
  csQNUMBER qn(csQRaw{});
  size_t adsz = a.denomSize, ansz = a.numSize;
  char*num = makeMultiplication(numerator, a.denominator,  numSize, adsz, qn.numSize);
  char*denom = makeMultiplication(denominator, a.numerator, denomSize, ansz, qn.denomSize);

  if(sign == CS_POSITIVE_NUMBER)
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      qn.sign = CS_NEGATIVE_NUMBER;
    }
  }
  else
  {
    if(a.sign == CS_POSITIVE_NUMBER)
    {
      qn.sign = CS_NEGATIVE_NUMBER;
    }
  }
  removeFrontZeros(num,qn.numSize);
  removeFrontZeros(denom,qn.denomSize);
  size_t gsz;
  char* rgcd = gcd(num,denom,qn.numSize,qn.denomSize,gsz);

  if(gsz == 1 && rgcd[0] == '1')
  {
    qn.numerator = num;
    qn.denominator = denom;
  }
  else
  {
    size_t ns, ds;

    qn.numerator = makeDivisionQ(num,rgcd, qn.numSize,gsz, ns);
    qn.denominator = makeDivisionQ(denom,rgcd, qn.denomSize, gsz, ds);
    removeFrontZeros(qn.numerator,ns);
    removeFrontZeros(qn.denominator,ds);
    qn.numSize = ns;
    qn.denomSize = ds;
    free(num);
    free(denom);
  }

  free(rgcd);
  return qn;
}

CS_FORCE_INLINE csQNUMBER& csQNUMBER::operator=(csQNUMBER a)
{
  if(numerator)
  {
    free(numerator);
    free(denominator);
    numerator = 0;
  }

  sign = a.sign;
  numSize = a.numSize;
  denomSize = a.denomSize;

  numerator = newString(a.numerator, numSize);
  denominator = newString(a.denominator, denomSize);
  a.clear();
  return *this;
}

CS_FORCE_INLINE csQNUMBER& csQNUMBER::operator=(long a)
{
  if(numerator)
  {
    free(numerator);
    free(denominator);
    numerator = 0;
  }
  if(a != 0)
  sign = a/abs(a) == -1 ? 1 : 0;
  numerator = uLongToString(a, numSize);
  
  denomSize = 1;
  denominator = newString("1", denomSize);

  return *this;
}



CS_FORCE_INLINE csQNUMBER csQNUMBER::operator+(long a)
{
  csQNUMBER qn(csQRaw{});

  size_t n1Size, n2Size, dsz, nsz;
  bool asign=0;
  if(a != 0)
    asign = (a/abs(a)) == -1 ? 1 : 0;
  char*num1 = uLongToString(a,n1Size);

  char*num2 = makeMultiplication(denominator, num1, denomSize, n1Size, n2Size);


  removeFrontZeros(num2,n2Size);

  char *num=0;
  if(sign == CS_POSITIVE_NUMBER)
  {
    if(asign == CS_NEGATIVE_NUMBER)
    {
      num = makeSubstraction(numerator,num2,numSize, n2Size, nsz, qn.sign);
    }
    else
    {

      num = makeAddition(numerator,num2,nsz);
    }
  }
  else
  {
    if(asign == CS_NEGATIVE_NUMBER)
    {
      num = makeAddition(numerator,num2,nsz);
      qn.sign = CS_NEGATIVE_NUMBER;
    }
    else
    {
      num = makeSubstraction(num2,numerator,n2Size, numSize, nsz,qn.sign);
    }
  }
  removeFrontZeros(num,nsz);

  qn.numerator = num;
  qn.numSize = nsz;
  qn.denominator = newString(denominator, denomSize);
  qn.denomSize = denomSize;

  free(num1);
  free(num2);

  return qn;
}


CS_FORCE_INLINE csQNUMBER csQNUMBER::operator-(long a)
{
  csQNUMBER qn(csQRaw{});

  size_t n1Size, n2Size, dsz, nsz;
  bool asign=0;
  if(a != 0)
    asign = (a/abs(a)) == -1 ? 1 : 0;
  char*num1 = uLongToString(a,n1Size);

  char*num2 = makeMultiplication(denominator, num1, denomSize, n1Size, n2Size);


  removeFrontZeros(num2,n2Size);

  char *num=0;
  if(sign == CS_POSITIVE_NUMBER)
  {
    if(asign == CS_NEGATIVE_NUMBER)
    {
      num = makeAddition(numerator,num2,nsz);
    }
    else
    {
      num = makeSubstraction(numerator,num2,numSize, n2Size, nsz, qn.sign);
    }
  }
  else
  {
    if(asign == CS_NEGATIVE_NUMBER)
    {
      num = makeSubstraction(num2,numerator,n2Size, numSize, nsz,qn.sign);
    }
    else
    {
      num = makeAddition(numerator,num2,nsz);
      qn.sign = CS_NEGATIVE_NUMBER;
    }
  }
  removeFrontZeros(num,nsz);

  qn.numerator = num;
  qn.numSize = nsz;
  qn.denominator = newString(denominator, denomSize);
  qn.denomSize = denomSize;

  free(num1);
  free(num2);

  return qn;
}


CS_FORCE_INLINE csQNUMBER csQNUMBER::operator*(long a)
{
  csQNUMBER qn(csQRaw{});

  size_t ansz;
  bool asign=0;
  if(a != 0)
    asign = (a/abs(a)) == -1 ? 1 : 0;

  char*num1 = uLongToString(a,ansz);

  char*num = makeMultiplication(numerator, num1,  numSize, ansz, qn.numSize);

  if(sign == CS_POSITIVE_NUMBER)
  {
    if(asign == CS_NEGATIVE_NUMBER)
    {
      qn.sign = CS_NEGATIVE_NUMBER;
    }
  }
  else
  {
    if(asign == CS_POSITIVE_NUMBER)
    {
      qn.sign = CS_NEGATIVE_NUMBER;
    }
  }
  removeFrontZeros(num,qn.numSize);

  qn.numerator = num;
  qn.denominator = newString(denominator, denomSize);
  qn.denomSize = denomSize;

  free(num1);

  return qn;
}


CS_FORCE_INLINE csQNUMBER csQNUMBER::operator/(long a)
{
  csQNUMBER qn(csQRaw{});

  size_t ansz;
  bool asign=0;
  if(a != 0)
    asign = (a/abs(a)) == -1 ? 1 : 0;

  char*num1 = uLongToString(a,ansz);

  char*denom = makeMultiplication(denominator, num1,  denomSize, ansz, qn.denomSize);

  if(sign == CS_POSITIVE_NUMBER)
  {
    if(asign == CS_NEGATIVE_NUMBER)
    {
      qn.sign = CS_NEGATIVE_NUMBER;
    }
  }
  else
  {
    if(asign == CS_POSITIVE_NUMBER)
    {
      qn.sign = CS_NEGATIVE_NUMBER;
    }
  }
  removeFrontZeros(denom,qn.denomSize);

  qn.numerator = newString(numerator, numSize);;
  qn.numSize = numSize;
  qn.denominator = denom;

  free(num1);

  return qn;
}

CS_FORCE_INLINE double csQNUMBER::getDouble()
{
  double res;
  size_t sz;
  char* nb = makeDivisionQ(numerator,denominator, numSize, denomSize, sz);
  removeFrontZeros(nb,sz);
  res = strtod(nb,0);
  free(nb);
  if(sign) res = -res;
  return res;
}

CS_FORCE_INLINE char* csQNUMBER::quotient()
{
  double res;
  size_t sz;
  char*resInt, *resDec;
  
  long precicion = getRNumberPrecision();
  size_t resz, risz, rdsz, rsz;
  char const* stat = makeDivisionR2(numerator,denominator,resInt,resDec, numSize, denomSize, resz, risz, rdsz, precicion);

  if(stat)
  {
    free(resInt);
    free(resDec);
    return newString(stat);
  }
  
  removeFrontZeros(resInt,risz);

  size_t t = risz + rdsz + 1;
  char*str = csAlloc<char>(t+2);

  if(sign)
    sprintf(str, "-%s.%s\0",resInt,resDec);
  else
    sprintf(str, "%s.%s\0",resInt,resDec);

  free(resInt);
  free(resDec);
  return str;
}

CS_FORCE_INLINE csQNUMBER::operator csRNUMBER()
{
  size_t aSize = numSize+RPRECISION;
  char* a = csAlloc<char>(aSize+1);
  a[aSize] = '\0';
  for(size_t i=0; i<numSize; i++)
  {
    a[i] = numerator[i];
  }
  for(size_t i=numSize; i<aSize; i++)
  {
    a[i] = '0';
  }
  
  int rs = (numSize-denomSize+1); // int est important
  size_t resSize = (rs>1)?rs:1;
  char* result = csAllocCharPtr(resSize, '0');
  result[resSize] = '\0';

  
  const char* status = makeDivisionQ(a, denominator, result, aSize, denomSize, resSize);
  
  if(status)
  {
 
    csRNUMBER rn("0");
    free(result);
    free(a); // the new modification (14/01/2025) of makeDivisionQ() allow to do that
    return rn;
  }

  removeFrontZeros(result,resSize);
  csRNUMBER rn(result, -RPRECISION, sign);

  free(result);
  
  free(a);
  
  return rn;
}


CS_FORCE_INLINE csQNUMBER CSARITHMETIC_API CSARITHMETIC::pow(csQNUMBER a, size_t p)
{
  csQNUMBER r("1");
  for(size_t i=0; i<p; i++)
  {
    r = r*a;
  }
  return r;
}

CS_FORCE_INLINE void csQNUMBER::copy(csQNUMBER a)
{
  if(numerator)
  {
    free(numerator);
    free(denominator);
    numerator = 0;
    denominator = 0;
  }

  sign = a.sign;
  numSize = a.numSize;
  denomSize = a.denomSize;

  numerator = newString(a.numerator, numSize);
  denominator = newString(a.denominator, denomSize);
}

CS_FORCE_INLINE bool csQNUMBER::isZero()
{
  for(size_t i=0; i<numSize; i++)
  {
    if(numerator[i] != '0')
      return 0;
  }
  return 1;
}

CS_FORCE_INLINE bool csQNUMBER::isNonZero()
{
  return !isZero();
}

/* ---- src/csRNUMBER.cpp ---- */
#include <cstddef>
#include <cstdio>
#include <cstring>

using namespace __mem_man;
using namespace __ar_man;
using namespace CSARITHMETIC;

int RPRECISION = 20;

CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::setRNumberPrecision(int precision)
{
  RPRECISION = precision;
}
CS_FORCE_INLINE int CSARITHMETIC_API CSARITHMETIC::getRNumberPrecision()
{
  return RPRECISION;
}

CS_FORCE_INLINE csRNUMBER::csRNUMBER(const char* _mantissa, int _exponent, bool _sign)
{
  set(_mantissa, _exponent, _sign);
}
CS_FORCE_INLINE csRNUMBER::csRNUMBER(unsigned long _mantissa, int _exponent, bool _sign)
{
  set(_mantissa, _exponent, _sign);
}
CS_FORCE_INLINE csRNUMBER::csRNUMBER(long _mantissa, int _exponent)
{
  set(_mantissa, _exponent);
}
CS_FORCE_INLINE csRNUMBER::csRNUMBER(bool evaluate, const char* number)
{
  set(evaluate, number);
}

CS_FORCE_INLINE csRNUMBER::~csRNUMBER()
{
}


CS_FORCE_INLINE void csRNUMBER::set(const char* _mantissa, int _exponent, bool _sign)
{
  if(mantissa)
  {
    free(mantissa);
    mantissa = 0;
  }
  mantSize = strlen(_mantissa);
  mantissa = newString(_mantissa, mantSize);
  sign = _sign;
  exponent = _exponent;
  precision = 0;
}

CS_FORCE_INLINE void csRNUMBER::set(unsigned long _mantissa, int _exponent, bool _sign)
{
  free(mantissa);

  mantissa = uLongToString(_mantissa, mantSize);
  sign = _sign;
  exponent = _exponent;
  precision = 0;
}

CS_FORCE_INLINE void csRNUMBER::set(long _mantissa, int _exponent)
{
  free(mantissa);

  long l = _mantissa < 0 ? -_mantissa : _mantissa;
  if(_mantissa) sign = (_mantissa / l == -1 ? 1 : 0);
  mantissa = uLongToString(l, mantSize);
  exponent = _exponent;
  precision = 0;
}

CS_FORCE_INLINE void csRNUMBER::set(bool evaluate, const char* _number)
{
  if(mantissa)
  {
    free(mantissa);
    mantissa = 0;
  }
  size_t start = 0, len = strlen(_number);
  char*number = newString(_number, len);

  if(number[0] == '-')
  {
    sign = 1;
    start = 1;
  }
  else if(number[0] == '+')
  {
    start = 1;
    sign = 0;
  }

  size_t dotpos = 0, epos = 0, withSign;
  for(size_t i = start; i<len; i++)
  {
    if(number[i] == '.')
    {
      dotpos = i;
      break;
    }
    if(number[i] != '1' && number[i] != '2' && number[i] != '3' &&
                        number[i] != '4' && number[i] != '5' && number[i] != '6' && number[i] != '7' && number[i] != '8' &&
                        number[i] != '9' && number[i] != '0')
    {
      number[i] = '0';
    }

  }

  if(dotpos == 0) dotpos = len;

  size_t dp1 = (len > 1) ? dotpos+1 : len;
  for(size_t i = dp1; i<len; i++)
  {
    if(number[i] == 'e' || number[i] == 'E')
    {
      epos = i;
      break;
    }
    if(number[i] != '1' && number[i] != '2' && number[i] != '3' &&
                        number[i] != '4' && number[i] != '5' && number[i] != '6' && number[i] != '7' && number[i] != '8' &&
                        number[i] != '9' && number[i] != '0')
    {
      number[i] = '0';
    }

  }

  if(epos == 0) epos = len;
  size_t ep1 = (epos == len)? len : epos+1;

  char *str1 = newString(number, start, dotpos);

  size_t d = 0;

  if(dp1 < epos)
  {
    d = epos-dp1;
    mantSize = dotpos-start + d;
    mantissa = csAlloc<char>(mantSize+1);
    char *str2 = newString(number, dp1, epos);
    sprintf(mantissa, "%s%s\0", str1, str2);
    free(str2);
  }
  else
  {
    mantSize = dotpos-start;
    mantissa = csAlloc<char>(mantSize+1);
    sprintf(mantissa, "%s\0", str1);
  }

  if(ep1 < len)
  {
    char* expon = newString(number, ep1, len);
    exponent = (long)strtod(expon,0) - (long)d;
    free(expon);
  }
  else exponent = -(long)d;

  free(str1);
  free(number);

  precision = 0;
}

CS_FORCE_INLINE void csRNUMBER::setAsTenPower(long power)
{
  if(mantissa)
  {
    free(mantissa);
  }
  mantSize = 1;
  mantissa = csAllocCharPtr(mantSize, '1');
  mantissa[0] = '1';
  exponent = power;
}

CS_FORCE_INLINE void csRNUMBER::init()
{
  mantissa=0;
}

CS_FORCE_INLINE csRNUMBER* CSARITHMETIC_API CSARITHMETIC::csRNUMBER_PTR_ALLOC(size_t nb)
{
  csRNUMBER *rn = csAlloc<csRNUMBER>(nb);
  for(size_t i=0; i<nb; i++)
  {
    rn[i].init();
  }
  return rn;
}

CS_FORCE_INLINE csRNUMBER* CSARITHMETIC_API CSARITHMETIC::csRNUMBER_PTR_ALLOC(size_t nb, csRNUMBER init)
{
  csRNUMBER *rn = csAlloc<csRNUMBER>(nb);
  for(size_t i=0; i<nb; i++)
  {
    rn[i].init();
    rn[i].set(init.mantSize,init.exponent,init.sign);
  }
  return rn;
}


CS_FORCE_INLINE void CSARITHMETIC_API CSARITHMETIC::csRNUMBER_PTR_FREE(csRNUMBER*& rn, size_t nb)
{
  for(size_t i=0; i<nb; i++)
  {
    rn[i].clear();
  }
  free(rn);
}

CS_FORCE_INLINE void csRNUMBER::random(size_t nDigits, char digitMin, char digitMax, long exponent, bool sign)
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

CS_FORCE_INLINE void csRNUMBER::setPrecision(int size)
{
  long diff;


  if(exponent <= 0)
  {
    if(-exponent > size) // let the number as it is, good for backend computing
      return;

    diff = exponent + size;


    exponent = -size;
    size_t msz = mantSize;
    mantSize += diff;

    csReallocString(&mantissa, msz, mantSize, '0');
  }
  else
  {
    size_t msz = mantSize;
    mantSize += exponent+size;
    exponent = -size;

    csReallocString(&mantissa, msz, mantSize, '0');
  }

}

CS_FORCE_INLINE size_t csRNUMBER::significantZerosCount(size_t initialPos)
{
  long i = (long)mantSize + exponent + initialPos;

  if(i < 0)
    return -i;

  if(!(i==1 && mantissa[0] == '0'))
    return 0;

  for(; i<mantSize; i++)
  {
    if(mantissa[i] != '0')
    {
      if(i == 0)
        return 0;
      return i-1;
    }
  }

  return i;
}

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

CS_FORCE_INLINE void csRNUMBER::forcePrecision(int size)
{
  long diff;

  //good for frontend, to show results in appropriate format
  if(exponent <= 0)
  {
    diff = exponent + size;
    long lmsize = (long)mantSize;
    long aDiff = diff<0 ? -diff : diff;

    if((diff > lmsize) || (aDiff < lmsize))
    {
      exponent = -size;
      size_t msz = mantSize;
      mantSize += diff;

      //Round
      if(mantSize < msz && mantissa[mantSize] > 53)
      {
        addOne(mantissa, mantSize);
      }

      csReallocString(&mantissa, msz, mantSize, '0');
      return;
    }


    mantissa = (char*)realloc(mantissa, 2);
    mantissa[0] = '0';
    mantissa[1] = '\0';
    mantSize = 1;
    exponent = 0;
  }
  else
  {
    size_t msz = mantSize;
    mantSize += exponent+size;
    exponent = -size;

    csReallocString(&mantissa, msz, mantSize, '0');
  }

}

CS_FORCE_INLINE void csRNUMBER::reshape(size_t newMantissaSize)
{
  if(newMantissaSize > mantSize)
  {
    increaseShape(newMantissaSize-mantSize);
  }
  else
  {
    decreaseShape(newMantissaSize-mantSize);
  }

}

CS_FORCE_INLINE void csRNUMBER::increaseShape(size_t size)
{

  //if(exponent > 0)
    exponent -= size;
 // else

  size_t s = mantSize + size;
  csReallocString(&mantissa, mantSize, s, '0');
  mantSize = s;
}

CS_FORCE_INLINE void csRNUMBER::decreaseShape(size_t size)
{

 // if(exponent > 0)
    exponent += size;
  size_t s = mantSize - size;
  csReallocString(&mantissa, s);
  mantSize = s;
}

CS_FORCE_INLINE void csRNUMBER::shapeOutZeros()
{
  size_t j, i;
  if(mantSize)
  for( i=0, j=mantSize-1; i<mantSize; i++, j--)
  {
    if(mantissa[j] != '0')
    {
      decreaseShape(i);
      break;
    }
  }
}

CS_FORCE_INLINE size_t csRNUMBER::getDigitNumber()
{
  size_t dn;
  if(exponent >=0)
    dn = exponent + mantSize;
  else
  {
    dn = std::max((int)mantSize, std::abs(exponent));
  }
  return dn;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::abs()
{
  csRNUMBER rn(mantissa,exponent,0);
  return rn;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator+(long a)
{
  csRNUMBER rn(a);
  csRNUMBER b = (*this + rn);
  rn.clear();
  return b;
}
CS_FORCE_INLINE csRNUMBER csRNUMBER::operator+(csRNUMBER _a)
{
  //because mantissa and exposant could be
  //modified, it is necessary to make a copy
  csRNUMBER rn, a(_a.mantissa, _a.exponent, _a.sign), b(mantissa,exponent,sign);
  free(rn.mantissa);

  if(b.exponent > a.exponent)
  {
    b.increaseShape(b.exponent-a.exponent);
  }
  else if(b.exponent < a.exponent)
  {
    a.increaseShape(-b.exponent+a.exponent);
  }

  if(b.sign == CS_POSITIVE_NUMBER)
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      rn.mantissa = makeSubstraction(b.mantissa,a.mantissa,b.mantSize, a.mantSize, rn.mantSize, rn.sign);

    }
    else
    {
      rn.mantissa = makeAddition(b.mantissa,a.mantissa,rn.mantSize);
    }
  }
  else
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      rn.mantissa = makeAddition(b.mantissa,a.mantissa,rn.mantSize);
      rn.sign = CS_NEGATIVE_NUMBER;
    }
    else
    {
      rn.mantissa = makeSubstraction(a.mantissa,b.mantissa,a.mantSize, b.mantSize, rn.mantSize,rn.sign);
    }
  }
  removeFrontZeros(rn.mantissa,rn.mantSize);
  rn.exponent = a.exponent;

  free(a.mantissa);
  free(b.mantissa);

  return rn;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator-(long a)
{
  csRNUMBER rn(a);
  csRNUMBER b = (*this - rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator-(csRNUMBER _a)
{
  //because mantissa and exposant could be
  //modified, it is necessary to make a copy

  csRNUMBER rn, a(_a.mantissa, _a.exponent, _a.sign), b(mantissa,exponent,sign);
  free(rn.mantissa);

  if(b.exponent > a.exponent)
  {
    b.increaseShape(b.exponent-a.exponent);
  }
  else if(b.exponent < a.exponent)
  {
    a.increaseShape(-b.exponent+a.exponent);
  }

  if(b.sign == CS_POSITIVE_NUMBER)
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      rn.mantissa = makeAddition(b.mantissa,a.mantissa,rn.mantSize);
    }
    else
    {
      rn.mantissa = makeSubstraction(b.mantissa,a.mantissa,b.mantSize, a.mantSize, rn.mantSize, rn.sign);

    }
  }
  else
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      rn.mantissa = makeSubstraction(a.mantissa,b.mantissa,a.mantSize, b.mantSize, rn.mantSize,rn.sign);
    }
    else
    {
      rn.mantissa = makeAddition(b.mantissa,a.mantissa,rn.mantSize);
      rn.sign = CS_NEGATIVE_NUMBER;
    }
  }
  removeFrontZeros(rn.mantissa,rn.mantSize);
  rn.exponent = a.exponent;

  free(a.mantissa);
  free(b.mantissa);

  return rn;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator*(long a)
{
  csRNUMBER rn(a);
  csRNUMBER b = (*this * rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator*(csRNUMBER _a)
{
  csRNUMBER rn, a(_a.mantissa, _a.exponent, _a.sign);
  free(rn.mantissa);
  rn.mantissa = 0;
  rn.mantissa = makeMultiplication(mantissa, a.mantissa, mantSize, a.mantSize, rn.mantSize);

  removeFrontZeros(rn.mantissa,rn.mantSize);
  rn.exponent = exponent + a.exponent;

  if(sign == CS_POSITIVE_NUMBER)
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      rn.sign = CS_NEGATIVE_NUMBER;
    }
    else
    {
      rn.sign = CS_POSITIVE_NUMBER;
    }
  }
  else
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      rn.sign = CS_POSITIVE_NUMBER;
    }
    else
    {
      rn.sign = CS_NEGATIVE_NUMBER;
    }
  }
  rn.precision = _a.precision > precision ? _a.precision : precision;
  rn.setPrecision(RPRECISION + rn.precision);
  free(a.mantissa);
  return rn;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator/(long a)
{
  csRNUMBER rn(a);
  csRNUMBER b = (*this / rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator/(csRNUMBER _a)
{
  //use a copy of this (ex _this) because "increaseShape" could change address
  csRNUMBER rn, a(_a.mantissa, _a.exponent, _a.sign), _this(mantissa, exponent, sign);
  free(rn.mantissa);
  rn.mantissa = 0;

  if(sign == CS_POSITIVE_NUMBER)
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      rn.sign = CS_NEGATIVE_NUMBER;
    }
    else
    {
      rn.sign = CS_POSITIVE_NUMBER;
    }
  }
  else
  {
    if(a.sign == CS_NEGATIVE_NUMBER)
    {
      rn.sign = CS_POSITIVE_NUMBER;
    }
    else
    {
      rn.sign = CS_NEGATIVE_NUMBER;
    }
  }

  //if(mantSize > _a.mantSize) // avoid precision increasing in some cases
  a.shapeOutZeros(); // could increase precision in some cases

  //put the both at the same size
  long precSize = RPRECISION + rn.precision;

  long diff = (long(a.mantSize) + a.exponent) - (long(mantSize) + exponent);

  if(diff > precSize)
  {
    free(a.mantissa);
    rn.set("0",0,0);
    return rn;
  }
  diff =  (long)_a.mantSize - (long)mantSize;

  if(diff > 0)
  {
    _this.increaseShape(diff+precSize);
  }
  else
  {
    diff = precSize + diff;
    if(diff > 0)
      _this.increaseShape(diff);
  }

  rn.exponent = exponent - a.exponent;

  rn.mantissa = makeDivisionQ(_this.mantissa, a.mantissa, mantSize, a.mantSize, rn.mantSize);

  removeFrontZeros(rn.mantissa,rn.mantSize);


  rn.precision = _a.precision > precision ? _a.precision : precision;
  rn.setPrecision(RPRECISION + rn.precision);
  free(a.mantissa);
  free(_this.mantissa);
  return rn;
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::operator-() const
{
  csRNUMBER rn(mantissa,exponent,!sign);

  return rn;
}

CS_FORCE_INLINE void csRNUMBER::clear()
{
  if(mantissa)
  {
    free(mantissa);
    sign = 0;
    exponent = 0;
    mantSize = 0;
    precision = 0;
    mantissa = 0;
  }
}

CS_FORCE_INLINE void csRNUMBER::copy(csRNUMBER a)
{
  if(mantissa != a.mantissa)
  {
    if(mantissa)
    {
      free(mantissa);
    }

    sign = a.sign;
    mantSize = a.mantSize;
    exponent = a.exponent;

    mantissa = newString(a.mantissa, mantSize);
    precision = a.precision > precision ? a.precision : precision;
  }
}

CS_FORCE_INLINE void csRNUMBER::operator=(csRNUMBER a)
{
  if(mantissa)
  {
    free(mantissa);
  }

  sign = a.sign;
  mantSize = a.mantSize;
  exponent = a.exponent;
  mantissa = newString(a.mantissa, mantSize);
  precision = a.precision > precision ? a.precision : precision;

  a.clear();
}

CS_FORCE_INLINE void csRNUMBER::operator=(long a)
{
  if(mantissa)
  {
    free(mantissa);
  }
  long aa = a < 0 ? -a : a;
  if(a) sign = (aa/a == -1) ? 1 : 0;
  mantissa = uLongToString(aa, mantSize);
}

CS_FORCE_INLINE void csRNUMBER::operator=(const char*a)
{
  size_t l = strlen(a);
  if(a && l)
  {
    if(mantissa)
    {
      free(mantissa);
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
    
    csRNUMBER rn(eval, a + start);
    sign = rn.sign;
    mantSize = rn.mantSize;
    exponent = rn.exponent;
    mantissa = rn.mantissa;
    precision = rn.precision > precision ? rn.precision : precision;
  }
}

CS_FORCE_INLINE void csRNUMBER::assign(csRNUMBER a)
{
  if(mantissa)
  {
    free(mantissa);
  }

  sign = a.sign;
  mantSize = a.mantSize;
  exponent = a.exponent;
  mantissa = a.mantissa;
  precision = a.precision > precision ? a.precision : precision;


}
CS_FORCE_INLINE void csRNUMBER::set(size_t id, char digit)
{
  if(mantSize < id)
  {
    increaseShape(digit-mantSize+1);
  }

  mantissa[id] = digit;

}
CS_FORCE_INLINE void csRNUMBER::setInt(size_t id, char digit)
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

CS_FORCE_INLINE csRNUMBER::operator csQNUMBER()
{
  csQNUMBER qn;
  free(qn.numerator);

  if(exponent >= 0)
  {
    size_t t = mantSize + exponent;
    char *nb = csAlloc<char>(t+1);
    nb[t] = '\0';

    for(size_t i=0; i<mantSize; i++)
      nb[i] = mantissa[i];

    for(size_t i=mantSize; i<t; i++)
      nb[i] = '0';

    qn.numerator = nb;
    qn.numSize = t;
  }
  else
  {
    free(qn.denominator);
    qn.numerator = newString(mantissa, mantSize);
    qn.numSize = mantSize;
    qn.denomSize = -exponent+1;
    qn.denominator = csAllocCharPtr(qn.denomSize, '0');
    qn.denominator[0] = '1';
  }

  return qn;
}

CS_FORCE_INLINE char* CSARITHMETIC_API CSARITHMETIC::getPrintFormat(csRNUMBER a) // a corriger car augmente une case vide a la fin de res, ajoutant sa taille reelle
{
  char*intPart=0, *decPart=0, *ret=0;
  size_t iSize = 0, dSize = 0;
  if(a.exponent < 0)
  {
    size_t exp = abs(a.exponent);
    if(a.mantSize <= exp)
    {
      //if(a.mantSize == exp)

      size_t diff = exp-a.mantSize;
      if(a.sign)
      {
        iSize = 3;
        intPart = newString("-0.",iSize);
      }
      else
      {
        iSize = 2;
        intPart = newString("0.",iSize);
      }
      decPart = csAlloc<char>(exp+1);
      for(int i=0; i<diff; i++)
      {
        decPart[i] = '0';
      }
      for(int i=diff; i<exp; i++)
      {
        decPart[i] = a.mantissa[i-diff];
      }
      decPart[exp] = '\0';
      dSize = exp;
    }
    else
    {
      size_t t = a.mantSize-exp;
      if(a.sign)
      {
        iSize = t+2;
        intPart = csAlloc<char>(iSize+1);
        size_t m = iSize-1;
        for(int i=1; i<m; i++)
        {
          intPart[i] = a.mantissa[i-1];
        }
        intPart[0] = '-';
        intPart[m] = '.';
        intPart[iSize] = '\0';

      }
      else
      {
        iSize = t+1;
        intPart = csAlloc<char>(iSize+1);
        intPart[t] = '.';
        intPart[iSize] = '\0';
        for(int i=0; i<t; i++)
        {
          intPart[i] = a.mantissa[i];
        }

      }
      dSize = exp;
      decPart = csAlloc<char>(dSize+1);
      decPart[dSize] = '\0';
      for(int i=t; i<a.mantSize; i++)
      {
        decPart[i-t] = a.mantissa[i];
      }
    }
  }
  else
  {

    if(a.exponent)
    {

      
      char*zeros = csAlloc2(a.exponent+1, '0');
      zeros[a.exponent] = '\0';
      if(a.sign)
      {
        intPart = csAlloc<char>(a.exponent+a.mantSize+2);
        sprintf(intPart, "-%s%s\0", a.mantissa, zeros);
      }
      else
      {
        intPart = csAlloc<char>(a.exponent+a.mantSize+1);
        sprintf(intPart, "%s%s\0", a.mantissa, zeros);
      }
      free(zeros);
    }
    else
    {
      if(a.sign)
      {
        intPart = csAlloc<char>(a.mantSize+2);
        sprintf(intPart, "-%s\0", a.mantissa);
      }
      else
      {
        intPart = csAlloc<char>(a.mantSize+1);
        sprintf(intPart, "%s\0", a.mantissa);
      }
    }
    iSize = strlen(intPart);

  }
  ret = csAlloc<char>(iSize+dSize+1);

  if(decPart)
    sprintf(ret, "%s%s\0", intPart, decPart);
  else
    sprintf(ret, "%s\0", intPart);
  free(intPart);
  free(decPart);

  return ret;
}

CS_FORCE_INLINE void csRNUMBER::print(const char*title)
{
  char*formated = getPrintFormat(*this);
  cout<<"\n"<<title<<formated<<"\n";
  free(formated);
}

CS_FORCE_INLINE void csRNUMBER::print2(const char*title)
{
  if(sign == CS_POSITIVE_NUMBER)
    cout<<"\n "<<title<<mantissa<<"x10^"<<exponent<<"\n";
  else
    cout<<"\n "<<title<<"-"<<mantissa<<"x10^"<<exponent<<"\n";
}

CS_FORCE_INLINE bool clearAndReturn(csRNUMBER& a, csRNUMBER& b, bool cndResult)
{
  a.clear();
  b.clear();
  return cndResult;
}

CS_FORCE_INLINE bool csRNUMBER::operator!=(csRNUMBER a)
{
  return !(*this == a);
}

CS_FORCE_INLINE bool csRNUMBER::operator!=(long a)
{
  csRNUMBER rn(a);
  bool b = !(*this == rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csRNUMBER::operator==(long a)
{
  csRNUMBER rn(a);
  bool b = (*this == rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csRNUMBER::equalZero()
{
  size_t sw = mantSize;
  for(int i = 0; i<sw; i++)
  {
    if(mantissa[i] != '0')
    {
      return 0;
    }
  }
  return 1;
}

CS_FORCE_INLINE bool csRNUMBER::operator==(csRNUMBER _a)
{
  csRNUMBER a(_a.mantissa, _a.exponent, _a.sign);
  csRNUMBER _this(mantissa, exponent, sign);
  if(a.mantSize > _this.mantSize) // put to same sizes
  {
    _this.reshape(a.mantSize);
  }
  else if(a.mantSize < _this.mantSize)
  {
    a.reshape(_this.mantSize);
  }

  if(_this.exponent != a.exponent)
  {
    a.clear();
    _this.clear();
    return 0;
  }
  for(size_t i=0; i<_this.mantSize; i++)
  {
    if(_this.mantissa[i]!=a.mantissa[i])
    {
      a.clear();
      _this.clear();
      return false;
    }
  }

  if(a.sign == sign)
  {
    a.clear();
    _this.clear();
    return 1;
  }

  a.clear();
  _this.clear();
  return 0;
}

CS_FORCE_INLINE bool csRNUMBER::operator<(long a)
{
  csRNUMBER rn(a);
  bool b = (*this < rn);
  rn.clear();
  return b;
}
CS_FORCE_INLINE bool csRNUMBER::operator<(csRNUMBER _a)
{
  csRNUMBER a(_a.mantissa, _a.exponent, _a.sign);
  csRNUMBER _this(mantissa, exponent, sign);
  if(a.mantSize > _this.mantSize) // put to same sizes
  {
    _this.reshape(a.mantSize);
  }
  else if(a.mantSize < _this.mantSize)
  {
    a.reshape(_this.mantSize);
  }

  if(_this.exponent < a.exponent)
  {
    if(a.sign == 1)
      return clearAndReturn(_this,a,0);
    else
      return clearAndReturn(_this,a,1);
  }
  else if(_this.exponent > a.exponent)
  {
    if(sign == 1)
      return clearAndReturn(_this,a,1);
    else
      return clearAndReturn(_this,a,0);
  }
  else
  {
    if(sign == 1 && a.sign == 0)
      return clearAndReturn(_this,a,1);
  }

  for(size_t i=0; i<_this.mantSize; i++)
  {
    if(_this.mantissa[i] < a.mantissa[i])
    {
      if(a.sign == 1)
        return clearAndReturn(_this,a,0);
      else
        return clearAndReturn(_this,a,1);
    }
    else if(_this.mantissa[i] > a.mantissa[i])
    {
      if(sign == 1)
        return clearAndReturn(_this,a,1);
      else
        return clearAndReturn(_this,a,0);
    }
  }

  return clearAndReturn(_this,a,0);
}

CS_FORCE_INLINE bool csRNUMBER::operator<=(long a)
{
  csRNUMBER rn(a);
  bool b = (*this <= rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csRNUMBER::operator<=(csRNUMBER _a)
{
  csRNUMBER a(_a.mantissa, _a.exponent, _a.sign);
  csRNUMBER _this(mantissa, exponent, sign);
  if(a.mantSize > _this.mantSize) // put to same sizes
  {
    _this.reshape(a.mantSize);
  }
  else if(a.mantSize < _this.mantSize)
  {
    a.reshape(_this.mantSize);
  }

  if(_this.exponent < a.exponent)
  {
    if(a.sign == 1)
      return clearAndReturn(_this,a,0);
    else
      return clearAndReturn(_this,a,1);
  }
  else if(_this.exponent > a.exponent)
  {
    if(sign == 1)
      return clearAndReturn(_this,a,1);
    else
      return clearAndReturn(_this,a,0);
  }
  else
  {
    if(sign == 1 && a.sign == 0)
      return clearAndReturn(_this,a,1);
  }

  for(size_t i=0; i<_this.mantSize; i++)
  {
    if(_this.mantissa[i] < a.mantissa[i])
    {
      if(a.sign == 1)
        return clearAndReturn(_this,a,0);
      else
        return clearAndReturn(_this,a,1);
    }
    else if(_this.mantissa[i] > a.mantissa[i])
    {
      if(sign == 1)
        return clearAndReturn(_this,a,1);
      else
        return clearAndReturn(_this,a,0);
    }

  }

  if(a.sign == sign)
    return clearAndReturn(_this,a,1);
  return clearAndReturn(_this,a,0);
}

CS_FORCE_INLINE bool csRNUMBER::operator>(long a)
{
  csRNUMBER rn(a);
  bool b = (*this > rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csRNUMBER::operator>(csRNUMBER _a)
{
  csRNUMBER a(_a.mantissa, _a.exponent, _a.sign);
  csRNUMBER _this(mantissa, exponent, sign);
  if(a.mantSize > _this.mantSize) // put to same sizes
  {
    _this.reshape(a.mantSize);
  }
  else if(a.mantSize < _this.mantSize)
  {
    a.reshape(_this.mantSize);
  }

  if(_this.exponent < a.exponent)
  {
    if(a.sign == 1)
      return clearAndReturn(_this,a,1);
    else
      return clearAndReturn(_this,a,0);
  }
  else if(_this.exponent > a.exponent)
  {
    if(sign == 1)
      return clearAndReturn(_this,a,0);
    else
      return clearAndReturn(_this,a,1);
  }
  else
  {
    if(sign == 0 && a.sign == 1)
      return clearAndReturn(_this,a,1);
  }

  for(size_t i=0; i<_this.mantSize; i++)
  {
    if(_this.mantissa[i] < a.mantissa[i])
    {
      if(a.sign == 1)
        return clearAndReturn(_this,a,1);
      else
        return clearAndReturn(_this,a,0);
    }
    else if(_this.mantissa[i] > a.mantissa[i])
    {
      if(sign == 1)
        return clearAndReturn(_this,a,0);
      else
        return clearAndReturn(_this,a,1);
    }
  }

  return clearAndReturn(_this,a,0);
}

CS_FORCE_INLINE bool csRNUMBER::operator>=(long a)
{
  csRNUMBER rn(a);
  bool b = (*this >= rn);
  rn.clear();
  return b;
}

CS_FORCE_INLINE bool csRNUMBER::operator>=(csRNUMBER _a)
{

  csRNUMBER a(_a.mantissa, _a.exponent, _a.sign);
  csRNUMBER _this(mantissa, exponent, sign);
  if(a.mantSize > _this.mantSize) // put to same sizes
  {
    _this.reshape(a.mantSize);
  }
  else if(a.mantSize < _this.mantSize)
  {
    a.reshape(_this.mantSize);
  }

  if(_this.exponent < a.exponent)
  {
    if(a.sign == 1)
      return clearAndReturn(_this,a,1);
    else
      return clearAndReturn(_this,a,0);
  }
  else if(_this.exponent > a.exponent)
  {
    if(sign == 1)
      return clearAndReturn(_this,a,0);
    else
      return clearAndReturn(_this,a,1);
  }
  else
  {
    if(sign == 0 && a.sign == 1)
      return clearAndReturn(_this,a,1);
  }

  for(size_t i=0; i<_this.mantSize; i++)
  {
    if(_this.mantissa[i] < a.mantissa[i])
    {
      if(a.sign == 1)
        return clearAndReturn(_this,a,1);
      else
        return clearAndReturn(_this,a,0);
    }
    else if(_this.mantissa[i] > a.mantissa[i])
    {
      if(sign == 1)
        return clearAndReturn(_this,a,0);
      else
        return clearAndReturn(_this,a,1);
    }
  }

  if(a.sign == sign)
    return clearAndReturn(_this,a,1);
  return clearAndReturn(_this,a,0);
}

CS_FORCE_INLINE csRNUMBER csRNUMBER::getMantissaSection(size_t first, size_t last)
{
  csRNUMBER rn;
  free(rn.mantissa);

  rn.mantissa = newString(mantissa,first,last);
  rn.mantSize = last-first;
  rn.exponent = exponent;

  return rn;
}

using namespace __mem_man;
using namespace __ar_man;
