
#include "csARITHMETIC.h"

using namespace __mem_man;
using namespace __ar_man;

extern csBIDIGITS** subTable;

void CSARITHMETIC_API CSARITHMETIC::makeSubstractionTable()
{
  subTable = csAlloc<csBIDIGITS*>(58);
  for(size_t j=0; j<58; j++)
  {
     subTable[j] = csAlloc2<csBIDIGITS>(58,{'0','0'});
  }
  for(int j=47; j<58; j++)
  {
    for(int i=47; i<58; i++)
    {
      subTable[j][i] = substractionTransform(j-i);

    }
  }
}



csBIDIGITS CSARITHMETIC_API CSARITHMETIC::substractionTransform(int i)
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

void CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a, char*b, char*&result, size_t opSize)
{
  csBIDIGITS prevCarry={'0','0'};
  size_t m=opSize-1;
  
  for(size_t i=m,j=0; j<opSize; j++, i-=1)
  {

    prevCarry = subTable[int(a[i])][prevCarry.tens];

    csBIDIGITS bd = subTable[prevCarry.units][b[i]];
    prevCarry.tens = prevCarry.tens>bd.tens?prevCarry.tens:bd.tens;
    result[i] = bd.units;
  }
}

char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a,char*b)
{
  size_t opSize;
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  char*a1, *b1;
  getReadySub2(a,b,a1,b1,opSize);
  
  char*result = csAllocCharPtr(opSize, '0');
  makeSubstraction(a1,b1,result,opSize);
  free(a1);
  free(b1);
  return result;
}

char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a,char*b, bool& sign)
{
  size_t opSize;
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  char*a1, *b1;
  sign = getReadySub2(a,b,a1,b1,opSize);
  
  char*result = csAllocCharPtr(opSize, '0');
  makeSubstraction(a1,b1,result,opSize);
  free(a1);
  free(b1);
  return result;
}

char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a,char*b, size_t aSize, size_t bSize, bool& sign)
{
  size_t opSize;
  char*a1, *b1;
  sign = getReadySub3(a,b,a1,b1,aSize, bSize, opSize);
  
  char*result = csAllocCharPtr(opSize, '0');
  makeSubstraction(a1,b1,result,opSize);
  free(a1);
  free(b1);
  return result;
}

char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(char*a,char*b, size_t aSize, size_t bSize, size_t& resSize, bool& sign)
{
  char*a1, *b1;
  
  sign = getReadySub3(a,b,a1,b1,aSize, bSize, resSize);

  char*result = csAllocCharPtr(resSize, '0');
  makeSubstraction(a1,b1,result,resSize);
  free(a1);
  free(b1);
  return result;
}



char* CSARITHMETIC_API CSARITHMETIC::makeSubstraction(const char*a,const char*b)
{
  size_t opSize;
  char *a1=0, *b1=0;

  int sign = getReadySub2(a,b,a1,b1,opSize);
  char*result = csAlloc<char>(opSize+1);
  result[opSize] = '\0';
  makeSubstraction(a1,b1,result,opSize);
  free(a1);
  free(b1);
  return result;
}
