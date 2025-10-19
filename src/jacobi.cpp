/*
 * jacobi.cpp
 *
 *  Created on: 13 mai 2023
 *      Author: azphi
 */

#include "jacobi.h"


char* JACOBI_SOLVER_ERR = (char*)"";
int JACOBI_MESSAGE_ID = 0;

void cPyramid(csQNUMBER* a, csQNUMBER* b, int n, csQNUMBER*& P, csQNUMBER*& Q, int*& R, csQNUMBER*& CF, int&PSize, int&QSize)
{
    int n2 = n*n;
	QSize = firstDegreePyramidalArrayIndexLevel(n);
	PSize = secondDegreePyramidalArrayIndexLevel(n);
//	std::cout<<"\n N = "<<N<<"\n\n";

	csQNUMBER *p=csQNUMBER_PTR_ALLOC(PSize);
	csQNUMBER *q=csQNUMBER_PTR_ALLOC(QSize);
	int *r=(int*)malloc(sizeof(int)*n); //pour retenir les indices de lignes avec premier terme non nul
	csQNUMBER zero("0");
	csQNUMBER *cf=csQNUMBER_PTR_ALLOC(n, zero);

	for (int k=0; k<n2; k++)
	{
		p[k].copy(a[k]);
	}
	for (int k=0; k<n; k++)
	{
		q[k].copy(b[k]);
		r[k] = 0;
		cf[k].set("0","1",0);
	}
	for (int k=n2; k<PSize; k++)
	{
		p[k].set("0","1",0);
	}
	for (int k=n; k<QSize; k++)
	{
		q[k].set("0","1",0);
	}
	P = p; Q = q; R = r; CF = cf;
	zero.clear();
}


void cPyramid2(csQNUMBER* a, csQNUMBER* b, int n, csQNUMBER*& P, csQNUMBER*& Q, int*& R, int&PSize, int&QSize)
{
    int n2 = n*n;
	QSize = firstDegreePyramidalArrayIndexLevel(n);
	PSize = secondDegreePyramidalArrayIndexLevel(n);
//	std::cout<<"\n N = "<<N<<"\n\n";

	csQNUMBER *p=csQNUMBER_PTR_ALLOC(PSize);
	csQNUMBER *q=csQNUMBER_PTR_ALLOC(QSize);
	int *r=(int*)malloc(sizeof(int)*n); //pour retenir les indices de lignes avec premier terme non nul

	for (int k=0; k<n2; k++)
	{
		p[k].copy(a[k]);
	}
	for (int k=0; k<n; k++)
	{
		q[k].copy(b[k]);
		r[k] = 0;
	}
	for (int k=n2; k<PSize; k++)
	{
		p[k].set("0","1",0);
	}
	for (int k=n; k<QSize; k++)
	{
		q[k].set("0","1",0);
	}
	P = p; Q = q; R = r;
}

void pyramidVars(int n, int& M, int& N, csQNUMBER*& p, csQNUMBER*& q, int*& r)
{
	M = firstDegreePyramidalArrayIndexLevel(n);
	N = secondDegreePyramidalArrayIndexLevel(n);
//	std::cout<<"\n N = "<<N<<"\n\n";

	p=csQNUMBER_PTR_ALLOC(N);
	q=csQNUMBER_PTR_ALLOC(M);
	r=(int*)malloc(sizeof(int)*n); //pour retenir les indices de lignes avec premier terme non nul
}


