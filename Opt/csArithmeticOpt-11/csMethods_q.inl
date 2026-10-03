@@SIG@@csRational::csRational(const char* _numerator, const char* _denominator, bool _sign)
@@BODY@@
  csFromDec_q(_numerator, numerator, numSize);
  csFromDec_q(_denominator, denominator, denomSize);
  sign = _sign;
@@END@@
@@SIG@@csRational::csRational(size_t num, size_t denom, bool _sign)
@@BODY@@
  sign = _sign;
  csLimbs_q n, d;
  n.setU64((uint64_t)num);
  d.setU64((uint64_t)denom);
  numerator = n.leak();
  numSize = n.n;
  denominator = d.leak();
  denomSize = d.n;
@@END@@
@@SIG@@void csRational::set(const char* _numerator, const char* _denominator, bool _sign)
@@BODY@@
  if (numerator) {
    free(numerator);
    free(denominator);
  }
  csFromDec_q(_numerator, numerator, numSize);
  csFromDec_q(_denominator, denominator, denomSize);
  sign = _sign;
@@END@@
@@SIG@@void csRational::set(size_t num, size_t denom, bool _sign)
@@BODY@@
  if (numerator) {
    free(numerator);
    free(denominator);
  }
  sign = _sign;
  csLimbs_q n, d;
  n.setU64((uint64_t)num);
  d.setU64((uint64_t)denom);
  numerator = n.leak();
  numSize = n.n;
  denominator = d.leak();
  denomSize = d.n;
@@END@@
@@SIG@@void csRational::setl(long num, size_t denom)
@@BODY@@
  if (numerator) {
    free(numerator);
    free(denominator);
  }
  sign = num < 0 ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  csLimbs_q n, d;
  n.setU64(csAbsLong_q(num));
  d.setU64((uint64_t)denom);
  numerator = n.leak();
  numSize = n.n;
  denominator = d.leak();
  denomSize = d.n;
@@END@@
@@SIG@@void csRational::reduce(size_t sizeCondition)
@@BODY@@
  csReducePair_q(numerator, numSize, denominator, denomSize, sizeCondition);
@@END@@
@@SIG@@csRational csReduce(csRational a, size_t sizeCondition)
@@BODY@@
  char* n = csDupLimbs_q(a.numerator, a.numSize);
  char* d = csDupLimbs_q(a.denominator, a.denomSize);
  size_t ns = a.numSize ? a.numSize : 1;
  size_t ds = a.denomSize ? a.denomSize : 1;
  csReducePair_q(n, ns, d, ds, sizeCondition);
  a.numerator = n;
  a.denominator = d;
  a.numSize = ns;
  a.denomSize = ds;
  return a;
@@END@@
@@SIG@@bool csRational::operator==(csRational a)
@@BODY@@
  
  return csCmpSigned_q(*this, a) == 0;
@@END@@
@@SIG@@bool csRational::isAbsEqual(csRational a)
@@BODY@@
  
  return csCmpAbs_q(*this, a) == 0;
@@END@@
@@SIG@@bool csRational::operator>(csRational a)
@@BODY@@
  
  return csCmpSigned_q(*this, a) > 0;
@@END@@
@@SIG@@bool csRational::isAbsGreater(csRational a)
@@BODY@@
  
  return csCmpAbs_q(*this, a) > 0;
@@END@@
@@SIG@@bool csRational::operator>=(csRational a)
@@BODY@@
  
  return csCmpSigned_q(*this, a) >= 0;
@@END@@
@@SIG@@bool csRational::isAbsGreaterEqual(csRational a)
@@BODY@@
  
  return csCmpAbs_q(*this, a) >= 0;
@@END@@
@@SIG@@bool csRational::operator<(csRational a)
@@BODY@@
  
  return csCmpSigned_q(*this, a) < 0;
@@END@@
@@SIG@@bool csRational::isAbsLess(csRational a)
@@BODY@@
  
  return csCmpAbs_q(*this, a) < 0;
@@END@@
@@SIG@@bool csRational::operator<=(csRational a)
@@BODY@@
  
  return csCmpSigned_q(*this, a) <= 0;
