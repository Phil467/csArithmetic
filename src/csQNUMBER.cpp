#include "csARITHMETIC.h"
#include <cstddef>
#include <cstdio>
#include <cstring>

using namespace __mem_man;
using namespace __ar_man;
using namespace CSARITHMETIC;

extern int RPRECISION;

csQNUMBER::csQNUMBER(const char* _numerator, const char* _denominator, bool _sign)
{
  //set(_numerator, _denominator, _sign);
  numSize = strlen(_numerator);
  denomSize = strlen(_denominator);

  numerator = newString(_numerator, numSize);
  denominator = newString(_denominator, denomSize);
  sign = _sign;
}

csQNUMBER::csQNUMBER(size_t num, size_t denom, bool _sign)
{
  sign = _sign;
  numerator = uLongToString(num, numSize);
  denominator = uLongToString(denom, denomSize);
}

void csQNUMBER::init()
{
  numerator=0;
  denominator=0;
}

csQNUMBER* CSARITHMETIC_API CSARITHMETIC::csQNUMBER_PTR_ALLOC(size_t nb)
{
  csQNUMBER *qn = csAlloc<csQNUMBER>(nb);
  for(size_t i=0; i<nb; i++)
  {
    qn[i].init();
  }
  return qn;
}

csQNUMBER* CSARITHMETIC_API CSARITHMETIC::csQNUMBER_PTR_ALLOC(size_t nb, csQNUMBER init)
{
  csQNUMBER *qn = csAlloc<csQNUMBER>(nb);
  for(size_t i=0; i<nb; i++)
  {
    qn[i].init();
    qn[i].set(init.numerator,init.denominator,init.sign);
  }
  return qn;
}

void CSARITHMETIC_API CSARITHMETIC::csQNUMBER_PTR_FREE(csQNUMBER*& qn, size_t nb)
{
  for(size_t i=0; i<nb; i++)
  {
    qn[i].clear();
  }
  free(qn);
}
/*csQNUMBER::~csQNUMBER()
{

  clear();
}*/

void csQNUMBER::set(const char* _numerator, const char* _denominator, bool _sign)
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

void csQNUMBER::set(size_t num, size_t denom, bool _sign)
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

void csQNUMBER::setl(long num, size_t denom)
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

size_t csQNUMBER::maxSize()
{
  return numSize>denomSize?numSize:denomSize;
}

void csQNUMBER::reduce(size_t sizeCondition)
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

csQNUMBER csReduce(csQNUMBER a, size_t sizeCondition)
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

bool csQNUMBER::isAbsDifferent(csQNUMBER a)
{
  return !isAbsEqual(a);
}

bool csQNUMBER::operator==(csQNUMBER a)
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

bool csQNUMBER::isAbsEqual(csQNUMBER a)
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

bool csQNUMBER::operator!=(csQNUMBER a)
{
  return !(*this == a);
}

bool csQNUMBER::operator>(csQNUMBER a)
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

bool csQNUMBER::isAbsGreater(csQNUMBER a)
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

bool csQNUMBER::operator>=(csQNUMBER a)
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

bool csQNUMBER::isAbsGreaterEqual(csQNUMBER a)
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

bool csQNUMBER::operator<(csQNUMBER a)
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

