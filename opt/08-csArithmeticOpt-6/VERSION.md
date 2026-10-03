# Version 6

Built from version 5. The base goes from 10^9 to 10^19.

## Changes

Each limb is a `uint64_t` of 19 digits. 10^19 fits in an unsigned 64-bit integer, but the sum of two limbs and the product overflow: the accumulator is an `__int128`. Addition, subtraction and Knuth division were rewritten with that accumulator. Multiplication stays schoolbook.

As in version 5, the decimal string exists only on input and on display. This version's header is required in order to print the limbs.

## Performance

Same benchmark as version 1 (40 products of 350-digit integers and one division, then 8 fits).

| | version 5 | version 6 |
|---|---:|---:|
| product and division | 0.44 ms | 1.22 ms |
| fit | 66–72 ms | 101–103 ms |
| total | 66–73 ms | 103–105 ms |

About 1.5 times slower on the fit, and about 2.7 times slower on the 350-digit product.

The ratio does not tighten as the integers grow. One operation (product then division):

| digits | version 5 | version 6 | ratio |
|---:|---:|---:|---:|
| 350 | 0.012 ms | 0.032 ms | 2.7 |
| 1 000 | 0.060 ms | 0.196 ms | 3.3 |
| 2 000 | 0.184 ms | 0.630 ms | 3.4 |
| 4 000 | 0.84 ms | 2.53 ms | 3.0 |
| 8 000 | 2.55 ms | 9.27 ms | 3.6 |
| 16 000 | 10.2 ms | 34.6 ms | 3.4 |
| 32 000 | 39 ms | 137 ms | 3.5 |

There are (19/9)^2 ≈ 4.5 times fewer schoolbook products, but each 128-bit step costs more than that saving. The ratio settles around 3.4 in favour of base 10^9.
