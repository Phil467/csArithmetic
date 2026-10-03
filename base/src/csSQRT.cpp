#include "csSQRT.h"
 
 using namespace __mem_man;
using namespace __ar_man;

csLOW_SQRT_COEFS LSQRTR(0);
csHIGHT_SQRT_COEFS HSQRTR(0);

csSQRTFAST::csSQRTFAST(size_t _nBlocks, size_t _precision)
{
    nBlocks = _nBlocks;
    precision = _precision;
    int prec = precision*2;
    prec = prec > 1000 ? 1000 : prec;
    unitsSqrt = csRNUMBER_PTR_ALLOC(10);
    nThread = 12;
}

csSQRTFAST::~csSQRTFAST()
{
    //dtor
}

void csSQRTFAST::getBlocks()
{
    double blockSz = 1.0*rn->mantSize/nBlocks;
    blockMantSize = size_t(blockSz);

    float dsz = blockSz-blockMantSize;
    size_t firstBlockSize = nBlocks*(dsz);

    if(blockSz < 1) {/*nBlocks = 1; */blockMantSize = 1;}
    if(dsz > 0) nBlocks++;
    firstBlockSize = firstBlockSize > 0 ? firstBlockSize : blockMantSize;

    blockCoef = csRNUMBER_PTR_ALLOC(nBlocks);
    blockCoefRoot = csRNUMBER_PTR_ALLOC(nBlocks);
    tenth = csRNUMBER_PTR_ALLOC(nBlocks);
    tenthSqrt = csRNUMBER_PTR_ALLOC(nBlocks);

    blockCoef[0].assign(rn->getMantissaSection(0, firstBlockSize));
    tenth[0].set("1", rn->mantSize-firstBlockSize, 0);
    tenth[0].reshape(tenth[0].exponent+1);
    

    for(size_t i=0; i<nBlocks-1; i++)
    {
        int i1 = i+1;
        blockCoef[i1].assign(rn->getMantissaSection(firstBlockSize+(i)*blockMantSize, firstBlockSize+(i1)*blockMantSize));
        tenth[i1].set("1", (nBlocks-2-i)*blockMantSize, 0);
        tenth[i1].reshape(tenth[i1].exponent+1);

    }

    
    csRNUMBER rt10("10");
    rt10 = csSqrt_Prec(rt10,20);

    for(size_t i=0; i<nBlocks; i++)
    {
        size_t bprec = 10;
        
        if(blockCoef[i].mantissa[0] == '0')
        {
            csRNUMBER tmp;
            tmp.copy(blockCoef[i]);
            removeFrontZeros(tmp.mantissa, tmp.mantSize);
            blockCoefRoot[i] = csSqrt_Prec(tmp,bprec);
            tmp.clear();
        }
        else
            blockCoefRoot[i] = csSqrt_Prec(blockCoef[i],bprec);

        if(tenth[i].mantSize%2 == 1)
            tenthSqrt[i].assign(tenth->getMantissaSection(0,(tenth[i].mantSize-1)/2 + 1));
        else
        {
            if(tenth[i].mantSize == 2)
                tenthSqrt[i].copy(rt10);
            else
            {
                tenthSqrt[i].assign(tenth->getMantissaSection(0,(tenth[i].mantSize)/2 - 1));
                tenthSqrt[i] = tenthSqrt[i] * rt10;
            }
        }

    }

    
}

void csSQRTFAST::makeSqrt()
{
    root = discreteSQRTComputing51(blockCoef, blockCoefRoot, nBlocks, blockMantSize, nThread, info);
}

void csSQRTFAST::printNumber(const char*title)
{
    rn->print(title);
}

void csSQRTFAST::printRoot(const char*title)
{
    root.print(title);
}

void csSQRTFAST::printError(const char*title)
{
    err = (root*root - *rn);
    err.print(title);
}

void csSQRTFAST::releaseBlocks()
{
    free(blockCoef);
    free(blockCoefRoot);
    free(tenth);
    free(tenthSqrt);
    if(unitsSqrt)
        free(unitsSqrt);
}

void csSQRTFAST::releaseNumber()
{
    rn->clear();
    root.clear();
    err.clear();
}

