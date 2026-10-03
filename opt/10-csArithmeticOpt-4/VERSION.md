# Version 4

Built from version 3. `csQNUMBER` values remain decimal digit strings.

## Changes

Ten digit loops walk the string with a pointer, instead of an index re-read on every iteration. The rest (stack, gcd, tables, inlining) is that of version 3.

## Performance

Same benchmark as version 1 (40 products of 350-digit integers and one division, then 8 fits).

| | version 3 | version 4 |
|---|---:|---:|
| product and division | 90–92 ms | ~90 ms |
| fit | 484–490 ms | 483–486 ms |
| total | 576–580 ms | 573–576 ms |

No measurable gain. At `-O2`, the compiler had already rewritten these loops the same way.