csQNUMBER* cJacobi(csQNUMBER*a1, csQNUMBER*b1, int n)
{
	csQNUMBER *a = 0, *b = 0, *cf = 0, aa;
	int *r = 0, aSize, bSize;
	
	cPyramid(a1, b1, n, a, b, r, cf, aSize, bSize);

	int k = 0, bj = 0, j = 0, jsz = 0, l = 0, nk = 0, i = 0;

	int Ka = 0, Kb = 0, iid, jjd, lld, _iid, _jjd, _lld;
	for (k = 0; k < n; k++)
	{
		jsz = 0; l = 0; bj = 0; j = 0;
		nk = n-k;

		for (l = 0; l < nk; l++)
		{
			if (a[Ka + l*nk + 0].isNonZero())
			{
				bj = 1;
				break;
			}
		}

		r[k]=l;
		if (bj == 0)
		{
			JACOBI_SOLVER_ERR = "\nJacobi Solver Warning : zeros colums found!";
			JACOBI_MESSAGE_ID += 1;
			break; //colone de 0
		}
		for (j = 0; j < nk; j++)
		{
			if (j != l)
			{
				iid = Ka + nk*nk + jsz*(nk-1);
				jjd = Ka + j*nk;
				lld = Ka + l*nk;
				aa = a[jjd + 0]/a[lld + 0];
				
				for (i = 1; i < nk; i++)
				{
					a[iid + i-1] = a[jjd + i] - aa*a[lld + i];
					//cout<<iid<<"\n";
				}
				_iid = Kb + nk + jsz;
				_jjd = Kb + j;
				_lld = Kb + l;
		
				b[_iid] = b[_jjd] - aa*b[_lld];
				jsz += 1;
			}
		}


		Ka += nk*nk;
		Kb += nk;
	}
	if (bj!=0)
	{
		k = n-1;
		Kb = firstDegreePyramidalArrayIndexLevel(n)-1;
		Ka = secondDegreePyramidalArrayIndexLevel(n)-1;

		b[Kb] = b[Kb]/a[Ka];
		cf[k].copy(b[Kb]);
		//for(int i = 0; i<bSize; i++)
			//cout<<i<<" "<<b[i]<<"  ----- cfk\n";
		//cout<<" "<<b[9]<<"  ----- ***\n";
		//cout<<" "<<a[26]<<"  ----- ***\n";
		//cf[k].print(" cfk : ");
		//k = n-2;
		csQNUMBER x("0");
		size_t t = 0, idj = 0, Na = 0, Nb = 0, Nbj, Naj;


		for (k = n-2; k >= 0; k--)
		{
			j = r[k];
			x.set("0","1",0);
			t = 0;
			nk = n-k;
			Na = Ka+1 - secondDegreePyramidalArrayIndexLevel(nk);
			Naj = Na + j*nk;
			for (i = n-k-1; i >= 1; i--)
			{
				idj = n-1-t;
				Nb = Kb+1 - firstDegreePyramidalArrayIndexLevel(t+1); //t+1 = n - idj
				x = x + b[Nb + r[idj]]*a[Naj + i];
				t += 1;
			}
			Nb = Kb+1 - firstDegreePyramidalArrayIndexLevel(nk);
			Nbj = Nb + j;

			b[Nbj] = ((b[Nbj]-x) / a[Naj + 0]);
			cf[k].copy(b[Nbj]);
		}
			
		x.clear();
	}
	//cout<<"55555555"<<"\n";
	csQNUMBER_PTR_FREE(a,aSize);
	csQNUMBER_PTR_FREE(b,bSize);
	//cout<<"55555555"<<"\n";
	free(r);
	return cf;
}


csQNUMBER* cJacobi2(csQNUMBER*a1, csQNUMBER*b1, int n, csQNUMBER*&cf)
{
	csQNUMBER *a = 0, *b = 0, aa;
	int *r = 0, aSize, bSize;
	cPyramid2(a1, b1, n, a, b, r, aSize, bSize);

	int k = 0, bj = 0, j = 0, jsz = 0, l = 0, nk = 0, i = 0;

	int Ka = 0, Kb = 0, iid, jjd, lld, _iid, _jjd, _lld;
	for (k = 0; k < n; k++)
	{
		jsz = 0; l = 0; bj = 0; j = 0;
		nk = n-k;

		for (l = 0; l < nk; l++)
		{
			if (a[Ka + l*nk + 0].isNonZero())
			{
				bj = 1;
				break;
			}
		}

		r[k]=l;
		if (bj == 0)
		{
			JACOBI_SOLVER_ERR = "\nJacobi Solver Warning : zeros colums found!";
			JACOBI_MESSAGE_ID += 1;
			break; //colone de 0
		}
		for (j = 0; j < nk; j++)
		{
			if (j != l)
			{
				iid = Ka + nk*nk + jsz*(nk-1);
				jjd = Ka + j*nk;
				lld = Ka + l*nk;
				aa = a[jjd + 0]/a[lld + 0];
				for (i = 1; i < nk; i++)
				{
					a[iid + i-1] = a[jjd + i] - aa*a[lld + i];
				}
				_iid = Kb + nk + jsz;
				_jjd = Kb + j;
				_lld = Kb + l;
				b[_iid] = b[_jjd] - aa*b[_lld];
				jsz += 1;
			}
		}

		Ka += nk*nk;
		Kb += nk;
	}
	if (bj!=0)
	{
		k = n-1;
		Kb = firstDegreePyramidalArrayIndexLevel(n)-1;
		Ka = secondDegreePyramidalArrayIndexLevel(n)-1;

		b[Kb] = b[Kb]/a[Ka];
		cf[k].copy(b[Kb]);
		k = n-2;
		csQNUMBER x("0");
		int t = 0, idj = 0, Na = 0, Nb = 0;

		for (k = n-2; k >= 0; k--)
		{
			j = r[k];
			x.set("0","1",0);
			t = 0;
			nk = n-k;
			Na = Ka+1 - secondDegreePyramidalArrayIndexLevel(nk);
			for (i = n-k-1; i >= 1; i--)
			{
				idj = n-1-t;
				Nb = Kb+1 - firstDegreePyramidalArrayIndexLevel(t+1); //t+1 = n - idj
				x = x + b[Nb + r[idj]]*a[Na + j*nk + i];
				t += 1;
			}
			Nb = Kb+1 - firstDegreePyramidalArrayIndexLevel(nk);

			b[Nb + j] = ((b[Nb + j]-x) / a[Na + nk*j + 0]);
			cf[k].copy(b[Nb + j]);
		}
	}
	csQNUMBER_PTR_FREE(a,aSize);
	csQNUMBER_PTR_FREE(b,bSize);
	free(r);
	return cf;
}

