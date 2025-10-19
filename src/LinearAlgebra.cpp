/*
 * LinearAlgebra.cpp
 *
 *  Created on: 14 mai 2023
 *      Author: azphi
 */

#include "LinearAlgebra.h"
//#include <windows.h>

extern int TAB_PRINT_ID;


/*void discreteInterpExample()
{
	size_t n = 600, wordLen=10, n_out=0, n_coefs_cells=0, cell_length_interp=0, n_interp = 0, dx = 4;
	int64_t min = 1, max = 10, exponent = 0;
	csQNUMBER*x_out;
	csQNUMBER*y = getRandomDoubleArray1D(n, min, max, exponent), *y_out;
	printArray1D(y, n, "y :", wordLen=20);
	int cell_length, n_overlap, polynom_degree, n_cell_interest=0;

	csQNUMBER** coefs = descretePolynomialInterpolationCoefs1D(y, n, cell_length=3,
													n_overlap=2, polynom_degree=7,
													y_out, n_out, n_coefs_cells);

	//printArray1D(y_out, n_out, "y_out :", wordLen=8);
	//printArray2D(coefs, polynom_degree+1, n_coefs_cells, "coefs :", wordLen);
	correctInterpolationSize(n, cell_length, &cell_length_interp, &n_interp, dx);
	std::cout<<n_interp;
	csQNUMBER*yinterp = descretePolynomialInterpolation1D(coefs, n_coefs_cells, polynom_degree,
											cell_length, n_overlap, &cell_length_interp, &n_interp);

	printArray1D(yinterp, n_interp, "yinterp :", wordLen=10);
}
*/

csQNUMBER* aplFunc01(size_t n, size_t prec, csQNUMBER*&x)
{
    csQNUMBER*y = csQNUMBER_PTR_ALLOC(n), _prec("10");
    x = csQNUMBER_PTR_ALLOC(n);
	csQNUMBER m(n-1), step("1","2");
	_prec = pow(_prec, prec);
	csQNUMBER j, _x, msr = NRSqrtQ(m,prec);

	for (size_t i=0; i<n; i++)
	{
		x[i].copy(j);
		y[i] = (csSqrtQ_Prec(m - j, prec) - csSqrtQ_Prec(j, prec))/msr;
		j = j+step;
	}
	
	m.clear();
	j.clear();
	msr.clear();
	return y;
}

csQNUMBER* aplFunc02(size_t n, size_t prec, csQNUMBER*&x)
{
    csQNUMBER*y = csQNUMBER_PTR_ALLOC(n), _prec("10");
    x = csQNUMBER_PTR_ALLOC(n);
	csQNUMBER m(n-1), step("1","2");
	_prec = pow(_prec, prec);
	csQNUMBER j, msr = csSqrtQ_Prec(m,prec);

	//csSqrt_Prec("465",)

	for (size_t i=0; i<n; i++)
	{
		x[i].copy(j);
		y[i] = (csSqrtQ_Prec(m - j, prec) + csSqrtQ_Prec(j, prec))/msr;
		j = j+step;
	}
	/*if(n > 4)
	{
		x[1].set("1","2",0);
		size_t n2 = n-2;
		x[n2] = m-x[1];
//x[n2].print(" X[n2] : ");
		y[1] = (NRSqrtQ(m - x[1], prec) - NRSqrtQ(x[1], prec))/msr;
		y[n2] = (NRSqrtQ(m - x[n2], prec) - NRSqrtQ(x[n2], prec))/msr;
	}*/
	m.clear();
	j.clear();
	msr.clear();
	return y;
}

csQNUMBER *normalizedCoefs(csQNUMBER* coefs, size_t n)
{
	csQNUMBER* ncf = csQNUMBER_PTR_ALLOC(n);
	size_t n1 = n-1;
	csQNUMBER prod("1");
	for(size_t i=0; i<n; i++)
	{
		ncf[n1-i] = coefs[n1-i] * prod;
		prod = prod * n1;
	}
	prod.clear();
	return ncf;
}

