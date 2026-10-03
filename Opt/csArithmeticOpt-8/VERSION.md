# Version 8

Built from version 7 (base 10^9, Karatsuba multiplication above 32 limbs).

## Changes

Knuth division remains the base case. As soon as the divisor and the quotient each have at least 48 limbs (432 digits), division is recursive (Burnikel-Ziegler): the quotient is obtained by halves, each half by a smaller division and a Karatsuba multiplication, then a correction of a few units. A dividend much longer than the divisor is processed in blocks of that size.

The threshold of 48 limbs is the one found by sweeping. Below it, the time matches Knuth. At a divisor of 813 limbs, 48 is the fastest of the thresholds tried.

## Performance

Same benchmark as version 1 (40 products of 350-digit integers and one division, then 8 fits).

| | version 7 | version 8 |
|---|---:|---:|
| product and division | 0.47 ms | 0.45 ms |
| fit | 59 ms | 56 ms |
| total | 60 ms | 56 ms |

The short benchmark does not change: 350 digits make a 9-limb divisor, which stays in Knuth division, and the fit handles small rationals.

One operation (product then division) on longer integers:

| digits | version 7 | version 8 |
|---:|---:|---:|
| 2 000 | 0.175 ms | 0.151 ms |
| 8 000 | 1.54 ms | 1.30 ms |
| 32 000 | 18.4 ms | 11.6 ms |

Division alone of a 32 000-digit integer (7 112 limbs) by an 813-limb divisor goes from 15.7 ms to 4.7 ms. The full operation gains a factor of about 1.6, Karatsuba multiplication taking the rest.
