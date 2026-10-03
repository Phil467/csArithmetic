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

**Description**

Returns the absolute value.

**Returns**

Non-negative integer.

---

### `csInteger quotientAndRemainder(const csInteger& divisor, csInteger& remainder) const`

```cpp
csInteger quotientAndRemainder(const csInteger& divisor, csInteger& remainder) const;
```

**Description**

Quotient and remainder of Euclidean division truncated toward zero.

**Parameters**

- **divisor** — Divisor.
- **remainder** — Receives the remainder, with the sign of the dividend.

**Returns**

Quotient.

---

### `csInteger power(size_t exponent) const`

```cpp
csInteger power(size_t exponent) const;
```

**Description**

Raises the integer to a non-negative integer power.

**Parameters**

- **exponent** — Exponent.

**Returns**

Resulting integer. Zero to the power zero is one.

---

## csRational

### `csRational toRational() const`

```cpp
csRational toRational() const;
```

**Description**

Converts the integer to a rational with denominator one.

**Returns**

Resulting rational.

---

### `csRational* csPtrAlloc_q(size_t nb)`

```cpp
csRational* csPtrAlloc_q(size_t nb);
```

**Description**

Allocates an array of rationals.

**Parameters**

- **nb** — Number of elements.

**Returns**

Pointer to the resulting rational or array.

---

### `csRational* csPtrAlloc_q(size_t nb, csRational init)`

```cpp
csRational* csPtrAlloc_q(size_t nb, csRational init);
```

**Description**

Allocates an array of rationals.

**Parameters**

- **nb** — Number of elements.
- **init** — Initial fill character.

**Returns**

Pointer to the resulting rational or array.

---

### `csRational norm()`

```cpp
csRational norm();
```

**Description**

Returns the squared modulus, a² + b², as an exact rational.

---

### `csRational pow(csRational a, size_t p)`

```cpp
csRational pow(csRational a, size_t p);
```

**Description**

Raises a rational to an integer power.

**Parameters**

- **a** — First operand.
- **p** — Power or degree.

**Returns**

Resulting rational.

---

## csReal

### `csReal* csPtrAlloc_r(size_t nb)`

```cpp
csReal* csPtrAlloc_r(size_t nb);
```

**Description**

Allocates an array of reals.

**Parameters**

- **nb** — Number of elements.

**Returns**

Pointer to the resulting real or array.

---

### `csReal* csPtrAlloc_r(size_t nb, csReal init)`

```cpp
csReal* csPtrAlloc_r(size_t nb, csReal init);
```

**Description**

Allocates an array of reals.

**Parameters**

- **nb** — Number of elements.
- **init** — Initial fill character.

**Returns**

Pointer to the resulting real or array.

---

### `csReal(const char* mantissa, int exponent, bool sign, int repeat)`

```cpp
csReal(const char* mantissa, int exponent, bool sign, int repeat);
```

**Description**

Builds a real by repeating the mantissa.

**Parameters**

- **mantissa** — Decimal text of the mantissa.
- **exponent** — Decimal exponent.
- **sign** — Sign. Zero when the number is positive.
- **repeat** — Number of times the mantissa is repeated. A value below 1 yields zero.

---

### `csReal(const char* mantissa, int exponent, bool sign, int repeat, int pos)`

```cpp
csReal(const char* mantissa, int exponent, bool sign, int repeat, int pos);
```

**Description**

Builds a real by repeating a section of the mantissa.

**Parameters**

- **mantissa** — Decimal text of the mantissa.
- **exponent** — Decimal exponent.
- **sign** — Sign. Zero when the number is positive.
- **repeat** — Number of times the section is repeated. A value below 1 yields zero.
- **pos** — Repeated section. Positive: from the first character through the pos-th, then the rest. Negative: from the pos-th character from the end through the last character, placed at the end.

---

### `csReal(const char* mantissa, int exponent, bool sign, int repeat, int length, int index)`

```cpp
csReal(const char* mantissa, int exponent, bool sign, int repeat, int length, int index);
```

**Description**

Builds a real by repeating a slice of the mantissa.

**Parameters**

- **mantissa** — Decimal text of the mantissa.
- **exponent** — Decimal exponent.
- **sign** — Sign. Zero when the number is positive.
- **repeat** — Number of times the slice is repeated. A value below 1 yields zero.
- **length** — Length of the slice, in characters.
- **index** — Start index of the slice, from zero.

---

### `csReal(double value)`

```cpp
csReal(double value);
```

**Description**

Builds a real from a double.

**Parameters**

- **value** — Value to convert to decimal.

---

### `csReal abs()`

```cpp
csReal abs();
```

**Description**

Returns the absolute value.

**Returns**

Resulting real.

---

### `csReal operator*(double a)`

```cpp
csReal operator*(double a);
```

**Description**

Multiplies by a double, including values between 0 and 1.

**Parameters**

- **a** — Decimal factor.

**Returns**

Resulting real.

---

### `csReal integerQuotient(long n)`

```cpp
csReal integerQuotient(long n);
```

**Description**

Returns the integer quotient of division by `n`, truncated toward zero.

**Parameters**

- **n** — Integer divisor.

**Returns**

Integer quotient, with a zero exponent.

---

### `csReal integer()`

```cpp
csReal integer();
```

**Description**

Returns the integer part, truncated toward zero.

**Returns**

Integer part, with a zero exponent.

---

### `csReal mantissaSection(size_t first, size_t last)`

```cpp
csReal mantissaSection(size_t first, size_t last);
```

**Description**

Returns a section of the mantissa.

**Parameters**

- **first** — First index, inclusive.
- **last** — Last index, exclusive.

**Returns**

Resulting real.

---

### `csReal norm()`

```cpp
csReal norm();
```

**Description**

Returns the squared modulus, a² + b².

---

### `csReal abs()`

```cpp
csReal abs();
```

**Description**

Returns the modulus.

---

### `csReal abs()`

```cpp
csReal abs();
```

**Description**

Returns the modulus as a real.

---

### `csReal* csSortMinR(csReal* rn, size_t size)`

```cpp
csReal* csSortMinR(csReal* rn, size_t size);
```

**Description**

Sorts reals into increasing order.

**Parameters**

- **rn** — Array of reals.
- **size** — Requested size.

**Returns**

Pointer to the resulting real or array.

---

## csComplex

### `csComplex conjugate()`

```cpp
csComplex conjugate();
```

**Description**

Returns the conjugate a - bi.

---

### `csComplex power(long exponent)`

```cpp
csComplex power(long exponent);
```

**Description**

Integer power. 0^0 is 1. A negative exponent takes the reciprocal.

---

### `csComplex reciprocal()`

```cpp
csComplex reciprocal();
```

**Description**

Returns the reciprocal.

---

## csComplex_q

### `csComplex_q conjugate()`

```cpp
csComplex_q conjugate();
```

**Description**

Returns the conjugate a - bi.

---

### `csComplex_q power(long exponent)`

```cpp
csComplex_q power(long exponent);
```

**Description**

Integer power. 0^0 is 1. A negative exponent takes the reciprocal.

---

