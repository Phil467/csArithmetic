@@SIG@@csRNUMBER::set(const char* _mantissa, int _exponent, bool _sign)
@@BODY@@
  QLimbs n;
  rFromDigits(_mantissa, n);
  rSave(*this, n);
  exponent = _exponent;
  sign = n.isZero() ? CS_POSITIVE_NUMBER : _sign;
  precision = 0;
@@END@@
@@SIG@@csRNUMBER::set(unsigned long _mantissa, int _exponent, bool _sign)
@@BODY@@
  QLimbs n;
  n.setU64((uint64_t)_mantissa);
  rSave(*this, n);
  exponent = _exponent;
  sign = n.isZero() ? CS_POSITIVE_NUMBER : _sign;
  precision = 0;
@@END@@
@@SIG@@csRNUMBER::set(long _mantissa, int _exponent)
@@BODY@@
  uint64_t mag = _mantissa < 0 ? (uint64_t)(-_mantissa) : (uint64_t)_mantissa;
  QLimbs n;
  n.setU64(mag);
  rSave(*this, n);
  exponent = _exponent;
  sign = _mantissa < 0 ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  precision = 0;
@@END@@
@@SIG@@csRNUMBER::set(bool evaluate, const char* _number)
@@BODY@@
  (void)evaluate;
  if (!_number)
    _number = "0";
  size_t len = strlen(_number);
  size_t start = 0;
  bool neg = 0;
  if (len && _number[0] == '-') {
    neg = 1;
    start = 1;
  } else if (len && _number[0] == '+') {
    start = 1;
  }
  size_t dot = len;
  size_t expPos = len;
  for (size_t i = start; i < len; ++i) {
    if (_number[i] == '.') {
      dot = i;
      break;
    }
  }
  size_t frac = dot < len ? dot + 1 : len;
  for (size_t i = frac; i < len; ++i) {
    if (_number[i] == 'e' || _number[i] == 'E') {
      expPos = i;
      break;
    }
  }
  size_t whole = dot - start;
  size_t fracN = expPos > frac ? expPos - frac : 0;
  size_t nd = whole + fracN;
  if (nd == 0)
    nd = 1;
  char* digs = (char*)malloc(nd + 1);
  size_t w = 0;
  if (whole == 0 && fracN == 0)
    digs[w++] = '0';
  for (size_t i = 0; i < whole; ++i) {
    char c = _number[start + i];
    digs[w++] = (c >= '0' && c <= '9') ? c : '0';
  }
  for (size_t i = 0; i < fracN; ++i) {
    char c = _number[frac + i];
    digs[w++] = (c >= '0' && c <= '9') ? c : '0';
  }
  digs[w] = 0;
  int exp = -(int)fracN;
  if (expPos < len && expPos + 1 < len)
    exp = (int)strtol(_number + expPos + 1, 0, 10) - (int)fracN;
  QLimbs n;
  rFromDigits(digs, n);
  free(digs);
  rSave(*this, n);
  exponent = exp;
  sign = n.isZero() ? CS_POSITIVE_NUMBER : neg;
  precision = 0;
@@END@@
@@SIG@@csRNUMBER::set(size_t id, char digit)
@@BODY@@
  char* s = rExportDigits(*this);
  size_t n = strlen(s);
  char ch = (digit >= '0' && digit <= '9') ? digit : (char)('0' + (digit % 10));
  if (id >= n) {
    char* g = (char*)malloc(id + 2);
    memcpy(g, s, n);
    for (size_t i = n; i < id; ++i)
      g[i] = '0';
    g[id] = ch;
    g[id + 1] = 0;
    free(s);
    s = g;
  } else {
    s[id] = ch;
  }
  int exp = exponent;
  bool sg = sign;
  long pr = precision;
  QLimbs v;
  rFromDigits(s, v);
  free(s);
  rSave(*this, v);
  exponent = exp;
  sign = v.isZero() ? CS_POSITIVE_NUMBER : sg;
  precision = pr;
@@END@@
@@SIG@@csRNUMBER::setAsTenPower(long power)
@@BODY@@
  QLimbs n;
  n.setOne();
  rSave(*this, n);
  exponent = (int)power;
  sign = CS_POSITIVE_NUMBER;
  precision = 0;
@@END@@
@@SIG@@csRNUMBER::setPrecision(int size)
@@BODY@@
  if (exponent <= 0 && -exponent > size)
    return;
  int diff = exponent + size;
  if (diff > 0) {
    QLimbs n;
    rLoad(*this, n);
    rMulPow10(n, diff);
    rSave(*this, n);
  }
  exponent = -size;
