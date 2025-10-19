#include "csARITHMETIC.h"

using namespace __mem_man;
using namespace __ar_man;
using namespace CSARITHMETIC;

csBIDIGITS** mulTable=0;
char** divTable=0;
csBIDIGITS** addTable=0;
csBIDIGITS** subTable=0;

char* mulTenTable=0;


void CSARITHMETIC_API CSARITHMETIC::init()
{
  //srand(time(0));
  if(!mulTable)
  {
    makeMultiplicationTable();
    createAdditionTable();
    makeSubstractionTable();
    makeDivisionTable();
    mulTenTable = csAllocCharPtr(58, '0');
    for(size_t i=48; i<58; i++)
      mulTenTable[i] = (i-48)*10;
  }
}

void CSARITHMETIC_API CSARITHMETIC::printNumber(char*nb, const char*seperator)
{
  cout<<nb<<seperator;
}

csBIDIGITS** CSARITHMETIC_API CSARITHMETIC::toBIDIGITS(int** opTable)
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


uchar CSARITHMETIC_API CSARITHMETIC::bicharIndex(uchar tens, uchar units)
{
  return (tens-48)*10+units;
  //return mulTenTable[tens]+units;
}

uchar CSARITHMETIC_API CSARITHMETIC::tricharIndex(int cents, uchar tens, uchar units)
{
  return uchar((cents-48)*100+(tens-48)*10+units);
  //return mulTenTable[tens]+units;
}

//unsigned uchar

