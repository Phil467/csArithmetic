#include "csARITHMETIC.h"
#include <cstddef>
#include <cstdio>
#include <cstring>

using namespace __mem_man;
using namespace __ar_man;
using namespace CSARITHMETIC;

int RPRECISION = 20;

void CSARITHMETIC_API CSARITHMETIC::setRNumberPrecision(int precision)
{
  RPRECISION = precision;
}
int CSARITHMETIC_API CSARITHMETIC::getRNumberPrecision()
{
  return RPRECISION;
}

csRNUMBER::csRNUMBER(const char* _mantissa, int _exponent, bool _sign)
{
  set(_mantissa, _exponent, _sign);
}
csRNUMBER::csRNUMBER(unsigned long _mantissa, int _exponent, bool _sign)
{
  set(_mantissa, _exponent, _sign);
}
csRNUMBER::csRNUMBER(long _mantissa, int _exponent)
{
  set(_mantissa, _exponent);
}
csRNUMBER::csRNUMBER(bool evaluate, const char* number)
{
  set(evaluate, number);
}

csRNUMBER::~csRNUMBER()
{
  //clear();
  //cout<< mantissa<<" destructor called ! \n";
}


void csRNUMBER::set(const char* _mantissa, int _exponent, bool _sign)
{
  if(mantissa)
  {//cout << " mantissa : "<<mantissa<<"\n";
    free(mantissa);
    mantissa = 0;
  }
  mantSize = strlen(_mantissa);
  mantissa = newString(_mantissa, mantSize);
  sign = _sign;
  exponent = _exponent;
  precision = 0;
  //setPrecision(RPRECISION);
}

void csRNUMBER::set(unsigned long _mantissa, int _exponent, bool _sign)
{
  free(mantissa);

  mantissa = uLongToString(_mantissa, mantSize);
  sign = _sign;
  exponent = _exponent;
  precision = 0;
}

void csRNUMBER::set(long _mantissa, int _exponent)
{
  free(mantissa);

  long l = _mantissa < 0 ? -_mantissa : _mantissa;
  if(_mantissa) sign = (_mantissa / l == -1 ? 1 : 0);
  mantissa = uLongToString(l, mantSize);
  exponent = _exponent;
  precision = 0;
}

void csRNUMBER::set(bool evaluate, const char* _number)
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
  //setPrecision(RPRECISION);
}

void csRNUMBER::setAsTenPower(long power)
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

void csRNUMBER::init()
{
  mantissa=0;
}

csRNUMBER* CSARITHMETIC_API CSARITHMETIC::csRNUMBER_PTR_ALLOC(size_t nb)
{
  csRNUMBER *rn = csAlloc<csRNUMBER>(nb);
  for(size_t i=0; i<nb; i++)
  {
    rn[i].init();
  }
  return rn;
}

csRNUMBER* CSARITHMETIC_API CSARITHMETIC::csRNUMBER_PTR_ALLOC(size_t nb, csRNUMBER init)
{
  csRNUMBER *rn = csAlloc<csRNUMBER>(nb);
  for(size_t i=0; i<nb; i++)
  {
    rn[i].init();
    rn[i].set(init.mantSize,init.exponent,init.sign);
  }
  return rn;
}


void CSARITHMETIC_API CSARITHMETIC::csRNUMBER_PTR_FREE(csRNUMBER*& rn, size_t nb)
{
  for(size_t i=0; i<nb; i++)
  {
    rn[i].clear();
  }
  free(rn);
}

void csRNUMBER::random(size_t nDigits, char digitMin, char digitMax, long exponent, bool sign)
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

void csRNUMBER::setPrecision(int size)
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

size_t csRNUMBER::significantZerosCount(size_t initialPos)
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

void addOne(char*&nb, size_t& size)
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

void csRNUMBER::forcePrecision(int size)
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

void csRNUMBER::reshape(size_t newMantissaSize)
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

void csRNUMBER::increaseShape(size_t size)
{

  //if(exponent > 0)
    exponent -= size;
 // else
    //exponent += size;

    //cout<< newMantissaSize<< "begin 1\n";
  size_t s = mantSize + size;
  csReallocString(&mantissa, mantSize, s, '0');
  mantSize = s;
}

