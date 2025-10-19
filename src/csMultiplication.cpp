#include "csARITHMETIC.h"

using namespace __mem_man;
using namespace __ar_man;

extern csBIDIGITS** mulTable;
extern csBIDIGITS** addTable;


void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationTable()
{
  //pour eviter de convertir des caracteres en entier
  //dans les operations, il sera plus efficace d'effectuer
  //directement les calculs sur les valeurs ASCII de chaque
  //nombre. Donc il faudra generer une table de calculs basique
  //contenant 48 (valeur du caractere '0') + 10. Les cases non necessaires
  //seront remplis par des zeros.

  int** tab = csAlloc<int*>(58);

  for(int j=0; j<58; j++)
  {
    tab[j] = csAlloc2<int>(58,0);
  }

  for(int j=48; j<58; j++)
  {
    for(int i=48; i<58; i++)
    {
      tab[j][i] = (i-48)*(j-48);
    }
  }
  mulTable = toBIDIGITS(tab);

  for(int j=0; j<58; j++)
    free(tab[j]);

  free(tab);

}

void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationBase(char*a, uchar b, char*&result, size_t aSize, size_t resSize)
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
  
  result[n-j] = prevCarry.units;
  free(result); // ----------------------------------------------------- check latter !!!!! 
}

void CSARITHMETIC_API CSARITHMETIC::makeMultiplicationBase2(char*a, uchar b, char*&result, size_t aSize, size_t resSize
    ,csBIDIGITS& prevCarry, size_t m, size_t n)
{
  prevCarry={'0','0'};
  size_t j, i;
  for(i=m,j=0; j<aSize; j++, i-=1)
  {
    csBIDIGITS bd1 = mulTable[int(a[i])][b];

    csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
    prevCarry = addTable[bd1.tens][bd2.tens];
    result[n-j] = bd2.units;
  }
  result[n-j] = prevCarry.units;
}

void CSARITHMETIC_API CSARITHMETIC::makeAdditionForMul(char*a, char*b, char*&result, size_t opSize, size_t resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  size_t m=opSize-1, n=resSize-opSize;

  for(size_t i=m,j=0; j<opSize; j++, i-=1)
  {
    size_t id = i+n;
    csBIDIGITS bd1 = addTable[int(a[id])][int(b[id])];
    csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
    prevCarry = addTable[bd1.tens][bd2.tens];
    result[id] = bd2.units;
    
  }
  
}

void CSARITHMETIC_API CSARITHMETIC::makeMultiplication(char*a, char*b, char*&result, char*& tmpResult,
 size_t aSize, size_t bSize, size_t resSize)
{
  
  size_t m=bSize-1, n = aSize + 1;

  csBIDIGITS prevCarry;
  size_t m1=aSize-1, n1=resSize-1;

  for(size_t i=m,j=0; j<bSize; j++, i-=1)
  {
    
    makeMultiplicationBase2(a, b[i], tmpResult, aSize, resSize,
                            prevCarry, m1, n1);
    shiftLeft(tmpResult, resSize, j);
    makeAdditionForMul(result, tmpResult, result, n+j, resSize);

  }
  
}

char* CSARITHMETIC_API CSARITHMETIC::makeMultiplication(char*a, char*b)
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
  char*tmpResult = csAllocCharPtr(opSize, sz, '0');

  makeMultiplication(a,b,result,tmpResult,aSize,bSize,opSize);
  free(tmpResult);
  return result;
}

char* CSARITHMETIC_API CSARITHMETIC::makeMultiplication(char*a, char*b, size_t aSize, size_t bSize)
{
  size_t opSize = aSize + bSize;
  size_t sz = opSize + 1;
  char*result = csAllocCharPtr(opSize, sz, '0');
  char*tmpResult = csAllocCharPtr(opSize, sz, '0');

  makeMultiplication(a,b,result,tmpResult,aSize,bSize,opSize);
  free(tmpResult);
  return result;
}

char* CSARITHMETIC_API CSARITHMETIC::makeMultiplication(char*a, char*b, size_t aSize, size_t bSize, size_t& resSize)
{
  resSize = aSize + bSize;
  size_t sz = resSize + 1;
  char*result = csAllocCharPtr(resSize, sz, '0');
  char*tmpResult = csAllocCharPtr(resSize, sz, '0');
  
  makeMultiplication(a,b,result,tmpResult,aSize,bSize,resSize);
   
  free(tmpResult);
  return result;
}


char* CSARITHMETIC_API CSARITHMETIC::makeMultiplication(const char*a, const char*b)
{
  return makeMultiplication((char*)a,(char*)b);
}