void pyramidalArrayIndexFunctionExample(unsigned int dimension)
{
	int wordLen=7;
	int xpow = dimension;       // puissance des x dont on recherche l'expression de la somme : x^xpow + (x-1)^xpow + ... + 2^xpow + 1^pow
	//int p = xpow + 1;   // puissance maximale d'entrainement de l'agorithme
	//int n = p*2+5 ;     // nombre de donnees d'entrainement de l'agorithme
	int p = xpow + 1;   // puissance maximale d'entrainement de l'agorithme
	int n = 5;     // nombre de donnees d'entrainement de l'agorithme
	int m = n;       // nombre de point de prediction de la fonction obtenue
setRNumberPrecision(40);
	//csQNUMBER*L = getIntPowList(n, xpow);
	//csQNUMBER*CUMLIST = getCumulList(L, n);
	csQNUMBER*X;
	csQNUMBER*CUMLIST = aplFunc02(n,8,X);
	
	//printArray1D(L, n, "List :", wordLen);
	//printArray1D(CUMLIST, n, "CUMLIST :", wordLen);
	printArray1D_Column(CUMLIST, n, "CUMLIST :",1);

    csQNUMBER one(1,1,0), qn(n,1,0), qm(m,1,0);
	csQNUMBER *x = linspace(one,qn,n), *y = CUMLIST, *xp = linspace(one,qm,m), *cf=NULL;
	printArray1D(X, n, "x : ", wordLen=9);
	printArray1D(y, n, "y : ", wordLen=9);

	p=n-1;
	csQNUMBER*yp = polynomialPrediction(X, y, n, p, X, n, cf);
	//int cf_size = (int)p+1;
	printArray1D_Column(cf, n, "cf :",0);
	//long nTerms = n, power=4;
	//csQNUMBER* yp = powerOffsetInterpolation(X, y, n, nTerms, power, X, n);
	
	/*csQNUMBER prec=1e-5;
	Fraction* cf_frac = findFractionList(cf, int(p), prec=1e-5);
	printFactionsArray(cf_frac, cf_size, "Coefs (Fraction)", wordLen=10);
*/	xp = X;
	printArray1D(xp, m, "xp :", wordLen=9);
	printArray1D_Column(yp, m, "yp :",1);

	csQNUMBER* normCoefs = normalizedCoefs(cf,n);
	printArray1D_Column(normCoefs, n, "normalized Coefs :",0);

    one.clear();
    qm.clear();
    qn.clear();


	/*csQNUMBER_PTR_FREE(L, n);
	csQNUMBER_PTR_FREE(CUMLIST, n);
	csQNUMBER_PTR_FREE(x, n);*/
	//free(L);
	free(CUMLIST);
	free(x);
	free(yp);
	free(xp);
	free(cf);
	//free(cf_frac);
}

//-------------- Power offset ---------------------------
csQNUMBER xPowSum_po(csQNUMBER*xarr, long n, long _i, long _j, long p)
{
	csQNUMBER ret("0");
	for (long i = 0; i<n; i++)
	{
		ret = ret + pow(xarr[i] - _i, p)*pow(xarr[i] - _j, p);
	}

	return ret;
}

csQNUMBER xPowDotYSum_po(csQNUMBER*xarr,csQNUMBER*yarr, long n, long _j, long N_exp_P, long p)
{
	csQNUMBER ret("0");
	for (long i = 0; i<n; i++)
	{
		ret = ret + pow(xarr[i] - _j, p)*yarr[i];
		//cout<<ret<<"  j = "<<_j<<"   ---\n";
	}

		//cout<<ret<<" ------------------\n";
	return ret*N_exp_P;
}

csQNUMBER* powerOffsetInterpolationCoefs(csQNUMBER*x, csQNUMBER*y, long n, long nTerms, long p)
{
	int n1 = n - 1, t = nTerms*nTerms;
	csQNUMBER*M = csQNUMBER_PTR_ALLOC(t);
	csQNUMBER*N = csQNUMBER_PTR_ALLOC(nTerms);
	
	long i = 0, j = 0;

	long N_exp_P = pow(n1, p); // optimiser avec csQNUMBER au lieu de long

	for (j=0; j<nTerms; j++)
	{
		for (i = 0; i < nTerms; i++)
        {
            M[j*nTerms + i] = xPowSum_po(x, n, i, j, p);
        }
		N[j] = xPowDotYSum_po(x, y, n, j, N_exp_P, p);
	}
	printArray1D_Column(N,nTerms," N : ",1);
	csQNUMBER* ret = cJacobi(M,N,nTerms);
	csQNUMBER_PTR_FREE(M, t);
	csQNUMBER_PTR_FREE(N, nTerms);
	return ret;
}