### `csComplex_q reciprocal()`

```cpp
csComplex_q reciprocal();
```

**Description**

Returns the reciprocal.

---

## Functions

### `template<class T> CS_FORCE_INLINE T* csAlloc(size_t n)`

```cpp
template<class T> CS_FORCE_INLINE T* csAlloc(size_t n);
```

**Description**

Allocates a memory block.

**Parameters**

- **n** — Number of elements.

**Returns**

Result.

---

### `template<class T> CS_FORCE_INLINE T* csAlloc(size_t n, T init)`

```cpp
template<class T> CS_FORCE_INLINE T* csAlloc(size_t n, T init);
```

**Description**

Allocates a memory block.

**Parameters**

- **n** — Number of elements.
- **init** — Initial fill character.

**Returns**

Result.

---

### `template<class T> CS_FORCE_INLINE T* csAlloc2(size_t n, T init)`

```cpp
template<class T> CS_FORCE_INLINE T* csAlloc2(size_t n, T init);
```

**Description**

Allocates a two-dimensional memory block.

**Parameters**

- **n** — Number of elements.
- **init** — Initial fill character.

**Returns**

Result.

---

### `char* CSARITHMETIC_API csAllocCharPtr(size_t n, size_t n1, char init)`

```cpp
char* CSARITHMETIC_API csAllocCharPtr(size_t n, size_t n1, char init);
```

**Description**

Allocates a character array.

**Parameters**

- **n** — Number of elements.
- **n1** — Size of the second dimension.
- **init** — Initial fill character.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* CSARITHMETIC_API csAllocCharPtr(size_t n, char init)`

```cpp
char* CSARITHMETIC_API csAllocCharPtr(size_t n, char init);
```

**Description**

Allocates a character array.

**Parameters**

- **n** — Number of elements.
- **init** — Initial fill character.

**Returns**

Resulting text. The caller frees the memory.

---

### `void CSARITHMETIC_API csReallocString(char** str, size_t size)`

```cpp
void CSARITHMETIC_API csReallocString(char** str, size_t size);
```

**Description**

Reallocates a character string.

**Parameters**

- **str** — Text to modify.
- **size** — Requested size.

**Returns**

Result.

---

### `void CSARITHMETIC_API csReallocString(char** str, size_t size, size_t newSize, char cFill)`

```cpp
void CSARITHMETIC_API csReallocString(char** str, size_t size, size_t newSize, char cFill);
```

**Description**

Reallocates a character string.

**Parameters**

- **str** — Text to modify.
- **size** — Requested size.
- **newSize** — New size.
- **cFill** — Fill character.

**Returns**

Result.

---

### `char* CSARITHMETIC_API newString(char*cstr, size_t size)`

```cpp
char* CSARITHMETIC_API newString(char*cstr, size_t size);
```

**Description**

Copies a text into a new buffer.

**Parameters**

- **cstr** — Source text.
- **size** — Requested size.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* CSARITHMETIC_API newString(const char*cstr, size_t size)`

```cpp
char* CSARITHMETIC_API newString(const char*cstr, size_t size);
```

**Description**

Copies a text into a new buffer.

**Parameters**

- **cstr** — Source text.
- **size** — Requested size.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* CSARITHMETIC_API newString(const char*cstr, size_t begin, size_t end)`

```cpp
char* CSARITHMETIC_API newString(const char*cstr, size_t begin, size_t end);
```

**Description**

Copies a text into a new buffer.

**Parameters**

- **cstr** — Source text.
- **begin** — Start index, inclusive.
- **end** — End index, exclusive.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* CSARITHMETIC_API newString(const char*cstr)`

```cpp
char* CSARITHMETIC_API newString(const char*cstr);
```

**Description**

Copies a text into a new buffer.

**Parameters**

- **cstr** — Source text.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* intToString(int nb, size_t& sz)`

```cpp
char* intToString(int nb, size_t& sz);
```

**Description**

Converts a signed integer to text.

**Parameters**

- **nb** — Number of elements.
- **sz** — Receives the size of the produced text.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* uLongToString(size_t nb, size_t& sz)`

```cpp
char* uLongToString(size_t nb, size_t& sz);
```

**Description**

Converts an unsigned integer to text.

**Parameters**

- **nb** — Number of elements.
- **sz** — Receives the size of the produced text.

**Returns**

Resulting text. The caller frees the memory.

---

### `void shiftRight(char*& nb, size_t size, size_t nShift)`

```cpp
void shiftRight(char*& nb, size_t size, size_t nShift);
```

**Description**

Shifts the digits of a text to the right.

**Parameters**

- **nb** — Number of elements.
- **size** — Requested size.
- **nShift** — Number of shift positions.

---

### `char* shiftRightCopy(char* nb, size_t nbSize, size_t nbCopySize)`

```cpp
char* shiftRightCopy(char* nb, size_t nbSize, size_t nbCopySize);
```

**Description**

Returns a copy of the text shifted to the right.

**Parameters**

- **nb** — Number of elements.
- **nbSize** — Size of the number.
- **nbCopySize** — Size of the copy.

**Returns**

Resulting text. The caller frees the memory.

---

### `void shiftLeft(char*& nb, size_t size, size_t nShift)`

```cpp
void shiftLeft(char*& nb, size_t size, size_t nShift);
```

**Description**

Shifts the digits of a text to the left.

**Parameters**

- **nb** — Number of elements.
- **size** — Requested size.
- **nShift** — Number of shift positions.

---

### `void shiftLeftCopy(char* nb, char*&nbCopy, size_t nbSize, size_t nbCopyPos)`

```cpp
void shiftLeftCopy(char* nb, char*&nbCopy, size_t nbSize, size_t nbCopyPos);
```

**Description**

Copies a text while shifting it to the left.

**Parameters**

- **nb** — Number of elements.
- **nbCopy** — Produced copy.
- **nbSize** — Size of the number.
- **nbCopyPos** — Write position in the copy.

---

### `void skipZeros(char* a, size_t aSize, size_t& skipLen)`

```cpp
void skipZeros(char* a, size_t aSize, size_t& skipLen);
```

**Description**

Skips the leading zeros of a text.

**Parameters**

- **a** — First operand.
- **aSize** — Number of digits of the first text.
- **skipLen** — Number of skipped zeros.

---

### `void skipZeros2(char* a, size_t aSize, size_t& skipLen, size_t&incr)`

```cpp
void skipZeros2(char* a, size_t aSize, size_t& skipLen, size_t&incr);
```

**Description**

Skips the leading zeros and measures the step.

**Parameters**

- **a** — First operand.
- **aSize** — Number of digits of the first text.
- **skipLen** — Number of skipped zeros.
- **incr** — Step associated with the skipped zeros.

---

### `bool isaGreater(char* a,char* b,size_t opSize, size_t ibegin)`

```cpp
bool isaGreater(char* a,char* b,size_t opSize, size_t ibegin);
```

**Description**

Reports whether the first text is greater than the second.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **opSize** — Number of aligned digits.
- **ibegin** — Index of the first digit compared.

**Returns**

True when the condition holds.

---

