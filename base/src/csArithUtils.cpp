#include "csArithUtils.h"
#include <string.h>

using namespace __mem_man;
using namespace __ar_man;

void CSARITHMETIC_API __mem_man::csReallocString(char** str, size_t size)
{
  *str = (char*)realloc(*str, size+1);
  str[0][size]= '\0';
}

void CSARITHMETIC_API __mem_man::csReallocString(char** str, size_t size, size_t newSize, char cFill)
{
  *str = (char*)realloc(*str, newSize+1);
  for(size_t i=size; i<newSize; i++)
  {
    (*str)[i] = cFill;
  }
  str[0][newSize]= '\0';
}

char* CSARITHMETIC_API __mem_man::csAllocCharPtr(size_t n,size_t n1, char init)
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

char* CSARITHMETIC_API __mem_man::csAllocCharPtr(size_t n, char init)
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


char* CSARITHMETIC_API __mem_man::newString(const char*cstr)
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

char* CSARITHMETIC_API __mem_man::newString(const char*cstr, size_t size)
{
  char* str = csAlloc<char>(size+1);
  for(size_t i=0; i<size; i++)
  {
    str[i] = cstr[i];
  }
  str[size] = '\0';
  return str;
}

char* CSARITHMETIC_API __mem_man::newString(char*cstr, size_t size)
{
  char* str = csAlloc<char>(size+1);
  for(size_t i=0; i<size; i++)
  {
    str[i] = cstr[i];
  }
  str[size] = '\0';
  return str;
}

char* CSARITHMETIC_API __mem_man::newString(const char*cstr, size_t begin, size_t end)
{
  size_t size = end-begin;
  /*if(size)
  {*/
    char* str = csAlloc<char>(size+1);
    for(size_t i=begin, j= 0; i<end; i++, j++)
    {
      str[j] = cstr[i];
    }
    str[size] = '\0';
    return str;
  /*}
  else
    return 0;*/
}


char* __mem_man::intToString(int nb, size_t& sz)
{
  char* str = csAlloc<char>(30);
  sprintf(str, "%d\0", nb);
  sz = strlen(str);
  
  str = (char*)realloc(str, sz+1);
  str[sz] = '\0';

  return str;
}

char* __mem_man::uLongToString(size_t nb, size_t& sz)
{
  char* str = csAlloc<char>(30);
  sprintf(str, "%ld\0", nb);
  sz = strlen(str);
  
  str = (char*)realloc(str, sz+1);
  str[sz] = '\0';

  return str;
}

void __ar_man::getReady(char*&a,char*&b, size_t&opSize)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  opSize = aSize>bSize?aSize:bSize;

  if(opSize > bSize)
  {
    size_t delta = opSize-bSize;
    b = (char*)realloc(b,opSize+1);
    //for(size_t i=bSize-1; i>=0; i--)
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
    //for(size_t i=aSize-1; i>=0; i--)
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

bool __ar_man::isaGreater(char*a,char*b,size_t opSize, size_t ibegin)
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

bool __ar_man::isaGreaterEqual(char*a,char*b,size_t opSize, size_t ibegin)
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

bool __ar_man::isaGreater2(char*a,char*b,size_t aSize,size_t bSize, size_t ibegin)
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
    //cout<< i << " - ai  "<<a[aid]<<" bi "<<b[i]<<"\n";
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
      //cout<< "  ai "<<a<<" bi "<<b<<" begin "<<ibegin<<"\n";
      bl = 1;
      break;
    }
    else if(k<j)
    {
     // cout<< "  ai "<<a[aid]<<" bi "<<b[i]<<" begin "<<ibegin<<"\n";
      break;
    }
  }
  }
  //cout<< "  bl  "<<bl<<"  "<<ibegin<<"\n";
  return bl;
}

bool __ar_man::isaGreaterEqual2(char*a,char*b,size_t aSize, size_t bSize, size_t ibegin)
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