csQNUMBER* powerOffsetValues(csQNUMBER*coefs, long nCoefs, csQNUMBER*x, long nx, long p)
{
	csQNUMBER*y = csQNUMBER_PTR_ALLOC(nx);
	for (long j=0; j<nx; j++)
	{
		csQNUMBER r("0");
		for (long i = 0; i < nCoefs; i++)
        {
            r = r + coefs[i]*pow(x[i] - i, p);
        }
		y[j] = r;
	}
	return y;
}

csQNUMBER* powerOffsetInterpolation(csQNUMBER*x, csQNUMBER*y, long n, long nTerms, long power, csQNUMBER*xp, long n_xp)
{
	csQNUMBER* coefs = powerOffsetInterpolationCoefs(x, y, n, nTerms, power);
	csQNUMBER* yp = powerOffsetValues(coefs, nTerms, xp, n_xp, power);

	return yp;
}

//----------poynomial-------------------------------------------------------

csQNUMBER xPowSum(csQNUMBER*xarr, int n, int p)
{
	csQNUMBER ret("0");
	for (int i = 0; i<n; i++)
	{
		ret = ret + pow(xarr[i],p);
	}
	//ret.print(" Ret : ");

	return ret;
}

csQNUMBER xPowSum2(csQNUMBER*xarr, int id1, int id2, int p)
{
	csQNUMBER ret("0");
	for (int i = id1; i<id2; i++)
		ret = ret + pow(xarr[i],p);
	return ret;
}

csQNUMBER xPowDotYSum(csQNUMBER*xarr, csQNUMBER*yarr, int n, int p)
{
	csQNUMBER ret("0");
	for (int i = 0; i<n; i++)
		ret = ret + pow(xarr[i],p)*yarr[i];

	return ret;
}

csQNUMBER xPowDotYSum2(csQNUMBER*xarr, csQNUMBER*yarr, int id1, int id2, int p)
{
	csQNUMBER ret("0");
	for (int i = id1; i<id2; i++)
		ret = ret + pow(xarr[i],p)*yarr[i];

	return ret;
}

csQNUMBER xPowDotYSum3(csQNUMBER*xarr, csQNUMBER*yarr, int id1, int id2, int p)
{
	csQNUMBER ret("0");
	for (int i = id1, j=0; i<id2; i++)
	{
		ret = ret + pow(xarr[j],p)*yarr[i];
		j++;
	}

	return ret;
}

csQNUMBER* polynomialPrediction(csQNUMBER*x, csQNUMBER*y, int n, int p, csQNUMBER*xp, int m, csQNUMBER*& cf)
{
	cf = polynomialInterpolationCoefs(x,y,n,p);
	
	csQNUMBER*yp = fx(xp, m, cf,(int)p+1);
	return yp;
}

csQNUMBER* polynomialInterpolationCoefs(csQNUMBER*x, csQNUMBER*y, int n, int p)
{
	int P = p + 1, t = P*P, P1=P-1;
	csQNUMBER*M = csQNUMBER_PTR_ALLOC(t);
	csQNUMBER*N = csQNUMBER_PTR_ALLOC(P);
	
	int i = 0, j = 0;

	for (j=0; j<P; j++)
	{
		for (i = P1; i>=0; i--)
        {
            M[j*P + (P1-i)] = xPowSum(x, n, i+j);
        }

		N[j] = xPowDotYSum(x, y, n,j);
	}
	//printArray1D_Column(N,P," P : ");
	csQNUMBER* ret = cJacobi(M,N,P);
	csQNUMBER_PTR_FREE(M, t);
	csQNUMBER_PTR_FREE(N, P);
	return ret;
}

