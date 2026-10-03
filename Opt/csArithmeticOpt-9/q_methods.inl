@@SIG@@csQNUMBER::csQNUMBER(const char* _numerator, const char* _denominator, bool _sign)
@@BODY@@
  qFromDec(_numerator, numerator, numSize);
  qFromDec(_denominator, denominator, denomSize);
  sign = _sign;
@@END@@
@@SIG@@csQNUMBER::csQNUMBER(size_t num, size_t denom, bool _sign)
@@BODY@@
  sign = _sign;
  QLimbs n, d;
  n.setU64((uint64_t)num);
  d.setU64((uint64_t)denom);
  numerator = n.leak();
  numSize = n.n;
  denominator = d.leak();
  denomSize = d.n;
@@END@@
@@SIG@@void csQNUMBER::set(const char* _numerator, const char* _denominator, bool _sign)
@@BODY@@
  if (numerator) {
    free(numerator);
    free(denominator);
  }
  qFromDec(_numerator, numerator, numSize);
  qFromDec(_denominator, denominator, denomSize);
  sign = _sign;
@@END@@
@@SIG@@void csQNUMBER::set(size_t num, size_t denom, bool _sign)
@@BODY@@
  if (numerator) {
    free(numerator);
    free(denominator);
  }
  sign = _sign;
  QLimbs n, d;
  n.setU64((uint64_t)num);
  d.setU64((uint64_t)denom);
  numerator = n.leak();
  numSize = n.n;
  denominator = d.leak();
  denomSize = d.n;
@@END@@
@@SIG@@void csQNUMBER::setl(long num, size_t denom)
@@BODY@@
  if (numerator) {
    free(numerator);
    free(denominator);
  }
  sign = num < 0 ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  QLimbs n, d;
  n.setU64(qAbsLong(num));
  d.setU64((uint64_t)denom);
  numerator = n.leak();
  numSize = n.n;
  denominator = d.leak();
  denomSize = d.n;
@@END@@
@@SIG@@void csQNUMBER::reduce(size_t sizeCondition)
@@BODY@@
  qReducePair(numerator, numSize, denominator, denomSize, sizeCondition);
@@END@@
@@SIG@@csQNUMBER csReduce(csQNUMBER a, size_t sizeCondition)
@@BODY@@
  char* n = qDupLimbs(a.numerator, a.numSize);
  char* d = qDupLimbs(a.denominator, a.denomSize);
  size_t ns = a.numSize ? a.numSize : 1;
  size_t ds = a.denomSize ? a.denomSize : 1;
  qReducePair(n, ns, d, ds, sizeCondition);
  a.numerator = n;
  a.denominator = d;
  a.numSize = ns;
  a.denomSize = ds;
  return a;
@@END@@
@@SIG@@bool csQNUMBER::operator==(csQNUMBER a)
@@BODY@@
  return qCmpSigned(*this, a) == 0;
@@END@@
@@SIG@@bool csQNUMBER::isAbsEqual(csQNUMBER a)
@@BODY@@
  return qCmpAbsQ(*this, a) == 0;
@@END@@
@@SIG@@bool csQNUMBER::operator>(csQNUMBER a)
@@BODY@@
  return qCmpSigned(*this, a) > 0;
@@END@@
@@SIG@@bool csQNUMBER::isAbsGreater(csQNUMBER a)
@@BODY@@
  return qCmpAbsQ(*this, a) > 0;
@@END@@
@@SIG@@bool csQNUMBER::operator>=(csQNUMBER a)
@@BODY@@
  return qCmpSigned(*this, a) >= 0;
@@END@@
@@SIG@@bool csQNUMBER::isAbsGreaterEqual(csQNUMBER a)
@@BODY@@
  return qCmpAbsQ(*this, a) >= 0;
@@END@@
@@SIG@@bool csQNUMBER::operator<(csQNUMBER a)
@@BODY@@
  return qCmpSigned(*this, a) < 0;
@@END@@
@@SIG@@bool csQNUMBER::isAbsLess(csQNUMBER a)
@@BODY@@
  return qCmpAbsQ(*this, a) < 0;
@@END@@
@@SIG@@bool csQNUMBER::operator<=(csQNUMBER a)
@@BODY@@
  return qCmpSigned(*this, a) <= 0;
@@END@@
@@SIG@@bool csQNUMBER::isAbsLessEqual(csQNUMBER a)
@@BODY@@
  return qCmpAbsQ(*this, a) <= 0;
