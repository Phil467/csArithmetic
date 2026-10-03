# csArithmeticOpt — API

**Edition 13 — Copyright Philippe Levang Azeufack**

Reference for the published edition of `csArithmeticOpt`. The interface is the expression: values and operators, not a list of buffers.

```cpp
#define CSARITHMETIC_STATIC
#include "csArithmetic.h"
using namespace CSARITHMETIC;

csInteger b("24"), c("7"), d("5"), e("3"), f("1");
csInteger a = (b * c + d) / e - f;
```

The same parentheses work for `csRational`, `csReal` and `csComplex`. In-place updates use `+=`, `-=`, `*=`, `/=`.

## Types

| Type | Role |
|---|---|
| `csInteger` | Exact integer. One machine word, then 32 limbs inside the object, then the heap. |
| `csRational` | Exact ratio of two integers in limbs. |
| `csReal` | Decimal mantissa and exponent. Precision is a property of the value and of `setRealPrecision`. |
| `csComplex` | `a + bi` with two `csReal` parts (`real`, `imag`). |
| `csComplex_q` | `a + bi` with two `csRational` parts. Sums, products and integer powers stay exact. |

Division by zero yields zero. For a complex value the sign is read on each part (`negativeSign`, `isNegative`), not on the complex number itself. `abs()` on a complex value is the modulus. `0^0` is 1.

Decimal texts (`addDecimal`, `subtractDecimal`, `multiplyDecimal`, `divideDecimal`, …) are the digit-string layer under the values. Prefer the types above when the calculation is a formula.

Old names from earlier editions remain as `[[deprecated]]` wrappers.

## Reference

Each entry is the declaration in `csArithmeticOpt13.h` and the documentation comment that introduces it.


## csInteger

### `csInteger absolute() const`

```cpp
csInteger absolute() const;
```

@brief Returns the absolute value.
@return Non-negative integer.

---

### `csInteger quotientAndRemainder(const csInteger& divisor, csInteger& remainder) const`

```cpp
csInteger quotientAndRemainder(const csInteger& divisor, csInteger& remainder) const;
```

@brief Quotient and remainder of Euclidean division truncated toward zero.
@param divisor Divisor.
@param remainder Receives the remainder, with the sign of the dividend.
@return Quotient.

---

### `csInteger power(size_t exponent) const`

```cpp
csInteger power(size_t exponent) const;
```

@brief Raises the integer to a non-negative integer power.
@param exponent Exponent.
@return Resulting integer. Zero to the power zero is one.

---

## csRational

### `csRational toRational() const`

```cpp
csRational toRational() const;
```

@brief Converts the integer to a rational with denominator one.
@return Resulting rational.

---

### `csRational* csPtrAlloc_q(size_t nb)`

```cpp
csRational* csPtrAlloc_q(size_t nb);
```

@brief Allocates an array of rationals.
@param nb Number of elements.
@return Pointer to the resulting rational or array.

---

### `csRational* csPtrAlloc_q(size_t nb, csRational init)`

```cpp
csRational* csPtrAlloc_q(size_t nb, csRational init);
```

@brief Allocates an array of rationals.
@param nb Number of elements.
@param init Initial fill character.
@return Pointer to the resulting rational or array.

---

### `csRational norm()`

```cpp
csRational norm();
```

@brief Returns the squared modulus, a² + b², as an exact rational.

---

### `csRational pow(csRational a, size_t p)`

```cpp
csRational pow(csRational a, size_t p);
```

@brief Raises a rational to an integer power.
@param a First operand.
@param p Power or degree.
@return Resulting rational.

---

## csReal

### `csReal* csPtrAlloc_r(size_t nb)`

```cpp
csReal* csPtrAlloc_r(size_t nb);
```

@brief Allocates an array of reals.
@param nb Number of elements.
@return Pointer to the resulting real or array.

---

### `csReal* csPtrAlloc_r(size_t nb, csReal init)`

```cpp
csReal* csPtrAlloc_r(size_t nb, csReal init);
```

@brief Allocates an array of reals.
@param nb Number of elements.
@param init Initial fill character.
@return Pointer to the resulting real or array.

---

### `csReal(const char* mantissa, int exponent, bool sign, int repeat)`

```cpp
csReal(const char* mantissa, int exponent, bool sign, int repeat);
```

@brief Builds a real by repeating the mantissa.
@param mantissa Decimal text of the mantissa.
@param exponent Decimal exponent.
@param sign Sign. Zero when the number is positive.
@param repeat Number of times the mantissa is repeated. A value below 1 yields zero.

---

### `csReal(const char* mantissa, int exponent, bool sign, int repeat, int pos)`

```cpp
csReal(const char* mantissa, int exponent, bool sign, int repeat, int pos);
```

@brief Builds a real by repeating a section of the mantissa.
@param mantissa Decimal text of the mantissa.
@param exponent Decimal exponent.
@param sign Sign. Zero when the number is positive.
@param repeat Number of times the section is repeated. A value below 1 yields zero.
@param pos Repeated section. Positive: from the first character through the pos-th, then the rest. Negative: from the pos-th character from the end through the last character, placed at the end.

---

### `csReal(const char* mantissa, int exponent, bool sign, int repeat, int length, int index)`

```cpp
csReal(const char* mantissa, int exponent, bool sign, int repeat, int length, int index);
```

@brief Builds a real by repeating a slice of the mantissa.
@param mantissa Decimal text of the mantissa.
@param exponent Decimal exponent.
@param sign Sign. Zero when the number is positive.
@param repeat Number of times the slice is repeated. A value below 1 yields zero.
@param length Length of the slice, in characters.
@param index Start index of the slice, from zero.

---

### `csReal(double value)`

```cpp
csReal(double value);
```

@brief Builds a real from a double.
@param value Value to convert to decimal.

---

### `csReal abs()`

```cpp
csReal abs();
```

@brief Returns the absolute value.
@return Resulting real.

---

### `csReal operator*(double a)`

```cpp
csReal operator*(double a);
```

@brief Multiplies by a double, including values between 0 and 1.
@param a Decimal factor.
@return Resulting real.

---

### `csReal integerQuotient(long n)`

```cpp
csReal integerQuotient(long n);
```

@brief Returns the integer quotient of division by @p n, truncated toward zero.
@param n Integer divisor.
@return Integer quotient, with a zero exponent.

---

### `csReal integer()`

```cpp
csReal integer();
```

@brief Returns the integer part, truncated toward zero.
@return Integer part, with a zero exponent.

---

### `csReal mantissaSection(size_t first, size_t last)`

```cpp
csReal mantissaSection(size_t first, size_t last);
```

@brief Returns a section of the mantissa.
@param first First index, inclusive.
@param last Last index, exclusive.
@return Resulting real.

---

### `csReal norm()`

```cpp
csReal norm();
```

@brief Returns the squared modulus, a² + b².

---

### `csReal abs()`

```cpp
csReal abs();
```

@brief Returns the modulus.

---

### `csReal abs()`

```cpp
csReal abs();
```