@@END@@
@@SIG@@csRNUMBER::forcePrecision(int size)
@@BODY@@
  char* s = rExportDigits(*this);
  size_t n = strlen(s);
  if (exponent <= 0) {
    long diff = exponent + size;
    if (diff < (long)n && (diff < 0 ? -diff : diff) < (long)n) {
      if (diff > 0) {
        char* g = (char*)malloc(n + (size_t)diff + 1);
        memcpy(g, s, n);
        memset(g + n, '0', (size_t)diff);
        g[n + (size_t)diff] = 0;
        free(s);
        s = g;
      } else if (diff < 0) {
        size_t keep = n + (size_t)diff;
        if (keep == 0) {
          s[0] = '0';
          s[1] = 0;
        } else {
          if (keep < n && s[keep] >= '5') {
            long i = (long)keep - 1;
            while (i >= 0 && s[i] == '9') {
              s[i] = '0';
              --i;
            }
            if (i < 0) {
              char* g = (char*)malloc(keep + 2);
              g[0] = '1';
              memcpy(g + 1, s, keep);
              g[keep + 1] = 0;
              free(s);
              s = g;
              keep += 1;
            } else {
              s[i] += 1;
              s[keep] = 0;
            }
          } else {
            s[keep] = 0;
          }
        }
      }
      exponent = -size;
    } else if (diff > (long)n) {
      char* g = (char*)malloc(n + (size_t)diff + 1);
      memcpy(g, s, n);
      memset(g + n, '0', (size_t)diff);
      g[n + diff] = 0;
      free(s);
      s = g;
      exponent = -size;
    } else {
      free(s);
      s = (char*)malloc(2);
      s[0] = '0';
      s[1] = 0;
      exponent = 0;
    }
  } else {
    size_t add = (size_t)exponent + (size_t)size;
    char* g = (char*)malloc(n + add + 1);
    memcpy(g, s, n);
    memset(g + n, '0', add);
    g[n + add] = 0;
    free(s);
    s = g;
    exponent = -size;
  }
  bool sg = sign;
  long pr = precision;
  int exp = exponent;
  QLimbs v;
  rFromDigits(s, v);
  free(s);
  rSave(*this, v);
  exponent = exp;
  sign = v.isZero() ? CS_POSITIVE_NUMBER : sg;
  precision = pr;
@@END@@
@@SIG@@csRNUMBER::increaseShape(size_t size)
@@BODY@@
  if (!size)
    return;
  QLimbs n;
  rLoad(*this, n);
  rMulPow10(n, (int)size);
  rSave(*this, n);
  exponent -= (int)size;
@@END@@
@@SIG@@csRNUMBER::decreaseShape(size_t size)
@@BODY@@
  if (!size)
    return;
  QLimbs n;
  rLoad(*this, n);
  rDivPow10(n, (int)size);
  rSave(*this, n);
  exponent += (int)size;
  if (n.isZero())
    sign = CS_POSITIVE_NUMBER;
@@END@@
@@SIG@@csRNUMBER::reshape(size_t newMantissaSize)
@@BODY@@
  QLimbs n;
  rLoad(*this, n);
  int d = rDecDigits(n);
  if (newMantissaSize > (size_t)d)
    increaseShape(newMantissaSize - (size_t)d);
  else if (newMantissaSize < (size_t)d)
    decreaseShape((size_t)d - newMantissaSize);
@@END@@
@@SIG@@csRNUMBER::shapeOutZeros()
@@BODY@@
  QLimbs n;
  rLoad(*this, n);
  int k = rTrim10(n);
  rSave(*this, n);
  exponent += k;
@@END@@
@@SIG@@csRNUMBER::getDigitNumber()
@@BODY@@
  QLimbs n;
  rLoad(*this, n);
  int d = rDecDigits(n);
  if (exponent >= 0)
    return (size_t)d + (size_t)exponent;
  int ae = exponent < 0 ? -exponent : exponent;
  return (size_t)(d > ae ? d : ae);
@@END@@
@@SIG@@csRNUMBER::significantZerosCount(size_t initialPos)
@@BODY@@
  char* s = rExportDigits(*this);
  size_t n = strlen(s);
  long i = (long)n + exponent + (long)initialPos;
  size_t ans = 0;
  if (i < 0)
    ans = (size_t)(-i);
  else if (!(i == 1 && s[0] == '0'))
    ans = 0;
  else {
    for (; i < (long)n; ++i) {
      if (s[i] != '0') {
        ans = i == 0 ? 0 : (size_t)(i - 1);
        free(s);
        return ans;
      }
    }
    ans = (size_t)i;
  }
  free(s);
  return ans;
@@END@@
@@SIG@@csRNUMBER::abs()
@@BODY@@
  csRNUMBER rn;
  QLimbs n;
  rLoad(*this, n);
  rSave(rn, n);
  rn.exponent = exponent;
  rn.sign = CS_POSITIVE_NUMBER;
  rn.precision = precision;
  return rn;