csRNUMBER rootApprox(csRNUMBER _a, csRNUMBER b, csRNUMBER aroot, csRNUMBER broot, size_t prec)
{
    csRNUMBER a;
    a.copy(_a);
    CSARITHMETIC_API CSARITHMETIC::setRNumberPrecision(prec);
    csRNUMBER n = a+b, n2 = n*n, n3 = n2*n;
    n2.forcePrecision(prec);
    n3.forcePrecision(prec);

    csRNUMBER a2 = a*a, a3 = a2*a;
    a2.forcePrecision(prec);
    a3.forcePrecision(prec);

    csRNUMBER cf11(223291, 0, 1), cf12(78125,0,0);
    csRNUMBER cf21(669873, 0, 0), cf22(156250,0,0);
    csRNUMBER cf31(535791, 0, 1), cf32(156250,0,0);

    csRNUMBER res = cf11*a3/(cf12*n3) + cf21*a2/(cf22*n2) + cf31*a/(cf32*n) + 1;
    res = (broot-aroot)/res;

    res.forcePrecision(prec);
    return res;
}

csRNUMBER rootApprox01(csRNUMBER _a, csRNUMBER b, csRNUMBER aroot, csRNUMBER broot, size_t prec)
{
    csRNUMBER a, ar, br;
    if(_a<b)
    {
        a.copy(_a);
        ar.copy(aroot);
        br.copy(broot);
    }
    else
    {
        a.copy(b);
        ar.copy(broot);
        br.copy(aroot);
    }
    CSARITHMETIC_API CSARITHMETIC::setRNumberPrecision(prec);
    csRNUMBER n = _a+b, n2 = n*n, n3 = n2*n, n4 = n3*n;
    n2.forcePrecision(prec);
    n3.forcePrecision(prec);
    n4.forcePrecision(prec);

    csRNUMBER a2 = a*a, a3 = a2*a, a4 = a3*a;
    a2.forcePrecision(prec);
    a3.forcePrecision(prec);
    a4.forcePrecision(prec);

    csRNUMBER cf11(30818487, 0, 1), cf12(6250000,0,0);
    csRNUMBER cf21(31171627, 0, 0), cf22(1875000,0,0);
    csRNUMBER cf31(27211823, 0, 1), cf32(781250,0,0);
    csRNUMBER cf41(31184132, 0, 0), cf42(1171875,0,0);
    
//(cf11*a/(cf12*n)+1).print( " a4 : ");
    csRNUMBER res = cf41*a4/(cf42*n4) + cf31*a3/(cf32*n3) + cf21*a2/(cf22*n2) + cf11*a/(cf12*n) + 1;
 
    res = (br-ar)/res;
//res.print("res : ");
    res.forcePrecision(prec);

    free(a.mantissa);
    free(ar.mantissa);
    free(br.mantissa);
    free(a2.mantissa);
    free(a3.mantissa);
    free(a4.mantissa);
    free(n.mantissa);
    free(n2.mantissa);
    free(n3.mantissa);
    free(n4.mantissa);
    free(cf11.mantissa);
    free(cf12.mantissa);
    free(cf21.mantissa);
    free(cf22.mantissa);
    free(cf31.mantissa);
    free(cf32.mantissa);
    free(cf41.mantissa);
    free(cf42.mantissa);
    return res;
}
csRNUMBER rootApprox02(csRNUMBER _a, csRNUMBER b, csRNUMBER aroot, csRNUMBER broot, size_t prec)
{
    csRNUMBER a, ar, br;
    if(_a<b)
    {
        a.copy(_a);
        ar.copy(aroot);
        br.copy(broot);
    }
    else
    {
        a.copy(b);
        ar.copy(broot);
        br.copy(aroot);
    }
    CSARITHMETIC_API CSARITHMETIC::setRNumberPrecision(prec);
    csRNUMBER n = _a+b, n2 = n*n, n3 = n2*n, n4 = n3*n;
    n2.forcePrecision(prec);
    n3.forcePrecision(prec);
    n4.forcePrecision(prec);

    csRNUMBER a2 = a*a, a3 = a2*a, a4 = a3*a;
    a2.forcePrecision(prec);
    a3.forcePrecision(prec);
    a4.forcePrecision(prec);

    csRNUMBER cf11(9829943, 0, 0), cf12(2500000,0,0);
    csRNUMBER cf21(79176791, 0, 1), cf22(4687500,0,0);
    csRNUMBER cf31(5435699, 0, 0), cf32(156250,0,0);
    csRNUMBER cf41(31454476, 0, 1), cf42(1171875,0,0);

    csRNUMBER res = cf41*a4/(cf42*n4) + cf31*a3/(cf32*n3) + cf21*a2/(cf22*n2) + cf11*a/(cf12*n) + 1;
    res = (br+ar)/res;

    res.forcePrecision(prec);

    free(a.mantissa);
    free(ar.mantissa);
    free(br.mantissa);
    free(a2.mantissa);
    free(a3.mantissa);
    free(a4.mantissa);
    free(n.mantissa);
    free(n2.mantissa);
    free(n3.mantissa);
    free(n4.mantissa);
    free(cf11.mantissa);
    free(cf12.mantissa);
    free(cf21.mantissa);
    free(cf22.mantissa);
    free(cf31.mantissa);
    free(cf32.mantissa);
    free(cf41.mantissa);
    free(cf42.mantissa);
    return res;
}