### `bool isaGreaterEqual(char* a,char* b,size_t opSize, size_t ibegin)`

```cpp
bool isaGreaterEqual(char* a,char* b,size_t opSize, size_t ibegin);
```

**Description**

Reports whether the first text is greater than or equal to the second.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **opSize** — Number of aligned digits.
- **ibegin** — Index of the first digit compared.

**Returns**

True when the condition holds.

---

### `bool isaGreater2(char* a,char* b,size_t aSize,size_t bSize, size_t ibegin)`

```cpp
bool isaGreater2(char* a,char* b,size_t aSize,size_t bSize, size_t ibegin);
```

**Description**

Reports whether the first text is greater than the second, at different sizes.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **ibegin** — Index of the first digit compared.

**Returns**

True when the condition holds.

---

### `bool isaGreaterEqual2(char* a,char* b,size_t aSize, size_t bSize, size_t ibegin)`

```cpp
bool isaGreaterEqual2(char* a,char* b,size_t aSize, size_t bSize, size_t ibegin);
```

**Description**

Reports whether the first text is greater than or equal to the second, at different sizes.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **ibegin** — Index of the first digit compared.

**Returns**

True when the condition holds.

---

### `int getReadySub(char*& a,char*& b, size_t&opSize)`

```cpp
int getReadySub(char*& a,char*& b, size_t&opSize);
```

**Description**

Prepares two texts for a subtraction.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **opSize** — Number of aligned digits.

**Returns**

Computed value.

---

### `bool getReadySub2(const char* a0,const char* b0, char*& a, char*& b, size_t& opSize)`

```cpp
bool getReadySub2(const char* a0,const char* b0, char*& a, char*& b, size_t& opSize);
```

**Description**

Prepares two source texts for a subtraction.

**Parameters**

- **a0** — Original text of the first operand.
- **b0** — Original text of the second operand.
- **a** — First operand.
- **b** — Second operand.
- **opSize** — Number of aligned digits.

**Returns**

True when the condition holds.

---

### `bool getReadySub3(const char* a0,const char* b0, char*& a, char*& b, size_t aSize, size_t bSize, size_t&opSize)`

```cpp
bool getReadySub3(const char* a0,const char* b0, char*& a, char*& b, size_t aSize, size_t bSize, size_t&opSize);
```

**Description**

Prepares two texts of different sizes for a subtraction.

**Parameters**

- **a0** — Original text of the first operand.
- **b0** — Original text of the second operand.
- **a** — First operand.
- **b** — Second operand.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **opSize** — Number of aligned digits.

**Returns**

True when the condition holds.

---

### `void getReady(char*& a,char*& b, size_t& opSize)`

```cpp
void getReady(char*& a,char*& b, size_t& opSize);
```

**Description**

Aligns two texts before an operation.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **opSize** — Number of aligned digits.

---

### `void getReady2(const char* a0,const char* b0, char*& a, char*& b, size_t& opSize)`

```cpp
void getReady2(const char* a0,const char* b0, char*& a, char*& b, size_t& opSize);
```

**Description**

Aligns two source texts before an operation.

**Parameters**

- **a0** — Original text of the first operand.
- **b0** — Original text of the second operand.
- **a** — First operand.
- **b** — Second operand.
- **opSize** — Number of aligned digits.

---

### `bool removeFrontZeros(char*& a, size_t& size)`

```cpp
bool removeFrontZeros(char*& a, size_t& size);
```

**Description**

Removes leading zeros from a text.

**Parameters**

- **a** — First operand.
- **size** — Requested size.

**Returns**

True when the condition holds.

---

### `void removeLeft(char*& nb, size_t& size, size_t remLen)`

```cpp
void removeLeft(char*& nb, size_t& size, size_t remLen);
```

**Description**

Removes characters from the left of a text.

**Parameters**

- **nb** — Number of elements.
- **size** — Requested size.
- **remLen** — Number of characters removed.

---

### `void removeRight(char*& nb, size_t& size, size_t remLen)`

```cpp
void removeRight(char*& nb, size_t& size, size_t remLen);
```

**Description**

Removes characters from the right of a text.

**Parameters**

- **nb** — Number of elements.
- **size** — Requested size.
- **remLen** — Number of characters removed.

---

### `void addFrontZeros(char*& a, size_t& size, size_t nZeros)`

```cpp
void addFrontZeros(char*& a, size_t& size, size_t nZeros);
```

**Description**

Adds leading zeros to a text.

**Parameters**

- **a** — First operand.
- **size** — Requested size.
- **nZeros** — Number of zeros added.

---

### `void fillString(char*& str, const char* cstr, size_t size)`

```cpp
void fillString(char*& str, const char* cstr, size_t size);
```

**Description**

Copies a text into an already allocated buffer.

**Parameters**

- **str** — Text to modify.
- **cstr** — Source text.
- **size** — Requested size.

---

### `char* filledString(const char* cstr, size_t size)`

```cpp
char* filledString(const char* cstr, size_t size);
```

**Description**

Returns a text padded to the requested size.

**Parameters**

- **cstr** — Source text.
- **size** — Requested size.

**Returns**

Resulting text. The caller frees the memory.

---

### `CS_INLINE static char* csLimbsToDec(const void* raw, size_t n)`

```cpp
CS_INLINE static char* csLimbsToDec(const void* raw, size_t n);
```

**Description**

Converts limbs to decimal text.

**Parameters**