@brief Returns the modulus as a real.

---

### `csReal* csSortMinR(csReal* rn, size_t size)`

```cpp
csReal* csSortMinR(csReal* rn, size_t size);
```

@brief Sorts reals into increasing order.
@param rn Array of reals.
@param size Requested size.
@return Pointer to the resulting real or array.

---

## csComplex

### `csComplex conjugate()`

```cpp
csComplex conjugate();
```

@brief Returns the conjugate a - bi.

---

### `csComplex power(long exponent)`

```cpp
csComplex power(long exponent);
```

@brief Integer power. 0^0 is 1. A negative exponent takes the reciprocal.

---

### `csComplex reciprocal()`

```cpp
csComplex reciprocal();
```

@brief Returns the reciprocal.

---

## csComplex_q

### `csComplex_q conjugate()`

```cpp
csComplex_q conjugate();
```

@brief Returns the conjugate a - bi.

---

### `csComplex_q power(long exponent)`

```cpp
csComplex_q power(long exponent);
```

@brief Integer power. 0^0 is 1. A negative exponent takes the reciprocal.

---

### `csComplex_q reciprocal()`

```cpp
csComplex_q reciprocal();
```

@brief Returns the reciprocal.

---

## Functions

### `template<class T> CS_FORCE_INLINE T* csAlloc(size_t n)`

```cpp
template<class T> CS_FORCE_INLINE T* csAlloc(size_t n);
```

@brief Allocates a memory block.
@param n Number of elements.
@return Result.

---

### `template<class T> CS_FORCE_INLINE T* csAlloc(size_t n, T init)`

```cpp
template<class T> CS_FORCE_INLINE T* csAlloc(size_t n, T init);
```

@brief Allocates a memory block.
@param n Number of elements.
@param init Initial fill character.
@return Result.

---

### `template<class T> CS_FORCE_INLINE T* csAlloc2(size_t n, T init)`

```cpp
template<class T> CS_FORCE_INLINE T* csAlloc2(size_t n, T init);
```

@brief Allocates a two-dimensional memory block.
@param n Number of elements.
@param init Initial fill character.
@return Result.

---

### `char* CSARITHMETIC_API csAllocCharPtr(size_t n, size_t n1, char init)`

```cpp
char* CSARITHMETIC_API csAllocCharPtr(size_t n, size_t n1, char init);
```

@brief Allocates a character array.
@param n Number of elements.
@param n1 Size of the second dimension.
@param init Initial fill character.
@return Resulting text. The caller frees the memory.

---

### `char* CSARITHMETIC_API csAllocCharPtr(size_t n, char init)`

```cpp
char* CSARITHMETIC_API csAllocCharPtr(size_t n, char init);
```

@brief Allocates a character array.
@param n Number of elements.
@param init Initial fill character.
@return Resulting text. The caller frees the memory.

---

### `void CSARITHMETIC_API csReallocString(char** str, size_t size)`

```cpp
void CSARITHMETIC_API csReallocString(char** str, size_t size);
```

@brief Reallocates a character string.
@param str Text to modify.
@param size Requested size.
@return Result.

---

### `void CSARITHMETIC_API csReallocString(char** str, size_t size, size_t newSize, char cFill)`

```cpp
void CSARITHMETIC_API csReallocString(char** str, size_t size, size_t newSize, char cFill);
```

@brief Reallocates a character string.
@param str Text to modify.
@param size Requested size.
@param newSize New size.
@param cFill Fill character.
@return Result.

---

### `char* CSARITHMETIC_API newString(char*cstr, size_t size)`

```cpp
char* CSARITHMETIC_API newString(char*cstr, size_t size);
```

@brief Copies a text into a new buffer.
@param cstr Source text.
@param size Requested size.
@return Resulting text. The caller frees the memory.

---

### `char* CSARITHMETIC_API newString(const char*cstr, size_t size)`

```cpp
char* CSARITHMETIC_API newString(const char*cstr, size_t size);
```

@brief Copies a text into a new buffer.
@param cstr Source text.
@param size Requested size.
@return Resulting text. The caller frees the memory.

---

### `char* CSARITHMETIC_API newString(const char*cstr, size_t begin, size_t end)`

```cpp
char* CSARITHMETIC_API newString(const char*cstr, size_t begin, size_t end);
```

@brief Copies a text into a new buffer.
@param cstr Source text.
@param begin Start index, inclusive.
@param end End index, exclusive.
@return Resulting text. The caller frees the memory.

---

### `char* CSARITHMETIC_API newString(const char*cstr)`

```cpp
char* CSARITHMETIC_API newString(const char*cstr);
```

@brief Copies a text into a new buffer.
@param cstr Source text.
@return Resulting text. The caller frees the memory.

---

### `char* intToString(int nb, size_t& sz)`

```cpp
char* intToString(int nb, size_t& sz);
```

@brief Converts a signed integer to text.
@param nb Number of elements.
@param sz Receives the size of the produced text.
@return Resulting text. The caller frees the memory.

---

### `char* uLongToString(size_t nb, size_t& sz)`

```cpp
char* uLongToString(size_t nb, size_t& sz);
```

@brief Converts an unsigned integer to text.
@param nb Number of elements.
@param sz Receives the size of the produced text.
@return Resulting text. The caller frees the memory.

---

### `void shiftRight(char*& nb, size_t size, size_t nShift)`

```cpp
void shiftRight(char*& nb, size_t size, size_t nShift);
```

@brief Shifts the digits of a text to the right.
@param nb Number of elements.
@param size Requested size.
@param nShift Number of shift positions.

---

### `char* shiftRightCopy(char* nb, size_t nbSize, size_t nbCopySize)`

```cpp
char* shiftRightCopy(char* nb, size_t nbSize, size_t nbCopySize);
```

@brief Returns a copy of the text shifted to the right.
@param nb Number of elements.
@param nbSize Size of the number.
@param nbCopySize Size of the copy.
@return Resulting text. The caller frees the memory.

---

### `void shiftLeft(char*& nb, size_t size, size_t nShift)`

```cpp
void shiftLeft(char*& nb, size_t size, size_t nShift);
```

@brief Shifts the digits of a text to the left.
@param nb Number of elements.
@param size Requested size.
@param nShift Number of shift positions.

---

### `void shiftLeftCopy(char* nb, char*&nbCopy, size_t nbSize, size_t nbCopyPos)`

```cpp
void shiftLeftCopy(char* nb, char*&nbCopy, size_t nbSize, size_t nbCopyPos);
```

@brief Copies a text while shifting it to the left.
@param nb Number of elements.
@param nbCopy Produced copy.
@param nbSize Size of the number.
@param nbCopyPos Write position in the copy.

---

### `void skipZeros(char* a, size_t aSize, size_t& skipLen)`

```cpp
void skipZeros(char* a, size_t aSize, size_t& skipLen);
```

@brief Skips the leading zeros of a text.
@param a First operand.
@param aSize Number of digits of the first text.
@param skipLen Number of skipped zeros.

---