csROOT_BOUNDS getRootBounds(csRNUMBER _a, csRNUMBER _b, csRNUMBER aroot, csRNUMBER broot, csRNUMBER tenPow, csRNUMBER tenPowSqrt, size_t prec)
{
    csRNUMBER a, ar;
    a = _a*tenPow;
    ar = aroot*tenPowSqrt;
    
    CSARITHMETIC_API CSARITHMETIC::setRNumberPrecision(prec);
    csRNUMBER n = a+_b, bn = _b/n, bn2 = bn*bn, bn3 = bn2*bn, bn4 = bn3*bn;// use _b cause _b < a
    bn2.forcePrecision(prec);
    bn3.forcePrecision(prec);
    bn4.forcePrecision(prec);

    csROOT_BOUNDS art;
    art.number.copy(n);

    art.hightRoot = HSQRTR.cf4*bn4 + HSQRTR.cf3*bn3 + HSQRTR.cf2*bn2 + HSQRTR.cf1*bn + 1;
    
    art.hightRoot = (ar+broot)/art.hightRoot;

    art.lowRoot = LSQRTR.cf4*bn4 + LSQRTR.cf3*bn3 + LSQRTR.cf2*bn2 + LSQRTR.cf1*bn + 1;
    
    art.lowRoot = (ar-broot)/art.lowRoot;

    art.hightRoot.forcePrecision(prec);
    art.lowRoot.forcePrecision(prec);

    free(a.mantissa);
    free(ar.mantissa);
    free(n.mantissa);
    free(bn.mantissa);
    free(bn2.mantissa);
    free(bn3.mantissa);
    free(bn4.mantissa);

    return art;
}

csRNUMBER discreteSQRTComputing01(csRNUMBER*&blockNumber, csRNUMBER*&blockNumberRoot, size_t size, size_t prec)
{
    csRNUMBER root;
    size_t m = size-1;
    size_t n = m; 
    for(size_t j=1; j<=m-1; j++)
    {
        for(size_t i=0; i<n; i+=2)
        {
            size_t i1 = i+1, i_2 = i/2;
            root = rootApprox01(blockNumber[i], blockNumber[i1], blockNumberRoot[i], blockNumberRoot[i1], prec);
            blockNumber[i_2] = blockNumber[i] + blockNumber[i1];
            blockNumberRoot[i_2] = NRSqrtR0(blockNumber[i_2], root, prec, 5);
        }
        n/=2;
    }
    root.copy(blockNumberRoot[0]);
    return root;
}