void csRNUMBER::decreaseShape(size_t size)
{

 // if(exponent > 0)
    exponent += size;
  size_t s = mantSize - size;
  csReallocString(&mantissa, s);
  mantSize = s;
}

void csRNUMBER::shapeOutZeros()
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

size_t csRNUMBER::getDigitNumber()
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

csRNUMBER csRNUMBER::abs()
{
  csRNUMBER rn(mantissa,exponent,0);
  return rn;
}

csRNUMBER csRNUMBER::operator+(long a)
{
  csRNUMBER rn(a);
  csRNUMBER b = (*this + rn);
  rn.clear();
  return b;
}
csRNUMBER csRNUMBER::operator+(csRNUMBER _a)
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

csRNUMBER csRNUMBER::operator-(long a)
{
  csRNUMBER rn(a);
  csRNUMBER b = (*this - rn);
  rn.clear();
  return b;
}

csRNUMBER csRNUMBER::operator-(csRNUMBER _a)
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

csRNUMBER csRNUMBER::operator*(long a)
{
  csRNUMBER rn(a);
  csRNUMBER b = (*this * rn);
  rn.clear();
  return b;
}

csRNUMBER csRNUMBER::operator*(csRNUMBER _a)
{
  csRNUMBER rn, a(_a.mantissa, _a.exponent, _a.sign);
  free(rn.mantissa);
  rn.mantissa = 0;
  //cout<<"  mantissa : "<<mantissa<<"\n";
  //char *str = newString(mantissa);
  rn.mantissa = makeMultiplication(mantissa, a.mantissa, mantSize, a.mantSize, rn.mantSize);
  //cout<<"  mantissa : "<<rn.mantissa<<"\n";

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
  //cout<<"  mantissa end : "<<rn.mantSize<<"\n";
  rn.precision = _a.precision > precision ? _a.precision : precision;
  rn.setPrecision(RPRECISION + rn.precision);
  //rn.forcePrecision(RPRECISION + rn.precision);//--------------------attention--------derniere modif
  free(a.mantissa);
  //cout<<"  mantissa : "<<rn.mantissa<<"\n";
  return rn;
}

csRNUMBER csRNUMBER::operator/(long a)
{
  csRNUMBER rn(a);
  csRNUMBER b = (*this / rn);
  rn.clear();
  return b;
}

csRNUMBER csRNUMBER::operator/(csRNUMBER _a)
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

  //print2("num : ");
  //cout<<a.mantissa<<" "<<a.exponent<<"\n";
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
  //a.print(" a : ");
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


  //cout<<RPRECISION<<",  rn : "<<rn.mantSize<<"\n";
  rn.precision = _a.precision > precision ? _a.precision : precision;
  rn.setPrecision(RPRECISION + rn.precision);
  //rn.forcePrecision(RPRECISION + rn.precision);//--------------------attention--------derniere modif
  free(a.mantissa);
  free(_this.mantissa);
  return rn;
}

csRNUMBER csRNUMBER::operator-() const
{
  csRNUMBER rn(mantissa,exponent,!sign);

  return rn;
}

void csRNUMBER::clear()
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

void csRNUMBER::copy(csRNUMBER a)
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

void csRNUMBER::operator=(csRNUMBER a)
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

void csRNUMBER::operator=(long a)
{
  if(mantissa)
  {
    free(mantissa);
  }
  long aa = a < 0 ? -a : a;
  if(a) sign = (aa/a == -1) ? 1 : 0;
  mantissa = uLongToString(aa, mantSize);
}

void csRNUMBER::operator=(const char*a)
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