@@END@@
@@SIG@@csRNUMBER::operator-() const
@@BODY@@
  csRNUMBER rn;
  QLimbs n;
  rLoad(*this, n);
  rSave(rn, n);
  rn.exponent = exponent;
  rn.sign = n.isZero() ? CS_POSITIVE_NUMBER : (sign ? CS_POSITIVE_NUMBER : CS_NEGATIVE_NUMBER);
  rn.precision = precision;
  return rn;
@@END@@
@@SIG@@csRNUMBER::operator+(csRNUMBER _a)
@@BODY@@
  csRNUMBER rn;
  QLimbs left, right, sum;
  rLoad(*this, left);
  rLoad(_a, right);
  int exp = exponent < _a.exponent ? exponent : _a.exponent;
  if (exponent > exp)
    rMulPow10(left, exponent - exp);
  if (_a.exponent > exp)
    rMulPow10(right, _a.exponent - exp);
  int outNeg = CS_POSITIVE_NUMBER;
  rCombine(left, sign ? 1 : 0, right, _a.sign ? 1 : 0, sum, outNeg);
  rSave(rn, sum);
  rn.sign = outNeg ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  rn.exponent = exp;
  rn.precision = _a.precision > precision ? _a.precision : precision;
  return rn;
@@END@@
@@SIG@@csRNUMBER::operator-(csRNUMBER _a)
@@BODY@@
  _a.sign = _a.sign ? CS_POSITIVE_NUMBER : CS_NEGATIVE_NUMBER;
  return *this + _a;
@@END@@
@@SIG@@csRNUMBER::operator*(csRNUMBER _a)
@@BODY@@
  csRNUMBER rn;
  QLimbs left, right, prod;
  rLoad(*this, left);
  rLoad(_a, right);
  qMul(left.p, left.n, right.p, right.n, prod);
  rSave(rn, prod);
  rn.exponent = exponent + _a.exponent;
  if (prod.isZero())
    rn.sign = CS_POSITIVE_NUMBER;
  else if (sign == CS_POSITIVE_NUMBER)
    rn.sign = _a.sign == CS_NEGATIVE_NUMBER ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  else
    rn.sign = _a.sign == CS_NEGATIVE_NUMBER ? CS_POSITIVE_NUMBER : CS_NEGATIVE_NUMBER;
  rn.precision = _a.precision > precision ? _a.precision : precision;
  rn.setPrecision(RPRECISION + (int)rn.precision);
  return rn;
@@END@@
@@SIG@@csRNUMBER::operator/(csRNUMBER _a)
@@BODY@@
  csRNUMBER rn;
  long precSize = RPRECISION + (_a.precision > precision ? _a.precision : precision);
  QLimbs left, right, quot, rem;
  rLoad(*this, left);
  rLoad(_a, right);
  long mag = (long)rDecDigits(right) + _a.exponent - ((long)rDecDigits(left) + exponent);
  if (mag > precSize) {
    rn.set("0", 0, 0);
    return rn;
  }
  int dropped = rTrim10(right);
  qDivMod(left.p, left.n, right.p, right.n, quot, rem);
  rSave(rn, quot);
  rn.exponent = exponent - (_a.exponent + dropped);
  if (quot.isZero())
    rn.sign = CS_POSITIVE_NUMBER;
  else if (sign == CS_POSITIVE_NUMBER)
    rn.sign = _a.sign == CS_NEGATIVE_NUMBER ? CS_NEGATIVE_NUMBER : CS_POSITIVE_NUMBER;
  else
    rn.sign = _a.sign == CS_NEGATIVE_NUMBER ? CS_POSITIVE_NUMBER : CS_NEGATIVE_NUMBER;
  rn.precision = _a.precision > precision ? _a.precision : precision;
  rn.setPrecision(RPRECISION + (int)rn.precision);
  return rn;
@@END@@
@@SIG@@csRNUMBER::copy(csRNUMBER a)
@@BODY@@
  if (mantissa == a.mantissa)
    return;
  QLimbs n;
  rLoad(a, n);
  rSave(*this, n);
  sign = a.sign;
  exponent = a.exponent;
  precision = a.precision > precision ? a.precision : precision;
@@END@@
@@SIG@@csRNUMBER::operator=(csRNUMBER a)
@@BODY@@
  if (mantissa != a.mantissa) {
    QLimbs n;
    rLoad(a, n);
    rSave(*this, n);
    sign = a.sign;
    exponent = a.exponent;
    precision = a.precision > precision ? a.precision : precision;
  }
  a.mantissa = 0;
@@END@@
@@SIG@@csRNUMBER::operator=(long a)
@@BODY@@
  int exp = exponent;
  long pr = precision;
  uint64_t mag = a < 0 ? (uint64_t)(-a) : (uint64_t)a;
  QLimbs n;
  n.setU64(mag);
  bool sg = a < 0 ? CS_NEGATIVE_NUMBER : (a ? CS_POSITIVE_NUMBER : sign);
  if (!a)
    sg = sign;
  rSave(*this, n);
  exponent = exp;
  precision = pr;
  sign = n.isZero() ? CS_POSITIVE_NUMBER : sg;
