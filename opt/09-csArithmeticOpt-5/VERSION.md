# Version 5

Built from version 3 (version 4, with pointer walks, brings nothing measurable). The `csQNUMBER` format changes.

## Changes

The numerator and the denominator are stored as little-endian limbs in base 10^9 (`uint32_t`, 9 decimal digits). The product of two limbs and the carry fit in a `uint64_t`.

The decimal string exists only on input (construction, assignment) and on output (display, quotient, conversion to `csRNUMBER` or `double`). All arithmetic stays in base 10^9.

Multiplication and division remain schoolbook. Multi-limb division is Knuth's; a one-limb divisor has its own loop. Temporaries of fewer than 256 limbs stay on the stack.

The caller must include this version's header: the display operator of the original header would read the limbs as a character string.

## Performance

Same benchmark as version 1 (40 products of 350-digit integers and one division, then 8 fits).

| | version 3 | version 5 |
|---|---:|---:|
| product and division | 90–92 ms | 0.44 ms |
| fit | 484–490 ms | 66–72 ms |
| total | 576–580 ms | 66–73 ms |

A 350-digit integer goes from 350 decimal limbs to 39 limbs. The short benchmark becomes about 200 times faster, the fit about 7 times.

On longer integers, one operation (product then division), version 5 alone:

| digits | time |
|---:|---:|
| 350 | 0.011 ms |
| 1 000 | 0.060 ms |
| 2 000 | 0.184 ms |
| 4 000 | 0.84 ms |
| 8 000 | 2.55 ms |
| 16 000 | 10.2 ms |
| 32 000 | 39 ms |

At 8 000, 16 000 and 32 000 digits, multiplication alone takes 1.62 ms, 6.84 ms and 27 ms. The rest is Knuth division.