csQNUMBER* polynomialInterpolationCoefs2(csQNUMBER*x, csQNUMBER*y, int id1, int id2, int p)
{
	int P = p + 1, t = P*P;
	csQNUMBER*M = csQNUMBER_PTR_ALLOC(t);
	csQNUMBER*N = csQNUMBER_PTR_ALLOC(P);
	int i = 0, j = 0;

	for (j=0; j<P; j++)
	{
		for (i = P-1; i>=0; i--)
        {
            M[j*P + (P-1-i)] = xPowSum2(x, id1, id2, i+j);
        }

		N[j] = xPowDotYSum2(x, y, id1, id2,j);
	}

	csQNUMBER* ret = cJacobi(M,N,P);
	csQNUMBER_PTR_FREE(M, t);
	csQNUMBER_PTR_FREE(N, P);
	return ret;
}

csQNUMBER* polynomialInterpolationCoefs3(csQNUMBER*x, csQNUMBER*y, int id1, int id2, int p)
{
	int P = p + 1, t = P*P;
	csQNUMBER*M = csQNUMBER_PTR_ALLOC(t);
	csQNUMBER*N = csQNUMBER_PTR_ALLOC(P);
	int i = 0, j = 0, nx = id2-id1;

	for (j=0; j<P; j++)
	{
		for (i = P-1; i>=0; i--)
        {
            M[j*P + (P-1-i)] = xPowSum(x, nx, i+j);
        }

		N[j] = xPowDotYSum3(x, y, id1, id2,j);
	}
	csQNUMBER* ret = cJacobi(M,N,P);
	csQNUMBER_PTR_FREE(M, t);
	csQNUMBER_PTR_FREE(N, P);
	return ret;
}

void polynomialInterpolationCoefs4(csQNUMBER*x, csQNUMBER*y, int id1, int id2, int p, csQNUMBER*& cf)
{
	int P = p + 1, t = P*P;
	csQNUMBER*M = csQNUMBER_PTR_ALLOC(t);
	csQNUMBER*N = csQNUMBER_PTR_ALLOC(P);
	int i = 0, j = 0, nx = id2-id1;

	for (j=0; j<P; j++)
	{
		for (i = p; i>=0; i--)
        {
            M[j*P + (p-i)] = xPowSum(x, nx, i+j);
        }

		N[j] = xPowDotYSum3(x, y, id1, id2,j);
	}

	cJacobi2(M,N,P, cf);
	csQNUMBER_PTR_FREE(M, t);
	csQNUMBER_PTR_FREE(N, P);
}

void polynomialInterpolationCoefs3(csQNUMBER*x, csQNUMBER*y, int id1, int id2, csQNUMBER** cf, csPYRAMID_VARS* pv)
{
	int P = pv->n, p = pv->n-1;
	csQNUMBER*M = pv->X;
	csQNUMBER*N = pv->Y;
	int i = 0, j = 0, nx = id2-id1;

	for (j=0; j<P; j++)
	{
		for (i = p; i>=0; i--)
        {
            M[j*P + (p-i)] = xPowSum(x, nx, i+j);
        }

		N[j] = xPowDotYSum3(x, y, id1, id2,j);
	}
	cJacobi3(cf, pv);

}


csQNUMBER** transpose(csQNUMBER**arr, size_t nx, size_t ny)
{
	csQNUMBER** ret = (csQNUMBER**)malloc(nx*sizeof(csQNUMBER*));
	for (size_t i=0; i<nx; i++)
	{
		ret[i] = csQNUMBER_PTR_ALLOC(ny);
		for (size_t j=0; j<ny; j++)
			ret[i][j] = arr[j][i];
	}
	return ret;
}