@@END@@
@@SIG@@csRNUMBER::equalZero()
@@BODY@@
  QLimbs n;
  rLoad(*this, n);
  return n.isZero();
@@END@@
@@SIG@@csRNUMBER::operator==(csRNUMBER _a)
@@BODY@@
  return rCmpValue(*this, _a) == 0;
@@END@@
@@SIG@@csRNUMBER::operator<(csRNUMBER _a)
@@BODY@@
  return rCmpValue(*this, _a) < 0;
@@END@@
@@SIG@@csRNUMBER::operator<=(csRNUMBER _a)
@@BODY@@
  return rCmpValue(*this, _a) <= 0;
@@END@@
@@SIG@@csRNUMBER::operator>(csRNUMBER _a)
@@BODY@@
  return rCmpValue(*this, _a) > 0;
@@END@@
@@SIG@@csRNUMBER::operator>=(csRNUMBER _a)
@@BODY@@
  return rCmpValue(*this, _a) >= 0;
@@END@@
@@SIG@@csRNUMBER::operator csQNUMBER()
@@BODY@@
  char* digs = rExportDigits(*this);
  size_t n = strlen(digs);
  csQNUMBER qn;
  if (exponent >= 0) {
    char* num = (char*)malloc(n + (size_t)exponent + 1);
    memcpy(num, digs, n);
    memset(num + n, '0', (size_t)exponent);
    num[n + (size_t)exponent] = 0;
    qn.set(num, "1", sign);
    free(num);
  } else {
    size_t z = (size_t)(-exponent);
    char* den = (char*)malloc(z + 2);
    den[0] = '1';
    memset(den + 1, '0', z);
    den[z + 1] = 0;
    qn.set(digs, den, sign);
    free(den);
  }
  free(digs);
  return qn;
@@END@@
@@SIG@@csRNUMBER::getMantissaSection(size_t first, size_t last)
@@BODY@@
  csRNUMBER rn;
  char* s = rExportDigits(*this);
  size_t n = strlen(s);
  if (first > n)
    first = n;
  if (last > n)
    last = n;
  if (last < first)
    last = first;
  size_t m = last - first;
  char* slice = (char*)malloc(m + 1);
  if (m)
    memcpy(slice, s + first, m);
  slice[m] = 0;
  if (m == 0) {
    slice[0] = '0';
    slice[1] = 0;
  }
  QLimbs v;
  rFromDigits(slice, v);
  free(slice);
  free(s);
  rSave(rn, v);
  rn.exponent = exponent;
  rn.sign = v.isZero() ? CS_POSITIVE_NUMBER : sign;
  rn.precision = precision;
  return rn;
@@END@@
@@SIG@@csRNUMBER::print2(const char*title)
@@BODY@@
  char* s = rExportDigits(*this);
  if (sign == CS_POSITIVE_NUMBER)
    cout << "\n " << title << s << "x10^" << exponent << "\n";
  else
    cout << "\n " << title << "-" << s << "x10^" << exponent << "\n";
  free(s);
@@END@@
@@SIG@@getPrintFormat(csRNUMBER a)
@@BODY@@
  char* digs = rExportDigits(a);
  size_t n = strlen(digs);
  bool neg = a.sign && !(n == 1 && digs[0] == '0');
  char* ret = 0;
  if (a.exponent >= 0) {
    size_t extra = (size_t)a.exponent;
    size_t len = n + extra + (neg ? 1 : 0);
    ret = (char*)malloc(len + 1);
    char* w = ret;
    if (neg)
      *w++ = '-';
    memcpy(w, digs, n);
    w += n;
    memset(w, '0', extra);
    w[extra] = 0;
  } else {
    size_t exp = (size_t)(-a.exponent);
    if (n <= exp) {
      size_t zeros = exp - n;
      size_t len = (neg ? 1 : 0) + 2 + exp;
      ret = (char*)malloc(len + 1);
      char* w = ret;
      if (neg)
        *w++ = '-';
      *w++ = '0';
      *w++ = '.';
      memset(w, '0', zeros);
      w += zeros;
      memcpy(w, digs, n);
      w[n] = 0;
    } else {
      size_t whole = n - exp;
      size_t len = (neg ? 1 : 0) + whole + 1 + exp;
      ret = (char*)malloc(len + 1);
      char* w = ret;
      if (neg)
        *w++ = '-';
      memcpy(w, digs, whole);
      w += whole;
      *w++ = '.';
      memcpy(w, digs + whole, exp);
      w[exp] = 0;
    }
  }
  free(digs);
  return ret;
@@END@@