csRNUMBER discreteSQRTComputing(csRNUMBER*&blockNumber, csRNUMBER*&blockNumberRoot, size_t nBlocks, size_t blockMantSize)
{
    csRNUMBER root;
    size_t m = nBlocks-1;
    size_t n = m, bs = blockMantSize; 
    csROOT_BOUNDS art;
    csRNUMBER tenPow("1",blockMantSize), tenPowSqrt = cs10PSqrt_Prec(blockMantSize, blockMantSize);
    
    for(size_t j=1; j<=m-1; j++)
    {
        for(size_t i=0; i<n; i+=2)
        {
            size_t i1 = i+1, i_2 = i/2;
            if(j > 1)
            {
                tenPow.set("1", blockNumber[i1].mantSize,0);
                tenPowSqrt.set("1", blockNumber[i1].mantSize/2,0);
            }
            art = getRootBounds(blockNumber[i], blockNumber[i1], blockNumberRoot[i], blockNumberRoot[i1], tenPow, tenPowSqrt, 10);
            cout<<blockNumber[i]<<" "<<blockNumber[i1]<<" => " << art.number << "  ---  ";
            //cout<<art.lowRoot<<" "<<art.hightRoot<<" => ";
            blockNumber[i_2].copy(art.number);
            blockNumberRoot[i_2] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, 10);
            cout<<blockNumberRoot[i_2]<<"   --root\n";
            art.clear();
        }
        tenPowSqrt.copy(tenPow);
        /*bs *= 2;
        tenPow.set("1", bs,0);*/
        n/=2;
    }
    root.copy(blockNumberRoot[0]);
    return root;
}