int __ar_man::getReadySub(char*&a,char*&b, size_t&opSize)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  opSize = aSize>bSize?aSize:bSize;

  int sign = 1;
  if(opSize > bSize)
  {
    size_t delta = opSize-bSize;
    b = (char*)realloc(b,opSize+1);
    //for(size_t i=bSize-1; i>=0; i--)
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
    //for(size_t i=aSize-1; i>=0; i--)
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

void __ar_man::getReady2(const char*a0,const char*b0, char*&a, char*&b, size_t&opSize)
{
  size_t aSize = strlen(a0);
  size_t bSize = strlen(b0);
  b = csAlloc<char>(bSize+1);
  a = csAlloc<char>(aSize+1);
  //sprintf(b,"%s\0",b0); //make bugs
  //sprintf(a,"%s\0",a0);
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
    //for(size_t i=bSize-1; i>=0; i--) // app bug
    for(size_t i=bSize-1,j=0; j<bSize; j++,i--)
    {
      b[i+delta] = b0[i];
    }
    for(size_t i=0; i<delta; i++)
    {
      b[i] = '0';
    }
   // bSize = opSize;

    for(size_t i=0; i<aSize; i++)
      a[i] = a0[i];
    a[aSize] = '\0';
  }
  else if(opSize > aSize)
  {
    size_t delta = opSize-aSize;
    a = (char*)realloc(a,opSize+1);
    a[opSize]='\0';
    //for(size_t i=aSize-1; i>=0; i--)
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


bool __ar_man::getReadySub2(const char*a0,const char*b0, char*&a, char*&b, size_t&opSize)
{// avoid putting zeros in front of numbers !!!!!
  size_t aSize = strlen(a0);
  size_t bSize = strlen(b0);
  b = csAlloc<char>(bSize+1);
  a = csAlloc<char>(aSize+1);
  //sprintf(b,"%s\0",b0); //make bugs
  //sprintf(a,"%s\0",a0);
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
    //for(size_t i=bSize-1; i>=0; i--) // app bug
    for(size_t i=bSize-1,j=0; j<bSize; j++,i--)
    {
      b[i+delta] = b0[i];
    }
    for(size_t i=0; i<delta; i++)
    {
      b[i] = '0';
    }
   // bSize = opSize;

    for(size_t i=0; i<aSize; i++)
      a[i] = a0[i];
    a[aSize] = '\0';
  }
  else if(opSize > aSize)
  {
    size_t delta = opSize-aSize;
    a = (char*)realloc(a,opSize+1);
    a[opSize]='\0';
    //for(size_t i=aSize-1; i>=0; i--)
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

bool __ar_man::getReadySub3(const char*a0,const char*b0, char*&a, char*&b, size_t aSize, size_t bSize, size_t&opSize)
{// avoid putting zeros in front of numbers !!!!!

  //sprintf(b,"%s\0",b0); //make bugs
  //sprintf(a,"%s\0",a0);

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
    //for(size_t i=bSize-1; i>=0; i--) // app bug
    for(size_t i=bSize-1,j=0; j<bSize; j++,i--)
    {
      b[i+delta] = b0[i];
    }
    for(size_t i=0; i<delta; i++)
    {
      b[i] = '0';
    }
   // bSize = opSize;
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
    //for(size_t i=aSize-1; i>=0; i--)
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


void __ar_man::shiftLeft(char*&nb, size_t size, size_t nShift)
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

void __ar_man::removeLeft(char*&nb, size_t& size, size_t remLen)
{
  size_t diff = size - remLen;
  char *ret = csAlloc<char>(diff+1);
  ret[diff] = '\0';
  for(size_t i=remLen, j=0; i<size; i++, j++)
  {
    ret[j] = nb[i];
  }
  //cout<<" mid ---- \n";
  free(nb); // --------------------------------------------------------------------------- to check for memory !!!!
  nb = ret;
  size = diff;
}
void __ar_man::removeRight(char*&nb, size_t& size, size_t remLen)
{
  size_t diff = size - remLen;
  char *ret = csAlloc<char>(diff+1);
  ret[diff] = '\0';
  for(size_t i=0; i<diff; i++)
  {
    ret[i] = nb[i];
  }
  //free(nb); // --------------------------------------------------------------------------- to check for memory !!!!
  nb = ret;
  size = diff;
}


void __ar_man::shiftLeftCopy(char* nb, char*&nbCopy, size_t nbSize, size_t nbCopyPos)
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

char* __ar_man::shiftRightCopy(char* nb, size_t nbSize, size_t nbCopySize)
{
  char* nbCopy = csAllocCharPtr(nbCopySize, '0');
  size_t nbCopyPos = nbCopySize-nbSize;
  for(size_t i=0; i<nbSize; i++)
  {
    nbCopy[nbCopyPos+i] = nb[i];
  }
  return nbCopy;
}


void __ar_man::shiftRight(char*&nb, size_t size, size_t nShift)
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


void __ar_man::skipZeros(char*a, size_t aSize, size_t& skipLen)
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


void __ar_man::skipZeros2(char*a, size_t aSize, size_t& skipLen, size_t&incr)
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


bool __ar_man::removeFrontZeros(char*&a, size_t& size)
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
    a = (char*)realloc(a,2);
    a[1] = '\0';
    size = 1;
  }
  //cout<<" end ---- ha\n";
  return 0;
}

void __ar_man::addFrontZeros(char*&a, size_t& size, size_t nZeros)
{
  size = size+nZeros;
  a = (char*)realloc(a,size+1);
  a[size] = '\0';
  shiftRight(a,size,nZeros);
}

void __ar_man::fillString(char*&str, const char* cstr, size_t size)
{
  for(size_t i=0; i<size; i++)
  {
    str[i] = cstr[i];
  }
  str[size] = '\0';
}

char* __ar_man::filledString(const char* cstr, size_t size)
{
  char*str = (char*)malloc(size+1);
  
  for(size_t i=0; i<size; i++)
  {
    str[i] = cstr[i];
  }
  str[size] = '\0';
  return str;
}
