# Version 9

Built from version 8 (base 10^9, Karatsuba above 32 limbs, recursive division above 48 limbs).

## Changes

Limbs are in base 2^32. Input and display stay in base 10: the text is converted at construction, then rewritten only by the display operator. In the loops, moving from one limb to the next is a 32-bit shift.

Karatsuba remains the multiplication used up to 4 096 limbs (about 39 000 digits). Above that, Toom-3 takes over (five products of one third of the size). The threshold is the one found by sweeping: below it, the additions and the exact divisions by 2 and by 3 of Toom-3 cost more than the saving on the number of products. Recursive division is unchanged in its scheme; it calls the same multiplication.

## Performance

One operation (product then division), warm-up excluded, same machine, versions 8 and 9 run one after the other.

| digits | version 8 | version 9 |
|---:|---:|---:|
| 2 000 | 0.151 ms | 0.155 ms |
| 8 000 | 1.39 ms | 1.22 ms |
| 32 000 | 11.9 ms | 10.9 ms |

At 2 000 digits the gap is noise. At 8 000 and 32 000 digits, base 2^32 gains about 12% and 8%.

## Real numbers

The mantissa of `csRNUMBER` stays in limbs between operations. Decimal text is produced only when constructing from a string, and on display. The exponent remains a power of ten. The product multiplies the limbs and adds the exponents. Division remains the integer quotient of the mantissas, then the default precision (20 decimals).

Product of two integers of the same size, warm-up excluded. "Before" is digit-by-digit multiplication. "Text" is the version that converted the mantissa back on every operation.

| digits | before | text | limbs | Python `int` |
|---:|---:|---:|---:|---:|
| 500 | 2.46 ms | 0.050 ms | 0.0049 ms | 0.0032 ms |
| 2 000 | 39.7 ms | 0.44 ms | 0.058 ms | 0.048 ms |
| 8 000 | — | 6.17 ms | 0.34 ms | 0.38 ms |
| 32 000 | — | 100 ms | 3.09 ms | 3.18 ms |

Division at 32 000 digits takes 3.13 ms in C++ and 1.95 ms in Python.