- **raw** — Parameter `raw`.
- **n** — Number of elements.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* formatReal(const csReal& a)`

```cpp
char* formatReal(const csReal& a);
```

**Description**

Prepares the display text of a real.

**Parameters**

- **a** — First operand.

**Returns**

Resulting text. The caller frees the memory.

---

### `void setRealPrecision(int precision)`

```cpp
void setRealPrecision(int precision);
```

**Description**

Sets the precision used for reals.

**Parameters**

- **precision** — Requested precision, in digits.

---

### `int realPrecision()`

```cpp
int realPrecision();
```

**Description**

Returns the precision used for reals.

**Returns**

Computed value.

---

### `template<class T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type = 0> csRational(T num, size_t denom=1) : csRational(csRaw_q{})`

```cpp
template<class T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type = 0> csRational(T num, size_t denom=1) : csRational(csRaw_q{});
```

**Description**

Builds a rational from a signed integer.

**Parameters**

- **num** — Signed integer of the numerator.
- **denom** — Positive denominator.

---

### `void init()`

```cpp
void init();
```

**Description**

Initializes the object or the library tables.

---

### `void set(const char* numerator, const char* denominator, bool sign)`

```cpp
void set(const char* numerator, const char* denominator, bool sign);
```

**Description**

Assigns a new value.

**Parameters**

- **numerator** — Decimal text of the numerator.
- **denominator** — Decimal text of the denominator.
- **sign** — Sign. Zero when the number is positive.

---

### `void set(size_t num, size_t denom=1, bool sign=0)`

```cpp
void set(size_t num, size_t denom=1, bool sign=0);
```

**Description**

Assigns a new value.

**Parameters**

- **num** — Parameter `num`.
- **denom** — Parameter `denom`.
- **sign** — Sign. Zero when the number is positive.

---

### `void assignValue(long num, size_t denom=1)`

```cpp
void assignValue(long num, size_t denom=1);
```

**Description**

Assigns a long integer.

**Parameters**

- **num** — Parameter `num`.
- **denom** — Parameter `denom`.

---

### `void reduce(size_t precision)`

```cpp
void reduce(size_t precision);
```

**Description**

Reduces the value to the requested size.

**Parameters**

- **precision** — Requested precision, in digits.

---

### `bool equalAbsolute(const csRational& a)`

```cpp
bool equalAbsolute(const csRational& a);
```

**Description**

Reports whether the absolute values are equal.

**Parameters**

- **a** — First operand.

**Returns**

True when the condition holds.

---

### `bool differAbsolute(const csRational& a)`

```cpp
bool differAbsolute(const csRational& a);
```

**Description**

Reports whether the absolute values differ.

**Parameters**

- **a** — First operand.

**Returns**

True when the condition holds.

---

### `bool greaterAbsolute(const csRational& a)`

```cpp
bool greaterAbsolute(const csRational& a);
```

**Description**

Reports whether the absolute value is greater.

**Parameters**

- **a** — First operand.

**Returns**

True when the condition holds.

---

### `bool lessAbsolute(const csRational& a)`

```cpp
bool lessAbsolute(const csRational& a);
```

**Description**

Reports whether the absolute value is less.

**Parameters**

- **a** — First operand.

**Returns**

True when the condition holds.

---

### `bool greaterOrEqualAbsolute(const csRational& a)`

```cpp
bool greaterOrEqualAbsolute(const csRational& a);
```

**Description**

Reports whether the absolute value is greater than or equal.

**Parameters**

- **a** — First operand.

**Returns**

True when the condition holds.

---

### `bool lessOrEqualAbsolute(const csRational& a)`

```cpp
bool lessOrEqualAbsolute(const csRational& a);
```

**Description**

Reports whether the absolute value is less than or equal.

**Parameters**

- **a** — First operand.

**Returns**

True when the condition holds.

---

### `operator csReal()`

```cpp
operator csReal();
```

**Description**

Real number as a mantissa and an exponent.

---

### `bool isZero()`

```cpp
bool isZero();
```

**Description**

Reports whether the value is zero.

**Returns**

True when the condition holds.

---

### `bool isNonZero()`

```cpp
bool isNonZero();
```

**Description**

Reports whether the value is non-zero.

**Returns**

True when the condition holds.

---

### `bool negativeSign() const`

```cpp
bool negativeSign() const;
```

**Description**

Returns the stored sign, without testing for zero.

**Returns**

True when the negative flag is set.

---

### `bool isNegative()`

```cpp
bool isNegative();
```

**Description**

Reports whether the value is strictly negative.

**Returns**

True when the sign is negative and the value is not zero.

---

### `void copy(const csRational& a)`

```cpp
void copy(const csRational& a);
```

**Description**

Copies the value.

**Parameters**

- **a** — First operand.

---

### `double getDouble()`

```cpp
double getDouble();
```

**Description**

Returns an approximation as a double.

**Returns**

Approximation as a double.

---

### `size_t maxLimbCount()`

```cpp
size_t maxLimbCount();
```

**Description**

Returns the larger of the two sizes.

**Returns**

Computed value.

---

### `char* decimalExpansion()`

```cpp
char* decimalExpansion();
```

**Description**

Returns the decimal expansion of the rational.

**Returns**

Resulting text. The caller frees the memory.

---

### `void print(const char*title)`

```cpp
void print(const char*title);
```

**Description**

Prints the value.

**Parameters**

- **title** — Title printed before the values.

---

### `void clear(const source_location loc = source_location::current())`

```cpp
void clear(const source_location loc = source_location::current());
```

**Description**

Releases the memory held by the object.

**Parameters**

- **loc** — Call site, for diagnostics.

---

### `struct CSARITHMETIC_API csInteger`

```cpp
struct CSARITHMETIC_API csInteger;
```

**Description**

Exact integer. One machine word while the value fits, then 32 internal limbs, then the heap.

---

### `void clear()`

```cpp
void clear();
```

**Description**

Sets the integer to zero and releases the limbs.

---

### `void copy(const csInteger& other)`

```cpp
void copy(const csInteger& other);
```

**Description**

Copies the value.

**Parameters**

- **other** — Source integer.

---

### `void set(long long number)`

```cpp
void set(long long number);
```

**Description**

Assigns a signed 64-bit integer.

**Parameters**

- **number** — Signed value.

---

### `void set(unsigned long long number, bool negativeSign = false)`

```cpp
void set(unsigned long long number, bool negativeSign = false);
```

**Description**

Assigns an unsigned 64-bit integer.

**Parameters**

- **number** — Absolute value.
- **negativeSign** — True when the result must be negative.

---

### `void set(const char* digits)`

```cpp
void set(const char* digits);
```

**Description**

Assigns a decimal text.

**Parameters**

- **digits** — Digits, with an optional sign.

---

### `void assignMagnitude(const uint32_t* digits, size_t limbCount, bool negativeSign)`

```cpp
void assignMagnitude(const uint32_t* digits, size_t limbCount, bool negativeSign);
```

**Description**

Assigns a magnitude in base 2^32, little-endian.

**Parameters**

- **digits** — Limbs. May be null.
- **limbCount** — Number of limbs.
- **negativeSign** — True when the result must be negative.

---

### `uint32_t modulo(uint32_t mod) const`

```cpp
uint32_t modulo(uint32_t mod) const;
```

**Description**

Remainder in [0, mod), sign included.

**Parameters**

- **mod** — Modulus.

**Returns**

Remainder.

---

### `size_t bitCount() const`

```cpp
size_t bitCount() const;
```

**Description**

Number of bits of the absolute value.

**Returns**

Number of bits.

---

### `bool isZero() const`

```cpp
bool isZero() const;
```

**Description**

Reports whether the value is zero.

**Returns**

True when the value is zero.

---

### `bool negativeSign() const`

```cpp
bool negativeSign() const;
```

**Description**

Returns the stored sign, without testing for zero.

**Returns**

True when the negative flag is set.

---

### `bool isNegative() const`

```cpp
bool isNegative() const;
```

**Description**

Reports whether the value is negative.

**Returns**

True when the value is strictly negative.

---

### `char* toDecimal() const`

```cpp
char* toDecimal() const;
```

**Description**

Returns the decimal digits, sign included.

**Returns**

Resulting text. The caller frees the memory.

---

### `void print(const char* title) const`

```cpp
void print(const char* title) const;
```

**Description**

Prints the value.

**Parameters**

- **title** — Title printed before the value.

---

### `void csPtrFree_q(csRational*& qn, size_t size)`

```cpp
void csPtrFree_q(csRational*& qn, size_t size);
```

**Description**

Releases an array of rationals.

**Parameters**

- **qn** — Array of rationals.
- **size** — Requested size.

---

### `void csPtrFree_r(csReal*& rn, size_t size)`

```cpp
void csPtrFree_r(csReal*& rn, size_t size);
```

**Description**

Releases an array of reals.

**Parameters**

- **rn** — Array of reals.
- **size** — Requested size.

---

### `void init()`

```cpp
void init();
```

**Description**

Initializes the object or the library tables.

---

### `void set(const char* mantissa, int exponent, bool sign)`

```cpp
void set(const char* mantissa, int exponent, bool sign);
```

**Description**

Assigns a new value.

**Parameters**

- **mantissa** — Decimal text of the mantissa.
- **exponent** — Decimal exponent.
- **sign** — Sign. Zero when the number is positive.

---

### `void set(const char* mantissa, int exponent, bool sign, int repeat)`

```cpp
void set(const char* mantissa, int exponent, bool sign, int repeat);
```

**Description**

Assigns a repeated mantissa to an existing real.

**Parameters**

- **mantissa** — Decimal text of the mantissa.
- **exponent** — Decimal exponent.
- **sign** — Sign. Zero when the number is positive.
- **repeat** — Number of times the mantissa is repeated. A value below 1 yields zero.

---

### `void set(const char* mantissa, int exponent, bool sign, int repeat, int pos)`

```cpp
void set(const char* mantissa, int exponent, bool sign, int repeat, int pos);
```

**Description**

Assigns a repeated section of the mantissa to an existing real.

**Parameters**

- **mantissa** — Decimal text of the mantissa.
- **exponent** — Decimal exponent.
- **sign** — Sign. Zero when the number is positive.
- **repeat** — Number of times the section is repeated. A value below 1 yields zero.
- **pos** — Repeated section. Positive: from the first character through the pos-th, then the rest. Negative: from the pos-th character from the end through the last character, placed at the end.

---

### `void set(const char* mantissa, int exponent, bool sign, int repeat, int length, int index)`

```cpp
void set(const char* mantissa, int exponent, bool sign, int repeat, int length, int index);
```

**Description**

Assigns a repeated slice of the mantissa to an existing real.

**Parameters**

- **mantissa** — Decimal text of the mantissa.
- **exponent** — Decimal exponent.
- **sign** — Sign. Zero when the number is positive.
- **repeat** — Number of times the slice is repeated. A value below 1 yields zero.
- **length** — Length of the slice, in characters.
- **index** — Start index of the slice, from zero.

---

### `void set(unsigned long mantissa, int exponent, bool sign)`

```cpp
void set(unsigned long mantissa, int exponent, bool sign);
```

**Description**

Assigns a new value.

**Parameters**

- **mantissa** — Decimal text of the mantissa.
- **exponent** — Decimal exponent.
- **sign** — Sign. Zero when the number is positive.

---

### `void set(long mantissa, int exponent)`

```cpp
void set(long mantissa, int exponent);
```

**Description**

Assigns a new value.

**Parameters**

- **mantissa** — Decimal text of the mantissa.
- **exponent** — Decimal exponent.

---

### `void set(bool evaluate, const char* number)`

```cpp
void set(bool evaluate, const char* number);
```

**Description**

Assigns a new value.

**Parameters**

- **evaluate** — Interprets the text as a number.
- **number** — Text of the number.

---

### `void setPrecision(int precision)`

```cpp
void setPrecision(int precision);
```

**Description**

Sets the precision of this real.

**Parameters**

- **precision** — Requested precision, in digits.

---

### `void setDecimalPlaces(int size)`

```cpp
void setDecimalPlaces(int size);
```

**Description**

Forces the number of mantissa digits.

**Parameters**

- **size** — Requested size.

---

### `void extendMantissa(size_t size)`

```cpp
void extendMantissa(size_t size);
```

**Description**

Increases the number of mantissa digits.

**Parameters**

- **size** — Requested size.

---

### `void shortenMantissa(size_t size)`

```cpp
void shortenMantissa(size_t size);
```

**Description**

Reduces the number of mantissa digits.

**Parameters**

- **size** — Requested size.

---

### `void resizeMantissa(size_t newMantissaSize)`

```cpp
void resizeMantissa(size_t newMantissaSize);
```

**Description**

Reshapes the mantissa to the requested form.

**Parameters**

- **newMantissaSize** — Parameter `newMantissaSize`.

---

### `void trimTrailingZeros()`

```cpp
void trimTrailingZeros();
```

**Description**

Removes cosmetic zeros from the mantissa.

---

### `void reduce(size_t size)`

```cpp
void reduce(size_t size);
```

**Description**

Reduces the value to the requested size.

**Parameters**

- **size** — Requested size.

---

### `size_t digitCount()`

```cpp
size_t digitCount();
```

**Description**

Returns the number of mantissa digits.

**Returns**

Computed value.

---

### `operator csRational()`

```cpp
operator csRational();
```

**Description**

Exact rational, numerator and denominator in limbs.

---

### `void copy(const csReal& a)`

```cpp
void copy(const csReal& a);
```

**Description**

Copies the value.

**Parameters**

- **a** — First operand.

---

### `bool isZero()`

```cpp
bool isZero();
```

**Description**

Sets the real to zero.

**Returns**

True when the condition holds.

---

### `bool negativeSign() const`

```cpp
bool negativeSign() const;
```

**Description**

Returns the stored sign, without testing for zero.

**Returns**

True when the negative flag is set.

---

### `bool isNegative()`

```cpp
bool isNegative();
```

**Description**

Reports whether the value is strictly negative.

**Returns**

True when the sign is negative and the value is not zero.

---

### `void print(const char* title)`

```cpp
void print(const char* title);
```

**Description**

Prints the value.

**Parameters**

- **title** — Title printed before the values.

---

### `void printScientific(const char* title)`

```cpp
void printScientific(const char* title);
```

**Description**

Prints the value with an extended format.

**Parameters**

- **title** — Title printed before the values.

---

### `void copyFrom(const csReal& a)`

```cpp
void copyFrom(const csReal& a);
```

**Description**

Assigns another real to this object.

**Parameters**

- **a** — First operand.

---

### `void setDigit(size_t id, char digit)`

```cpp
void setDigit(size_t id, char digit);
```

**Description**

Assigns a new value.

**Parameters**

- **id** — Index of the element.
- **digit** — Digit to write.

---

### `void setIntegerDigit(size_t id, char digit)`

```cpp
void setIntegerDigit(size_t id, char digit);
```

**Description**

Assigns the integer part.

**Parameters**

- **id** — Index of the element.
- **digit** — Digit to write.

---

### `void setFractionalDigit(size_t id, char digit)`

```cpp
void setFractionalDigit(size_t id, char digit);
```

**Description**

Assigns the fractional part.

**Parameters**

- **id** — Index of the element.
- **digit** — Digit to write.

---

### `void setPowerOfTen(long power)`

```cpp
void setPowerOfTen(long power);
```

**Description**

Assigns a power of ten.

**Parameters**

- **power** — Power of ten.

---

### `void random(size_t nDigits, char digitMin=0, char digitMax=9, long exponent=0, bool sign=0)`

```cpp
void random(size_t nDigits, char digitMin=0, char digitMax=9, long exponent=0, bool sign=0);
```

**Description**

Draws a random value.

**Parameters**

- **nDigits** — Number of digits.
- **digitMin** — Parameter `digitMin`.
- **digitMax** — Parameter `digitMax`.
- **exponent** — Decimal exponent.
- **sign** — Sign. Zero when the number is positive.

---

### `size_t significantZeroCount(size_t initialPos=0)`

```cpp
size_t significantZeroCount(size_t initialPos=0);
```

**Description**

Counts the significant zeros of the mantissa.

**Parameters**

- **initialPos** — Parameter `initialPos`.

**Returns**

Computed value.

---

### `void clear()`

```cpp
void clear();
```

**Description**

Releases the memory held by the object.

---

### `struct csComplex`

```cpp
struct csComplex;
```

**Description**

Complex number a + bi. Both parts are reals.
The sign of each part is read on that part: real.negativeSign(), imag.isNegative().
Division by zero yields zero, as for the integer.

---

### `void set(const csReal& re, const csReal& im)`

```cpp
void set(const csReal& re, const csReal& im);
```

**Description**

Assigns both parts.

---

### `void set(long re, long im)`

```cpp
void set(long re, long im);
```

**Description**

Assigns two integers.

---

### `void set(double re, double im)`

```cpp
void set(double re, double im);
```

**Description**

Assigns two machine decimals.

---

### `void copy(const csComplex& other)`

```cpp
void copy(const csComplex& other);
```

**Description**

Copies the value.

---

### `void clear()`

```cpp
void clear();
```

**Description**

Releases the memory of both parts.

---

### `bool isZero()`

```cpp
bool isZero();
```

**Description**

Reports whether both parts are zero.

---

### `bool isReal()`

```cpp
bool isReal();
```

**Description**

Reports whether the imaginary part is zero.

---

### `bool isImaginary()`

```cpp
bool isImaginary();
```

**Description**

Reports whether the real part is zero.

---

### `static csComplex imaginaryUnit()`

```cpp
static csComplex imaginaryUnit();
```

**Description**

Returns the imaginary unit.

---

### `void print(const char* title)`

```cpp
void print(const char* title);
```

**Description**

Prints the value.

**Parameters**

- **title** — Title printed before the values.

---

### `struct csComplex_q`

```cpp
struct csComplex_q;
```

**Description**

Complex number a + bi. Both parts are exact rationals.
The sign of each part is read on that part: real.negativeSign(), imag.isNegative().
Division by zero yields zero. The squared modulus stays rational; the modulus itself is a real.

---

### `void set(const csRational& re, const csRational& im)`

```cpp
void set(const csRational& re, const csRational& im);
```

**Description**

Assigns both parts.

---

### `void set(long re, long im)`

```cpp
void set(long re, long im);
```

**Description**

Assigns two integers.

---

### `void copy(const csComplex_q& other)`

```cpp
void copy(const csComplex_q& other);
```

**Description**

Copies the value.

---

### `void clear()`

```cpp
void clear();
```

**Description**

Releases the memory of both parts.

---

### `bool isZero()`

```cpp
bool isZero();
```

**Description**

Reports whether both parts are zero.

---

### `bool isReal()`

```cpp
bool isReal();
```

**Description**

Reports whether the imaginary part is zero.

---

### `bool isImaginary()`

```cpp
bool isImaginary();
```

**Description**

Reports whether the real part is zero.

---

### `static csComplex_q imaginaryUnit()`

```cpp
static csComplex_q imaginaryUnit();
```

**Description**

Returns the imaginary unit.

---

### `void print(const char* title)`

```cpp
void print(const char* title);
```

**Description**

Prints the value.

**Parameters**

- **title** — Title printed before the values.

---

### `void init()`

```cpp
void init();
```

**Description**

Initializes the object or the library tables.

---

### `void printDigits(char* nb, const char* separator=" ")`

```cpp
void printDigits(char* nb, const char* separator=" ");
```

**Description**

Prints an array of digits.

**Parameters**

- **nb** — Number of elements.
- **separator** — Parameter `separator`.

---

### `uchar digitPairIndex(uchar tens, uchar units)`

```cpp
uchar digitPairIndex(uchar tens, uchar units);
```

**Description**

Returns the index of a digit pair.

**Parameters**

- **tens** — Parameter `tens`.
- **units** — Parameter `units`.

**Returns**

Result.

---

### `uchar digitTripleIndex(int cents, uchar tens, uchar units)`

```cpp
uchar digitTripleIndex(int cents, uchar tens, uchar units);
```

**Description**

Returns the index of a digit triple.

**Parameters**

- **cents** — Parameter `cents`.
- **tens** — Parameter `tens`.
- **units** — Parameter `units`.

**Returns**

Result.

---

### `csBIDIGITS subtractionStep(int i)`

```cpp
csBIDIGITS subtractionStep(int i);
```

**Description**

Transforms the digits of a subtraction.

**Parameters**

- **i** — Parameter `i`.

**Returns**

Result.

---

### `void multiplyDigitBase2(char* a, uchar b, char*& result, size_t aSize, size_t resSize,csBIDIGITS& prevCarry, size_t m, size_t n)`

```cpp
void multiplyDigitBase2(char* a, uchar b, char*& result, size_t aSize, size_t resSize,csBIDIGITS& prevCarry, size_t m, size_t n);
```

**Description**

Computes a decimal product in the secondary base.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **result** — Parameter `result`.
- **aSize** — Number of digits of the first text.
- **resSize** — Parameter `resSize`.
- **prevCarry** — Parameter `prevCarry`.
- **m** — Number of evaluation points.
- **n** — Number of elements.

---

### `void addShiftedProduct(char* a, char* b, char*& result, size_t opSize, size_t resSize)`

```cpp
void addShiftedProduct(char* a, char* b, char*& result, size_t opSize, size_t resSize);
```

**Description**

Adds two texts during a multiplication.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **result** — Parameter `result`.
- **opSize** — Number of aligned digits.
- **resSize** — Parameter `resSize`.

---

### `void subtractPartialQuotient(char* a, char* b, char*& result, size_t opSize, size_t frontOffset)`

```cpp
void subtractPartialQuotient(char* a, char* b, char*& result, size_t opSize, size_t frontOffset);
```

**Description**

Subtracts two texts during a division.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **result** — Parameter `result`.
- **opSize** — Number of aligned digits.
- **frontOffset** — Parameter `frontOffset`.

---

### `void multiplyDigitForDivision(char* a, uchar b, char*& result, size_t aSize, size_t resSize)`

```cpp
void multiplyDigitForDivision(char* a, uchar b, char*& result, size_t aSize, size_t resSize);
```

**Description**

Multiplies during a division, in the first base.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **result** — Parameter `result`.
- **aSize** — Number of digits of the first text.
- **resSize** — Parameter `resSize`.

---

### `void multiplyDigitPairForDivision(char* a, uchar b, char*& result, size_t aSize, size_t resSize)`

```cpp
void multiplyDigitPairForDivision(char* a, uchar b, char*& result, size_t aSize, size_t resSize);
```

**Description**

Multiplies during a division, in the second base.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **result** — Parameter `result`.
- **aSize** — Number of digits of the first text.
- **resSize** — Parameter `resSize`.

---

### `csBIDIGITS** packDigitPairs(int** opTable)`

```cpp
csBIDIGITS** packDigitPairs(int** opTable);
```

**Description**

Groups decimal digits into pairs.

**Parameters**

- **opTable** — Parameter `opTable`.

**Returns**

Result.

---

### `void buildMultiplicationTable()`

```cpp
void buildMultiplicationTable();
```

**Description**

Builds the multiplication table.

---

### `void buildAdditionTable()`

```cpp
void buildAdditionTable();
```

**Description**

Builds the addition table.

---

### `void buildDivisionTable()`

```cpp
void buildDivisionTable();
```

**Description**

Builds the division table.

---

### `void buildSubtractionTable()`

```cpp
void buildSubtractionTable();
```

**Description**

Builds the subtraction table.

---

### `void printDigitTable(char* opName)`

```cpp
void printDigitTable(char* opName);
```

**Description**

Prints a digit table.

**Parameters**

- **opName** — Parameter `opName`.

---

### `void addDecimal(char* a, char* b, char*& result, size_t opSize, size_t& resSize)`

```cpp
void addDecimal(char* a, char* b, char*& result, size_t opSize, size_t& resSize);
```

**Description**

Adds two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **result** — Parameter `result`.
- **opSize** — Number of aligned digits.
- **resSize** — Parameter `resSize`.

---

### `char* addDecimal(char* a,char* b)`

```cpp
char* addDecimal(char* a,char* b);
```

**Description**

Adds two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* addDecimal(char* a,char* b, size_t& resSize)`

