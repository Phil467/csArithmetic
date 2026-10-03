# csArithmeticOpt 13 with Cxxdroid

This folder contains `libcsArithmeticOpt13.a`, a static library built with the Android NDK for **arm64-v8a** (64-bit ARM), C++17, with `CSARITHMETIC_STATIC` and the symbols emitted for a normal link (`CS_STATIC_LIB`).

Cxxdroid compiles your program on the phone with the NDK. The headers are not inside the library: copy them next to the `.a` file.

## Files to copy into the Cxxdroid project

From `csArithmeticOpt-13/`:

- `csArithmetic.h`
- `csArithmeticOpt13.h`

From this folder:

- `libcsArithmeticOpt13.a`

Put all three in the project directory, next to your `.cpp` file. The header includes `csArithmeticOpt13.h` from the same directory.

## Compiler flags

In Cxxdroid, open the compiler arguments for the file (the wrench, or a comment the IDE reads as flags) and add:

```text
-std=c++17 -DCSARITHMETIC_STATIC -I. -L. -lcsArithmeticOpt13
```

`CSARITHMETIC_STATIC` must be visible when the header is parsed. Defining it on the command line does that for every file. You can also write it yourself before the include:

```cpp
#define CSARITHMETIC_STATIC
#include "csArithmetic.h"

using namespace CSARITHMETIC;

int main()
{
    csInteger b("24"), c("7"), d("5"), e("3"), f("1");
    csInteger a = (b * c + d) / e - f;
    a.print("a = ");
    return 0;
}
```

`-lcsArithmeticOpt13` is the file `libcsArithmeticOpt13.a` with the `lib` prefix and the `.a` suffix removed. `-L.` tells the linker to look in the project directory.

## If the link fails

- The phone is 64-bit ARM. This library is not built for 32-bit ARM (`armeabi-v7a`) or for x86.
- The header and the library must be the same edition. Do not mix edition 13 headers with an older library.
- A missing `CSARITHMETIC_STATIC` on Windows is a DLL-import problem. On Android the macro is empty either way; defining it keeps the same source as on Windows.
- Undefined references to `std::` symbols mean the file is being linked as C. Keep `-std=c++17` and compile the file as C++.

## What the library gives you

Edition 13 is the one with `csInteger`, `csRational`, `csReal`, `csComplex` and `csComplex_q`. Operators are the interface: `a = (b * c + d) / e - f`. The API page is `docs/csArithmeticOpt_API.md` in this publication.