@@END@@
@@SIG@@void csQNUMBER::print(const char*title)
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
@@SIG@@csQNUMBER csQNUMBER::operator+(csQNUMBER a)
@@BODY@@
  csQNUMBER qn(csQRaw{});
  QLimbs n1, n2, den, num;
  qMul(qL(numerator), numSize, qL(a.denominator), a.denomSize, n1);
  qMul(qL(denominator), denomSize, qL(a.numerator), a.numSize, n2);
  qMul(qL(denominator), denomSize, qL(a.denominator), a.denomSize, den);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (a.sign == CS_NEGATIVE_NUMBER)
      qSubAbs(n1.p, n1.n, n2.p, n2.n, num, neg);
    else
      qAdd(n1.p, n1.n, n2.p, n2.n, num);
  } else if (a.sign == CS_NEGATIVE_NUMBER) {
    qAdd(n1.p, n1.n, n2.p, n2.n, num);
    neg = true;
  } else {
    qSubAbs(n2.p, n2.n, n1.p, n1.n, num, neg);
  }
  if (num.isZero())
    neg = false;
  qNormalize(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
@@END@@
@@SIG@@csQNUMBER csQNUMBER::operator-(csQNUMBER a)
@@BODY@@
  csQNUMBER qn(csQRaw{});
  QLimbs n1, n2, den, num;
  qMul(qL(numerator), numSize, qL(a.denominator), a.denomSize, n1);
  qMul(qL(denominator), denomSize, qL(a.numerator), a.numSize, n2);
  qMul(qL(denominator), denomSize, qL(a.denominator), a.denomSize, den);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (a.sign == CS_NEGATIVE_NUMBER)
      qAdd(n1.p, n1.n, n2.p, n2.n, num);
    else
      qSubAbs(n1.p, n1.n, n2.p, n2.n, num, neg);
  } else if (a.sign == CS_NEGATIVE_NUMBER) {
    qSubAbs(n2.p, n2.n, n1.p, n1.n, num, neg);
  } else {
    qAdd(n1.p, n1.n, n2.p, n2.n, num);
    neg = true;
  }
  if (num.isZero())
    neg = false;
  qNormalize(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
@@END@@
@@SIG@@csQNUMBER csQNUMBER::operator*(csQNUMBER a)
@@BODY@@
  csQNUMBER qn(csQRaw{});
  QLimbs num, den;
  qMul(qL(numerator), numSize, qL(a.numerator), a.numSize, num);
  qMul(qL(denominator), denomSize, qL(a.denominator), a.denomSize, den);
  bool neg = !num.isZero() && (sign != a.sign);
  qNormalize(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
@@END@@
@@SIG@@csQNUMBER csQNUMBER::operator/(csQNUMBER a)
@@BODY@@
  csQNUMBER qn(csQRaw{});
  QLimbs num, den;
  qMul(qL(numerator), numSize, qL(a.denominator), a.denomSize, num);
  qMul(qL(denominator), denomSize, qL(a.numerator), a.numSize, den);
  bool neg = !num.isZero() && (sign != a.sign);
  qNormalize(num, den);
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
@@END@@
@@SIG@@csQNUMBER& csQNUMBER::operator=(csQNUMBER a)
@@BODY@@
  if (numerator) {
    free(numerator);
    free(denominator);
    numerator = 0;
  }
  sign = a.sign;
  if (a.numerator && a.numSize) {
    numerator = qDupLimbs(a.numerator, a.numSize);
    numSize = a.numSize;
  } else {
    numerator = qDupLimbs(0, 0);
    numSize = 1;
  }
  if (a.denominator && a.denomSize) {
    denominator = qDupLimbs(a.denominator, a.denomSize);
    denomSize = a.denomSize;
  } else {
    denominator = qLimbOne();
    denomSize = 1;
  }
  a.clear();
  return *this;
@@END@@
@@SIG@@csQNUMBER& csQNUMBER::operator=(long a)
@@BODY@@
  if (numerator) {
    free(numerator);
    free(denominator);
    numerator = 0;
  }
  sign = a < 0 ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  QLimbs n;
  n.setU64(qAbsLong(a));
  numerator = n.leak();
  numSize = n.n;
  denomSize = 1;
  denominator = qLimbOne();
  return *this;
@@END@@
@@SIG@@csQNUMBER csQNUMBER::operator+(long a)
@@BODY@@
  csQNUMBER qn(csQRaw{});
  bool asign = a < 0;
  QLimbs mag, prod, num;
  mag.setU64(qAbsLong(a));
  qMul(qL(denominator), denomSize, mag.p, mag.n, prod);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (asign)
      qSubAbs(qL(numerator), numSize, prod.p, prod.n, num, neg);
    else
      qAdd(qL(numerator), numSize, prod.p, prod.n, num);
  } else if (asign) {
    qAdd(qL(numerator), numSize, prod.p, prod.n, num);
    neg = true;
  } else {
    qSubAbs(prod.p, prod.n, qL(numerator), numSize, num, neg);
  }
  if (num.isZero())
    neg = false;
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = qDupLimbs(denominator, denomSize);
  qn.denomSize = denomSize;
  return qn;
@@END@@
@@SIG@@csQNUMBER csQNUMBER::operator-(long a)
@@BODY@@
  csQNUMBER qn(csQRaw{});
  bool asign = a < 0;
  QLimbs mag, prod, num;
  mag.setU64(qAbsLong(a));
  qMul(qL(denominator), denomSize, mag.p, mag.n, prod);
  bool neg = false;
  if (sign == CS_POSITIVE_NUMBER) {
    if (asign)
      qAdd(qL(numerator), numSize, prod.p, prod.n, num);
    else
      qSubAbs(qL(numerator), numSize, prod.p, prod.n, num, neg);
  } else if (asign) {
    qSubAbs(prod.p, prod.n, qL(numerator), numSize, num, neg);
  } else {
    qAdd(qL(numerator), numSize, prod.p, prod.n, num);
    neg = true;
  }
  if (num.isZero())
    neg = false;
  qn.sign = neg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = qDupLimbs(denominator, denomSize);
  qn.denomSize = denomSize;
  return qn;
@@END@@
@@SIG@@csQNUMBER csQNUMBER::operator*(long a)
@@BODY@@
  csQNUMBER qn(csQRaw{});
  QLimbs mag, num;
  mag.setU64(qAbsLong(a));
  qMul(qL(numerator), numSize, mag.p, mag.n, num);
  qn.sign = (!num.isZero() && (sign != (a < 0))) ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = num.leak();
  qn.numSize = num.n;
  qn.denominator = qDupLimbs(denominator, denomSize);
  qn.denomSize = denomSize;
  return qn;
@@END@@
@@SIG@@csQNUMBER csQNUMBER::operator/(long a)
@@BODY@@
  csQNUMBER qn(csQRaw{});
  QLimbs mag, den;
  mag.setU64(qAbsLong(a));
  qMul(qL(denominator), denomSize, mag.p, mag.n, den);
  qn.sign = (!qRawZero(numerator, numSize) && (sign != (a < 0))) ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  qn.numerator = qDupLimbs(numerator, numSize);
  qn.numSize = numSize;
  qn.denominator = den.leak();
  qn.denomSize = den.n;
  return qn;
@@END@@
@@SIG@@double csQNUMBER::getDouble()
@@BODY@@
  double d = qLimbsToDouble(denominator, denomSize);
  if (d == 0)
    return 0;
  double r = qLimbsToDouble(numerator, numSize) / d;
  return sign ? -r : r;
@@END@@
@@SIG@@char* csQNUMBER::quotient()
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
@@SIG@@csQNUMBER::operator csRNUMBER()
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
    csRNUMBER rn("0");
    free(result);
    free(a);
    free(ds);
    return rn;
  }
  removeFrontZeros(result, resSize);
  csRNUMBER rn(result, -RPRECISION, sign);
  free(result);
  free(a);
  free(ds);
  return rn;
