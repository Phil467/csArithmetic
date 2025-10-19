/*
 * utilities.h
 *
 *  Created on: 13 mai 2023
 *      Author: azphi
 */

#ifndef UTILITIES_H_INCLUDED
#define UTILITIES_H_INCLUDED


#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <string.h>
#include <cmath>
#include <iomanip>
#include <time.h>
#include "csARITHMETIC.h"

using namespace std;
using namespace CSARITHMETIC;


typedef struct
{
	csQNUMBER a;
	csRNUMBER quotient()
	{
		if(a.denominator[0] == '0' && a.denomSize == 1)
		{
			std::cout<<"\nFraction computing warning : \n  b must be different of zero! Set to 1 by default.\n";
			a.denominator[0] = '1';
		}
        csRNUMBER n(a.numerator,0,a.sign), d(a.denominator);
		csRNUMBER r = n/d;
        n.clear();
        d.clear();
        return r;
	}
}Fraction;

extern int TAB_PRINT_ID;

csQNUMBER* fx(csQNUMBER*x, size_t n, csQNUMBER*cf, size_t P);
void fx2(csQNUMBER*x, csQNUMBER*& y, size_t n, csQNUMBER*cf, size_t P);
void fx21(csQNUMBER*x, csQNUMBER*& y, size_t id1, size_t id2, csQNUMBER*cf, size_t P);
void array2dToArray1D(csQNUMBER** a2d, size_t nx, size_t ny, csQNUMBER*& a1d);
csQNUMBER* array2dToArray1D2(csQNUMBER** a2d, size_t nx, size_t ny);

size_t firstDegreePyramidalArrayIndexLevel(size_t level);
size_t secondDegreePyramidalArrayIndexLevel(size_t level);
csQNUMBER* linspace(csQNUMBER a, csQNUMBER b, size_t n);
csQNUMBER* linspaceExcb(csQNUMBER a, csQNUMBER b, size_t n);
Fraction findFraction(csQNUMBER nb, csQNUMBER prec);
Fraction* findFractionList(csQNUMBER*nbs, size_t n, double prec=1e-5);
double getRandomDouble(int64_t n1, int64_t n2, size_t exponent);
double*getRandomDoubleArray1D(size_t n, int64_t min, int64_t max, size_t exponent);
double**getRandomDoubleArray2D(size_t nx, size_t ny, int64_t min, int64_t max, size_t exponent);
csQNUMBER* getIntPowList(size_t n, int p);
csQNUMBER* getCumulList(csQNUMBER*l, size_t n);

template<class T> void printArray2D(T** arr, size_t nx, size_t ny, const char*title, size_t wordLen)
{
    if (strcasecmp(title,"Table")==0)
        std::cout<<"Table id : "<<TAB_PRINT_ID<<"\n";
    else
        std::cout<<title<<"\n";
    for (int j=0; j<ny; j++)
    {
    	for (int i=0; i<nx; i++)
		{
			std::cout<<setw(wordLen)<<arr[j][i]<<"  ";
		}
    	std::cout<<"\n";
    }
    std::cout<<"\n\n";
    TAB_PRINT_ID++;
}

template<class T> void printArray1D(T* arr, size_t n, const char*title, size_t wordLen)
{
    if (strcasecmp(title,"Table")==0)
        std::cout<<"Table id : "<<TAB_PRINT_ID<<"\n";
    else
        std::cout<<title<<"\n";
    for (int i=0; i<n; i++)
    {
        std::cout<<setw(wordLen)<<arr[i];
    }
    std::cout<<"\n\n";
    TAB_PRINT_ID++;
}
void printArray2D(csQNUMBER** arr, size_t nx, size_t ny, const char*title, size_t wordLen);
void printArray1D(size_t* arr, size_t n, const char*title, size_t wordLen);
void printArray2D(size_t** arr, size_t nx, size_t ny, const char*title, size_t wordLen);
void printArray1D(csQNUMBER* arr, size_t n, const char*title, size_t wordLen);
void printArray1D_Column(csQNUMBER* arr, size_t n, const char*title, bool showQuotient=0);
void printFactionsArray(Fraction* arr, size_t n, const char*title, size_t wordLen);