csQNUMBER** descretePolynomialInterpolationCoefs1D( csQNUMBER* y, size_t n, size_t cell_length,
												size_t n_overlap, size_t polynom_degree,
												csQNUMBER*& y_out, size_t& n_out, size_t& n_coefs_cells)
{

	csQNUMBER** coefs=NULL;
	if (cell_length < 1)
	{
		cout<<"Error message from descretePolynomialInterpolationCoefs():\n "
				"Invalid Values. Make sure you have cell_length > 2*n_overlap\n\n";
		exit(0);
	}
	else
	{
		size_t r = n%(cell_length);

		n_out = n + (cell_length - r) + 2*n_overlap; // 2*n_overlap = extra zeros on the left and right sides of arrle

		if (r == 0)
			n_out = n + 2*n_overlap;

		y_out = csQNUMBER_PTR_ALLOC(n_out);
		for (size_t i = 0; i<n_overlap; i++)
		{
			y_out[i] = 0;
		}

		size_t n1 = n + n_overlap;
		for (size_t i = n1; i<n_out; i++)
		{
			y_out[i] = 0;
		}

		for (size_t i = n_overlap; i<n1; i++)
		{
			y_out[i] = y[i-n_overlap];
		}

		n_coefs_cells = (n_out-2*n_overlap)/cell_length;
		size_t Total_length = cell_length + 2*n_overlap;

		coefs = (csQNUMBER**)malloc(n_coefs_cells*sizeof(csQNUMBER*));


        csQNUMBER one("1"), qn(Total_length,1,0);
        csQNUMBER *x = linspace(one,qn, Total_length);
        one.clear();
        qn.clear();

		size_t id1=0, id2 = Total_length;
		for (size_t i = 0; i<n_coefs_cells; i++)
		{
			coefs[i] = polynomialInterpolationCoefs3(x,y_out,id1, id2,polynom_degree);
			id1 += cell_length;
			id2 = id1 + Total_length;
		}
		csQNUMBER_PTR_FREE(x, Total_length+1);
		csQNUMBER_PTR_FREE(y_out, Total_length+1);
	}
	return coefs;
}


void descretePolynomialInterpolationCoefs1D2(csQNUMBER* y, csQNUMBER**&coefs, size_t n, size_t cell_length,
												size_t n_overlap, size_t polynom_degree,
												size_t n_out, size_t n_coefs_cells)
{

	if (cell_length < 1)
	{
		cout<<"Error message from descretePolynomialInterpolationCoefs():\n "
				"Invalid Values. Make sure you have cell_length > 2*n_overlap\n\n";
		exit(0);
	}
	else
	{

		csQNUMBER *y_out = csQNUMBER_PTR_ALLOC(n_out);
		for (size_t i = 0; i<n_overlap; i++)
		{
			y_out[i] = 0;
		}

		size_t n1 = n + n_overlap;
		for (size_t i = n1; i<n_out; i++)
		{
			y_out[i] = 0;
		}

		for (size_t i = n_overlap; i<n1; i++)
		{
			y_out[i] = y[i-n_overlap];
		}


		size_t Total_length = cell_length + 2*n_overlap;

        csQNUMBER one("1"), qn(Total_length,1,0);
		csQNUMBER *x = linspace(one,qn, Total_length);
        one.clear();
        qn.clear();
		//printArray1D<csQNUMBER>(*coefs, polynom_degree, "coefs -------- :", 9);
		size_t id1=0, id2 = Total_length;
		for (size_t i = 0; i<n_coefs_cells; i++)
		{
			polynomialInterpolationCoefs4(x,y_out,id1, id2,polynom_degree, coefs[i]);

			id1 += cell_length;
			id2 = id1 + Total_length;
		}
		csQNUMBER_PTR_FREE(x, Total_length+1);
		csQNUMBER_PTR_FREE(y_out, Total_length+1);
	}
}


/*********************************************************************************************************************************/