@@END@@
@@SIG@@void csQNUMBER::copy(csQNUMBER a)
@@BODY@@
  if (numerator) {
    free(numerator);
    free(denominator);
    numerator = 0;
    denominator = 0;
  }
  sign = a.sign;
  if (a.numerator && a.numSize) {
    numerator = qDupLimbs(a.numerator, a.numSize);
    numSize = a.numSize;
  } else {
    numerator = qDupLimbs(0, 0);
    numSize = 1;
  }
  if (a.denominator && a.denomSize) {
    denominator = qDupLimbs(a.denominator, a.denomSize);
    denomSize = a.denomSize;
  } else {
    denominator = qLimbOne();
    denomSize = 1;
  }
@@END@@
@@SIG@@bool csQNUMBER::isZero()
@@BODY@@
  return qRawZero(numerator, numSize);
@@END@@
@@SIG@@csQNUMBER* CSARITHMETIC_API CSARITHMETIC::csQNUMBER_PTR_ALLOC(size_t nb, csQNUMBER init)
@@BODY@@
  csQNUMBER *qn = csAlloc<csQNUMBER>(nb);
  for (size_t i = 0; i < nb; i++) {
    qn[i].sign = init.sign;
    if (init.numerator && init.numSize) {
      qn[i].numerator = qDupLimbs(init.numerator, init.numSize);
      qn[i].numSize = init.numSize;
    } else {
      qn[i].numerator = qDupLimbs(0, 0);
      qn[i].numSize = 1;
    }
    if (init.denominator && init.denomSize) {
      qn[i].denominator = qDupLimbs(init.denominator, init.denomSize);
      qn[i].denomSize = init.denomSize;
    } else {
      qn[i].denominator = qLimbOne();
      qn[i].denomSize = 1;
    }
  }
  return qn;
@@END@@