### `void skipZeros2(char* a, size_t aSize, size_t& skipLen, size_t&incr)`

```cpp
void skipZeros2(char* a, size_t aSize, size_t& skipLen, size_t&incr);
```

@brief Skips the leading zeros and measures the step.
@param a First operand.
@param aSize Number of digits of the first text.
@param skipLen Number of skipped zeros.
@param incr Step associated with the skipped zeros.

---

### `bool isaGreater(char* a,char* b,size_t opSize, size_t ibegin)`

```cpp
bool isaGreater(char* a,char* b,size_t opSize, size_t ibegin);
```

@brief Reports whether the first text is greater than the second.
@param a First operand.
@param b Second operand.
@param opSize Number of aligned digits.
@param ibegin Index of the first digit compared.
@return True when the condition holds.

---

### `bool isaGreaterEqual(char* a,char* b,size_t opSize, size_t ibegin)`

```cpp
bool isaGreaterEqual(char* a,char* b,size_t opSize, size_t ibegin);
```

@brief Reports whether the first text is greater than or equal to the second.
@param a First operand.
@param b Second operand.
@param opSize Number of aligned digits.
@param ibegin Index of the first digit compared.
@return True when the condition holds.

---

### `bool isaGreater2(char* a,char* b,size_t aSize,size_t bSize, size_t ibegin)`

```cpp
bool isaGreater2(char* a,char* b,size_t aSize,size_t bSize, size_t ibegin);
```

@brief Reports whether the first text is greater than the second, at different sizes.
@param a First operand.
@param b Second operand.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param ibegin Index of the first digit compared.
@return True when the condition holds.

---

### `bool isaGreaterEqual2(char* a,char* b,size_t aSize, size_t bSize, size_t ibegin)`

```cpp
bool isaGreaterEqual2(char* a,char* b,size_t aSize, size_t bSize, size_t ibegin);
```

@brief Reports whether the first text is greater than or equal to the second, at different sizes.
@param a First operand.
@param b Second operand.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param ibegin Index of the first digit compared.
@return True when the condition holds.

---

### `int getReadySub(char*& a,char*& b, size_t&opSize)`

```cpp
int getReadySub(char*& a,char*& b, size_t&opSize);
```

@brief Prepares two texts for a subtraction.
@param a First operand.
@param b Second operand.
@param opSize Number of aligned digits.
@return Computed value.

---

### `bool getReadySub2(const char* a0,const char* b0, char*& a, char*& b, size_t& opSize)`

```cpp
bool getReadySub2(const char* a0,const char* b0, char*& a, char*& b, size_t& opSize);
```

@brief Prepares two source texts for a subtraction.
@param a0 Original text of the first operand.
@param b0 Original text of the second operand.
@param a First operand.
@param b Second operand.
@param opSize Number of aligned digits.
@return True when the condition holds.

---

### `bool getReadySub3(const char* a0,const char* b0, char*& a, char*& b, size_t aSize, size_t bSize, size_t&opSize)`

```cpp
bool getReadySub3(const char* a0,const char* b0, char*& a, char*& b, size_t aSize, size_t bSize, size_t&opSize);
```

@brief Prepares two texts of different sizes for a subtraction.
@param a0 Original text of the first operand.
@param b0 Original text of the second operand.
@param a First operand.
@param b Second operand.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param opSize Number of aligned digits.
@return True when the condition holds.

---

### `void getReady(char*& a,char*& b, size_t& opSize)`

```cpp
void getReady(char*& a,char*& b, size_t& opSize);
```

@brief Aligns two texts before an operation.
@param a First operand.
@param b Second operand.
@param opSize Number of aligned digits.

---

### `void getReady2(const char* a0,const char* b0, char*& a, char*& b, size_t& opSize)`

```cpp
void getReady2(const char* a0,const char* b0, char*& a, char*& b, size_t& opSize);
```

@brief Aligns two source texts before an operation.
@param a0 Original text of the first operand.
@param b0 Original text of the second operand.
@param a First operand.
@param b Second operand.
@param opSize Number of aligned digits.

---

### `bool removeFrontZeros(char*& a, size_t& size)`

```cpp
bool removeFrontZeros(char*& a, size_t& size);
```

@brief Removes leading zeros from a text.
@param a First operand.
@param size Requested size.
@return True when the condition holds.

---

### `void removeLeft(char*& nb, size_t& size, size_t remLen)`

```cpp
void removeLeft(char*& nb, size_t& size, size_t remLen);
```

@brief Removes characters from the left of a text.
@param nb Number of elements.
@param size Requested size.
@param remLen Number of characters removed.

---

### `void removeRight(char*& nb, size_t& size, size_t remLen)`

```cpp
void removeRight(char*& nb, size_t& size, size_t remLen);
```

@brief Removes characters from the right of a text.
@param nb Number of elements.
@param size Requested size.
@param remLen Number of characters removed.

---

### `void addFrontZeros(char*& a, size_t& size, size_t nZeros)`

```cpp
void addFrontZeros(char*& a, size_t& size, size_t nZeros);
```

@brief Adds leading zeros to a text.
@param a First operand.
@param size Requested size.
@param nZeros Number of zeros added.

---

### `void fillString(char*& str, const char* cstr, size_t size)`

```cpp
void fillString(char*& str, const char* cstr, size_t size);
```

@brief Copies a text into an already allocated buffer.
@param str Text to modify.
@param cstr Source text.
@param size Requested size.

---

### `char* filledString(const char* cstr, size_t size)`

```cpp
char* filledString(const char* cstr, size_t size);
```

@brief Returns a text padded to the requested size.
@param cstr Source text.
@param size Requested size.
@return Resulting text. The caller frees the memory.

---

### `CS_INLINE static char* csLimbsToDec(const void* raw, size_t n)`

```cpp
CS_INLINE static char* csLimbsToDec(const void* raw, size_t n);
```

@brief Converts limbs to decimal text.
@param raw Parameter @p raw.
@param n Number of elements.
@return Resulting text. The caller frees the memory.

---

### `char* formatReal(const csReal& a)`

```cpp
char* formatReal(const csReal& a);
```

@brief Prepares the display text of a real.
@param a First operand.
@return Resulting text. The caller frees the memory.

---

### `void setRealPrecision(int precision)`

```cpp
void setRealPrecision(int precision);
```

@brief Sets the precision used for reals.
@param precision Requested precision, in digits.

---

### `int realPrecision()`

```cpp
int realPrecision();
```

@brief Returns the precision used for reals.
@return Computed value.

---

### `template<class T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type = 0> csRational(T num, size_t denom=1) : csRational(csRaw_q{})`

```cpp
template<class T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type = 0> csRational(T num, size_t denom=1) : csRational(csRaw_q{});
```

@brief Builds a rational from a signed integer.
@param num Signed integer of the numerator.
@param denom Positive denominator.

---

### `void init()`

```cpp
void init();
```

@brief Initializes the object or the library tables.

---

### `void set(const char* numerator, const char* denominator, bool sign)`