csQNUMBER** getReadyArray2D(csQNUMBER** map, size_t nx, size_t ny,
                    size_t xcell_length,size_t ycell_length,
					size_t nx_overlap, size_t ny_overlap,
					size_t* nx_out,size_t* ny_out)
{

	if (xcell_length < 1||ycell_length < 1)
	{
		cout<<"Error message from descretePolynomialInterpolationCoefs():\n "
				"Invalid Values. Make sure you have cell_length > 2*n_overlap\n\n";
		exit(0);
	}
	else
	{
        size_t r_xcell_length = nx%xcell_length, r_ycell_length = ny%ycell_length;
        size_t zeropadx_bottom = (xcell_length + nx_overlap)-r_xcell_length, zeropady_bottom = (ycell_length + ny_overlap)-r_ycell_length;
        if(r_xcell_length == 0)  zeropadx_bottom = nx_overlap;
        if(r_ycell_length == 0)  zeropady_bottom = ny_overlap;

        size_t zeropadx_top = nx_overlap, zeropady_top = ny_overlap;
        size_t psz = sizeof(csQNUMBER*), sz = sizeof(csQNUMBER);
        size_t n = zeropadx_top+nx, n1 = zeropady_top + ny;

        *nx_out = n + zeropadx_bottom;
        *ny_out = n1 + zeropady_bottom;

		size_t t = *nx_out*sz;
		csQNUMBER **map_out = (csQNUMBER**)malloc(*ny_out*psz);
		for(size_t j = 0; j<zeropady_top; j++)
        {
            //map_out[j] = (csQNUMBER*)calloc(*nx_out, sz);
			map_out[j] = csQNUMBER_PTR_ALLOC(t);
			for(size_t i = 0, i0 =0 ; i<*nx_out; i++)
			{
				if(i>=zeropadx_top && i<n)
					//map_out[j][i] = map[0][i0++];
					map_out[j][i] = map[zeropady_top-j][i0++];
				//else
					//map_out[j][i] = 0;
				else if(i<zeropadx_top)
					map_out[j][i] = map[zeropady_top-j][zeropadx_top-i];
				else if(i>=n)
					map_out[j][i] = map[zeropady_top-j][nx-1-(i-n+1)];
			}
        }

        for(size_t j = n1; j<*ny_out; j++)
        {
            //map_out[j] = (csQNUMBER*)calloc(*nx_out, sz);
			map_out[j] = csQNUMBER_PTR_ALLOC(t);
			for(size_t i = 0, i0=0; i<*nx_out; i++)
			{
				if(i>=zeropadx_top && i<n)
					//map_out[j][i] = map[ny-1][i0++];
					map_out[j][i] = map[ny-1-(j-n1+1)][i0++];
				//else
					//map_out[j][i] = 0;
				else if(i<zeropadx_top)
					map_out[j][i] = map[ny-1-(j-n1+1)][zeropadx_top-i];
				else if(i>=n)
					map_out[j][i] = map[ny-1-(j-n1+1)][nx-1-(i-n+1)];
			}
        }

        for(size_t j = zeropady_top, j0 = 0; j<n1; j++, j0++)
		{
		    map_out[j] = csQNUMBER_PTR_ALLOC(t);
		    for (size_t i = 0; i<zeropadx_top; i++)
            {
                //map_out[j][i] = map[j0][0];
                map_out[j][i] = map[j0][zeropadx_top-i];
            }

            for (size_t i = n; i<*nx_out; i++)
            {
                map_out[j][i] = map[j0][nx-1-(i-n+1)];
            }

            for (size_t i = zeropadx_top, i0=0; i<n; i++)
            {
                map_out[j][i] = map[j0][i0++];
            }
		}

		return map_out;
	}
}

csQNUMBER* getReadyArray1D(csQNUMBER* map, size_t nx,
                    size_t xcell_length,
					size_t nx_overlap,
					size_t* nx_out)
{

	if (xcell_length < 1)
	{
		cout<<"Error message from descretePolynomialInterpolationCoefs():\n "
				"Invalid Values. Make sure you have cell_length > 2*n_overlap\n\n";
		exit(0);
	}
	else
	{
        size_t r_xcell_length = nx%xcell_length;

        size_t zeropadx = nx_overlap;
        size_t sz = sizeof(csQNUMBER);
        size_t n = nx_overlap+nx;

		*nx_out = n + nx_overlap;
		size_t t = *nx_out*sz;
		csQNUMBER *map_out = csQNUMBER_PTR_ALLOC(t);
		for(size_t i = 0; i<nx_overlap; i++)
		{
			map_out[i] = map[nx_overlap-i];
		}
		for(size_t i = nx_overlap, i0 = 0; i<n; i++)
		{
			map_out[i] = map[i0++];
		}
		for(size_t i = n; i<*nx_out; i++)
		{
			map_out[i] = map[nx-1-(i-n+1)];
		}
		return map_out;
	}
}

