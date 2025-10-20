<h1 align="center">
🧩 csArithmetic
</h1>

<div align="center">
Educational library for calculating with n-digit numbers.
</div>


## ABOUT

This library was initially developed for educational purposes, to learn how to teach a computer to calculate using basic arithmetic logic rules. It includes functions for basic arithmetic operations such as addition, subtraction, multiplication, and division, as well as modular arithmetic and gcd calculations. It performs calculations on numerical strings grouped into two main sets: Reals (**`RNUMBER`**) and Rationals (**`QNUMBER`**). Each operation's functions contain small local optimizations depending on the type of operation. However, they are not optimized for applications requiring high-speed performance, because it is also intended for research I am conducting on nth roots of real numbers, particularly square roots. I propose an interesting approach, presenting a good compromise between precision and time efficiency. I called it **"weighted convergence method"**. It is implemented in several versions with prefix **`csSqrt`** and has the advantage of being easily parallelizable. The combination of this method with that of bisection allows to have much better results. It presents a quadratic convergence. Its comparison with Newton-Raphson shows that it is more efficient. 

The weighted convergence method is a consequence of a larger study that I carried out on the theory of numbers, associated with linear algebra and of which I named the discipline: "Arithmetic barycenters". I will gradually detail here the content of this work. 

Moreover, the parallelizable version of its implementation in the class **`csSQRTFAST`** contains an identified bug, but not corrected for the moment, given the complexity of its implementation. 

I will also do the documentation of the library very soon.


<h1 align="center">
🧩 csArithmetic
</h1>

<div align="center">
Bibliothèque pédagogique pour le calcul avec des nombres à n chiffres.
</div>


## A PROPOS

Cette bibliothèque est développée initialement pour un but éducatif, afin d’apprendre comment apprendre à un ordinateur à calculer à partir des règles de la logique arithmétique de base. Elle possède des fonctions pour les calculs arithmétiques de base à savoir l’addition, la soustraction, la multiplication et la division, mais aussi le calcul modulaire et le pgcd. Elle effectue des calculs sur des chaines de caractères numériques regroupés en deux grands ensembles : les Réels (**`RNUMBER`**) et les Rationnels (**`QNUMBER`**). Les fonctions de chaque opération comportent des petites optimisations locales qui dépendent du type d’opération. Mais elles ne sont toutefois pas optimisées pour des applications nécessitant de grandes performances en temps pour la raison qu’elle est aussi destinée à une recherche que je mène sur les racines n-ièmes des nombres réels, et plus particulièrement sur les racines carrées. Je propose en effet une approche intéressante, présentant un bon compromis entre précision et efficacité en temps. Je l’ai appelée **« méthode par convergence pondérée »**. Elle est implémentée en plusieurs versions avec préfix **`csSqrt`** et présente l’avantage d’être facilement parallélisable. La combinaison de cette méthode avec celle de la bissection permet d’avoir de bien meilleurs résultats. Elle présente une convergence quadratique. Sa comparaison avec Newton-Raphson montre qu’elle est plus efficace.

La méthode par convergence pondérée est une conséquence d’une étude plus vaste que j’ai effectuée sur la théorie des nombres, associée à de l’algèbre linéaire et dont j’ai nommé la discipline : **« Barycentres arithmétiques »**. Je détaillerai progressivement ici le contenu de ces travaux.

Par ailleurs la version parallélisable de son implémentation dans la classe **`csSQRTFAST`** contient un bug identifié, mais non corrigé pour l’instant, étant donnée la complexité de sa mise en œuvre.

Je ferai aussi la documentation de la bibliothèque très prochainement.