```cpp
void set(const char* numerator, const char* denominator, bool sign);
```

@brief Assigns a new value.
@param numerator Decimal text of the numerator.
@param denominator Decimal text of the denominator.
@param sign Sign. Zero when the number is positive.

---

### `void set(size_t num, size_t denom=1, bool sign=0)`

```cpp
void set(size_t num, size_t denom=1, bool sign=0);
```

@brief Assigns a new value.
@param num Parameter @p num.
@param denom Parameter @p denom.
@param sign Sign. Zero when the number is positive.

---

### `void assignValue(long num, size_t denom=1)`

```cpp
void assignValue(long num, size_t denom=1);
```

@brief Assigns a long integer.
@param num Parameter @p num.
@param denom Parameter @p denom.

---

### `void reduce(size_t precision)`

```cpp
void reduce(size_t precision);
```

@brief Reduces the value to the requested size.
@param precision Requested precision, in digits.

---

### `bool equalAbsolute(const csRational& a)`

```cpp
bool equalAbsolute(const csRational& a);
```

@brief Reports whether the absolute values are equal.
@param a First operand.
@return True when the condition holds.

---

### `bool differAbsolute(const csRational& a)`

```cpp
bool differAbsolute(const csRational& a);
```

@brief Reports whether the absolute values differ.
@param a First operand.
@return True when the condition holds.

---

### `bool greaterAbsolute(const csRational& a)`

```cpp
bool greaterAbsolute(const csRational& a);
```

@brief Reports whether the absolute value is greater.
@param a First operand.
@return True when the condition holds.

---

### `bool lessAbsolute(const csRational& a)`

```cpp
bool lessAbsolute(const csRational& a);
```

@brief Reports whether the absolute value is less.
@param a First operand.
@return True when the condition holds.

---

### `bool greaterOrEqualAbsolute(const csRational& a)`

```cpp
bool greaterOrEqualAbsolute(const csRational& a);
```

@brief Reports whether the absolute value is greater than or equal.
@param a First operand.
@return True when the condition holds.

---

### `bool lessOrEqualAbsolute(const csRational& a)`

```cpp
bool lessOrEqualAbsolute(const csRational& a);
```

@brief Reports whether the absolute value is less than or equal.
@param a First operand.
@return True when the condition holds.

---

### `operator csReal()`

```cpp
operator csReal();
```

@brief Real number as a mantissa and an exponent.

---

### `bool isZero()`

```cpp
bool isZero();
```

@brief Reports whether the value is zero.
@return True when the condition holds.

---

### `bool isNonZero()`

```cpp
bool isNonZero();
```

@brief Reports whether the value is non-zero.
@return True when the condition holds.

---

### `bool negativeSign() const`

```cpp
bool negativeSign() const;
```

@brief Returns the stored sign, without testing for zero.
@return True when the negative flag is set.

---

### `bool isNegative()`

```cpp
bool isNegative();
```

@brief Reports whether the value is strictly negative.
@return True when the sign is negative and the value is not zero.

---

### `void copy(const csRational& a)`

```cpp
void copy(const csRational& a);
```

@brief Copies the value.
@param a First operand.

---

### `double getDouble()`

```cpp
double getDouble();
```

@brief Returns an approximation as a double.
@return Approximation as a double.

---

### `size_t maxLimbCount()`

```cpp
size_t maxLimbCount();
```

@brief Returns the larger of the two sizes.
@return Computed value.

---

### `char* decimalExpansion()`

```cpp
char* decimalExpansion();
```

@brief Returns the decimal expansion of the rational.
@return Resulting text. The caller frees the memory.

---

### `void print(const char*title)`

```cpp
void print(const char*title);
```

@brief Prints the value.
@param title Title printed before the values.

---

### `void clear(const source_location loc = source_location::current())`

```cpp
void clear(const source_location loc = source_location::current());
```

@brief Releases the memory held by the object.
@param loc Call site, for diagnostics.

---

### `struct CSARITHMETIC_API csInteger`

```cpp
struct CSARITHMETIC_API csInteger;
```

@brief Exact integer. One machine word while the value fits, then 32 internal limbs, then the heap.

---

### `void clear()`

```cpp
void clear();
```

@brief Sets the integer to zero and releases the limbs.

---

### `void copy(const csInteger& other)`

```cpp
void copy(const csInteger& other);
```

@brief Copies the value.
@param other Source integer.

---

### `void set(long long number)`

```cpp
void set(long long number);
```

@brief Assigns a signed 64-bit integer.
@param number Signed value.

---

### `void set(unsigned long long number, bool negativeSign = false)`

```cpp
void set(unsigned long long number, bool negativeSign = false);
```

@brief Assigns an unsigned 64-bit integer.
@param number Absolute value.
@param negativeSign True when the result must be negative.

---

### `void set(const char* digits)`

```cpp
void set(const char* digits);
```

@brief Assigns a decimal text.
@param digits Digits, with an optional sign.

---

### `void assignMagnitude(const uint32_t* digits, size_t limbCount, bool negativeSign)`

```cpp
void assignMagnitude(const uint32_t* digits, size_t limbCount, bool negativeSign);
```

@brief Assigns a magnitude in base 2^32, little-endian.
@param digits Limbs. May be null.
@param limbCount Number of limbs.
@param negativeSign True when the result must be negative.

---

### `uint32_t modulo(uint32_t mod) const`

```cpp
uint32_t modulo(uint32_t mod) const;
```

@brief Remainder in [0, mod), sign included.
@param mod Modulus.
@return Remainder.

---

### `size_t bitCount() const`

```cpp
size_t bitCount() const;
```

@brief Number of bits of the absolute value.
@return Number of bits.

---

### `bool isZero() const`

```cpp
bool isZero() const;
```

@brief Reports whether the value is zero.
@return True when the value is zero.

---

### `bool negativeSign() const`

```cpp
bool negativeSign() const;
```

@brief Returns the stored sign, without testing for zero.
@return True when the negative flag is set.

---

### `bool isNegative() const`

```cpp
bool isNegative() const;
```

@brief Reports whether the value is negative.
@return True when the value is strictly negative.

---

### `char* toDecimal() const`

```cpp
char* toDecimal() const;
```

@brief Returns the decimal digits, sign included.
@return Resulting text. The caller frees the memory.

---

### `void print(const char* title) const`

```cpp
void print(const char* title) const;
```

@brief Prints the value.
@param title Title printed before the value.

---

### `void csPtrFree_q(csRational*& qn, size_t size)`

```cpp
void csPtrFree_q(csRational*& qn, size_t size);
```

@brief Releases an array of rationals.
@param qn Array of rationals.
@param size Requested size.

---

### `void csPtrFree_r(csReal*& rn, size_t size)`

```cpp
void csPtrFree_r(csReal*& rn, size_t size);
```

@brief Releases an array of reals.
@param rn Array of reals.
@param size Requested size.

---

### `void init()`

```cpp
void init();
```

@brief Initializes the object or the library tables.

---

