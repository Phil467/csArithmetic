#pragma once

#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_CSARITHMETIC_DLL
    #define CSARITHMETIC_API __declspec(dllexport)
  #else
    #define CSARITHMETIC_API __declspec(dllimport)
  #endif
#else
  #ifdef BUILDING_CSARITHMETIC_DLL
    #define CSARITHMETIC_API __attribute__ ((visibility ("default")))
  #else
    #define CSARITHMETIC_API
  #endif
#endif

#ifndef CSSQRT_H
#define CSSQRT_H
#include "csARITHMETIC.h"
#include <thread>
using namespace CSARITHMETIC;

typedef struct csROOT_BOUNDS
{
  csRNUMBER number;
  csRNUMBER hightRoot;
  csRNUMBER lowRoot;
  void clear()
  {
    /*free(number.mantissa);
    free(hightRoot.mantissa);
    free(lowRoot.mantissa);
    number.mantissa = 0;
    hightRoot.mantissa = 0;
    lowRoot.mantissa = 0;*/
    number.clear();
    hightRoot.clear();
    lowRoot.clear();
  }
};

typedef struct csLOW_SQRT_COEFS
{
  csRNUMBER cf11, cf12;
  csRNUMBER cf21, cf22;
  csRNUMBER cf31, cf32;
  csRNUMBER cf41, cf42;

  csRNUMBER cf1, cf2, cf3, cf4;

  csLOW_SQRT_COEFS(long localPrecision=0)
  {
    CSARITHMETIC_API CSARITHMETIC::init();
    set(localPrecision);
  }
  
  void set(long localPrecision)
  {
    cf11.precision = localPrecision;
    cf12.precision = localPrecision;
    cf21.precision = localPrecision;
    cf22.precision = localPrecision;
    cf31.precision = localPrecision;
    cf32.precision = localPrecision;
    cf41.precision = localPrecision;
    cf42.precision = localPrecision;
    
    cf11.set(30818487, 0, 1); cf12.set(6250000,0,0);
    cf21.set(31171627, 0, 0); cf22.set(1875000,0,0);
    cf31.set(27211823, 0, 1); cf32.set(781250,0,0);
    cf41.set(31184132, 0, 0); cf42.set(1171875,0,0);

    cf1 = cf11/cf12;
    cf2 = cf21/cf22;
    cf3 = cf31/cf32;
    cf4 = cf41/cf42;
  }
};

typedef struct csHIGHT_SQRT_COEFS
{
  csRNUMBER cf11, cf12;
  csRNUMBER cf21, cf22;
  csRNUMBER cf31, cf32;
  csRNUMBER cf41, cf42;

  csRNUMBER cf1, cf2, cf3, cf4;

  csHIGHT_SQRT_COEFS(long localPrecision=0)
  {
    set(localPrecision);
  }
  
  void set(long localPrecision)
  {
    cf11.precision = localPrecision;
    cf12.precision = localPrecision;
    cf21.precision = localPrecision;
    cf22.precision = localPrecision;
    cf31.precision = localPrecision;
    cf32.precision = localPrecision;
    cf41.precision = localPrecision;
    cf42.precision = localPrecision;

    cf11.set(9829943, 0, 0); cf12.set(2500000,0,0);
    cf21.set(79176791, 0, 1); cf22.set(4687500,0,0);
    cf31.set(5435699, 0, 0); cf32.set(156250,0,0);
    cf41.set(31454476, 0, 1); cf42.set(1171875,0,0);

    cf1 = cf11/cf12;
    cf2 = cf21/cf22;
    cf3 = cf31/cf32;
    cf4 = cf41/cf42;
  }

};

class csSQRTFAST
{
    public:
        typedef struct  csSQRTFAST_INFO
        {
          int STEP1_MAX, STEP1_ITER;
          int STEP2_MAX, STEP2_ITER;
          int STEP3_MAX, STEP3_ITER;
        };
        
        csRNUMBER *rn;
        csSQRTFAST(size_t nBlocks, size_t precision);
        void getBlocks();
        void makeSqrt();
        void printNumber(const char*title);
        void printRoot(const char*title);
        void printError(const char*title);
        void releaseBlocks();
        void releaseNumber();
        void getUnitsSqrt();
        virtual ~csSQRTFAST();
        
        csSQRTFAST_INFO info;

    protected:

    private:
        csRNUMBER* blockCoef;
        csRNUMBER* blockCoefRoot;
        csRNUMBER* tenth;
        csRNUMBER* tenthSqrt;
        csRNUMBER* unitsSqrt;
        csRNUMBER root;
        csRNUMBER err;
        size_t nBlocks;
        size_t precision;
        size_t blockMantSize;
        size_t nThread;
};


csRNUMBER rootApprox(csRNUMBER a, csRNUMBER b, csRNUMBER aroot, csRNUMBER broot, size_t prec);
csRNUMBER rootApprox01(csRNUMBER a, csRNUMBER b, csRNUMBER aroot, csRNUMBER broot, size_t prec);
csRNUMBER rootApprox02(csRNUMBER a, csRNUMBER b, csRNUMBER aroot, csRNUMBER broot, size_t prec);
csROOT_BOUNDS getRootBounds(csRNUMBER _a, csRNUMBER _b, csRNUMBER aroot, csRNUMBER broot, csRNUMBER tenPow, csRNUMBER tenPowSqrt, size_t prec);
csRNUMBER discreteSQRTComputing01(csRNUMBER*& blockNumber, csRNUMBER*& blockNumberRoot, size_t size, size_t prec);
csRNUMBER discreteSQRTComputing(csRNUMBER*& blockNumber, csRNUMBER*& blockNumberRoot, size_t size, size_t prec);
csRNUMBER discreteSQRTComputing2(csRNUMBER*& blockNumber, csRNUMBER*& blockNumberRoot, size_t nBlocks, size_t blockSize);
csRNUMBER discreteSQRTComputing3(csRNUMBER*& blockNumber, csRNUMBER*& blockNumberRoot, size_t nBlocks, size_t blockSize);
csRNUMBER discreteSQRTComputing4(csRNUMBER*& blockNumber, csRNUMBER*& blockNumberRoot, size_t nBlocks, size_t blockSize);
csRNUMBER discreteSQRTComputing41(csRNUMBER*& blockNumber, csRNUMBER*& blockNumberRoot, size_t nBlocks, size_t blockSize);
csRNUMBER discreteSQRTComputing5(csRNUMBER*& blockNumber, csRNUMBER*& blockNumberRoot, size_t nBlocks, size_t blockSize, size_t nBlocksPerThread);
csRNUMBER discreteSQRTComputing51(csRNUMBER*& blockNumber, csRNUMBER*& blockNumberRoot, size_t nBlocks, size_t blockSize, size_t nThread, csSQRTFAST::csSQRTFAST_INFO& info);

#endif // CSSQRT_H