```cpp
char* addDecimal(char* a,char* b, size_t& resSize);
```

**Description**

Adds two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **resSize** — Parameter `resSize`.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* addDecimal(const char* a,const char* b)`

```cpp
char* addDecimal(const char* a,const char* b);
```

**Description**

Adds two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.

**Returns**

Resulting text. The caller frees the memory.

---

### `void subtractDecimal(char* a, char* b, char*& result, size_t opSize)`

```cpp
void subtractDecimal(char* a, char* b, char*& result, size_t opSize);
```

**Description**

Subtracts two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **result** — Parameter `result`.
- **opSize** — Number of aligned digits.

---

### `char* subtractDecimal(char* a,char* b)`

```cpp
char* subtractDecimal(char* a,char* b);
```

**Description**

Subtracts two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* subtractDecimal(char* a,char* b, bool& sign)`

```cpp
char* subtractDecimal(char* a,char* b, bool& sign);
```

**Description**

Subtracts two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **sign** — Sign. Zero when the number is positive.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize)`

```cpp
char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize);
```

**Description**

Subtracts two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize, bool&sign)`

```cpp
char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize, bool&sign);
```

**Description**

Subtracts two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **sign** — Sign. Zero when the number is positive.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* subtractDecimal(const char* a,const char*b)`

```cpp
char* subtractDecimal(const char* a,const char*b);
```

**Description**

Subtracts two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize, size_t& resSize, bool& sign)`

