# Version 7

Built from version 5 (base 10^9). Version 6 (base 10^19) is slower and is not carried forward.

## Changes

Schoolbook multiplication remains the base case. As soon as both operands have at least 32 limbs (288 digits), it is replaced by Karatsuba: three recursive products of half the size, plus additions. Recursion temporaries come from a single allocation per product.

The threshold of 32 limbs is the one found by sweeping: at 39 limbs the time matches the schoolbook method, and beyond a few hundred limbs it is the fastest of the thresholds tried (16, 24, 32, 48, 64, 96).

Division remains Knuth's.

## Performance

Same benchmark as version 1 (40 products of 350-digit integers and one division, then 8 fits).

| | version 5 | version 7 |
|---|---:|---:|
| product and division | 0.44 ms | 0.47 ms |
| fit | 66–72 ms | 59 ms |
| total | 66–73 ms | 60 ms |

The short benchmark barely changes: 350 digits are 39 limbs, just above the threshold, and the fit handles small rationals that stay schoolbook.

One operation (product then division) on longer integers:

| digits | version 5 | version 7 |
|---:|---:|---:|
| 350 | 0.011 ms | 0.014 ms |
| 2 000 | 0.179 ms | 0.154 ms |
| 8 000 | 2.55 ms | 1.54 ms |
| 32 000 | 39.2 ms | 18.5 ms |

At 32 000 digits, multiplication alone goes from 26 ms to 5.9 ms. The full operation gains only a factor of about 2.1, because Knuth division still takes most of the time.
