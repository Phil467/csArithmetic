# Version 1

Built from the original library (`csArithmetic`). `csQNUMBER` values remain decimal digit strings.

## Changes

Definitions in the generated code are marked `always_inline`, so digit addition, subtraction, multiplication and division are copied into their callers instead of being called.

## Performance

Same machine, same benchmark: 40 products of two 350-digit integers followed by a division by an 80-digit integer, then 8 polynomial fits (18 points, degree 10). The warm-up pass is not counted.

| | original | version 1 |
|---|---:|---:|
| product and division | 208 ms | 86 ms |
| fit | 1323 ms | 603 ms |
| total | 1531 ms | 688 ms |

The product is about 2.4 times faster, the fit about 2.2 times.