```cpp
char* subtractDecimal(char* a,char* b, size_t aSize, size_t bSize, size_t& resSize, bool& sign);
```

**Description**

Subtracts two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **resSize** — Parameter `resSize`.
- **sign** — Sign. Zero when the number is positive.

**Returns**

Resulting text. The caller frees the memory.

---

### `void multiplyByDigit(char* a, uchar b, char*& result, size_t aSize, size_t resSize)`

```cpp
void multiplyByDigit(char* a, uchar b, char*& result, size_t aSize, size_t resSize);
```

**Description**

Computes a product in the current base.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **result** — Parameter `result`.
- **aSize** — Number of digits of the first text.
- **resSize** — Parameter `resSize`.

---

### `void multiplyDecimal(char* a, char* b, char*& result, char*& tmpResult, size_t aSize, size_t bSize, size_t resSize)`

```cpp
void multiplyDecimal(char* a, char* b, char*& result, char*& tmpResult, size_t aSize, size_t bSize, size_t resSize);
```

**Description**

Multiplies two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **result** — Parameter `result`.
- **tmpResult** — Parameter `tmpResult`.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **resSize** — Parameter `resSize`.

---

### `char* multiplyDecimal(char* a, char* b)`

```cpp
char* multiplyDecimal(char* a, char* b);
```