### `void set(const char* mantissa, int exponent, bool sign)`

```cpp
void set(const char* mantissa, int exponent, bool sign);
```

@brief Assigns a new value.
@param mantissa Decimal text of the mantissa.
@param exponent Decimal exponent.
@param sign Sign. Zero when the number is positive.

---

### `void set(const char* mantissa, int exponent, bool sign, int repeat)`

```cpp
void set(const char* mantissa, int exponent, bool sign, int repeat);
```

@brief Assigns a repeated mantissa to an existing real.
@param mantissa Decimal text of the mantissa.
@param exponent Decimal exponent.
@param sign Sign. Zero when the number is positive.
@param repeat Number of times the mantissa is repeated. A value below 1 yields zero.

---

### `void set(const char* mantissa, int exponent, bool sign, int repeat, int pos)`

```cpp
void set(const char* mantissa, int exponent, bool sign, int repeat, int pos);
```

@brief Assigns a repeated section of the mantissa to an existing real.
@param mantissa Decimal text of the mantissa.
@param exponent Decimal exponent.
@param sign Sign. Zero when the number is positive.
@param repeat Number of times the section is repeated. A value below 1 yields zero.
@param pos Repeated section. Positive: from the first character through the pos-th, then the rest. Negative: from the pos-th character from the end through the last character, placed at the end.

---

### `void set(const char* mantissa, int exponent, bool sign, int repeat, int length, int index)`

```cpp
void set(const char* mantissa, int exponent, bool sign, int repeat, int length, int index);
```

@brief Assigns a repeated slice of the mantissa to an existing real.
@param mantissa Decimal text of the mantissa.
@param exponent Decimal exponent.
@param sign Sign. Zero when the number is positive.
@param repeat Number of times the slice is repeated. A value below 1 yields zero.
@param length Length of the slice, in characters.
@param index Start index of the slice, from zero.

---

### `void set(unsigned long mantissa, int exponent, bool sign)`

```cpp
void set(unsigned long mantissa, int exponent, bool sign);
```

@brief Assigns a new value.
@param mantissa Decimal text of the mantissa.
@param exponent Decimal exponent.
@param sign Sign. Zero when the number is positive.

---

### `void set(long mantissa, int exponent)`

```cpp
void set(long mantissa, int exponent);
```

@brief Assigns a new value.
@param mantissa Decimal text of the mantissa.
@param exponent Decimal exponent.

---

### `void set(bool evaluate, const char* number)`

```cpp
void set(bool evaluate, const char* number);
```

@brief Assigns a new value.
@param evaluate Interprets the text as a number.
@param number Text of the number.

---

### `void setPrecision(int precision)`

```cpp
void setPrecision(int precision);
```

@brief Sets the precision of this real.
@param precision Requested precision, in digits.

---

### `void setDecimalPlaces(int size)`

```cpp
void setDecimalPlaces(int size);
```

@brief Forces the number of mantissa digits.
@param size Requested size.

---

### `void extendMantissa(size_t size)`

```cpp
void extendMantissa(size_t size);
```

@brief Increases the number of mantissa digits.
@param size Requested size.

---

### `void shortenMantissa(size_t size)`

```cpp
void shortenMantissa(size_t size);
```

@brief Reduces the number of mantissa digits.
@param size Requested size.

---

### `void resizeMantissa(size_t newMantissaSize)`

```cpp
void resizeMantissa(size_t newMantissaSize);
```

@brief Reshapes the mantissa to the requested form.
@param newMantissaSize Parameter @p newMantissaSize.

---

### `void trimTrailingZeros()`

```cpp
void trimTrailingZeros();
```

@brief Removes cosmetic zeros from the mantissa.

---

### `void reduce(size_t size)`

```cpp
void reduce(size_t size);
```

@brief Reduces the value to the requested size.
@param size Requested size.

---

### `size_t digitCount()`

```cpp
size_t digitCount();
```

@brief Returns the number of mantissa digits.
@return Computed value.

---

### `operator csRational()`

```cpp
operator csRational();
```

@brief Exact rational, numerator and denominator in limbs.

---

### `void copy(const csReal& a)`

```cpp
void copy(const csReal& a);
```

@brief Copies the value.
@param a First operand.

---

### `bool isZero()`

```cpp
bool isZero();
```

@brief Sets the real to zero.
@return True when the condition holds.

---

### `bool negativeSign() const`

```cpp
bool negativeSign() const;
```

@brief Returns the stored sign, without testing for zero.
@return True when the negative flag is set.

---

### `bool isNegative()`

```cpp
bool isNegative();
```

@brief Reports whether the value is strictly negative.
@return True when the sign is negative and the value is not zero.

---

### `void print(const char* title)`

```cpp
void print(const char* title);
```

@brief Prints the value.
@param title Title printed before the values.

---

### `void printScientific(const char* title)`

```cpp
void printScientific(const char* title);
```

@brief Prints the value with an extended format.
@param title Title printed before the values.

---

### `void copyFrom(const csReal& a)`

```cpp
void copyFrom(const csReal& a);
```

@brief Assigns another real to this object.
@param a First operand.

---

### `void setDigit(size_t id, char digit)`

```cpp
void setDigit(size_t id, char digit);
```

@brief Assigns a new value.
@param id Index of the element.
@param digit Digit to write.

---

### `void setIntegerDigit(size_t id, char digit)`

```cpp
void setIntegerDigit(size_t id, char digit);
```

@brief Assigns the integer part.
@param id Index of the element.
@param digit Digit to write.

---

### `void setFractionalDigit(size_t id, char digit)`

```cpp
void setFractionalDigit(size_t id, char digit);
```

@brief Assigns the fractional part.
@param id Index of the element.
@param digit Digit to write.

---

### `void setPowerOfTen(long power)`

```cpp
void setPowerOfTen(long power);
```

@brief Assigns a power of ten.
@param power Power of ten.

---

### `void random(size_t nDigits, char digitMin=0, char digitMax=9, long exponent=0, bool sign=0)`

```cpp
void random(size_t nDigits, char digitMin=0, char digitMax=9, long exponent=0, bool sign=0);
```

@brief Draws a random value.
@param nDigits Number of digits.
@param digitMin Parameter @p digitMin.
@param digitMax Parameter @p digitMax.
@param exponent Decimal exponent.
@param sign Sign. Zero when the number is positive.

---

### `size_t significantZeroCount(size_t initialPos=0)`

```cpp
size_t significantZeroCount(size_t initialPos=0);
```

@brief Counts the significant zeros of the mantissa.
@param initialPos Parameter @p initialPos.
@return Computed value.

---

### `void clear()`

```cpp
void clear();
```

@brief Releases the memory held by the object.

---

### `struct csComplex`

```cpp
struct csComplex;
```

@brief Complex number a + bi. Both parts are reals.
The sign of each part is read on that part: real.negativeSign(), imag.isNegative().
Division by zero yields zero, as for the integer.

---

### `void set(const csReal& re, const csReal& im)`

```cpp
void set(const csReal& re, const csReal& im);
```