/**************************************************************/

void cPyramid3(csPYRAMID_VARS* pv)
{
	csQNUMBER* a = pv->X, *b = pv->Y;
	int n = pv->n;
	int M = pv->M, N = pv->N;
	csQNUMBER *p = pv->A, *q = pv->B;
	int *r = pv->C;

    int n2 = n*n;

	for (int k=0; k<n2; k++)
	{
		p[k] = a[k];
	}
	for (int k=0; k<n; k++)
	{
		q[k] = b[k];
		r[k] = 0;
	}
	for (int k=n2; k<N; k++)
	{
		p[k] = 0;
	}
	for (int k=n; k<M; k++)
	{
		q[k] = 0;
	}
}

void cJacobi3(csQNUMBER**cf, csPYRAMID_VARS* pv)
{
	cPyramid3(pv);

	int n = pv->n;
	csQNUMBER *a = pv->A, *b = pv->B, aa;
	/*int N = pv->N, M = pv->M;
	csQNUMBER a[N], b[M], aa;
	for(int i = 0; i<N; i++)
	{
		a[i] = pv->A[i];
	}
	for(int i = 0; i<M; i++)
	{
		b[i] = pv->B[i];
	}*/

	int *r = pv->C;

	int k = 0, bj = 0, j = 0, jsz = 0, l = 0, nk = 0, nk1, i = 0;

	int Ka = 0, Kb = 0, iid, jjd, lld, _iid, _jjd, _lld;
	for (k = 0; k < n; k++)
	{
		jsz = 0; l = 0; bj = 0; j = 0;
		nk = n-k; nk1 = nk-1;

		for (l = 0; l < nk; l++)
		{
			if (a[Ka + l*nk + 0].isNonZero())
			{
				bj = 1;
				break;
			}
		}

		r[k]=l;
		if (bj == 0)
		{
			JACOBI_SOLVER_ERR = "\nJacobi Solver Warning : zeros colums found!";
			JACOBI_MESSAGE_ID += 1;
			break; //colone de 0
		}
		for (j = 0; j < nk; j++)
		{
			if (j != l)
			{
				iid = Ka + nk*nk + jsz*(nk1);
				jjd = Ka + j*nk;
				lld = Ka + l*nk;
				aa = a[jjd + 0]/a[lld + 0];
				for (i = 1; i < nk; i++)
				{
					a[iid + i-1] = a[jjd + i] - aa*a[lld + i];
				}
				_iid = Kb + nk + jsz;
				_jjd = Kb + j;
				_lld = Kb + l;
				b[_iid] = b[_jjd] - aa*b[_lld];
				jsz += 1;
			}
		}

		Ka += nk*nk;
		Kb += nk;
	}

	if (bj!=0)
	{
		k = n-1;
		Kb = firstDegreePyramidalArrayIndexLevel(n)-1;
		Ka = secondDegreePyramidalArrayIndexLevel(n)-1;

		b[Kb] = b[Kb]/a[Ka];
		(*cf)[k].copy(b[Kb]);
		k = n-2;
		csQNUMBER x("0");
		int t = 0, idj = 0, Na = 0, Nb = 0;

		for (k = n-2; k >= 0; k--)
		{
			j = r[k];
			x = 0;
			t = 0;
			nk = n-k;
			Na = Ka+1 - secondDegreePyramidalArrayIndexLevel(nk);
			for (i = n-k-1; i >= 1; i--)
			{
				idj = n-1-t;
				Nb = Kb+1 - firstDegreePyramidalArrayIndexLevel(t+1); //t+1 = n - idj
				x = x + b[Nb + r[idj]]*a[Na + j*nk + i];
				t += 1;
			}
			Nb = Kb+1 - firstDegreePyramidalArrayIndexLevel(nk);

			b[Nb + j] = ((b[Nb + j]-x) / a[Na + nk*j + 0]);
			(*cf)[k].copy(b[Nb + j]);
		}
	}
}


