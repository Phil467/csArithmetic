# Version 2

Built from version 1. `csQNUMBER` values remain decimal digit strings.

## Changes

The forced inlining of version 1 is kept. The addition, multiplication, subtraction and division tables become contiguous `static constexpr` arrays. They are no longer allocated at startup.

The addition and multiplication tables are 58×58 (useful cells 48..57, ASCII digits). Subtraction uses the same size. The division table goes from 1048×1048 down to 256×256, the size that is actually read.

## Performance

Same benchmark as version 1 (40 products of 350-digit integers and one division, then 8 fits).

| | version 1 | version 2 |
|---|---:|---:|
| product and division | 86 ms | 90–93 ms |
| fit | 603 ms | 616–658 ms |

A slight drop. Removing the startup allocation does not speed up the calculation, which spends its time in the digit loops.
