
#include "csARITHMETIC.h"

using namespace __mem_man;
using namespace __ar_man;

extern char** divTable;

char const* CSARITHMETIC_API CSARITHMETIC::makeModulusQ(char*a, char* b, char*&remain,
        size_t& aSize, size_t& bSize, size_t& remSize)
{
  
  if(removeFrontZeros(b,bSize)==0)
  {
    remSize = 1;
    remain = csAlloc<char>(2);
    remain[0] = '0';
    remain[remSize] = '\0';
    return "Invalid Input !";
  }
  if(removeFrontZeros(a,aSize)==0)
  {
    remSize = 1;
    remain = csAlloc<char>(2);
    remain[0] = '0';
    remain[remSize] = '\0';
    return "0";
  }

  if((bSize == 1 && b[0] == '1')||(aSize == 1 && a[0] == '1'))
  {
    remSize = 1;
    remain = csAlloc<char>(2);
    remain[0] = '0';
    remain[remSize] = '\0';
    return "0";
  }

  size_t tmpResSize = bSize+1;
  char*tmpRes = csAllocCharPtr(tmpResSize, '0');

  size_t skipZerosLen = 0;
  bool cmp;

  uchar quotientDigit = '1';// or 49

  size_t tmpResSizeForSub=bSize;
  char* bCpy = shiftRightCopy(b, bSize, tmpResSize);

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
      remain = (char*)realloc(remain,diff+1);
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
        //quotientDigit = ((a[skipZerosLen]-48)*10+a[skipZerosLen+1]-48)/(b[0]-48) + 48 -1;
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

  free(tmpRes); // ----------------------------------------------------- check latter !!!!! 
  free(bCpy);
  
  return 0;
}

void CSARITHMETIC_API CSARITHMETIC::makeModulusQ(char*a, char* b, char*&remain)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t remSize = 0;
  remSize = bSize;
  remain = csAllocCharPtr(remSize, '0');
  makeModulusQ(a, b, remain, aSize, bSize, remSize);
}

char* CSARITHMETIC_API CSARITHMETIC::makeModulusQ(char*a, char* b)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t remSize = 0;
  remSize = bSize;
  char* remain = csAllocCharPtr(remSize, '0');
  makeModulusQ(a, b, remain, aSize, bSize, remSize);
  return remain;
}

char* CSARITHMETIC_API CSARITHMETIC::makeModulusQ(const char*a, const char* b)
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