**Description**

Multiplies two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* multiplyDecimal(char* a, char* b, size_t aSize, size_t bSize)`

```cpp
char* multiplyDecimal(char* a, char* b, size_t aSize, size_t bSize);
```

**Description**

Multiplies two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* multiplyDecimal(const char* a, const char* b)`

```cpp
char* multiplyDecimal(const char* a, const char* b);
```

**Description**

Multiplies two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* multiplyDecimal(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize)`

```cpp
char* multiplyDecimal(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize);
```

**Description**

Multiplies two decimal texts.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **resSize** — Parameter `resSize`.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* divideDecimal(char*a, char* b, char*& remain)`

```cpp
char* divideDecimal(char*a, char* b, char*& remain);
```

**Description**

Divides two decimal texts and returns the quotient.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **remain** — Receives the remainder of the division.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* divideDecimal(char*a, char* b)`

```cpp
char* divideDecimal(char*a, char* b);
```

**Description**

Divides two decimal texts and returns the quotient.

**Parameters**

- **a** — First operand.
- **b** — Second operand.

**Returns**

Resulting text. The caller frees the memory.

---

### `char const* divideDecimal(char* a, char* b, char*& result, char*& remain, size_t& aSize, size_t& bSize, size_t& resSize, size_t& remSize)`

```cpp
char const* divideDecimal(char* a, char* b, char*& result, char*& remain, size_t& aSize, size_t& bSize, size_t& resSize, size_t& remSize);
```

**Description**

Divides two decimal texts and returns the quotient.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **result** — Parameter `result`.
- **remain** — Receives the remainder of the division.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **resSize** — Parameter `resSize`.
- **remSize** — Parameter `remSize`.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* divideDecimal(const char* a, const char* b, char*&remain)`

```cpp
char* divideDecimal(const char* a, const char* b, char*&remain);
```

**Description**

Divides two decimal texts and returns the quotient.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **remain** — Receives the remainder of the division.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* divideDecimal(const char* a, const char* b)`

```cpp
char* divideDecimal(const char* a, const char* b);
```

**Description**

Divides two decimal texts and returns the quotient.

**Parameters**

- **a** — First operand.
- **b** — Second operand.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* divideDecimal(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize)`

```cpp
char* divideDecimal(char* a, char* b, size_t aSize, size_t bSize, size_t& resSize);
```

**Description**

Divides two decimal texts and returns the quotient.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **resSize** — Parameter `resSize`.

**Returns**

Resulting text. The caller frees the memory.

---

### `char const* divideWithFraction(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals)`

```cpp
char const* divideWithFraction(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals);
```

**Description**

Divides two decimal texts and keeps a fractional part.

