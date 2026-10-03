# Version 13

Takes version 12. The algorithms for integers, rationals and reals do not change.

## Complex

`csComplex` is a number a + bi. Both parts are `csReal` values, reached through `real` and `imag`. The operations `+`, `-`, `*`, `/` exist as an expression and as the in-place forms `+=`, `-=`, `*=`, `/=`, including with a real, an integer or a double. `conjugate` returns a - bi, `norm` the squared modulus, `abs` the modulus, `power` an integer power, `reciprocal` the inverse. `0^0` is 1. Division by zero yields zero.

The sign is not carried by the complex number. It is read on each part, with `negativeSign` and `isNegative`.

`csComplex_q` is the same number, with two `csRational` parts. Sums, products, quotients and integer powers stay exact. `norm` is the squared modulus, still rational. `abs` converts that square to a real and takes its square root.

The modulus uses a short Newton iteration written with the real operators. The personal root experiments are not part of this edition.

# Version 12

Takes version 11. The algorithms and the thresholds do not change.

## Exact integer

`csInteger` is the set of integers. While the value fits in 64 bits, it stays in one word. Up to 32 limbs, it stays inside the object. Beyond that, it moves to the heap. Division yields the quotient truncated toward zero, and the remainder has the sign of the dividend. `toRational` produces a rational with denominator 1.

## Names

Methods whose name did not say the operation were renamed. Those that already said it stay: `isZero`, `copy`, `print`, `clear`, `reduce`, `abs`, `setPrecision`, `integer`, `integerQuotient`.

Decimal texts follow the same verb: `addDecimal`, `subtractDecimal`, `multiplyDecimal`, `divideDecimal`, `divideWithFraction`, `remainderDecimal`. Personal root and trial functions, `csSqrt`, `NRSqrt`, `nsr`, `aplr`, `findGcd`, kept their names in the working tree. They are omitted from this published edition.

Old names remain callable and are marked `[[deprecated]]`. The warning is issued at compile time. The call is forwarded to the new name, at no extra cost.

`negativeSign`, on the integer, the rational and the real, reads the stored flag. `isNegative` is true only when that flag is set and the value is not zero.

# Version 11

Takes version 10 without changing the algorithms or the thresholds.

## Names

The types `csQNUMBER` and `csRNUMBER` become `csRational` and `csReal`.
Engine functions and variables start with `cs`.
Those for rationals end with `_q`, those for reals with `_r`.
Engine files follow the same rule: `csBase_q.inl`, `csFft_q.inl`, `csNewton_q.inl`, `csMethods_q.inl`, `csMethods_r.inl`.

Each function carries an `@brief`, `@param` and `@return` comment, on the model of csParallelTask, for the generated documentation.
`csAddShifted` and a mantissa store that had no caller were removed.

# Version 10

Built from version 9 (base 2^32, Karatsuba, Toom-3 above 4 096 limbs, recursive division).

## Changes

Multiplication of large integers goes through a modular Fourier transform. Each 32-bit limb is split into two 16-bit digits. The convolution product fits in a single modulus (`2^28 * 8589934597 + 1`), then the carry rebuilds base 2^32. The threshold is 1 024 limbs, about 10 000 digits. Below it, Karatsuba or Toom-3 stay in place. Rationals and reals call the same limb product.

Newton division is in the engine: an exact reciprocal of `2^(2n) / v`, then a quotient by blocks, with a correction of a few units. It is slower than recursive division up to at least 256 000 digits, because its intermediate products and its shifts cost more than the gain. It therefore starts only from 100 000 limbs, about one million digits, for the divisor and for the quotient. Below that, recursive division stays in place and benefits from Fourier products as soon as its multiplications pass the threshold.

## Performance

Product of two integers of the same size, then division of an integer twice as long by an integer of that size. Warm-up excluded.

| digits | product v9 | product v10 | division v9 | division v10 |
|---:|---:|---:|---:|---:|
| 8 000 | 0.29 ms | 0.53 ms | 1.24 ms | 2.03 ms |
| 32 000 | 2.91 ms | 2.17 ms | 10.7 ms | 13.5 ms |
| 64 000 | 12.0 ms | 4.71 ms | 31.8 ms | 33.9 ms |
| 128 000 | 28.0 ms | 9.93 ms | 100 ms | 79.5 ms |
| 256 000 | — | — | 254 ms | 231 ms |

At 8 000 digits both operands are under the Fourier threshold. From 32 000 digits the product is shorter, and the gap grows: at 128 000 digits it is about 2.8 times shorter. Recursive division follows from 128 000 digits.