/*csQNUMBER sum(csQNUMBER*arr, size_t n);
csQNUMBER round(csQNUMBER nb, size_t ndigits);
csQNUMBER roundZero(csQNUMBER nb);
csQNUMBER* arrayRound(csQNUMBER*arr, size_t n, size_t ndigits);
void arrayRound2(csQNUMBER*&arr, size_t n, size_t ndigits);
csQNUMBER* arrayBoost(csQNUMBER* arr, size_t n, csQNUMBER fact);
void arrayBoost2(csQNUMBER*& arr, size_t n, csQNUMBER fact);
void minmax1D(csQNUMBER*arr, size_t n, size_t& minval, size_t& maxval);
void minmax2D(csQNUMBER**arr, size_t nx, size_t ny, csQNUMBER& minval, csQNUMBER& maxval);
size_t**fullUInt2D(size_t nx, size_t ny, size_t nb);
int**fullInt2D(size_t nx, size_t ny, int nb);*/
csQNUMBER**fullDouble2D(size_t nx, size_t ny, csQNUMBER nb);
template<class T> T*full1D(size_t nx, T nb)
{
    T*arr = (T*)malloc(sizeof(T)*nx);
    for (int i=0; i<nx; i++)
    {
        arr[i]=nb;
    }
	return arr;
}

template<class T> T**full2D(size_t nx, size_t ny, T nb)
{
    T**arr = (T**)malloc(sizeof(T*)*ny);
	for (int j=0; j<ny; j++)
	{
		arr[j] = (T*)malloc(sizeof(T)*nx);
		for (int i=0; i<nx; i++)
		{
			arr[j][i]=nb;
		}
	}
	return arr;
}

//void setStrRGB(char*str,   size_t pos,  csRGBA c);

void setStrRGB2(char*str,   size_t pos,  size_t r, size_t g, size_t b);


template<class T> T* trim1D(T*map, size_t nx, int x, int cx)
{
    T*ret=0;
    if(x>=0 && x<=nx && cx>x && cx<nx)
    {
        T*ret=(T*)malloc(cx*sizeof(T));
        for(int i=0; i<cx; i++)
        {
            ret[i] = map[i+x];
        }
    }
    else
    {
        std::cout<<"From trimDouble1D() : Invalid trimming sizes."<<std::endl;
        ret = map;
    }
    return ret;
}

template<class T> T** trim2D(T**map, size_t nx, size_t ny, int x, int y, int cx, int cy)
{
    T**ret=0;
    if(x>=0 && x<=nx && y>=0 && y<=ny && cx>x && cx<nx && cy>y && cy<=ny)
    {
        T**ret=(T**)malloc(cy*sizeof(T*));
        for(int j=0; j<cy; j++)
        {
            ret[j] = (T*)malloc(cx*sizeof(T));
            for(int i=0; i<cx; i++)
                ret[j][i] = map[j+y][i+x];
        }
    }
    else
    {
        std::cout<<"From trimDouble2D() : Invalid trimming sizes."<<std::endl;
        ret = map;
    }
    return ret;
}

csQNUMBER fxi(csQNUMBER xi, csQNUMBER*cf, size_t P);
csQNUMBER* trimDouble1D(csQNUMBER*map, size_t nx, size_t ny, int x, int cx);
int* trimInt1D(int*map, size_t nx, size_t ny, int x, int cx);
char* trimDouble1D(char*map, size_t nx, size_t ny, int x, int cx);
csQNUMBER** trimDouble2D(csQNUMBER**map, size_t nx, size_t ny, int x, int y, int cx, int cy);
int** trimInt2D(int**map, size_t nx, size_t ny, int x, int y, int cx, int cy);
char** trimChar2D(char**map, size_t nx, size_t ny, int x, int y, int cx, int cy);

#endif /* UTILITIES_H_INCLUDED */
