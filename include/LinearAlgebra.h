/*
 * LinearAlgebra.h
 *
 *  Created on: 14 mai 2023
 *      Author: azphi
 */

#ifndef LINEARALGEBRA_H_INCLUDED
#define LINEARALGEBRA_H_INCLUDED

#include <iostream>
#include "jacobi.h"
#include "utilities.h"
//#include "genStruct.hpp"
//#include "utlFx.h"

typedef struct
{
	csQNUMBER h, a1, c1, a2, c2;
	void setFromPolynomParams(int nOverlap, int cellLength);
	csQNUMBER getRatio(csQNUMBER x);
}csDOOR_FUNC;

typedef struct
{
  long cx,cy;
}SIZE;

typedef struct
{
  unsigned char r,g,b,a;
}csRGBA;

csQNUMBER xPowSum_po(csQNUMBER*xarr, long n, long _i, long _j, long p);
csQNUMBER xPowDotYSum_po(csQNUMBER*xarr,csQNUMBER*yarr, long n, long _j, long N_exp_P, long p);
csQNUMBER* powerOffsetInterpolationCoefs(csQNUMBER*x, csQNUMBER*y, long n, long nTerms, long p);
csQNUMBER* powerOffsetValues(csQNUMBER*coefs, long nCoefs, csQNUMBER*x, long nx, long p);
csQNUMBER* powerOffsetInterpolation(csQNUMBER*x, csQNUMBER*y, long n, long nTerms, long power, csQNUMBER*xp, long n_xp);

csQNUMBER xPowSum(csQNUMBER*xarr, int n, int p);
csQNUMBER xPowSum2(csQNUMBER*xarr, int id1, int id2, int p);
csQNUMBER xPowDotYSum(csQNUMBER*xarr, csQNUMBER*yarr, int n, int p);
csQNUMBER xPowDotYSum2(csQNUMBER*xarr, csQNUMBER*yarr, int id1, int id2, int p);
csQNUMBER xPowDotYSum3(csQNUMBER*xarr, csQNUMBER*yarr, int id1, int id2, int p);
csQNUMBER* polynomialInterpolationCoefs(csQNUMBER*x, csQNUMBER*y, int n, int p);
csQNUMBER* polynomialInterpolationCoefs2(csQNUMBER*x, csQNUMBER*y, int id1, int id2, int p);
csQNUMBER* polynomialInterpolationCoefs3(csQNUMBER*x, csQNUMBER*y, int id1, int id2, int p);

void polynomialInterpolationCoefs3(csQNUMBER*x, csQNUMBER*y, int id1, int id2, csQNUMBER** cf, csPYRAMID_VARS* pv);

csQNUMBER* polynomialPrediction(csQNUMBER*x, csQNUMBER*y, int n, int p, csQNUMBER*xp, int m, csQNUMBER*& cf);


void pyramidalArrayIndexFunctionExample(unsigned int dimension);

csQNUMBER** descretePolynomialInterpolationCoefs1D(csQNUMBER* y, size_t n, size_t cell_length,
												size_t n_overlap, size_t polynom_degree,
												csQNUMBER*& y_out, size_t& n_out, size_t& n_coefs_cell);

void descretePolynomialInterpolationCoefs1D2(csQNUMBER* y, csQNUMBER**&coefs, size_t n, size_t cell_length,
												size_t n_overlap, size_t polynom_degree,
												size_t n_out, size_t n_coefs_cells);

csQNUMBER** getCoefsArray(size_t n, size_t n_overlap, size_t cell_length, size_t polynom_degree,
						size_t*n_coefs_cells, size_t* n_out);
void correctInterpolationSize(size_t n, size_t cell_length, size_t* cell_length_interp, size_t*n_interp, size_t dx);
csQNUMBER*descretePolynomialInterpolation1D(csQNUMBER**coefs, size_t n_coefs_cells, size_t polynom_degree,
										size_t cell_length, size_t n_overlap, size_t* cell_length_interp, size_t* n_interp);
void discreteInterpExample();

csQNUMBER** cellPolynomialInterpolation2D(csQNUMBER **x_interp, csQNUMBER **y_interp, size_t nx_interp, size_t ny_interp,
								size_t dx, size_t dy, size_t nx, size_t ny);
csQNUMBER**multilinePolynomialInterpolation(csQNUMBER**arr, size_t nx, size_t ny, size_t cell_length,
											size_t n_overlap, size_t polynom_degree, size_t dx,
											size_t* cell_length_interp, size_t* n_interp);