@@END@@
@@SIG@@bool csRational::isAbsLessEqual(csRational a)
@@BODY@@
  
  return csCmpAbs_q(*this, a) <= 0;
@@END@@
@@SIG@@void csRational::print(const char*title)
@@BODY@@
  char* ns = csLimbsToDec(numerator, numSize);
  char* ds = csLimbsToDec(denominator, denomSize);
  if (sign == CS_POSITIVE_NUMBER)
    cout << "\n " << title << ns << " / " << ds << "\n";
  else
    cout << "\n " << title << "-" << ns << " / " << ds << "\n";
  free(ns);
  free(ds);
@@END@@
@@SIG@@csRational csRational::operator+(csRational a)
@@BODY@@

  csRational qn(csRaw_q{});
  csLimbs_q n1, n2, den, num;
  csMul_q(csPtr_q(numerator), numSize, csPtr_q(a.denominator), a.denomSize, n1);
  csMul_q(csPtr_q(denominator), denomSize, csPtr_q(a.numerator), a.numSize, n2);
  csMul_q(csPtr_q(denominator), denomSize, csPtr_q(a.denominator), a.denomSize, den);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (a.sign == CS_NEGATIVE_NUMBER)
      csSubAbs_q(n1.p, n1.n, n2.p, n2.n, num, neg);
    else
      csAdd_q(n1.p, n1.n, n2.p, n2.n, num);
  } else if (a.sign == CS_NEGATIVE_NUMBER) {
    csAdd_q(n1.p, n1.n, n2.p, n2.n, num);
    neg = true;
  } else {
    csSubAbs_q(n2.p, n2.n, n1.p, n1.n, num, neg);
  }
  if (num.isZero())
    neg = false;
  csNormalize_q(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
@@END@@
@@SIG@@csRational csRational::operator-(csRational a)
@@BODY@@

  csRational qn(csRaw_q{});
  csLimbs_q n1, n2, den, num;
  csMul_q(csPtr_q(numerator), numSize, csPtr_q(a.denominator), a.denomSize, n1);
  csMul_q(csPtr_q(denominator), denomSize, csPtr_q(a.numerator), a.numSize, n2);
  csMul_q(csPtr_q(denominator), denomSize, csPtr_q(a.denominator), a.denomSize, den);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (a.sign == CS_NEGATIVE_NUMBER)
      csAdd_q(n1.p, n1.n, n2.p, n2.n, num);
    else
      csSubAbs_q(n1.p, n1.n, n2.p, n2.n, num, neg);
  } else if (a.sign == CS_NEGATIVE_NUMBER) {
    csSubAbs_q(n2.p, n2.n, n1.p, n1.n, num, neg);
  } else {
    csAdd_q(n1.p, n1.n, n2.p, n2.n, num);
    neg = true;
  }
  if (num.isZero())
    neg = false;
  csNormalize_q(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
@@END@@
@@SIG@@csRational csRational::operator*(csRational a)
@@BODY@@

  csRational qn(csRaw_q{});
  csLimbs_q num, den;
  csMul_q(csPtr_q(numerator), numSize, csPtr_q(a.numerator), a.numSize, num);
  csMul_q(csPtr_q(denominator), denomSize, csPtr_q(a.denominator), a.denomSize, den);
  bool neg = !num.isZero() && (sign != a.sign);
  csNormalize_q(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
@@END@@
@@SIG@@csRational csRational::operator/(csRational a)
@@BODY@@

  csRational qn(csRaw_q{});
  csLimbs_q num, den;
  csMul_q(csPtr_q(numerator), numSize, csPtr_q(a.denominator), a.denomSize, num);
  csMul_q(csPtr_q(denominator), denomSize, csPtr_q(a.numerator), a.numSize, den);
  bool neg = !num.isZero() && (sign != a.sign);
  csNormalize_q(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
@@END@@
@@SIG@@csRational& csRational::operator=(csRational a)
@@BODY@@
  if (numerator) {
    free(numerator);
    free(denominator);
    numerator = 0;
  }
  sign = a.sign;
  if (a.numerator && a.numSize) {
    numerator = csDupLimbs_q(a.numerator, a.numSize);
    numSize = a.numSize;
  } else {
    numerator = csDupLimbs_q(0, 0);
    numSize = 1;
  }
  if (a.denominator && a.denomSize) {
    denominator = csDupLimbs_q(a.denominator, a.denomSize);
    denomSize = a.denomSize;
  } else {
    denominator = csLimbOne_q();
    denomSize = 1;
  }
  a.clear();
  return *this;
@@END@@
@@SIG@@csRational& csRational::operator=(long a)
@@BODY@@
  if (numerator) {
    free(numerator);
    free(denominator);
    numerator = 0;
  }
  sign = a < 0 ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  csLimbs_q n;
  n.setU64(csAbsLong_q(a));
  numerator = n.leak();
  numSize = n.n;
  denomSize = 1;
  denominator = csLimbOne_q();
  return *this;
@@END@@
@@SIG@@csRational csRational::operator+(long a)
@@BODY@@

  csRational qn(csRaw_q{});
  bool asign = a < 0;
  csLimbs_q mag, prod, num;
  mag.setU64(csAbsLong_q(a));
  csMul_q(csPtr_q(denominator), denomSize, mag.p, mag.n, prod);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (asign)
      csSubAbs_q(csPtr_q(numerator), numSize, prod.p, prod.n, num, neg);
    else
      csAdd_q(csPtr_q(numerator), numSize, prod.p, prod.n, num);
  } else if (asign) {
    csAdd_q(csPtr_q(numerator), numSize, prod.p, prod.n, num);
    neg = true;
  } else {
    csSubAbs_q(prod.p, prod.n, csPtr_q(numerator), numSize, num, neg);
  }
  if (num.isZero())
    neg = false;
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = csDupLimbs_q(denominator, denomSize);
  qn.denomSize = denomSize;
  return qn;
@@END@@
@@SIG@@csRational csRational::operator-(long a)
@@BODY@@

  csRational qn(csRaw_q{});
  bool asign = a < 0;
  csLimbs_q mag, prod, num;
  mag.setU64(csAbsLong_q(a));
  csMul_q(csPtr_q(denominator), denomSize, mag.p, mag.n, prod);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (asign)
      csAdd_q(csPtr_q(numerator), numSize, prod.p, prod.n, num);
    else
      csSubAbs_q(csPtr_q(numerator), numSize, prod.p, prod.n, num, neg);
  } else if (asign) {
    csSubAbs_q(prod.p, prod.n, csPtr_q(numerator), numSize, num, neg);
  } else {
    csAdd_q(csPtr_q(numerator), numSize, prod.p, prod.n, num);
    neg = true;
  }
  if (num.isZero())
    neg = false;
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = csDupLimbs_q(denominator, denomSize);
  qn.denomSize = denomSize;
  return qn;
@@END@@
@@SIG@@csRational csRational::operator*(long a)
@@BODY@@

  csRational qn(csRaw_q{});
  csLimbs_q mag, num;
  mag.setU64(csAbsLong_q(a));
  csMul_q(csPtr_q(numerator), numSize, mag.p, mag.n, num);
  qn.sign = (!num.isZero() && (sign != (a < 0))) ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = csDupLimbs_q(denominator, denomSize);
  qn.denomSize = denomSize;
  return qn;
@@END@@
@@SIG@@csRational csRational::operator/(long a)
@@BODY@@

  csRational qn(csRaw_q{});
  csLimbs_q mag, den;
  mag.setU64(csAbsLong_q(a));
  csMul_q(csPtr_q(denominator), denomSize, mag.p, mag.n, den);
  qn.sign = (!csRawZero_q(numerator, numSize) && (sign != (a < 0))) ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = csDupLimbs_q(numerator, numSize);
  qn.numSize = numSize;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
@@END@@
@@SIG@@double csRational::getDouble()
@@BODY@@
  double d = csLimbsToDouble_q(denominator, denomSize);
  if (d == 0)
    return 0;
  double r = csLimbsToDouble_q(numerator, numSize) / d;
  return sign ? -r : r;
@@END@@
@@SIG@@char* csRational::quotient()
@@BODY@@
  char* ns = csLimbsToDec(numerator, numSize);
  char* ds = csLimbsToDec(denominator, denomSize);
  size_t nlen = strlen(ns), dlen = strlen(ds);
  char* resInt = 0;
  char* resDec = 0;
  long precicion = getRNumberPrecision();
  size_t resz = 0, risz = 0, rdsz = 0;
  char const* stat = makeDivisionR2(ns, ds, resInt, resDec, nlen, dlen, resz, risz, rdsz, (size_t)precicion);
  if (stat) {
    free(resInt);
    free(resDec);
    free(ns);
    free(ds);
    
    return newString(stat);
  }
  removeFrontZeros(resInt, risz);
  size_t t = risz + rdsz + 1;
  char* str = csAlloc<char>(t + 2);
  if (sign)
    sprintf(str, "-%s.%s", resInt, resDec);
  else
    sprintf(str, "%s.%s", resInt, resDec);
  free(resInt);
  free(resDec);
  free(ns);
  free(ds);
  return str;
@@END@@
@@SIG@@csRational::operator csReal()
@@BODY@@
  char* ns = csLimbsToDec(numerator, numSize);
  char* ds = csLimbsToDec(denominator, denomSize);
  size_t nlen = strlen(ns);
  size_t dlen = strlen(ds);
  size_t aSize = nlen + RPRECISION;
  char* a = csAlloc<char>(aSize + 1);
  a[aSize] = '\0';
  for (size_t i = 0; i < nlen; i++)
    a[i] = ns[i];
  for (size_t i = nlen; i < aSize; i++)
    a[i] = '0';
  int rs = (int)nlen - (int)dlen + 1;
  size_t resSize = (rs > 1) ? (size_t)rs : 1;
  char* result = csAllocCharPtr(resSize, '0');
  result[resSize] = '\0';
  size_t bSize = dlen;
  const char* status = makeDivisionQ(a, ds, result, aSize, bSize, resSize);
  free(ns);
  if (status) {

    csReal rn("0");
    free(result);
    free(a);
    free(ds);
    return rn;
  }
  removeFrontZeros(result, resSize);

  csReal rn(result, -RPRECISION, sign);
  free(result);
  free(a);
  free(ds);
  return rn;
@@END@@
@@SIG@@void csRational::copy(csRational a)
@@BODY@@
  if (numerator) {
    free(numerator);
    free(denominator);
    numerator = 0;
    denominator = 0;
  }
  sign = a.sign;
  if (a.numerator && a.numSize) {
    numerator = csDupLimbs_q(a.numerator, a.numSize);
    numSize = a.numSize;
  } else {
    numerator = csDupLimbs_q(0, 0);
    numSize = 1;
  }
  if (a.denominator && a.denomSize) {
    denominator = csDupLimbs_q(a.denominator, a.denomSize);
    denomSize = a.denomSize;
  } else {
    denominator = csLimbOne_q();
    denomSize = 1;
  }
@@END@@
@@SIG@@bool csRational::isZero()
@@BODY@@
  
  return csRawZero_q(numerator, numSize);
@@END@@
@@SIG@@csRational* CSARITHMETIC_API CSARITHMETIC::csPtrAlloc_q(size_t nb, csRational init)
@@BODY@@
  csRational *qn = csAlloc<csRational>(nb);
  for (size_t i = 0; i < nb; i++) {
    qn[i].sign = init.sign;
    if (init.numerator && init.numSize) {
      qn[i].numerator = csDupLimbs_q(init.numerator, init.numSize);
      qn[i].numSize = init.numSize;
    } else {
      qn[i].numerator = csDupLimbs_q(0, 0);
      qn[i].numSize = 1;
    }
    if (init.denominator && init.denomSize) {
      qn[i].denominator = csDupLimbs_q(init.denominator, init.denomSize);
      qn[i].denomSize = init.denomSize;
    } else {
      qn[i].denominator = csLimbOne_q();
      qn[i].denomSize = 1;
    }
  }
  return qn;
@@END@@