@brief Assigns both parts.

---

### `void set(long re, long im)`

```cpp
void set(long re, long im);
```

@brief Assigns two integers.

---

### `void set(double re, double im)`

```cpp
void set(double re, double im);
```

@brief Assigns two machine decimals.

---

### `void copy(const csComplex& other)`

```cpp
void copy(const csComplex& other);
```

@brief Copies the value.

---

### `void clear()`

```cpp
void clear();
```

@brief Releases the memory of both parts.

---

### `bool isZero()`

```cpp
bool isZero();
```

@brief Reports whether both parts are zero.

---

### `bool isReal()`

```cpp
bool isReal();
```

@brief Reports whether the imaginary part is zero.

---

### `bool isImaginary()`

```cpp
bool isImaginary();
```

@brief Reports whether the real part is zero.

---

### `static csComplex imaginaryUnit()`

```cpp
static csComplex imaginaryUnit();
```

@brief Returns the imaginary unit.

---

### `void print(const char* title)`

```cpp
void print(const char* title);
```

@brief Prints the value.
@param title Title printed before the values.

---

### `struct csComplex_q`

```cpp
struct csComplex_q;
```

@brief Complex number a + bi. Both parts are exact rationals.
The sign of each part is read on that part: real.negativeSign(), imag.isNegative().
Division by zero yields zero. The squared modulus stays rational; the modulus itself is a real.

---

### `void set(const csRational& re, const csRational& im)`

```cpp
void set(const csRational& re, const csRational& im);
```

@brief Assigns both parts.

---

### `void set(long re, long im)`

```cpp
void set(long re, long im);
```

@brief Assigns two integers.

---

### `void copy(const csComplex_q& other)`

```cpp
void copy(const csComplex_q& other);
```

@brief Copies the value.

---

### `void clear()`

```cpp
void clear();
```

@brief Releases the memory of both parts.

---

### `bool isZero()`

```cpp
bool isZero();
```

@brief Reports whether both parts are zero.

---

### `bool isReal()`

```cpp
bool isReal();
```

@brief Reports whether the imaginary part is zero.

---

### `bool isImaginary()`

```cpp
bool isImaginary();
```

@brief Reports whether the real part is zero.

---

### `static csComplex_q imaginaryUnit()`

```cpp
static csComplex_q imaginaryUnit();
```

@brief Returns the imaginary unit.

---

### `void print(const char* title)`

```cpp
void print(const char* title);
```

@brief Prints the value.
@param title Title printed before the values.

---

### `void init()`

```cpp
void init();
```

@brief Initializes the object or the library tables.

---

### `void printDigits(char* nb, const char* separator=" ")`

```cpp
void printDigits(char* nb, const char* separator=" ");
```

@brief Prints an array of digits.
@param nb Number of elements.
@param separator Parameter @p separator.

---

### `uchar digitPairIndex(uchar tens, uchar units)`

```cpp
uchar digitPairIndex(uchar tens, uchar units);
```

@brief Returns the index of a digit pair.
@param tens Parameter @p tens.
@param units Parameter @p units.
@return Result.

---

### `uchar digitTripleIndex(int cents, uchar tens, uchar units)`

```cpp
uchar digitTripleIndex(int cents, uchar tens, uchar units);
```

@brief Returns the index of a digit triple.
@param cents Parameter @p cents.
@param tens Parameter @p tens.
@param units Parameter @p units.
@return Result.

---

### `csBIDIGITS subtractionStep(int i)`

```cpp
csBIDIGITS subtractionStep(int i);
```

@brief Transforms the digits of a subtraction.
@param i Parameter @p i.
@return Result.

---

### `void multiplyDigitBase2(char* a, uchar b, char*& result, size_t aSize, size_t resSize,csBIDIGITS& prevCarry, size_t m, size_t n)`

```cpp
void multiplyDigitBase2(char* a, uchar b, char*& result, size_t aSize, size_t resSize,csBIDIGITS& prevCarry, size_t m, size_t n);
```

@brief Computes a decimal product in the secondary base.
@param a First operand.
@param b Second operand.
@param result Parameter @p result.
@param aSize Number of digits of the first text.
@param resSize Parameter @p resSize.
@param prevCarry Parameter @p prevCarry.
@param m Number of evaluation points.
@param n Number of elements.

---

### `void addShiftedProduct(char* a, char* b, char*& result, size_t opSize, size_t resSize)`

```cpp
void addShiftedProduct(char* a, char* b, char*& result, size_t opSize, size_t resSize);
```

@brief Adds two texts during a multiplication.
@param a First operand.
@param b Second operand.
@param result Parameter @p result.
@param opSize Number of aligned digits.
@param resSize Parameter @p resSize.

---

### `void subtractPartialQuotient(char* a, char* b, char*& result, size_t opSize, size_t frontOffset)`

```cpp
void subtractPartialQuotient(char* a, char* b, char*& result, size_t opSize, size_t frontOffset);
```

@brief Subtracts two texts during a division.
@param a First operand.
@param b Second operand.
@param result Parameter @p result.
@param opSize Number of aligned digits.
@param frontOffset Parameter @p frontOffset.

---

### `void multiplyDigitForDivision(char* a, uchar b, char*& result, size_t aSize, size_t resSize)`

```cpp
void multiplyDigitForDivision(char* a, uchar b, char*& result, size_t aSize, size_t resSize);
```

@brief Multiplies during a division, in the first base.
@param a First operand.
@param b Second operand.
@param result Parameter @p result.
@param aSize Number of digits of the first text.
@param resSize Parameter @p resSize.

---

### `void multiplyDigitPairForDivision(char* a, uchar b, char*& result, size_t aSize, size_t resSize)`

```cpp
void multiplyDigitPairForDivision(char* a, uchar b, char*& result, size_t aSize, size_t resSize);
```

@brief Multiplies during a division, in the second base.
@param a First operand.
@param b Second operand.
@param result Parameter @p result.
@param aSize Number of digits of the first text.
@param resSize Parameter @p resSize.

---

### `csBIDIGITS** packDigitPairs(int** opTable)`

```cpp
csBIDIGITS** packDigitPairs(int** opTable);
```

@brief Groups decimal digits into pairs.
@param opTable Parameter @p opTable.
@return Result.

---

### `void buildMultiplicationTable()`

```cpp
void buildMultiplicationTable();
```

@brief Builds the multiplication table.

---

### `void buildAdditionTable()`

```cpp
void buildAdditionTable();
```

@brief Builds the addition table.

---

### `void buildDivisionTable()`

```cpp
void buildDivisionTable();
```

@brief Builds the division table.

---

### `void buildSubtractionTable()`

```cpp
void buildSubtractionTable();
```

@brief Builds the subtraction table.

---

### `void printDigitTable(char* opName)`

```cpp
void printDigitTable(char* opName);
```

@brief Prints a digit table.
@param opName Parameter @p opName.

---

### `void addDecimal(char* a, char* b, char*& result, size_t opSize, size_t& resSize)`