csQNUMBER**polynomialInterpolation2D(csQNUMBER**arr, size_t nx, size_t ny,
								size_t xcell_length, size_t ycell_length,
								size_t nx_overlap, size_t ny_overlap,
								size_t xpolynom_degree, size_t ypolynom_degree,
								size_t dx, size_t dy,
								size_t* nx_interp, size_t* ny_interp);

csQNUMBER** getReadyArray2D(csQNUMBER** map, size_t nx, size_t ny,
                    size_t xcell_length,size_t ycell_length,
					size_t nx_overlap, size_t ny_overlap,
					size_t* nx_out,size_t* ny_out);


csQNUMBER*** getDiscreteCoefs2D(csQNUMBER** map, size_t nx, size_t ny,size_t _nx, size_t _ny,
                    size_t cell_length,
					size_t nx_overlap,size_t ny_overlap,
					size_t polynom_degree, size_t *n_cells_coefs, csPYRAMID_VARS*pv);


csQNUMBER polynomialCellPointValue2D(size_t x_dest,size_t y_dest, csQNUMBER** map_src, size_t nx_src, size_t ny_src, csQNUMBER*** x_coefs,
                                  csQNUMBER*** y_coefs, size_t nx_dest, size_t ny_dest,
                                  size_t xdegree, size_t ydegree, size_t xcell_length,
                                  size_t ycell_length, size_t nx_overlap, size_t ny_overlap);

csQNUMBER polynomialCellPointValue2D_RP(size_t x_dest,size_t y_dest, csQNUMBER** map_src, size_t nx_src, size_t ny_src, csQNUMBER*** x_coefs,
                                  csQNUMBER*** y_coefs, size_t nx_dest, size_t ny_dest,
                                  size_t xdegree, size_t ydegree, size_t xcell_length,
                                  size_t ycell_length, size_t nx_overlap, size_t ny_overlap);

csQNUMBER** getInterpolatedArray2D(csQNUMBER** map_src, size_t nx_src,
                          size_t ny_src, csQNUMBER*** x_coefs,
                          csQNUMBER*** y_coefs, size_t nx_dest, size_t ny_dest,
                          size_t xdegree, size_t ydegree, size_t xcell_length,
                          size_t ycell_length, size_t nx_overlap, size_t ny_overlap);


csQNUMBER* getReadyArray1D(csQNUMBER* map, size_t nx,
                    size_t xcell_length,
					size_t nx_overlap,
					size_t* nx_out);
csQNUMBER** getDiscreteCoefs1D(csQNUMBER* map, size_t nx, size_t _nx,
                    size_t cell_length,
					size_t nx_overlap,
					size_t polynom_degree, size_t *n_cells_coefs, csPYRAMID_VARS* pv);
csQNUMBER polynomialCellPointValue1D_Converge(size_t x_dest, csQNUMBER* map_src, size_t nx_src, csQNUMBER** x_coefs,
								size_t nx_dest, size_t xdegree, size_t xcell_length,size_t nx_overlap, csDOOR_FUNC df);
csQNUMBER* getInterpolatedArray1D(csQNUMBER* map_src, size_t nx_src, csQNUMBER** x_coefs, size_t nx_dest,
                          size_t xdegree, size_t xcell_length, size_t nx_overlap);
csQNUMBER* getInterpolatedArray1D_Converge(csQNUMBER* map_src, size_t nx_src, csQNUMBER** x_coefs, size_t nx_dest,
                          size_t xdegree, size_t xcell_length, size_t nx_overlap, csDOOR_FUNC df);

/*void csDrawCurve(unsigned char*&map, SIZE mapSz, csRGBA color,
					csQNUMBER*xarr, csQNUMBER*yarr, int nx, csQNUMBER density=10, size_t cellLength=1, size_t nOverlaped=2,
					size_t polynomDegree=5);*/
void linalgTest();
csQNUMBER** linalgTestWithImageChanel(csQNUMBER**arr, int nx, int ny, csQNUMBER zoom);
void freePtr3(void***ptr, int nx, int ny);
void freePtr2(void**ptr, int n);
csQNUMBER** transpose(csQNUMBER**arr, size_t nx, size_t ny);
void interp2DExample();
csQNUMBER* curvefittingTest(csQNUMBER*arr, int nx, csQNUMBER ratio, size_t cellLength=1, size_t nOverlaped=2, size_t polynomDegree=0, bool boundConverge=0);


#endif /* LINEARALGEBRA_H_INCLUDED */
