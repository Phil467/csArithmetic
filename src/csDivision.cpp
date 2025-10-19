
#include "csARITHMETIC.h"

using namespace __mem_man;
using namespace __ar_man;

extern csBIDIGITS** addTable;
extern csBIDIGITS** mulTable;
extern csBIDIGITS** subTable;
extern char** divTable;


void CSARITHMETIC_API CSARITHMETIC::makeDivisionTable()
{

  divTable = csAlloc<char*>(1048);
  for(size_t j=0; j<1048; j++)
  {
     divTable[j] = csAllocCharPtr(1048, '0');
     if(j > 48)
     for(size_t i=49; i<1048; i++)
     {
       divTable[j][i] = (j-48)/(i-48) + 48;
     }
  }

}


void CSARITHMETIC_API CSARITHMETIC::makeSubstractionForDiv(char*a, char*b, char*&result, size_t opSize, size_t frontOffset)
{
  csBIDIGITS prevCarry={'0','0'};
  size_t m=opSize-1, n=m+frontOffset;
  
  for(size_t i=n,j=0; j<opSize; j++, i-=1)
  {

    prevCarry = subTable[int(a[i])][prevCarry.tens];

    csBIDIGITS bd = subTable[prevCarry.units][b[m-j]];
    prevCarry.tens = prevCarry.tens>bd.tens?prevCarry.tens:bd.tens;
    result[i] = bd.units;
  }
  
}


void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationForDivBase1(char*a, uchar b, char*&result, size_t aSize, size_t resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  size_t m=aSize-1, i = 0, j=0, n=resSize-1;
  
  for(i=m,j=0; j<aSize; j++, i-=1)
  {
    csBIDIGITS bd1 = mulTable[int(a[i])][b];

    csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
    prevCarry = addTable[bd1.tens][bd2.tens];
    result[n-j] = bd2.units;
    
  }
  // la case pour la retenue n'existe pas ici, car b est choisi tel qu'il n'y aura pas de retenue
  
}
void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationForDivBase2(char*a, uchar b, char*&result, size_t aSize, size_t resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  size_t m=aSize-1, i = 0, j=0, n=resSize-1;
  
  for(i=m,j=0; j<aSize; j++, i-=1)
  {
    csBIDIGITS bd1 = mulTable[int(a[i])][b];

    csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
    prevCarry = addTable[bd1.tens][bd2.tens];
    result[n-j] = bd2.units;
    
  }
  // la case pour la retenue existe bien ici, car b est choisi tel qu'il est possible d'avoir la retenue
  result[n-j] = prevCarry.units;
  
}


