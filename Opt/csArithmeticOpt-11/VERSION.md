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