**Parameters**

- **_a** — First operand.
- **_b** — Second operand.
- **resInt** — Parameter `resInt`.
- **resDec** — Parameter `resDec`.
- **remain** — Receives the remainder of the division.
- **_aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **resSize** — Parameter `resSize`.
- **resIntSize** — Parameter `resIntSize`.
- **resDecSize** — Parameter `resDecSize`.
- **remSize** — Parameter `remSize`.
- **nDecimals** — Parameter `nDecimals`.

**Returns**

Resulting text. The caller frees the memory.

---

### `char const* divideWithScale(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals)`

```cpp
char const* divideWithScale(char*_a, char* _b, char*& resInt, char*& resDec, char*& remain, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t& remSize, size_t nDecimals);
```

**Description**

Computes a real division, second variant.

**Parameters**

- **_a** — First operand.
- **_b** — Second operand.
- **resInt** — Parameter `resInt`.
- **resDec** — Parameter `resDec`.
- **remain** — Receives the remainder of the division.
- **_aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **resSize** — Parameter `resSize`.
- **resIntSize** — Parameter `resIntSize`.
- **resDecSize** — Parameter `resDecSize`.
- **remSize** — Parameter `remSize`.
- **nDecimals** — Parameter `nDecimals`.

**Returns**

Resulting text. The caller frees the memory.

---

### `char const* divideWithScale(char*_a, char* _b, char*& resInt, char*& resDec, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t nDecimals)`

```cpp
char const* divideWithScale(char*_a, char* _b, char*& resInt, char*& resDec, size_t _aSize, size_t bSize, size_t& resSize, size_t& resIntSize, size_t& resDecSize, size_t nDecimals);
```

**Description**

Computes a real division, second variant.

**Parameters**

- **_a** — First operand.
- **_b** — Second operand.
- **resInt** — Parameter `resInt`.
- **resDec** — Parameter `resDec`.
- **_aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **resSize** — Parameter `resSize`.
- **resIntSize** — Parameter `resIntSize`.
- **resDecSize** — Parameter `resDecSize`.
- **nDecimals** — Parameter `nDecimals`.

**Returns**

Resulting text. The caller frees the memory.

---

### `char const* divideDecimal(char*a, char* b, char*& result, size_t& aSize, size_t& bSize, size_t& resSize)`

```cpp
char const* divideDecimal(char*a, char* b, char*& result, size_t& aSize, size_t& bSize, size_t& resSize);
```

**Description**

Divides two decimal texts and returns the quotient.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **result** — Parameter `result`.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **resSize** — Parameter `resSize`.

**Returns**

Resulting text. The caller frees the memory.

---

### `char const* divideWithFraction(char* a, char* b, char*& resInt, char*& resDec, char*& remain, size_t nDecimals)`

```cpp
char const* divideWithFraction(char* a, char* b, char*& resInt, char*& resDec, char*& remain, size_t nDecimals);
```

**Description**

Divides two decimal texts and keeps a fractional part.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **resInt** — Parameter `resInt`.
- **resDec** — Parameter `resDec`.
- **remain** — Receives the remainder of the division.
- **nDecimals** — Parameter `nDecimals`.

**Returns**

Resulting text. The caller frees the memory.

---

### `char const* remainderDecimal(char*a, char* b, char*& remain, size_t& aSize, size_t& bSize, size_t& remSize)`

```cpp
char const* remainderDecimal(char*a, char* b, char*& remain, size_t& aSize, size_t& bSize, size_t& remSize);
```

**Description**

Computes the remainder of a rational division.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **remain** — Receives the remainder of the division.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **remSize** — Parameter `remSize`.

**Returns**

Resulting text. The caller frees the memory.

---

### `void remainderDecimal(char*a, char* b, char*&remain)`

```cpp
void remainderDecimal(char*a, char* b, char*&remain);
```

**Description**

Computes the remainder of a rational division.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **remain** — Receives the remainder of the division.

---

### `char* remainderDecimal(char*a, char* b)`

```cpp
char* remainderDecimal(char*a, char* b);
```

**Description**

Computes the remainder of a rational division.

**Parameters**

- **a** — First operand.
- **b** — Second operand.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* remainderDecimal(const char*a, const char* b)`

```cpp
char* remainderDecimal(const char*a, const char* b);
```

**Description**

Computes the remainder of a rational division.

**Parameters**

- **a** — First operand.
- **b** — Second operand.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* gcd(const char* a, const char* b, size_t& gcdSize)`

```cpp
char* gcd(const char* a, const char* b, size_t& gcdSize);
```

**Description**

Computes the greatest common divisor.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **gcdSize** — Parameter `gcdSize`.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* gcd(char* a, char* b, size_t& gcdSize)`

```cpp
char* gcd(char* a, char* b, size_t& gcdSize);
```

**Description**

Computes the greatest common divisor.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **gcdSize** — Parameter `gcdSize`.

**Returns**

Resulting text. The caller frees the memory.

---

### `char* gcd(char* a, char* b, size_t aSize, size_t bSize, size_t& gcdSize)`

```cpp
char* gcd(char* a, char* b, size_t aSize, size_t bSize, size_t& gcdSize);
```

**Description**

Computes the greatest common divisor.

**Parameters**

- **a** — First operand.
- **b** — Second operand.
- **aSize** — Number of digits of the first text.
- **bSize** — Number of digits of the second text.
- **gcdSize** — Parameter `gcdSize`.

**Returns**

Resulting text. The caller frees the memory.

---

### `void csSortMinR(csReal*& rn, size_t size)`

```cpp
void csSortMinR(csReal*& rn, size_t size);
```

**Description**

Sorts reals into increasing order.

**Parameters**

- **rn** — Array of reals.
- **size** — Requested size.

---

### `void csSortMin_CoordsByRNumber(csFCOORDS*& fc, csReal*& rn, size_t size)`

```cpp
void csSortMin_CoordsByRNumber(csFCOORDS*& fc, csReal*& rn, size_t size);
```

**Description**

Sorts points by increasing real.

**Parameters**

- **fc** — Parameter `fc`.
- **rn** — Array of reals.
- **size** — Requested size.

---

### `void csSortMax_CoordsByRNumber(csFCOORDS*& fc, csReal*& rn, size_t size)`

```cpp
void csSortMax_CoordsByRNumber(csFCOORDS*& fc, csReal*& rn, size_t size);
```

**Description**

Sorts points by decreasing real.

**Parameters**

- **fc** — Parameter `fc`.
- **rn** — Array of reals.
- **size** — Requested size.

---

### `CSARITHMETIC_API CSARITHMETIC::csRational csReduce(CSARITHMETIC_API CSARITHMETIC::csRational a, size_t sizeCondition)`

```cpp
CSARITHMETIC_API CSARITHMETIC::csRational csReduce(CSARITHMETIC_API CSARITHMETIC::csRational a, size_t sizeCondition);
```

**Description**

Reduces the value to the requested size.

**Parameters**

- **a** — First operand.
- **sizeCondition** — Parameter `sizeCondition`.

**Returns**

Resulting rational.

---
