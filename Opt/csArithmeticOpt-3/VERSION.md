# Version 3

Built from version 2. `csQNUMBER` values remain decimal digit strings.

## Changes

Two changes to temporaries:

- Buffers whose lifetime ends inside the function stay on the stack up to 2048 digits, instead of being allocated on the heap.
- The gcd reuses three buffers for the whole remainder sequence, instead of allocating on every modulo.

## Performance

Same benchmark as version 1 (40 products of 350-digit integers and one division, then 8 fits).

| | version 2 | version 3 |
|---|---:|---:|
| product and division | 90–93 ms | 90–92 ms |
| fit | 616–658 ms | 484–490 ms |
| total | | 576–580 ms |

Multiplication of 350-digit numbers does not move. The fit, which does many small rationals and gcds, gains about a quarter.