```cpp
void addDecimal(char* a, char* b, char*& result, size_t opSize, size_t& resSize);
```

@brief Adds two decimal texts.
@param a First operand.
@param b Second operand.
@param result Parameter @p result.
@param opSize Number of aligned digits.
@param resSize Parameter @p resSize.

---

### `char* addDecimal(char* a,char* b)`

```cpp
char* addDecimal(char* a,char* b);
```

@brief Adds two decimal texts.
@param a First operand.
@param b Second operand.
@return Resulting text. The caller frees the memory.

---

### `char* addDecimal(char* a,char* b, size_t& resSize)`

```cpp
char* addDecimal(char* a,char* b, size_t& resSize);
```

@brief Adds two decimal texts.
@param a First operand.
@param b Second operand.
@param resSize Parameter @p resSize.
@return Resulting text. The caller frees the memory.

---

### `char* addDecimal(const char* a,const char* b)`

```cpp
char* addDecimal(const char* a,const char* b);
```

@brief Adds two decimal texts.
@param a First operand.
@param b Second operand.
@return Resulting text. The caller frees the memory.

---

### `void subtractDecimal(char* a, char* b, char*& result, size_t opSize)`

```cpp
void subtractDecimal(char* a, char* b, char*& result, size_t opSize);
```

@brief Subtracts two decimal texts.
@param a First operand.
@param b Second operand.
@param result Parameter @p result.
@param opSize Number of aligned digits.

---

### `char* subtractDecimal(char* a,char* b)`

```cpp
char* subtractDecimal(char* a,char* b);
```

@brief Subtracts two decimal texts.
@param a First operand.
@param b Second operand.
@return Resulting text. The caller frees the memory.

---

### `char* subtractDecimal(char* a,char* b, bool& sign)`

```cpp
char* subtractDecimal(char* a,char* b, bool& sign);
```

@brief Subtracts two decimal texts.
@param a First operand.
@param b Second operand.
@param sign Sign. Zero when the number is positive.
@return Resulting text. The caller frees the memory.

---

### `char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize)`

```cpp
char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize);
```

@brief Subtracts two decimal texts.
@param a First operand.
@param b Second operand.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@return Resulting text. The caller frees the memory.

---

### `char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize, bool&sign)`

```cpp
char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize, bool&sign);
```

@brief Subtracts two decimal texts.
@param a First operand.
@param b Second operand.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param sign Sign. Zero when the number is positive.
@return Resulting text. The caller frees the memory.

---

### `char* subtractDecimal(const char* a,const char*b)`

```cpp
char* subtractDecimal(const char* a,const char*b);
```

@brief Subtracts two decimal texts.
@param a First operand.
@param b Second operand.
@return Resulting text. The caller frees the memory.

---

### `char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize, size_t& resSize, bool& sign)`

```cpp
char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize, size_t& resSize, bool& sign);
```

@brief Subtracts two decimal texts.
@param a First operand.
@param b Second operand.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param resSize Parameter @p resSize.
@param sign Sign. Zero when the number is positive.
@return Resulting text. The caller frees the memory.

---

### `void multiplyByDigit(char* a, uchar b, char*& result, size_t aSize, size_t resSize)`

```cpp
void multiplyByDigit(char* a, uchar b, char*& result, size_t aSize, size_t resSize);
```

@brief Computes a product in the current base.
@param a First operand.
@param b Second operand.
@param result Parameter @p result.
@param aSize Number of digits of the first text.
@param resSize Parameter @p resSize.

---

### `void multiplyDecimal(char* a, char* b, char*& result, char*& tmpResult, size_t aSize, size_t bSize, size_t resSize)`

```cpp
void multiplyDecimal(char* a, char* b, char*& result, char*& tmpResult, size_t aSize, size_t bSize, size_t resSize);
```

@brief Multiplies two decimal texts.
@param a First operand.
@param b Second operand.
@param result Parameter @p result.
@param tmpResult Parameter @p tmpResult.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param resSize Parameter @p resSize.

---

### `char* multiplyDecimal(char* a, char* b)`

```cpp
char* multiplyDecimal(char* a, char* b);
```

@brief Multiplies two decimal texts.
@param a First operand.
@param b Second operand.
@return Resulting text. The caller frees the memory.

---

### `char* multiplyDecimal(char* a, char* b, size_t aSize, size_t bSize)`

```cpp
char* multiplyDecimal(char* a, char* b, size_t aSize, size_t bSize);
```

@brief Multiplies two decimal texts.
@param a First operand.
@param b Second operand.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@return Resulting text. The caller frees the memory.

---

### `char* multiplyDecimal(const char* a, const char* b)`

```cpp
char* multiplyDecimal(const char* a, const char* b);
```

@brief Multiplies two decimal texts.
@param a First operand.
@param b Second operand.
@return Resulting text. The caller frees the memory.

---

### `char* multiplyDecimal(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize)`

```cpp
char* multiplyDecimal(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize);
```

@brief Multiplies two decimal texts.
@param a First operand.
@param b Second operand.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param resSize Parameter @p resSize.
@return Resulting text. The caller frees the memory.

---

### `char* divideDecimal(char*a, char* b, char*& remain)`

```cpp
char* divideDecimal(char*a, char* b, char*& remain);
```

@brief Divides two decimal texts and returns the quotient.
@param a First operand.
@param b Second operand.
@param remain Receives the remainder of the division.
@return Resulting text. The caller frees the memory.

---

### `char* divideDecimal(char*a, char* b)`

```cpp
char* divideDecimal(char*a, char* b);
```

@brief Divides two decimal texts and returns the quotient.
@param a First operand.
@param b Second operand.
@return Resulting text. The caller frees the memory.

---

### `char const* divideDecimal(char* a, char* b, char*& result, char*& remain, size_t& aSize, size_t& bSize, size_t& resSize, size_t& remSize)`

```cpp
char const* divideDecimal(char* a, char* b, char*& result, char*& remain, size_t& aSize, size_t& bSize, size_t& resSize, size_t& remSize);
```

@brief Divides two decimal texts and returns the quotient.
@param a First operand.
@param b Second operand.
@param result Parameter @p result.
@param remain Receives the remainder of the division.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param resSize Parameter @p resSize.
@param remSize Parameter @p remSize.
@return Resulting text. The caller frees the memory.

---

### `char* divideDecimal(const char* a, const char* b, char*&remain)`

```cpp
char* divideDecimal(const char* a, const char* b, char*&remain);
```

@brief Divides two decimal texts and returns the quotient.
@param a First operand.
@param b Second operand.
@param remain Receives the remainder of the division.
@return Resulting text. The caller frees the memory.

---

### `char* divideDecimal(const char* a, const char* b)`

```cpp
char* divideDecimal(const char* a, const char* b);
```

@brief Divides two decimal texts and returns the quotient.
@param a First operand.
@param b Second operand.
@return Resulting text. The caller frees the memory.

---

### `char* divideDecimal(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize)`

```cpp
char* divideDecimal(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize);
```