csRNUMBER discreteSQRTComputing2(csRNUMBER*&blockNumber, csRNUMBER*&blockNumberRoot, size_t nBlocks, size_t blockMantSize)
{
    csRNUMBER root;
    size_t m = nBlocks-1;
    size_t n = m, bs = blockMantSize; 
    csROOT_BOUNDS art;
    csRNUMBER tenPow("1",blockMantSize), tenPowSqrt = cs10PSqrt_Prec(blockMantSize, blockMantSize);
    for(size_t j=1; j<nBlocks; j++)
    {
        art = getRootBounds(blockNumber[0], blockNumber[j], blockNumberRoot[0], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
        //cout<<blockNumber[0]<<" "<<blockNumber[j]<<" => " << art.number << "  ---  ";
        //cout<<art.lowRoot<<" "<<art.hightRoot<<" => ";
        blockNumber[0].copy(art.number);
        blockNumberRoot[0] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
        //cout<<blockNumberRoot[0]<<"   --root\n";
        art.clear();
    }
    root.copy(blockNumberRoot[0]);
    return root;
}

csRNUMBER discreteSQRTComputing3(csRNUMBER*&blockNumber, csRNUMBER*&blockNumberRoot, size_t nBlocks, size_t blockMantSize)
{
    csRNUMBER root;
    size_t m = nBlocks/2;
    size_t n = m, bs = blockMantSize; 
    csROOT_BOUNDS art;
    csRNUMBER tenPow("1",blockMantSize), rt10("10")/*, tenPowSqrt = cs10PSqrt_Prec(blockMantSize, blockMantSize)*/;
    rt10 = csSqrt_Prec(rt10, blockMantSize*nBlocks/2);
    
    csRNUMBER tenPowSqrt = cs10PSqrt_Prec(rt10, blockMantSize);
    
    for(size_t j=1; j<m; j++)
    {
        art = getRootBounds(blockNumber[0], blockNumber[j], blockNumberRoot[0], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
        //cout<<blockNumber[0]<<" "<<blockNumber[j]<<" => " << art.number << "  ---  ";
        //cout<<art.lowRoot<<" "<<art.hightRoot<<" => ";
        blockNumber[0].copy(art.number);
        blockNumberRoot[0] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
        //cout<<blockNumberRoot[0]<<"   --root\n";
        art.clear();
    }
    for(size_t j=m+1; j<nBlocks; j++)
    {
        art = getRootBounds(blockNumber[m], blockNumber[j], blockNumberRoot[m], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
        //cout<<blockNumber[0]<<" "<<blockNumber[j]<<" => " << art.number << "  ---  ";
        //cout<<art.lowRoot<<" "<<art.hightRoot<<" => ";
        blockNumber[m].copy(art.number);
        blockNumberRoot[m] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
        //cout<<blockNumberRoot[0]<<"   --root\n";
        art.clear();
    }
    long sz = blockNumber[m].mantSize+blockNumber[m].exponent;
    tenPow.set("1", sz,0);
    tenPowSqrt = cs10PSqrt_Prec(rt10, sz);
    art = getRootBounds(blockNumber[0], blockNumber[m], blockNumberRoot[0], blockNumberRoot[m], tenPow, tenPowSqrt, 10);
    //cout<<blockNumber[0]<<" "<<blockNumber[j]<<" => " << art.number << "  ---  ";
    //cout<<art.lowRoot<<" "<<art.hightRoot<<" => ";
    blockNumber[0].copy(art.number);
    blockNumberRoot[0] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
    //cout<<blockNumberRoot[0]<<"   --root\n";
    art.clear();

    root.copy(blockNumberRoot[0]);
    return root;
}

csRNUMBER discreteSQRTComputing4(csRNUMBER*&blockNumber, csRNUMBER*&blockNumberRoot, size_t nBlocks, size_t blockMantSize)
{
    csRNUMBER root;
    size_t m = nBlocks/3, m2 = 2*m;
    size_t n = m, bs = blockMantSize; 
    csROOT_BOUNDS art;
    csRNUMBER tenPow("1",blockMantSize), rt10("10")/*, tenPowSqrt = cs10PSqrt_Prec(blockMantSize, blockMantSize)*/;
    rt10 = csSqrt_Prec(rt10, blockMantSize*nBlocks/3);
    
    csRNUMBER tenPowSqrt = cs10PSqrt_Prec(rt10, blockMantSize);
    
    for(size_t j=1; j<m; j++)
    {
        art = getRootBounds(blockNumber[0], blockNumber[j], blockNumberRoot[0], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
        blockNumber[0].copy(art.number);
        blockNumberRoot[0] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
        art.clear();
    }

    size_t mTopZeroCount = blockNumber[m].mantissa[0] == '0' ? 1 : 0;

    for(size_t j=m+1; j<m2; j++)
    {
        art = getRootBounds(blockNumber[m], blockNumber[j], blockNumberRoot[m], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
        blockNumber[m].copy(art.number);
        blockNumberRoot[m] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
        art.clear();
    }

    size_t m2TopZeroCount = blockNumber[m2].mantissa[0] == '0' ? 1 : 0;
    
    for(size_t j=m2+1; j<nBlocks; j++)
    {
        art = getRootBounds(blockNumber[m2], blockNumber[j], blockNumberRoot[m2], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
        blockNumber[m2].copy(art.number);
        blockNumberRoot[m2] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
        art.clear();
    }
    
    long sz = blockNumber[m].mantSize+blockNumber[m].exponent + mTopZeroCount;
    tenPow.set("1", sz,0);
    tenPowSqrt = cs10PSqrt_Prec(rt10, sz);
    art = getRootBounds(blockNumber[0], blockNumber[m], blockNumberRoot[0], blockNumberRoot[m], tenPow, tenPowSqrt, 10);
    blockNumber[0].copy(art.number);
    blockNumberRoot[0] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
    art.clear();

    sz = blockNumber[m2].mantSize+blockNumber[m2].exponent + m2TopZeroCount;
    tenPow.set("1", sz,0);
    tenPowSqrt = cs10PSqrt_Prec(rt10, sz);
    art = getRootBounds(blockNumber[0], blockNumber[m2], blockNumberRoot[0], blockNumberRoot[m2], tenPow, tenPowSqrt, 10);
    blockNumber[0].copy(art.number);
    blockNumberRoot[0] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
    art.clear();

    root.copy(blockNumberRoot[0]);
    return root;
}

csRNUMBER discreteSQRTComputing41(csRNUMBER*&blockNumber, csRNUMBER*&blockNumberRoot, size_t nBlocks, size_t blockMantSize)
{
    size_t m = nBlocks/4, m2 = 2*m, m3 = 3*m;
    size_t n = m, bs = blockMantSize; 
    csROOT_BOUNDS art;
    csRNUMBER tenPow("1",blockMantSize), rt10("10")/*, tenPowSqrt = cs10PSqrt_Prec(blockMantSize, blockMantSize)*/;
    rt10 = csSqrt_Prec(rt10, blockMantSize*nBlocks/4);
    
    csRNUMBER tenPowSqrt = cs10PSqrt_Prec(rt10, blockMantSize);
    
    for(size_t j=1; j<m; j++)
    {
        art = getRootBounds(blockNumber[0], blockNumber[j], blockNumberRoot[0], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
        blockNumber[0].copy(art.number);
        blockNumberRoot[0] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
        art.clear();
    }
    
    size_t mTopZeroCount = blockNumber[m].mantissa[0] == '0' ? 1 : 0; // important if blockNumber[m] contains '0' on top, to preserve this size
    
    for(size_t j=m+1; j<m2; j++)
    {
        //blockNumber[j].print(" block : ");
        art = getRootBounds(blockNumber[m], blockNumber[j], blockNumberRoot[m], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
        blockNumber[m].copy(art.number);
        blockNumberRoot[m] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
        art.clear();
    }
    
    size_t m2TopZeroCount = blockNumber[m2].mantissa[0] == '0' ? 1 : 0; // important if blockNumber[m] contains '0' on top, to preserve this size
    
    for(size_t j=m2+1; j<m3; j++)
    {
        //blockNumber[j].print(" block : ");
        art = getRootBounds(blockNumber[m2], blockNumber[j], blockNumberRoot[m2], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
        blockNumber[m2].copy(art.number);
        blockNumberRoot[m2] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
        art.clear();
    }

    size_t m3TopZeroCount = blockNumber[m3].mantissa[0] == '0' ? 1 : 0; // important if blockNumber[m] contains '0' on top, to preserve this size

    for(size_t j=m3+1; j<nBlocks; j++)
    {
        art = getRootBounds(blockNumber[m3], blockNumber[j], blockNumberRoot[m3], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
        blockNumber[m3].copy(art.number);
        blockNumberRoot[m3] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
        art.clear();
    }
    
    long sz = blockNumber[m].mantSize+blockNumber[m].exponent + mTopZeroCount;
    tenPow.set("1", sz,0);
    tenPowSqrt = cs10PSqrt_Prec(rt10, sz);
    art = getRootBounds(blockNumber[0], blockNumber[m], blockNumberRoot[0], blockNumberRoot[m], tenPow, tenPowSqrt, 10);
    blockNumber[0].copy(art.number);
    blockNumberRoot[0] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
    art.clear();

    sz = blockNumber[m3].mantSize+blockNumber[m3].exponent + m3TopZeroCount;
    tenPow.set("1", sz,0);
    tenPowSqrt = cs10PSqrt_Prec(rt10, sz);
    art = getRootBounds(blockNumber[m2], blockNumber[m3], blockNumberRoot[m2], blockNumberRoot[m3], tenPow, tenPowSqrt, 10);
    blockNumber[m2].copy(art.number);
    blockNumberRoot[m2] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
    art.clear();

    sz = blockNumber[m2].mantSize+blockNumber[m2].exponent + m2TopZeroCount; // (m2-m)*blockMantSize
    tenPow.set("1", sz,0);
    tenPowSqrt = cs10PSqrt_Prec(rt10, sz);
    art = getRootBounds(blockNumber[0], blockNumber[m2], blockNumberRoot[0], blockNumberRoot[m2], tenPow, tenPowSqrt, 10);
    blockNumber[0].copy(art.number);
    blockNumberRoot[0] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
    art.clear();

    csRNUMBER root;
    root.copy(blockNumberRoot[0]);
    
    return root;
}

csRNUMBER discreteSQRTComputing5(csRNUMBER*&blockNumber, csRNUMBER*&blockNumberRoot, size_t nBlocks, size_t blockMantSize, size_t nBlocksPerThread)
{
    csRNUMBER root;
    size_t m = nBlocks/nBlocksPerThread, nBlocksRemain = nBlocks % nBlocksPerThread;
    size_t n = m, bs = blockMantSize; 
    csROOT_BOUNDS art;
    csRNUMBER tenPow("1",blockMantSize), rt10("10")/*, tenPowSqrt = cs10PSqrt_Prec(blockMantSize, blockMantSize)*/;
    rt10 = csSqrt_Prec(rt10, blockMantSize*m);

    csRNUMBER tenPowSqrt = cs10PSqrt_Prec(rt10, blockMantSize);
    size_t n1, n2, topZeroCount[m+1];
    for(size_t i=0; i<m; i++)
    {
        n1 = i*nBlocksPerThread;
        //n2 = nBlocksPerThread*(i+1);
        n2 = n1 + nBlocksPerThread;
        topZeroCount[i] = (blockNumber[n1].mantissa[0] == '0' ? 1 : 0);

        for(size_t j=n1+1; j<n2; j++)
        {
            art = getRootBounds(blockNumber[n1], blockNumber[j], blockNumberRoot[n1], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
            blockNumber[n1].copy(art.number);
            blockNumberRoot[n1] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
            art.clear();
        }
        blockNumber[i].copy(blockNumber[n1]);// non parallelisable pour des raisons de synchronisation
        blockNumberRoot[i].copy(blockNumberRoot[n1]);

    }

    if(nBlocksRemain)
    {
        n1 = n2;
        n2 += nBlocksRemain;
        if(n2-n1 > 1)
            topZeroCount[m] = (blockNumber[n1].mantissa[0] == '0' ? 1 : 0);
        for(size_t j=n1+1; j<n2; j++)
        {
            art = getRootBounds(blockNumber[n1], blockNumber[j], blockNumberRoot[n1], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
            blockNumber[n1].copy(art.number);
            blockNumberRoot[n1] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
            art.clear();
        }
    
        blockNumber[m].copy(blockNumber[n1]);
        blockNumberRoot[m].copy(blockNumberRoot[n1]);
        
        m++;
    }
    size_t last;
    do
    {
        nBlocksRemain = m % 2; // nBlocksPerThread = 2 pour le reste des calculs, et nblocks devient m (la taille du tableau)
        //cout<<m <<"  r = "<<nBlocksRemain<<" end \n";
        last = m-nBlocksRemain;
        m = m/2; // calul de la nouvelle taille

        for(size_t i=0; i<m; i++)
        {
            n1 = i*2;
            n2 = n1 + 1;
            long sz = blockNumber[n2].mantSize+blockNumber[n2].exponent + topZeroCount[n2];
            topZeroCount[n2] = 0;
            tenPow.set("1", sz,0);
            tenPowSqrt = cs10PSqrt_Prec(rt10, sz);

            art = getRootBounds(blockNumber[n1], blockNumber[n2], blockNumberRoot[n1], blockNumberRoot[n2], tenPow, tenPowSqrt, 10);
            blockNumber[n1].copy(art.number);
            blockNumberRoot[n1] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, art.number.mantSize);
            art.clear();

            blockNumber[i].copy(blockNumber[n1]);// non parallelisable pour des raisons de synchronisation
            blockNumberRoot[i].copy(blockNumberRoot[n1]);
            topZeroCount[i] = topZeroCount[n1];
        }
        if(nBlocksRemain == 1)
        {
            blockNumber[m].copy(blockNumber[last]);
            blockNumberRoot[m].copy(blockNumberRoot[last]);
            topZeroCount[m] = topZeroCount[last];
            
            m++;
        }
        
    }
    while (m > 1);

    root.copy(blockNumberRoot[0]);
    return root;
}

csRNUMBER discreteSQRTComputing51(csRNUMBER*&blockNumber, csRNUMBER*&blockNumberRoot, size_t nBlocks, size_t blockMantSize, size_t nThread, csSQRTFAST::csSQRTFAST_INFO& info)
{
    csRNUMBER root;
    if(nThread*2 > nBlocks)
    {
        cout<<" Please choose nThread such as nThread*2 < nBlocks !";
        return root;
    }
    size_t m = nThread, nBlocksPerThread = nBlocks/m, nBlocksRemain = nBlocks % m;
    size_t m1 = m-1, bs = blockMantSize; 
    csROOT_BOUNDS art;
    csRNUMBER tenPow("1",blockMantSize), rt10("10")/*, tenPowSqrt = cs10PSqrt_Prec(blockMantSize, blockMantSize)*/;
    rt10 = csSqrt_Prec(rt10, blockMantSize*nBlocksPerThread);

    csRNUMBER tenPowSqrt = cs10PSqrt_Prec(rt10, blockMantSize);
    size_t n1=0, n2=0, topZeroCount[m+1];

    info.STEP1_MAX = m1;

    for(size_t i=0; i<m1; i++)
    {
        //n1 = i*nBlocksPerThread;
        //n2 = nBlocksPerThread*(i+1);
        n2 = n1 + nBlocksPerThread;
        topZeroCount[i] = (blockNumber[n1].mantissa[0] == '0' ? 1 : 0);
        for(size_t j=n1+1; j<n2; j++)
        {
            art = getRootBounds(blockNumber[n1], blockNumber[j], blockNumberRoot[n1], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
            blockNumber[n1].copy(art.number);
            blockNumberRoot[n1] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, 20);
            art.clear();
        }
        blockNumber[i].copy(blockNumber[n1]);// non parallelisable pour des raisons de synchronisation
        blockNumberRoot[i].copy(blockNumberRoot[n1]);

        n1 += nBlocksPerThread;

        info.STEP1_ITER = i;
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    info.STEP1_ITER = m;

    n1 = n2;
    n2 = nBlocks;
    info.STEP2_MAX = n2-n1-1;
    if(n2 > n1+1)
        topZeroCount[m1] = (blockNumber[n1].mantissa[0] == '0' ? 1 : 0);
    for(size_t j=n1+1; j<n2; j++)
    {
        art = getRootBounds(blockNumber[n1], blockNumber[j], blockNumberRoot[n1], blockNumberRoot[j], tenPow, tenPowSqrt, 10);
        blockNumber[n1].copy(art.number);
        blockNumberRoot[n1] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, 20);
        art.clear();

        info.STEP2_ITER = j;
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    blockNumber[m1].copy(blockNumber[n1]);
    blockNumberRoot[m1].copy(blockNumberRoot[n1]);

    info.STEP2_ITER = info.STEP2_MAX;
    
    size_t last;
    info.STEP3_MAX = ceil(log(m)/log(2));
    do
    {
        nBlocksRemain = m % 2; // nBlocksPerThread = 2 pour le reste des calculs, et nblocks devient m (la taille du tableau)
        //cout<<m <<"  r = "<<nBlocksRemain<<" end \n";
        last = m-nBlocksRemain;
        m = m/2; // calul de la nouvelle taille

        for(size_t i=0; i<m; i++)
        {
            n1 = i*2;
            n2 = n1 + 1;
            long sz = blockNumber[n2].mantSize+blockNumber[n2].exponent + topZeroCount[n2];
            topZeroCount[n2] = 0;
            tenPow.set("1", sz,0);
            tenPowSqrt = cs10PSqrt_Prec(rt10, sz);

            art = getRootBounds(blockNumber[n1], blockNumber[n2], blockNumberRoot[n1], blockNumberRoot[n2], tenPow, tenPowSqrt, 10);
            blockNumber[n1].copy(art.number);
            blockNumberRoot[n1] = csSqrt0_Prec(art.number, art.lowRoot, art.hightRoot, 20);
            art.clear();

            blockNumber[i].copy(blockNumber[n1]);// non parallelisable pour des raisons de synchronisation
            blockNumberRoot[i].copy(blockNumberRoot[n1]);
            topZeroCount[i] = topZeroCount[n1];
        }
        if(nBlocksRemain == 1)
        {
            blockNumber[m].copy(blockNumber[last]);
            blockNumberRoot[m].copy(blockNumberRoot[last]);
            topZeroCount[m] = topZeroCount[last];
            //blockNumber[m].print(" --- ");
            m++;
        }

        info.STEP3_ITER++;
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        
    }
    while (m > 1);

    root.copy(blockNumberRoot[0]);

    info.STEP3_ITER = info.STEP3_MAX;
    return root;
}

void csSQRTFAST::getUnitsSqrt()
{
    
}