bool csQNUMBER::isAbsLess(csQNUMBER a)
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
     // return 1;
    //else
      return 0;
  }
  else if(n1Size < n2Size)
  {
    free(num1);
    free(num2);
    //if(sign < a.sign || (a.sign == 1 && sign == 1))
      //return 0;
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

bool csQNUMBER::operator<=(csQNUMBER a)
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

bool csQNUMBER::isAbsLessEqual(csQNUMBER a)
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

void csQNUMBER::print(const char*title)
{
  if(sign == CS_POSITIVE_NUMBER)
  cout<<"\n "<<title<<numerator<<" / "<<denominator<<"\n";
  else
  cout<<"\n "<<title<<"-"<<numerator<<" / "<<denominator<<"\n";
}

void csQNUMBER::clear(const source_location loc)
//void csQNUMBER::clear()
{
  if(numerator)
  {
    free(numerator);
    free(denominator);
    numerator = 0;
    denominator = 0;
    numSize = 0;
    denomSize = 0;

    //std::cout<<"\n(Line "<<loc.line()<<") variable released.\n";
  }
}

csQNUMBER csQNUMBER::operator+(csQNUMBER a)
{
  csQNUMBER qn;
  free(qn.numerator);
  qn.numerator=0;
  free(qn.denominator);
  qn.denominator=0;
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
//cout<<" ---- ha\n";
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

csQNUMBER csQNUMBER::operator-(csQNUMBER a)
{
  csQNUMBER qn;
  free(qn.numerator);
  qn.numerator=0;
  free(qn.denominator);
  qn.denominator=0;

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

csQNUMBER csQNUMBER::operator*(csQNUMBER a)
{
  csQNUMBER qn;
  free(qn.numerator);
  free(qn.denominator);
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

csQNUMBER csQNUMBER::operator/(csQNUMBER a)
{
  csQNUMBER qn;
  free(qn.numerator);
  free(qn.denominator);
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

csQNUMBER& csQNUMBER::operator=(csQNUMBER a)
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

csQNUMBER& csQNUMBER::operator=(long a)
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
  /* numSize = 2;
  numerator = newString("20", numSize); */
  denomSize = 1;
  denominator = newString("1", denomSize);

  return *this;
}
/*
void csQNUMBER::operator=(double a)
{
  if(numerator)
  {
    free(numerator);
    free(denominator);
    numerator = 0;
  }
  if(a != 0)
  sign = a/abs(a);

  numerator = csAlloc<char>(65);
  numerator[65] = '\0';
  sprintf(numerator,"%ld", long(a*1e19));
  numSize = strlen(numerator);

  denominator = csAlloc2(20,'0');
  denominator[0] = '1';
  denominator[20] = '\0';
  denomSize = strlen(denominator);

  size_t gsz;
  char*g = gcd(numerator,denominator,gsz);
  char* newNum = makeDivisionQ(numerator, g, numSize, gsz)
}*/


csQNUMBER csQNUMBER::operator+(long a)
{
  csQNUMBER qn;
  free(qn.numerator);
  qn.numerator=0;
  free(qn.denominator);
  qn.denominator=0;

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


csQNUMBER csQNUMBER::operator-(long a)
{
  csQNUMBER qn;
  free(qn.numerator);
  qn.numerator=0;
  free(qn.denominator);
  qn.denominator=0;

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


csQNUMBER csQNUMBER::operator*(long a)
{
  csQNUMBER qn;
  free(qn.numerator);
  free(qn.denominator);

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


csQNUMBER csQNUMBER::operator/(long a)
{
  csQNUMBER qn;
  free(qn.numerator);
  free(qn.denominator);

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

double csQNUMBER::getDouble()
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

char* csQNUMBER::quotient()
{
  double res;
  size_t sz;
  //char* nb = makeDivisionQ(numerator,denominator, numSize, denomSize, sz);
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

csQNUMBER::operator csRNUMBER()
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
    //free(a); // dont free cause it is already done in makeDivisionQ() for that case
    free(a); // the new modification (14/01/2025) of makeDivisionQ() allow to do that
    return rn;
  }

  removeFrontZeros(result,resSize);
  csRNUMBER rn(result, -RPRECISION, sign);

  free(result);
  
  free(a);
  
  return rn;
}


csQNUMBER CSARITHMETIC_API CSARITHMETIC::pow(csQNUMBER a, size_t p)
{
  csQNUMBER r("1");
  for(size_t i=0; i<p; i++)
  {
    r = r*a;
  }
  return r;
}

void csQNUMBER::copy(csQNUMBER a)
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

bool csQNUMBER::isZero()
{
  for(size_t i=0; i<numSize; i++)
  {
    if(numerator[i] != '0')
      return 0;
  }
  return 1;
}

bool csQNUMBER::isNonZero()
{
  return !isZero();
}