void csRNUMBER::assign(csRNUMBER a)
{
  if(mantissa)
  {
    free(mantissa);
  }

  sign = a.sign;
  mantSize = a.mantSize;
  exponent = a.exponent;
  mantissa = a.mantissa;
  //precision = a.precision;
  precision = a.precision > precision ? a.precision : precision;

  //a.mantissa = 0; ///// -

}
void csRNUMBER::set(size_t id, char digit)
{
  if(mantSize < id)
  {
    increaseShape(digit-mantSize+1);
  }

  mantissa[id] = digit;

}
void csRNUMBER::setInt(size_t id, char digit)
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
      //exponent -= l;
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
      //exponent += l;
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

csRNUMBER::operator csQNUMBER()
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

char* CSARITHMETIC_API CSARITHMETIC::getPrintFormat(csRNUMBER a) // a corriger car augmente une case vide a la fin de res, ajoutant sa taille reelle
{
  char*intPart=0, *decPart=0, *ret=0;
  size_t iSize = 0, dSize = 0;
  if(a.exponent < 0)
  {
    size_t exp = abs(a.exponent);
    if(a.mantSize <= exp)
    {
      //if(a.mantSize == exp)

        //cout<<" " <<a.mantissa<<" --hghhj---------\n";
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
       // cout<<" " <<decPart[i]<<" --hghhj---------\n";
      }
      for(int i=diff; i<exp; i++)
      {
        decPart[i] = a.mantissa[i-diff];
      }
      decPart[exp] = '\0';
      dSize = exp;
    }
    else
    { //cout<<"hghhj ----- \n";
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

      /*if(a.sign)
        sprintf(intPart, "-%sx10^%d\0", a.mantissa, a.exponent);
      else
        sprintf(intPart, "%sx10^%d\0", a.mantissa, a.exponent);*/
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
  //ret[iSize+dSize] = '\0';
  free(intPart);
  free(decPart);

  return ret;
}

void csRNUMBER::print(const char*title)
{
  char*formated = getPrintFormat(*this);
  cout<<"\n"<<title<<formated<<"\n";
  free(formated);
}

void csRNUMBER::print2(const char*title)
{
  if(sign == CS_POSITIVE_NUMBER)
    cout<<"\n "<<title<<mantissa<<"x10^"<<exponent<<"\n";
  else
    cout<<"\n "<<title<<"-"<<mantissa<<"x10^"<<exponent<<"\n";
}

bool clearAndReturn(csRNUMBER& a, csRNUMBER& b, bool cndResult)
{
  a.clear();
  b.clear();
  return cndResult;
}

bool csRNUMBER::operator!=(csRNUMBER a)
{
  return !(*this == a);
}

bool csRNUMBER::operator!=(long a)
{
  csRNUMBER rn(a);
  bool b = !(*this == rn);
  rn.clear();
  return b;
}

bool csRNUMBER::operator==(long a)
{
  csRNUMBER rn(a);
  bool b = (*this == rn);
  rn.clear();
  return b;
}

bool csRNUMBER::equalZero()
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

bool csRNUMBER::operator==(csRNUMBER _a)
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
  //a.shapeOutZeros();
  //_this.shapeOutZeros();

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

bool csRNUMBER::operator<(long a)
{
  csRNUMBER rn(a);
  bool b = (*this < rn);
  rn.clear();
  return b;
}
bool csRNUMBER::operator<(csRNUMBER _a)
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

bool csRNUMBER::operator<=(long a)
{
  csRNUMBER rn(a);
  bool b = (*this <= rn);
  rn.clear();
  return b;
}

bool csRNUMBER::operator<=(csRNUMBER _a)
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

bool csRNUMBER::operator>(long a)
{
  csRNUMBER rn(a);
  bool b = (*this > rn);
  rn.clear();
  return b;
}

bool csRNUMBER::operator>(csRNUMBER _a)
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

bool csRNUMBER::operator>=(long a)
{
  csRNUMBER rn(a);
  bool b = (*this >= rn);
  rn.clear();
  return b;
}

bool csRNUMBER::operator>=(csRNUMBER _a)
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

csRNUMBER csRNUMBER::getMantissaSection(size_t first, size_t last)
{
  csRNUMBER rn;
  free(rn.mantissa);

  rn.mantissa = newString(mantissa,first,last);
  rn.mantSize = last-first;
  rn.exponent = exponent;

  return rn;
}
