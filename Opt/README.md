# csArithmeticOpt

Optimized editions of [csArithmetic](https://github.com/Phil467/csArithmetic). The library does exact and high-precision arithmetic that you write the way you write a formula.

## A calculation you can read

Many big-number libraries are organized around the speed of one primitive. You allocate a number, call a multiply, pass the buffer to an add, then to a divide, and you keep every temporary yourself. The formula that motivated the code is no longer on the page. What remains is a sequence of calls, scratch space and size arguments.

csArithmeticOpt keeps the formula. An integer, a rational, a real or a complex value is an object, and the usual operators apply to it. Parentheses do the grouping. The line

```cpp
a = (b * c + d) / e - f;
```

is the calculation. The product, the sum, the quotient and the difference stay visible, in that order, with the same parentheses you would use on paper. The same shape works for `csInteger`, `csRational`, `csReal` and, in the latest edition, `csComplex`. Where the type allows it, a machine integer or a `double` can stand in one of the places. When you want to update a value without naming a new one, the in-place forms `+=`, `-=`, `*=` and `/=` are there.

That is the point of the library. The cost of each operation is real, and the later editions spend their effort on it: base 10^9, Karatsuba, recursive division, base 2^32, Toom-3, a modular Fourier product, Newton division for very large quotients. Those choices sit under the operators. Calling code does not switch algorithms, and it does not thread a carry by hand. You pick an edition, you write the expression, and that edition does the rest.

## What this chooses, and what it leaves aside

A library built only for the fastest primitive is the right tool when the formula is fixed and the numbers are enormous: you drop to limbs, windows and scratch buffers, and you accept that the program no longer looks like the mathematics. It is a poor way to try a formula, change a sign, or read the program again six months later.

csArithmeticOpt takes the other side of that trade. The operators allocate and release what the expression needs. They are optimized, and edition 13 is far from edition 1, but the public surface stays a value you can print, compare and combine. The precision of a real is a property of that value, and of the library default, not an extra argument on every operator.

Clarity here is not a lack of structure. It means the structure you see is the structure of the calculation.

## Editions

| Folder | What changes |
|---|---|
| `csArithmeticOpt` | Forced inlining of the digit operations, still in base 10 |
| `csArithmeticOpt-2` … `4` | Tables, stack temporaries, pointer walks |
| `csArithmeticOpt-5` | Limbs in base 10^9 |
| `csArithmeticOpt-6` | Base 10^19, not carried forward |
| `csArithmeticOpt-7` | Karatsuba |
| `csArithmeticOpt-8` | Recursive division |
| `csArithmeticOpt-9` | Limbs in base 2^32, Toom-3 |
| `csArithmeticOpt-10` | Modular Fourier multiplication, Newton division at large sizes |
| `csArithmeticOpt-11` | Names: `csRational`, `csReal` |
| `csArithmeticOpt-12` | `csInteger` |
| `csArithmeticOpt-13` | `csComplex` and `csComplex_q` |

Each folder has a `VERSION.md` with the measurements that justified the change. For new work, use edition 13. Earlier editions stay so a result can be tied to the code that produced it.

Edition 13 also ships compiled static libraries in `csArithmeticOpt-13/build/`: Windows (MinGW, UCRT64) and Android arm64, for [Cxxdroid](csArithmeticOpt-13/build/android/CXXDROID.md).

## Not in these editions

Personal experiments on roots are not part of this publication. The arithmetic, the rational and real types, and the complex numbers of edition 13 remain.

The complex modulus still needs a square root. It uses a short Newton iteration written with the real operators.

## Using edition 13

The static library is linked with `CSARITHMETIC_STATIC` defined before the header, so the declarations are not imported from a DLL.

```cpp
#define CSARITHMETIC_STATIC
#include "csArithmetic.h"

using namespace CSARITHMETIC;

int main()
{
    csInteger b("24"), c("7"), d("5"), e("3"), f("1");
    csInteger a = (b * c + d) / e - f;
    a.print("a = ");
}
```

The same expression with rationals stays exact. With reals, the default precision applies to the quotient.

```cpp
csRational b(2, 1), c(3, 1), d(1, 1), e(5, 1), f(1, 7);
csRational a = (b * c + d) / e - f;
```

## Documentation

The API reference is [docs/csArithmeticOpt_API.md](docs/csArithmeticOpt_API.md). It is the page to open for the types, the operators and the functions.

## License

GNU GPL v3, the same license as csArithmetic. Copyright Philippe Levang Azeufack.

## À propos

csArithmeticOpt reprend csArithmetic en gardant la formule visible. Là où d'autres bibliothèques enchaînent des appels et des tampons, celle-ci accepte

```cpp
a = (b * c + d) / e - f;
```

pour un entier, un rationnel, un réel ou un complexe. Les algorithmes rapides (Karatsuba, Toom-3, Fourier, division de Newton) sont sous les opérateurs. Le détail de chaque édition est dans son `VERSION.md`. L'édition à utiliser est la 13.