void CSARITHMETIC_API CSARITHMETIC::printTable(char*opName)
{
  char* title;
  csBIDIGITS**tab;
  char**ctab;
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
  /*size_t min = 48;
  for(size_t j=min; j<max; j++)
  {
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
  }*/
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



char* CSARITHMETIC_API CSARITHMETIC::gcd(char*_a, char*_b, size_t aSize, size_t bSize, size_t& gdcSize)
{
  char *a = newString(_a,aSize);
  if(aSize == 1 && a[0] == '1')
  {
    gdcSize = 1;
    return a;
  }
  char *b = newString(_b,bSize);
  if(bSize == 1 && b[0] == '1')
  {
    free(a);
    gdcSize = 1;
    return b;
  }
  //cout<<"  a "<<a<<"\n";
  //cout<<"  b "<<b<<"\n";
  /*bool bl = isaGreater2(a, b, aSize, bSize, 0);
  if(!bl)
  {
    char*ch = a;
    a = b;
    b = ch;

    size_t sz = aSize;
    aSize = bSize;
    bSize = sz;
  }*/
  /* if(bSize == 1)
  {
    if(b[0]=='1')
    {
      gdcSize = 1;
      free(a);
      return b;
    }
    if(b[0]=='0')
    {
      free(b);
      gdcSize = aSize;
      return a;
    }
  } */
  size_t remSize = bSize;
  char* remain;
  do
  {
    remain = csAllocCharPtr(remSize, '0');
    //cout<< aSize<<" mod "<<b<<" ";
    makeModulusQ(a, b, remain, aSize, bSize, remSize);
    //cout<<remain<<" --- rem\n";
    free(a);
    //cout<<remain<<" --- rem\n";
    a = b; aSize = bSize;
    b = remain; bSize = remSize;
    //cout<<"\n gdcRem : "<<a<<"\n";
  }  while(remSize > 1);

  while(!(remain[0] == '0' || remain[0] == '1'))
  {
    remain = csAllocCharPtr(remSize, '0');
    makeModulusQ(a, b, remain, aSize, bSize, remSize);
    free(a);
    a = b; aSize = bSize;
    b = remain; bSize = remSize;
  }
  
  if(remain[0] == '1')
  {
    gdcSize = bSize;
    free(a);
    removeFrontZeros(b,gdcSize);
    return b;
  }
  gdcSize = aSize;
  free(b);
  removeFrontZeros(a,gdcSize);
  return a;


}

char*CSARITHMETIC_API CSARITHMETIC::gcd(char*a, char*b,size_t& gdcSize)
{
  size_t aSize = strlen(a);
  size_t bSize = strlen(b);
  char* res = gcd(a,b,aSize,bSize,gdcSize);
  return res;
}

char*CSARITHMETIC_API CSARITHMETIC::gcd(const char*a, const char*b,size_t& gdcSize)
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

char* CSARITHMETIC_API CSARITHMETIC::findGcd(char*_a)
{
  char*g="";
  size_t i = 1;
  char* inc = newString("2");
  char* inc2 = newString("1");
  char* a = makeMultiplication(_a,_a);
  char* b, *b2;
  cout<<" b : "<<b<<"\n";
  size_t k=0;
  size_t sz=0;
  do
  {
    cout<<" gdc : "<<_a<<"\n";
    b = makeAddition(_a,inc);
    g = gcd(a,b,sz);
    char*inc3 = makeAddition(inc,inc2);
    free(inc);
    inc = inc3;

    free(b);
    k++;
   // cout<<" gdc : "<<inc<<"\n";
    free(g);
  }
  while( sz > 1 || (sz==1 && g[0]=='1'));
  //cout<<gcd("1385047","20796936",sz)<<"\n";u
  return g;
}

csQNUMBER gethightsrnum(csQNUMBER a, csQNUMBER&sr)
{
  size_t t = a.numSize%2 + a.numSize;
  char* n = csAllocCharPtr(t+1, '0');
  n[0] = '1';
  csQNUMBER mx(n,"1", 0);
  free(n);

  t = t/2;
  n = csAllocCharPtr(t+1, '0');
  n[0] = '1';

  sr.set(n,"1",0);
  free(n);

  return mx;
}

csRNUMBER gethighsrnum(csRNUMBER _a, csRNUMBER&sr)
{
  csRNUMBER a(_a.mantissa,_a.exponent,_a.sign);
  a.reshape(a.mantSize+a.exponent);
  size_t t = a.mantSize%2 + a.mantSize;
  //a.print2("aaa : ");
  
  char* n = csAllocCharPtr(t+1, '0');
  n[0] = '1';

  csRNUMBER mx(n,a.exponent, a.sign);
  //mx.print("mx : ");
  free(n);

  t = t/2;
  n = csAllocCharPtr(t+1, '0');
  n[0] = '1';

  //sr.set(n,a.exponent,a.sign);
  sr.set(n,a.exponent/2,a.sign);
  free(n);

  return mx;
}

csQNUMBER getlowsrnum(csQNUMBER a, csQNUMBER&sr)
{
  size_t t =  a.numSize - a.numSize%2 - 2*(a.numSize%2 == 0) ;
  char* n = csAllocCharPtr(t+1, '0');
  n[0] = '1';
  csQNUMBER mx(n,"1", 0);
  free(n);

  t = t/2;
  n = csAllocCharPtr(t+1, '0');
  n[0] = '1';

  sr.set(n,"1",0);
  free(n);
  a.clear();

  return mx;
}

csRNUMBER getlowsrnum(csRNUMBER _a, csRNUMBER&sr)
{
  csRNUMBER a(_a.mantissa,_a.exponent,_a.sign);
  a.reshape(a.mantSize+a.exponent);
  size_t mod = a.mantSize%2;
  size_t t =  a.mantSize - mod - 2*(mod == 0) ;
  char* n = csAllocCharPtr(t+1, '0');
  n[0] = '1';
  csRNUMBER mx(n, a.exponent, a.sign);
  //cout<< " ha "<<mx<<"\n";
  free(n);

  t = t/2;
  n = csAllocCharPtr(t+1, '0');
  n[0] = '1';

  //sr.set(n,a.exponent,a.sign);
  sr.set(n,a.exponent/2,a.sign);
  free(n);
  a.clear();
  return mx;
}

csQNUMBER nextWeight(csQNUMBER w1, csQNUMBER w2)
{
  char*resInt1, *resDec1, *rem1;
  char*resInt2, *resDec2, *rem2;
  size_t precision=0;
  makeDivisionR(w1.numerator, w1.denominator, resInt1, resDec1, rem1, precision);
  makeDivisionR(w2.numerator, w2.denominator, resInt2, resDec2, rem2, precision);

  size_t sz1 = strlen(resInt1);
  size_t sz2 = strlen(resInt2);
  removeFrontZeros(resInt1, sz1);
  removeFrontZeros(resInt2, sz2);

  sz1 = sz1<sz2?sz1:sz2;

  char* nw = csAllocCharPtr(sz1, '0');
  nw[0] = '5';
  nw[sz1] = '\0';

  csQNUMBER qn(nw);

  free(resInt1);
  free(resInt2);
  free(resDec1);
  free(resDec2);
  free(rem1);
  free(rem2);
  free(nw);

  return qn;
}

csQNUMBER CSARITHMETIC_API CSARITHMETIC::csSqrtQ(csQNUMBER n, int precision)
{
  csRNUMBER rn;
  long oldprec = getRNumberPrecision();
  setRNumberPrecision(precision);
  rn.copy((csRNUMBER)n);
  
  csRNUMBER rns = csSqrt(rn, precision);
  csQNUMBER ret = csQNUMBER(rns);
  CSARITHMETIC::setRNumberPrecision(oldprec);
  
  free(rn.mantissa);
  free(rns.mantissa);
  return ret;
}

csQNUMBER CSARITHMETIC_API CSARITHMETIC::csSqrtQ_Prec(csQNUMBER n, int precision)
{
  csRNUMBER rn;
  long oldprec = getRNumberPrecision();
  setRNumberPrecision(precision);
  csRNUMBER rns;
  
  rn.copy((csRNUMBER)n);
  rns = csSqrt_Prec(rn, precision);
  
  csQNUMBER ret = csQNUMBER(rns);
  setRNumberPrecision(oldprec);
  
  free(rn.mantissa);
  free(rns.mantissa);
  return ret;
}

csRNUMBER CSARITHMETIC_API CSARITHMETIC::csSqrt(csRNUMBER C2, int precision)
{
  //setRNumberPrecision(C2.getDigitNumber()+10);
  if(C2 == 0)
  {
    csRNUMBER res("0");
    return res;
  }
  if(C2 == 1)
  {
    csRNUMBER res("1");
    return res;
  }
  setRNumberPrecision(precision);
  csRNUMBER A1;
  csRNUMBER A2 = getlowsrnum(C2, A1);
  csRNUMBER B1;
  csRNUMBER B2 = gethighsrnum(C2,B1);
  
  //long ex = 308;
  //cout<<A1.mantSize<<"  A1 sw \n";
  //cout<<A2.mantSize<<"  A2 sz \n";
  //A1.set("1",ex,0);
  //A2.set("1",2*ex,0);
  //A1.reshape(ex+1);
  //A2.reshape(2*ex+1);
/*C2.print(" C2 : ");
A2.print(" A2 : ");
A1.print(" A1 : ");
B2.print(" B2 : ");
B1.print(" B1 : ");*/

  csRNUMBER dl, dr, d, e, l1, l2, mid("5",-1)/* , prec("1",-5) */, err, err2, diff;
  int i=0;
  
  d = B2 - A2;
  dl = (B2 - C2);
  dr = (C2 - A2);
  do
  {
    /*dl = B2-C2;
    dr = C2 - A2;
    d = B2 - A2;
    A1 = (A1*dl + dr*B1) / d;
    A1.forcePrecision(precision);

    l1 = (A1+B1)*mid;
   
    err = l1*l1 - C2;
    if(err.sign == 0)
    {
      B1.copy(l1);
      B2 = B1*B1;
      B2.forcePrecision(precision);
    }
    else
    {
      A1.copy(l1);

    }
    A2 = A1*A1;*/
    //A2.forcePrecision(precision);
    l1 = (A1*dl + dr*B1)/d;
    l1.forcePrecision(precision);
    l2 = l1*l1;
    err = l2 - C2;
    if(err.sign == 0)
    {
      /* B1.copy(l1);
      B2.copy(l2);
      l1 = (l1 + A1)*mid;
      l2 = l1*l1;
      err2 = l2 - C2;
      if(err2.sign == 0 )
      {
        B1.copy(l1);
        B2.copy(l2);
        d = B2 - A2;
        dl.copy(err2);
        dr = (C2 - A2);
      }
      else
      {
        A1.copy(l1);
        A2.copy(l2);
        
        d = B2 - A2;
        dl.copy(err);
        dr = (C2 - A2);
      } */

      B1.copy(l1);
      d = l2 - A2;
      dl.copy(err);
      dr = (C2 - A2);
      //cout<<" i : "<<i<<"\n";
      
      /*e = e / d;
      e.print(" e : ");
      e.copy(d);*/
    }
    else
    {
      A1.copy(l1);
      A2.copy(l2);
    }
    if(i==27)
    {
      diff = B1-A1;
      diff.print(" diff : ");
    }
    i++;
    //err2 = err.abs();
    //err2.print("err2 : ");
    //prec.print("prec : ");
  //}while(!(err2<prec || A1 == B1));
  }while(i<28);
  //cout<<(err2.mantSize+err2.exponent) << "  "<<  prec.exponent<<"\n";
  //A1.print("A1 : ");
  err=C2-A1*A1;
  err.print("err : ");
  free(A2.mantissa);
  free(B2.mantissa);
  free(B1.mantissa);
  free(dl.mantissa);
  free(dr.mantissa);
  free(d.mantissa);
  free(l1.mantissa);
  free(mid.mantissa);
  free(err.mantissa);

  A1.forcePrecision(precision);
  A1.print("A1 : ");
  return A1;
}

csRNUMBER CSARITHMETIC_API CSARITHMETIC::csSqrt_Prec(csRNUMBER C2, int precision)
{
  //CSARITHMETIC_API CSARITHMETIC::setRNumberPrecision(C2.getDigitNumber()+10);
  /*long sz = 2*C2.mantSize;
  int prec = precision > sz ? precision : sz;*/
  int prec = precision + 2*C2.mantSize; 
  if(C2.equalZero())
  {
    csRNUMBER res("0");
    return res;
  }
  if(C2 == 1)
  {
    csRNUMBER res("1");
    return res;
  }
  long p = getRNumberPrecision();
  setRNumberPrecision(prec);
  csRNUMBER A1;
  csRNUMBER A2 = getlowsrnum(C2, A1);
  csRNUMBER B1;
  csRNUMBER B2 = gethighsrnum(C2,B1);
  csRNUMBER dl, dr, d, e, l1, l2, err;
  //int i=0;

  size_t zerosCountPos = 0, zerosCountPos0 = 0;
  
  d = B2 - A2;
  dl = (B2 - C2);
  dr = (C2 - A2);
  do
  {
    //zerosCountPos0 = zerosCountPos;
    l1 = (A1*dl + dr*B1)/d;
    l1.forcePrecision(prec);
    l2 = l1*l1;
    err = l2 - C2;
    if(err.sign == 0)
    {
      B1.copy(l1);
      d = l2 - A2;
      dl.copy(err);
      dr = (C2 - A2);
    }
    else
    {
      A1.copy(l1);
      A2.copy(l2);
    }
    //i++;
    
  }while((zerosCountPos = err.significantZerosCount(zerosCountPos0)) < precision);
  
  free(A2.mantissa);
  free(B2.mantissa);
  free(B1.mantissa);
  free(dl.mantissa);
  free(dr.mantissa);
  free(d.mantissa);
  free(l1.mantissa);
  free(err.mantissa);

  //A1.forcePrecision(precision);
  setRNumberPrecision(p);
  return A1;
}

csRNUMBER CSARITHMETIC_API CSARITHMETIC::csSqrt0_Prec(csRNUMBER _C2, csRNUMBER a, csRNUMBER b, int precision)
{
  //CSARITHMETIC_API CSARITHMETIC::setRNumberPrecision(C2.getDigitNumber()+10);
  csRNUMBER C2;
  C2.copy(_C2);
  long sz = 2*C2.mantSize;
  int prec = precision > sz ? precision : sz; 
  //int prec = 2*C2.mantSize + precision; 
  if(C2.equalZero())
  {
    csRNUMBER res("0");
    return res;
  }
  if(C2 == 1)
  {
    csRNUMBER res("1");
    return res;
  }
  
  long p = getRNumberPrecision();
  setRNumberPrecision(prec);
  csRNUMBER A1, B1, A2, B2;
  
  
  if(a == b) A1 = a/2;
  else A1.copy(a);
  B1.copy(b);
  A2 = A1*A1;
  B2 = B1*B1;

  if(A2 >= C2)
  {
    std::cout<<"overflow A\n";
    do
    {
      A1 = A1/2;
      A2 = A1*A1;
    }while (A2 >= C2);
    
  }
  if(B2 <= C2)
  {
    std::cout<<"overflow B\n";
    do
    {
      B1 = B1*2;
      B2 = B1*B1;
    }while (B2 <= C2);
  }

  /*A1.print("A1 : ");
  B1.print("B1 : ");
  A2.print("A2 : ");
  B2.print("B2 : ");
  C2.print("C2 : ");*/

  csRNUMBER dl, dr, d, e, l1, l2, err;
  int i=0;

  size_t zerosCountPos = 0, zerosCountPos0 = 0;
  
  d = B2 - A2;
  dl = (B2 - C2);
  dr = (C2 - A2);

  do
  {
    //zerosCountPos0 = zerosCountPos;
    l1 = (A1*dl + dr*B1)/d;
    l1.forcePrecision(prec);
    l2 = l1*l1;
    err = l2 - C2;
    
    if(err.sign == 0)
    {
      B1.copy(l1);
      d = l2 - A2;
      dl.copy(err);
      dr = (C2 - A2);
    }
    else
    {
      A1.copy(l1);
      A2.copy(l2);
    }
    i++;
    
    zerosCountPos = err.significantZerosCount(zerosCountPos0);
    
  }while(zerosCountPos < precision);
  
  setRNumberPrecision(p);

  free(C2.mantissa);
  free(A2.mantissa);
  free(B2.mantissa);
  free(B1.mantissa);
  free(dl.mantissa);
  free(dr.mantissa);
  free(d.mantissa);
  free(l1.mantissa);
  free(err.mantissa);

  return A1;
}

csRNUMBER CSARITHMETIC_API CSARITHMETIC::cs10PSqrt_Prec(long power, long precision)
{
  csRNUMBER rt10("10"), tenthSqrt;
  rt10 = csSqrt_Prec(rt10, precision);

  if(power%2 == 0)
      tenthSqrt.set("1",power/2, 0);
  else
  {
      if(power == 1)
          tenthSqrt.copy(rt10);
      else if(power > 1)
      {
          tenthSqrt.set("1",(power-1)/2, 0);
          tenthSqrt = tenthSqrt * rt10;
      }
  }
  rt10.clear();
  return tenthSqrt;
}

csRNUMBER CSARITHMETIC_API CSARITHMETIC::cs10PSqrt_Prec(csRNUMBER rt10, long power)
{
  csRNUMBER tenthSqrt;

  if(power%2 == 0)
      tenthSqrt.set("1",power/2, 0);
  else
  {
      if(power == 1)
          tenthSqrt.copy(rt10);
      else if(power > 1)
      {
          tenthSqrt.set("1",(power-1)/2, 0);
          tenthSqrt = tenthSqrt * rt10;
      }
  }
  
  return tenthSqrt;
}

csRNUMBER CSARITHMETIC_API CSARITHMETIC::BSqrtR_Prec(csRNUMBER C2, int precision)
{
  int prec = precision*2 + C2.mantSize;
  setRNumberPrecision(prec);
  csRNUMBER A1;
  csRNUMBER A2 = getlowsrnum(C2, A1); // Choose a near and lower number

  csRNUMBER B1;
  csRNUMBER B2 = gethighsrnum(C2,B1); // Choose a near and higher number

  csRNUMBER l1, l2, mid("5",-1), err;
  //int i=0;
  do
  {
    l1 = (A1+B1)*mid;
    l2 = l1*l1;
    err = l2 - C2;
    
    if(err.sign == 0)
    {
      B1.copy(l1);
    }
    else
    {
      A1.copy(l1);

    }
    //i++;
    
  }while(err.significantZerosCount() < precision);
  
  l1.clear();
  l2.clear();
  free(A2.mantissa);
  free(B2.mantissa);
  free(B1.mantissa);

  A1.forcePrecision(precision);
  return A1;
}

csRNUMBER CSARITHMETIC_API CSARITHMETIC::NRSqrt(csRNUMBER _C2)
{
  csRNUMBER C2(_C2.mantissa, _C2.exponent,_C2.sign);
  int precision = C2.getDigitNumber()+10;
  setRNumberPrecision(precision);
  csRNUMBER A1;
  csRNUMBER A2 = getlowsrnum(C2, A1); // Choose a near and lower number

  C2.print("C2 : ");
  csRNUMBER l1, l2, mid("5",-1), prec("1",-5), err, err2;
  int i=0;
  
  do
  {
    A1 = (A1+(C2/A1))*mid;
    A1.forcePrecision(precision);
    i++;
  }while(i<10);
  
  return A1;
}

csQNUMBER CSARITHMETIC_API CSARITHMETIC::NRSqrtQ(csQNUMBER n, long precision, size_t nIter)
{
  csRNUMBER rn;
  long oldprec = getRNumberPrecision();
  setRNumberPrecision(precision);
  rn.copy((csRNUMBER)n);
  
  csRNUMBER rns = NRSqrtR(rn, precision, nIter);
  csQNUMBER ret = csQNUMBER(rns);
  setRNumberPrecision(oldprec);

  free(rn.mantissa);
  free(rns.mantissa);
  return ret;
}

csRNUMBER CSARITHMETIC_API CSARITHMETIC::NRSqrtR(csRNUMBER _C2, long prec, size_t nIter)
{
  if(_C2.equalZero())
  {
    csRNUMBER rn("0");
    return rn;
  }
  else if(_C2 < 1)
  {
    csRNUMBER denom("1",prec,0);
    csRNUMBER num = _C2*denom;
    csRNUMBER r1 = NRSqrtR(num, prec);
    csRNUMBER r2 = NRSqrtR(denom, prec);
    
    csRNUMBER rn = r1/r2;
    denom.clear();
    num.clear();
    r1.clear();
    r2.clear();
    
    return rn;
  }

  csRNUMBER C2(_C2.mantissa, _C2.exponent,_C2.sign);
  int precision = C2.getDigitNumber()+10;
  setRNumberPrecision(precision);
  csRNUMBER A1;
  csRNUMBER A2 = getlowsrnum(C2, A1);

  csRNUMBER mid("5",-1);
  int i=0;
  
  for(int i=0; i<nIter; i++)
  {
    A1 = (A1+(C2/A1))*mid;
    A1.forcePrecision(precision);
  }

  A2.clear();
  C2.clear();
  mid.clear();
  A1.forcePrecision(prec);

  return A1;
}

csRNUMBER CSARITHMETIC_API CSARITHMETIC::NRSqrtR_Bissec(csRNUMBER _C2, long prec, size_t nIter)
{
  if(_C2.equalZero())
  {
    csRNUMBER rn("0");
    return rn;
  }
  else if(_C2 < 1)
  {
    csRNUMBER denom("1",prec,0);
    csRNUMBER num = _C2*denom;
    csRNUMBER r1 = NRSqrtR(num, prec);
    csRNUMBER r2 = NRSqrtR(denom, prec);
    
    csRNUMBER rn = r1/r2;
    denom.clear();
    num.clear();
    r1.clear();
    r2.clear();
    
    return rn;
  }
  csRNUMBER C2(_C2.mantissa, _C2.exponent,_C2.sign);
  int precision = C2.getDigitNumber()+10;
  setRNumberPrecision(precision);
  csRNUMBER A1;
  csRNUMBER A2 = gethighsrnum(C2, A1);

  csRNUMBER mid("5",-1), err_old, err1, err, A1_old, A;
  int i=0;
  do
  {
    
    A1 = (A1+(C2/A1))*mid;
    A1.forcePrecision(precision);
    err1 = A1*A1 - C2;

    if(err1.sign != err_old.sign)
    {
      
      do
      {
        A = (A1+A1_old)*mid;
        err = A*A - C2;
        if(err.sign == err1.sign)
        {
          A1.copy(A);
        }
        else
        {
          A1_old.copy(A);
        }
        
      } while (err.significantZerosCount(0) < prec);
      break;
    }
    A1_old.copy(A1);
    err_old.copy(err1);
    i++;
  }
  while(err.significantZerosCount(0) < prec);
  
  A2.clear();
  C2.clear();
  mid.clear();
  err.clear();
  err_old.clear();
  A1_old.clear();
  A1.forcePrecision(prec);
  return A1;
}

csRNUMBER CSARITHMETIC_API CSARITHMETIC::NRSqrtR0(csRNUMBER _C2, csRNUMBER x0, long prec, size_t nIter)
{
  if(_C2 == 0)
  {
    csRNUMBER rn("0");
    return rn;
  }
  else if(_C2 < 1)
  {
    csRNUMBER denom("1",prec,0);
    csRNUMBER num = _C2*denom;
    csRNUMBER r1 = NRSqrtR(num, prec);
    csRNUMBER r2 = NRSqrtR(denom, prec);
    
    csRNUMBER rn = r1/r2;
    denom.clear();
    num.clear();
    r1.clear();
    r2.clear();
    
    return rn;
  }

  csRNUMBER C2(_C2.mantissa, _C2.exponent,_C2.sign);
  int precision = C2.getDigitNumber()+10;
  setRNumberPrecision(precision);
  csRNUMBER A1;
  A1.copy(x0);

  csRNUMBER mid("5",-1);
  int i=0;
  do
  {
    A1 = (A1+(C2/A1))*mid;
    A1.forcePrecision(precision);

    i++;
  }
  while(i<nIter);
  C2.clear();
  mid.clear();
  A1.forcePrecision(prec);
  return A1;
}

csQNUMBER CSARITHMETIC_API CSARITHMETIC::nsr(csQNUMBER a)
{
  /*csQNUMBER a0("9999999999999999999999"
               "9999993999999999999999"
               "9999999999999991999999"
               "9999999999599999999999"
               "99995999999999999999999"
               "9999999999999999999999"
               "9999999999999999999999"), one("1"), two("2");
//  cout<<a0.denominator<<"ghghhj\n";
  csQNUMBER a = a0*a0;
//  cout<<"ghghhj\n";
  csQNUMBER mn1 = a0-two;
  csQNUMBER mn2 = mn1*mn1;

  csQNUMBER mx1 = a0+one;
  csQNUMBER mx2 = mx1*mx1;*/

  csQNUMBER mn1;
  csQNUMBER mn2 = getlowsrnum(a,mn1);
  //mn2.print("mn2");
  //csQNUMBER mn1("1","1",0);
  //csQNUMBER mn2("1","1",0);

  csQNUMBER mx1;
  csQNUMBER mx2 = gethightsrnum(a,mx1);

  //a.print("---a : ");
 // mx2.print("mx2 : ");
 // a.print("a : ");
 // dr2.print("dr2 : ");
  size_t reduction = a.numSize + 100;
  size_t mireduction = reduction/2;

  csQNUMBER dl2, dr2, d2, _mn1, _mx1, one("1");
  _mn1.copy(mn1);
  _mx1.copy(mx1);
  dl2 = a - mn2;
  dr2 = mx2 - a;
  d2 = mx2 - mn2;

  mn1 = ((_mx1*dl2) + dr2*_mn1)/d2;

  csQNUMBER nw = nextWeight(dl2,dr2);
  nw.print("nw : ");
  cout<<" --- "<<(dr2-nw)<< "  ----\n";
  mx1 = (_mx1*(dl2+nw) + (dr2-nw)*_mn1)/d2;
  /*if(dl2 < dr2)
  {
    mx1 = mx1/two;
    mx2 = mx1*mx1;
  }*/
  cout<<(mx1 > mn1)<<" compare mx, mn\n";
  mx2 = mx1*mx1;
  dr2 = mx2 - a;

  dr2.print("dr2 : ");

  mx2.reduce(reduction);
  dr2.reduce(reduction);
  mx1.reduce(mireduction);

  for(size_t i=0; i<10; i++)
  {
    mn2 = mn1*mn1;
    mn2.reduce(reduction);
    dl2 = a - mn2;
    d2 = mx2 - mn2;
    //if(i<4)
    d2.reduce(reduction);
    dl2.reduce(reduction);
    //mn1 = ((mx1*dl2) + dr2*mn1)/d2;
    mn1 = (csReduce(mx1*dl2,mireduction) + csReduce(dr2*mn1,mireduction))/d2;
    mn1.reduce(mireduction);

    if(mn2 > a)
    cout<<"hebnbbnnnnb\n";

  }
  cout<<(mn1*mn1 > a)<<" compare mx, mn\n";
  csQNUMBER two("2"), tmp, tmp2;
  for(size_t i=0; i<50; i++)
  {
    tmp = (mn1 + mx1)/two;
    tmp2 = (tmp*tmp);
    if(tmp2 > a)
    {
      mx1.copy(tmp);
    }
    else
    {
      mn1.copy(tmp);
    }
  }

  //mn1.print("mn1 : ");
  //mx1.clear();
  //mx2.clear();
  //mn2.clear();
  //tmp2.clear();
  //mn1.clear();
  //tmp2.clear();

  //return mn1;
  return tmp;
  //return mn1;
}

csQNUMBER CSARITHMETIC_API CSARITHMETIC::nsr2(csQNUMBER a)
{

  csQNUMBER mn("1"), two("2"), zero, tmp, tmp2;

  csQNUMBER mn1;
  csQNUMBER mn2 = getlowsrnum(a,mn1);

  csQNUMBER mx1;
  csQNUMBER mx2 = gethightsrnum(a,mx1);

  csQNUMBER mx, diff, prec("1","10");
  mx.copy(a);
  //mx.print("mx : ");
  size_t reduction = a.numSize + 20;
  for(size_t i=0; i<100; i++)
  {
    //cout<<"haaaaaa 1\n";
    tmp = (mn1 + mx1)/two;
    //cout<<"haaaaaa 2\n";
    //tmp.print("tmp = ");
    tmp2 = (tmp*tmp);
    //cout<<"haaaaaa 3\n";
    if(tmp2 > a)
    {
      mx1.copy(tmp);
    }
    else
    {
      mn1.copy(tmp);
    }
    //cout<<"haaaaaa 4\n";
  }
  //mn1.print("mn1 : ");
  /*tmp.set("147","1",0);
  tmp2 = (tmp*tmp);
  tmp2 = (tmp*tmp);
  tmp2.print("tmp2 : ");*/
  mx.clear();
  mn.clear();
  two.clear();
  mn1.clear();
  mn2.clear();
  mx1.clear();
  mx2.clear();

  return tmp2;
}

csRNUMBER CSARITHMETIC_API CSARITHMETIC::aplr(csRNUMBER _C2)
{
  setRNumberPrecision(_C2.getDigitNumber()+10);
  csRNUMBER C2;
  C2.copy(_C2);

  //C2.setPrecision(20);
  C2.print("C2 : ");

  csRNUMBER A1, A2 = getlowsrnum(C2,A1);
  csRNUMBER B1, B2 = gethighsrnum(C2,B1);
  csRNUMBER one("1"), two("2"), four("4");

  A2.print2("A2 : ");
  B2.print2("B2 : ");
  cout<< "  "<< (B2-C2) << "\n";
  cout<< "  "<< (B2-A2) << "\n";
  //C2.setPrecision(20);
  csRNUMBER a = (B2-C2)/(B2-A2), b = one-a;
  a.print("a : ");
  b.print("b : ");

  csRNUMBER _cf1 = (A1*a+b*B1), _cf0 = B1-A1, cf2 = a*(one-a)*_cf0*_cf0;
  csRNUMBER ek0 = C2;

  ek0.print("ek0 : ");

  csRNUMBER k0 = -_cf1/_cf0;
  k0.print("k0 : ");

  function<csRNUMBER(csRNUMBER, csRNUMBER, csRNUMBER, csRNUMBER)> func;

  csRNUMBER cf0 = _cf0*_cf0, cf1 = _cf1*_cf0;
  func = [](csRNUMBER k, csRNUMBER cf0, csRNUMBER cf1, csRNUMBER cf2)
  {
    static csRNUMBER two("2");
    return cf2 - k*(two*cf1 + k*cf0);
  };

  csRNUMBER ck("1",0), k1 = ck*k0;
  csRNUMBER ek1 = func(k1, cf0, cf1, cf2);
  k1.print("k1 : ");
  ek1.print("ek1 : ");

  function<csRNUMBER(csRNUMBER)> findSubLimit;
  findSubLimit = [one,cf0,cf1,cf2,func,ek0](csRNUMBER init)
  {
    csRNUMBER limit, y, inc("5",-2);
    limit.copy(init);
    int i=0;
    do
    {
      i++;
      limit = limit + inc;
      y = func(limit,cf0,cf1,cf2);
      //cout<<"i : "<<i;
      //y.print("y = ");

    }
    while(y.sign == ek0.sign);
    y.clear();
    inc.clear();
    return limit;
  };

  csRNUMBER limit = findSubLimit(k0);
  limit.print(" Limit : ");
  k0.print("k0 : ");

  function<csRNUMBER(csRNUMBER, csRNUMBER)> converge;
  converge = [two,cf0,cf1,cf2,_C2,a,b,func](csRNUMBER limit1, csRNUMBER limit2)
  {
    csRNUMBER half("5",-1), k, solution;
    size_t n = 4*(getRNumberPrecision()-10);
    for(size_t i=0; i<400; i++)
    {
      k = (limit1 + limit2)*half;
      //k = (limit1*a + limit2*b);
     // limit1.print("lim1 : ");
     cout<< "  "<<i;
    //  k.print("k : ");
     // limit2.print("lim2 : ");
      solution = func(k,cf0,cf1,cf2);
      if(solution.sign == 0)
      {
        limit1.copy(k);
      }
      else
      {
        limit2.copy(k);
      }
    }
    return solution;
  };

  function<csRNUMBER(csRNUMBER)> converge2;
  converge2 = [A1,B1,two](csRNUMBER C2)
  {
    //CSARITHMETIC_API CSARITHMETIC::setRNumberPrecision(10);
    csRNUMBER zero("0"), k,k2, err, limit1, limit2;
    size_t n = 4*(getRNumberPrecision()-10);
    //limit1.set("1",0,0);
    //limit2.copy(C2);
    limit1.copy(A1);
    limit2.copy(B1);
    for(size_t i=0; i<370; i++)
    {
      k = (limit1 + limit2)/two;
      //limit1.print2("lim1 : ");
      //k.print("k : ");
      //limit2.print2("lim2 : ");
      k2 = k*k;
      if(C2 > k2)
      {
        limit1.copy(k);
      }
      else
      {
        limit2.copy(k);
      }

    }
    return k;
  };

  csRNUMBER solution = converge(k0, limit);
  solution.print("solution : ");

  csRNUMBER nb("121");
  csRNUMBER solution2 = converge2(C2), err = C2-solution2*solution2;
  solution2.print("solution2 : ");
  err.print("err : ");
  //csRNUMBER cent("1000000");
  //cout<< "  "<< (cf2 - (C2 - _cf1*_cf1))<<"\n";

  return C2;
}


void CSARITHMETIC_API CSARITHMETIC::csSortMinR(csRNUMBER*& rn, size_t size)
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

void CSARITHMETIC_API CSARITHMETIC::csSortMin_CoordsByRNumber(csFCOORDS*& fc, csRNUMBER*& rn, size_t size)
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
  /*for(size_t j=0; j<size; j++)
  {
    cout<<rn[j]<< " ";
  }
  cout<<"\n";*/
  tmp.clear();
}
void CSARITHMETIC_API CSARITHMETIC::csSortMax_CoordsByRNumber(csFCOORDS*& fc, csRNUMBER*& rn, size_t size)
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