char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*_a, char* _b, char*&result,
        size_t& aSize, size_t& bSize, size_t& resSize)
{
    char*a = newString(_a,aSize);
    char*b = newString(_b,bSize);
  // dangerous when changing addresses of a or b by reallocating eg with removeFrontZeros()
  if(removeFrontZeros(b,bSize)==0)
  {  
    resSize = 1;
    result = newString("0", 1);
    free(b); 
    free(a);//important cause b == 0, and is reallocated, so it is useless outside
    //this function. make sure not to free it outside of here
    return "Invalid input !";
  }
  if(removeFrontZeros(a,aSize)==0)
  {
    resSize = 1;
    result = newString("0", 1);
    free(a);
    free(b); //important cause a == 0, and is reallocated, so it is useless outside
    //this function. make sure not to free it outside of here
    return "0";
  }

  if(bSize == 1)
  {
    if(b[0] == '1')
    {
      resSize = aSize;
      result = newString(a, resSize);
      free(a);
      free(b);
      return 0;
    }
    if(b[0] == '0')
    {
      resSize = 1;
      if(aSize == 1 && a[0] == '0')
      {
        result = newString("ind", 3);
        free(a);
        free(b);
        return "Indeterminate";
      }
      else
      {
        result = newString("inf", 3);
        free(a);
        free(b);
        return "Infinite";
      }
    }
  }

  size_t tmpResSize = bSize+1;
  char*tmpRes = csAllocCharPtr(tmpResSize, '0');
  size_t skipZerosLen = 0, lastSzl=0;
  bool cmp;

  size_t resSize1= resSize-1;

  uchar quotientDigit = '1';// or 49

  size_t tmpResSizeForSub=bSize, qid=0;
  char* bCpy = shiftRightCopy(b, bSize, tmpResSize);


  size_t nextSize = bSize-1;
  
  while(1)
  {
    skipZeros(a, aSize, skipZerosLen);
    size_t diff = aSize-skipZerosLen;
    nextSize++;
    tmpResSizeForSub = nextSize-skipZerosLen;
    //cmp = isaGreater2(a,b,tmpResSizeForSub,bSize,skipZerosLen);
    cmp = (nextSize>skipZerosLen&&nextSize<=aSize)&&isaGreaterEqual2(a,b,tmpResSizeForSub,bSize,skipZerosLen);
    if(diff < bSize || (diff==bSize && !cmp) || nextSize>aSize)
    {
      if(nextSize < skipZerosLen)
      {
        resSize = qid + skipZerosLen-nextSize+2; // +2 cause nextSize was increased
        csReallocString(&result,qid,resSize, '0');
        
        break;
      }
      
      resSize = strlen(result);
      result = (char*)realloc(result, resSize+1);
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
  free(tmpRes);
  free(bCpy);
  free(a);
  free(b);
  
  return 0;
}

char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*_a, char* _b, char*&result, char*&remain,
        size_t& aSize, size_t& bSize, size_t& resSize, size_t& remSize)
{
  
 char*a = newString(_a,aSize);
 char*b = newString(_b,bSize);
 
  if(removeFrontZeros(b,bSize)==0)
  {  
    remSize = 1;
    remain = newString("0", 1);

    resSize = 1;
    result = newString("0", 1);
    free(a);
    free(b);
    return "Invalid input !";
  }
  if(removeFrontZeros(a,aSize)==0)
  {
    remSize = 1;
    remain = newString("0", 1);
    resSize = 1;
    result = newString("0", 1);
    free(a);
    free(b);
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
      free(a);
      free(b);
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
        
        free(a);
        free(b);
        return "Indeterminate";
      }
      else
      {
        result = newString("inf", 3);
        
        free(a);
        free(b);
        return "Infinite";
      }
    }
  }

  size_t tmpResSize = bSize+1;
  char*tmpRes = csAllocCharPtr(tmpResSize, '0');
  size_t skipZerosLen = 0;
  bool cmp;

  size_t resSize1= resSize-1;

  uchar quotientDigit = '1';// or 49

  size_t tmpResSizeForSub=bSize, qid=0;
  char* bCpy = shiftRightCopy(b, bSize, tmpResSize);
  

  size_t nextSize = bSize-1;
  
  while(1)
  {

    skipZeros(a, aSize, skipZerosLen);
    size_t diff = aSize-skipZerosLen;
    
    nextSize++;
    
    tmpResSizeForSub = nextSize-skipZerosLen;
    //cmp = isaGreater2(a,b,tmpResSizeForSub,bSize,skipZerosLen);
    cmp =  (nextSize>skipZerosLen&&nextSize<=aSize)&&isaGreaterEqual2(a,b,tmpResSizeForSub,bSize,skipZerosLen);
    if(diff < bSize || (diff==bSize && !cmp) || nextSize>aSize)
    {
      remain = (char*)realloc(remain,diff+1);
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
      result = (char*)realloc(result, resSize+1);
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
  free(tmpRes);
  free(bCpy);
  free(a);
  free(b);
  
  return 0;
}

char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*a, char* b, char*&remain)
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

char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*a, char* b)
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

char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(char*a, char* b, size_t aSize, size_t bSize, size_t& resSize)
{
  int rs;
  resSize = (rs=(aSize-bSize+1))>1?rs:1;
  size_t remSize = bSize, remSize1 = remSize+1;

  char*quotient = csAllocCharPtr(resSize, '0');
  
  makeDivisionQ(a, b, quotient, aSize, bSize, resSize);

  return quotient;
}

char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(const char*_a, const char* _b, char*&remain)
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

char* CSARITHMETIC_API CSARITHMETIC::makeDivisionQ(const char*_a, const char* _b)
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

char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionR(char*_a, char* _b, char*&resInt, char*&resDec, char*&remain,
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

char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionR2(char*_a, char* _b, char*&resInt, char*&resDec, char*&remain,
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

char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionR2(char*_a, char* _b, char*&resInt, char*&resDec,
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

char const* CSARITHMETIC_API CSARITHMETIC::makeDivisionR(char*a, char* b, char*&resInt, char*&resDec, char*&remain, size_t nDecimals)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  size_t remSize=0, resSize=0, resIntSize, resDecSize;
  return makeDivisionR(a, b, resInt, resDec, remain,
                    aSize, bSize, resSize, resIntSize, resDecSize, remSize, nDecimals);

}
