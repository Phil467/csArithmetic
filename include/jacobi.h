/*
 * jacobi.h
 *
 *  Created on: 13 mai 2023
 *      Author: azphi
 */

#ifndef JACOBI_H_INCLUDED
#define JACOBI_H_INCLUDED


#include "utilities.h"

typedef struct
{
    csQNUMBER* A,*B;
    int*C;
    size_t M,N, n2, n;
    csQNUMBER *X, *Y;
    void set(size_t _n)
    {
        n = _n;
        n2 = n*n;
        M = firstDegreePyramidalArrayIndexLevel(n);
        N = secondDegreePyramidalArrayIndexLevel(n);
    //	std::cout<<"\n N = "<<N<<"\n\n";

        A=csQNUMBER_PTR_ALLOC(N);
        B=csQNUMBER_PTR_ALLOC(M);
        C=(int*)malloc(sizeof(int)*n); //pour retenir les indices de lignes avec premier terme non nul

        X = csQNUMBER_PTR_ALLOC(n2);
        Y = csQNUMBER_PTR_ALLOC(n);
    }
    void clear()
    {
      csQNUMBER_PTR_FREE(A, N);
      csQNUMBER_PTR_FREE(B, M);
      csQNUMBER_PTR_FREE(X, n2);
      csQNUMBER_PTR_FREE(Y, n);
      free(C);
        A=0;
        B=0;
        C=0;
        X=0;
        Y=0;
    }
}csPYRAMID_VARS;

csQNUMBER* cJacobi(csQNUMBER*a1, csQNUMBER*b1, int n);
csQNUMBER* cJacobi2(csQNUMBER*a1, csQNUMBER*b1, int n, csQNUMBER*&cf);
void cPyramid(csQNUMBER* a, csQNUMBER* b, int n, csQNUMBER*& P, csQNUMBER*& Q, int*& R, csQNUMBER*& CF, int&PSize, int&QSize);
void cPyramid2(csQNUMBER* a, csQNUMBER* b, int n, csQNUMBER*& P, csQNUMBER*& Q, int*& R, int&PSize, int&QSize);
void cJacobi3(csQNUMBER**cf, csPYRAMID_VARS* pv);
void cPyramid3(csPYRAMID_VARS* pv);


#endif /* JACOBI_H_INCLUDED */