@brief Divides two decimal texts and returns the quotient.
@param a First operand.
@param b Second operand.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param resSize Parameter @p resSize.
@return Resulting text. The caller frees the memory.

---

### `char const* divideWithFraction(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals)`

```cpp
char const* divideWithFraction(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals);
```

@brief Divides two decimal texts and keeps a fractional part.
@param _a First operand.
@param _b Second operand.
@param resInt Parameter @p resInt.
@param resDec Parameter @p resDec.
@param remain Receives the remainder of the division.
@param _aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param resSize Parameter @p resSize.
@param resIntSize Parameter @p resIntSize.
@param resDecSize Parameter @p resDecSize.
@param remSize Parameter @p remSize.
@param nDecimals Parameter @p nDecimals.
@return Resulting text. The caller frees the memory.

---

### `char const* divideWithScale(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals)`

```cpp
char const* divideWithScale(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals);
```

@brief Computes a real division, second variant.
@param _a First operand.
@param _b Second operand.
@param resInt Parameter @p resInt.
@param resDec Parameter @p resDec.
@param remain Receives the remainder of the division.
@param _aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param resSize Parameter @p resSize.
@param resIntSize Parameter @p resIntSize.
@param resDecSize Parameter @p resDecSize.
@param remSize Parameter @p remSize.
@param nDecimals Parameter @p nDecimals.
@return Resulting text. The caller frees the memory.

---

### `char const* divideWithScale(char*_a, char* _b, char*& resInt, char*& resDec, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t nDecimals)`

```cpp
char const* divideWithScale(char*_a, char* _b, char*& resInt, char*& resDec, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t nDecimals);
```

@brief Computes a real division, second variant.
@param _a First operand.
@param _b Second operand.
@param resInt Parameter @p resInt.
@param resDec Parameter @p resDec.
@param _aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param resSize Parameter @p resSize.
@param resIntSize Parameter @p resIntSize.
@param resDecSize Parameter @p resDecSize.
@param nDecimals Parameter @p nDecimals.
@return Resulting text. The caller frees the memory.

---

### `char const* divideDecimal(char*a, char* b, char*& result, size_t& aSize, size_t& bSize, size_t& resSize)`

```cpp
char const* divideDecimal(char*a, char* b, char*& result, size_t& aSize, size_t& bSize, size_t& resSize);
```

@brief Divides two decimal texts and returns the quotient.
@param a First operand.
@param b Second operand.
@param result Parameter @p result.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param resSize Parameter @p resSize.
@return Resulting text. The caller frees the memory.

---

### `char const* divideWithFraction(char* a, char* b, char*& resInt, char*& resDec, char*& remain, size_t nDecimals)`

```cpp
char const* divideWithFraction(char* a, char* b, char*& resInt, char*& resDec, char*& remain, size_t nDecimals);
```

@brief Divides two decimal texts and keeps a fractional part.
@param a First operand.
@param b Second operand.
@param resInt Parameter @p resInt.
@param resDec Parameter @p resDec.
@param remain Receives the remainder of the division.
@param nDecimals Parameter @p nDecimals.
@return Resulting text. The caller frees the memory.

---

### `char const* remainderDecimal(char*a, char* b, char*& remain, size_t& aSize, size_t& bSize, size_t& remSize)`

```cpp
char const* remainderDecimal(char*a, char* b, char*& remain, size_t& aSize, size_t& bSize, size_t& remSize);
```

@brief Computes the remainder of a rational division.
@param a First operand.
@param b Second operand.
@param remain Receives the remainder of the division.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param remSize Parameter @p remSize.
@return Resulting text. The caller frees the memory.

---

### `void remainderDecimal(char*a, char* b, char*&remain)`

```cpp
void remainderDecimal(char*a, char* b, char*&remain);
```

@brief Computes the remainder of a rational division.
@param a First operand.
@param b Second operand.
@param remain Receives the remainder of the division.

---

### `char* remainderDecimal(char*a, char* b)`

```cpp
char* remainderDecimal(char*a, char* b);
```

@brief Computes the remainder of a rational division.
@param a First operand.
@param b Second operand.
@return Resulting text. The caller frees the memory.

---

### `char* remainderDecimal(const char*a, const char* b)`

```cpp
char* remainderDecimal(const char*a, const char* b);
```

@brief Computes the remainder of a rational division.
@param a First operand.
@param b Second operand.
@return Resulting text. The caller frees the memory.

---

### `char* gcd(const char* a, const char* b, size_t& gcdSize)`

```cpp
char* gcd(const char* a, const char* b, size_t& gcdSize);
```

@brief Computes the greatest common divisor.
@param a First operand.
@param b Second operand.
@param gcdSize Parameter @p gcdSize.
@return Resulting text. The caller frees the memory.

---

### `char* gcd(char* a, char* b, size_t& gcdSize)`

```cpp
char* gcd(char* a, char* b, size_t& gcdSize);
```

@brief Computes the greatest common divisor.
@param a First operand.
@param b Second operand.
@param gcdSize Parameter @p gcdSize.
@return Resulting text. The caller frees the memory.

---

### `char* gcd(char* a, char* b, size_t aSize, size_t bSize, size_t& gcdSize)`

```cpp
char* gcd(char* a, char* b, size_t aSize, size_t bSize, size_t& gcdSize);
```

@brief Computes the greatest common divisor.
@param a First operand.
@param b Second operand.
@param aSize Number of digits of the first text.
@param bSize Number of digits of the second text.
@param gcdSize Parameter @p gcdSize.
@return Resulting text. The caller frees the memory.

---

### `void csSortMinR(csReal*& rn, size_t size)`

```cpp
void csSortMinR(csReal*& rn, size_t size);
```

@brief Sorts reals into increasing order.
@param rn Array of reals.
@param size Requested size.

---

### `void csSortMin_CoordsByRNumber(csFCOORDS*& fc, csReal*& rn, size_t size)`

```cpp
void csSortMin_CoordsByRNumber(csFCOORDS*& fc, csReal*& rn, size_t size);
```

@brief Sorts points by increasing real.
@param fc Parameter @p fc.
@param rn Array of reals.
@param size Requested size.

---

### `void csSortMax_CoordsByRNumber(csFCOORDS*& fc, csReal*& rn, size_t size)`

```cpp
void csSortMax_CoordsByRNumber(csFCOORDS*& fc, csReal*& rn, size_t size);
```

@brief Sorts points by decreasing real.
@param fc Parameter @p fc.
@param rn Array of reals.
@param size Requested size.

---

### `CSARITHMETIC_API CSARITHMETIC::csRational csReduce(CSARITHMETIC_API CSARITHMETIC::csRational a, size_t sizeCondition)`

```cpp
CSARITHMETIC_API CSARITHMETIC::csRational csReduce(CSARITHMETIC_API CSARITHMETIC::csRational a, size_t sizeCondition);
```

@brief Reduces the value to the requested size.
@param a First operand.
@param sizeCondition Parameter @p sizeCondition.
@return Resulting rational.

---
