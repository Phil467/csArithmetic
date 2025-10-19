
#include "utilities.h"

int TAB_PRINT_ID;

csQNUMBER fxi(csQNUMBER xi, csQNUMBER*cf, size_t P)
{
    csQNUMBER y("0");
    for (int i=P-1; i>=0; i--)
    {
        y = y + cf[P-1-i]*(pow(xi, i));
    }
	return y;
}

csQNUMBER* fx(csQNUMBER*x, size_t n, csQNUMBER*cf, size_t P)
{
    csQNUMBER*y = csQNUMBER_PTR_ALLOC(n);
    int P1 = P-1;
    printArray1D_Column(cf,P, " coefs : ", 1);
	for (int j=0; j<n; j++)
	{
		csQNUMBER k("0");
		for (int i=P-1; i>=0; i--)
		{
            k = k + cf[P1-i] * (pow(x[j], i));
		}
		y[j] = k;
	}
	return y;
}

void fx2(csQNUMBER*x, csQNUMBER*& y, size_t n, csQNUMBER*cf, size_t P)
{
    for (int j=0; j<n; j++)
	{
		csQNUMBER k("0");
		for (int i=P-1; i>=0; i--)
		{
			k = k + cf[P-1-i]*(pow(x[j], i));
		}
		y[j] = k;
	}
}

void fx21(csQNUMBER*x, csQNUMBER*& y, size_t id1, size_t id2, csQNUMBER*cf, size_t P)
{
    for (int j=id1; j<id2; j++)
	{
		csQNUMBER k("0");
		for (int i=P-1; i>=0; i--)
		{
			k = k + cf[P-1-i]*(pow(x[j-id1], i));
		}
		y[j] = k;
	}
}


csQNUMBER* getIntPowList(size_t n, int p)
{
	csQNUMBER* l = csQNUMBER_PTR_ALLOC(n);
	for (int i=1; i<n+1; i++)
		l[i-1] = (int)pow(i,p);
	return l;
}

csQNUMBER* getCumulList(csQNUMBER*l, size_t n)
{
	csQNUMBER* r = csQNUMBER_PTR_ALLOC(n);
	r[0].copy(l[0]);
	for (int i = 1; i<n; i++)
		r[i] = r[i-1] + l[i];
	return r;
}


void printArray1D(csQNUMBER* arr, size_t n, const char*title, size_t wordLen)
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
void printArray1D_Column(csQNUMBER* arr, size_t n, const char*title, bool showQuotient)
{
    if (strcasecmp(title,"Table")==0)
        std::cout<<"Table id : "<<TAB_PRINT_ID<<"\n";
    else
        std::cout<<title<<"\n";

	if(showQuotient)
    for (int i=0; i<n; i++)
    {
        std::cout<<arr[i].quotient()<<"\n";
    }

	else
	for (int i=0; i<n; i++)
    {
        std::cout<<arr[i]<<"\n";
    }
    std::cout<<"\n";
    TAB_PRINT_ID++;
}

void printArray1D(size_t* arr, size_t n, const char*title, size_t wordLen)
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

void printArray2D(csQNUMBER** arr, size_t nx, size_t ny, const char*title, size_t wordLen)
{
    if (strcasecmp(title,"Table")==0)
        std::cout<<"Table id : "<<TAB_PRINT_ID<<"\n";
    else
        std::cout<<title<<"\n";
    for (int j=0; j<ny; j++)
    {
    	for (int i=0; i<nx; i++)
		{
			std::cout<<setw(wordLen)<<setprecision(2)<<arr[j][i]<<"  ";
		}
    	std::cout<<"\n";
    }
    std::cout<<"\n\n";
    TAB_PRINT_ID++;
}

void printArray2D(size_t** arr, size_t nx, size_t ny, const char*title, size_t wordLen)
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

/*Fraction findFraction(csQNUMBER nb, double  prec)
{
	csQNUMBER anb = fabs(nb), r("1";
	int n1 = int(anb);
	int n2 = n1 + 1;
	int i=0, j=0;

	while (abs(r) > prec)
	{
		if (r > 0)
			i = i+1;
		if (r < 0)
			j = j+1;
		r = csQNUMBER(n1*i + n2*j)/(i+j) - anb;
		// if j == 100 or i == 100:
			// break
	}
	Fraction f = {int64_t((nb/anb)*(n1*i + n2*j)), int64_t(i+j)};

	return f;
}

Fraction* findFractionList(csQNUMBER*nbs, size_t n, double  prec)
{
	Fraction*frac = (Fraction*)malloc(sizeof(Fraction)*n);
	for (int i=0; i<n; i++)
		frac[i] = findFraction(nbs[i], prec);
	return frac;
}

void printFactionsArray(Fraction* arr, size_t n, const char*title, size_t wordLen)
{
    if (strcasecmp(title,"Table")==0)
        std::cout<<"Table id : "<<TAB_PRINT_ID<<"\n";
    else
        std::cout<<title<<"\n";
    for (int i=0; i<n; i++)
    {
        std::cout<<arr[i].a<<"/"<<arr[i].b<<"  ";
    }
    std::cout<<"\n\n";
    TAB_PRINT_ID++;
}
*/

csQNUMBER* linspace(csQNUMBER a, csQNUMBER b, size_t n)
{
    if (n<2)
    {
        std::cout<<"\nlinspace message : n must be greater than 2 !\n";
        exit(0);
    }
    else
    {
        csQNUMBER *r = csQNUMBER_PTR_ALLOC(n);
        double _a = a.getDouble(), _b = b.getDouble();
//cout<<" +++ yuuj \n";
        //double step = (_b-_a)/(n-1);
		size_t n1 = n-1;
        for (size_t i=0; i<n; i++)
        {
            csQNUMBER I(_a*(n1-i) + _b*i, n1);
            r[i] = I;
        }
        return r;
    }
}

csQNUMBER* linspaceExcb(csQNUMBER a, csQNUMBER b, size_t n)
{
    if (n<1)
    {
        std::cout<<"\nError from linspaceExcb() : n must be greater than 2 !\n";
        exit(0);
    }
    else
    {
        csQNUMBER *r = csQNUMBER_PTR_ALLOC(n+1);// inclusive
        double _a = a.getDouble(), _b = b.getDouble();

        for (size_t i=0; i<=n; i++)
        {
            csQNUMBER I(_a*(n-i) + _b*i, n);
            r[i] = I;
        }

        return r;
    }
}



csQNUMBER* array2dToArray1D2(csQNUMBER** a2d, size_t nx, size_t ny)
{
	csQNUMBER *a1d = csQNUMBER_PTR_ALLOC(nx*ny);
	size_t k;
	for(size_t j = 0; j<ny; j++)
	{
		for(size_t i = 0; i<nx; i++)
		{
			k = j*nx + i;
			a2d[j][i] = a1d[k];
		}
	}
	return a1d;
}


void array2dToArray1D(csQNUMBER** a2d, size_t nx, size_t ny, csQNUMBER*& a1d)
{
	size_t k;
	for(size_t j = 0; j<ny; j++)
	{
		for(size_t i = 0; i<nx; i++)
		{
			k = j*nx + i;
			a2d[j][i] = a1d[k];
		}
	}
}

size_t firstDegreePyramidalArrayIndexLevel(size_t level)
{
	return (level*(level+1)/2);
}

size_t secondDegreePyramidalArrayIndexLevel(size_t level)
{
	//csQNUMBER idf = level*level*level/3.0 + level*level/2.0 + level/6.0 , dec = idf - (csQNUMBER)(int)(idf);
	double idf = level*(level*(level/3.0 + 0.5) + 1/6.0) , dec = idf - (double)(int)(idf);
	size_t id = size_t(idf);
	if (dec >= 0.5)
		id += 1;
	return id;
}

double getRandomDouble(int64_t min, int64_t max, size_t exponent)
{
	return (min + rand()%(max-min))/pow(10,double(exponent));
}

double*getRandomDoubleArray1D(size_t n, int64_t min, int64_t max, size_t exponent)
{
	double*arr = (double*)malloc(n*sizeof(double));
	for (int i=0; i<n; i++)
	{
		arr[i] = getRandomDouble(min, max, exponent);
	}
	return arr;
}

double**getRandomDoubleArray2D(size_t nx, size_t ny, int64_t min, int64_t max, size_t exponent)
{
	double**arr = (double**)malloc(ny*sizeof(double*));
	for (int i=0; i<ny; i++)
	{
		arr[i] = getRandomDoubleArray1D(nx, min, max, exponent);
	}
	return arr;
}

csQNUMBER sum(csQNUMBER*arr, size_t n)
{
	csQNUMBER s("0");
	for (int i=0; i<n; i++)
		s = s + arr[i];
	return s;
}
/*
csQNUMBER round(csQNUMBER nb, size_t ndigits)
{
	double pw = pow(10,(double)ndigits),  nb1 = nb*pw;
	double decimal = nb1-(double)(int)nb1;
	if (decimal >= 0.5)
		nb1 += 1;
	return (double)((int)nb1)/pw;
}

csQNUMBER roundZero(csQNUMBER nb)
{
	csQNUMBER nb1 = nb, decimal = nb-(csQNUMBER)(int)nb;
	if (decimal >= 0.5)
		nb1 += 1;
	return (csQNUMBER)((int)nb1);
}

csQNUMBER* arrayRound(csQNUMBER*arr, size_t n, size_t ndigits)
{
	csQNUMBER*arr2 = (csQNUMBER*)malloc(n*sizeof(csQNUMBER));
	for (int i=0; i<n; i++)
		arr2[i] = round(arr[i], ndigits);
	return arr2;
}

void arrayRound2(csQNUMBER*&arr, size_t n, size_t ndigits)
{
	csQNUMBER*arr2 = (csQNUMBER*)malloc(n*sizeof(csQNUMBER));
	for (int i=0; i<n; i++)
		arr2[i] = round(arr2[i], ndigits);
}

csQNUMBER* arrayBoost(csQNUMBER* arr, size_t n, csQNUMBER fact)
{
	csQNUMBER*arr2 = (csQNUMBER*)malloc(n*sizeof(csQNUMBER));
	for (int i=0; i<n; i++)
		arr2[i] = arr[i]*fact;
	return arr2;
}

void minmax1D(csQNUMBER*arr, size_t n, size_t& minval, size_t& maxval)
{
	minval = arr[0];
	maxval = arr[0];
	for (int i=0; i<n; i++)
	{
		if(minval > arr[i])
			minval = arr[i];
		if(maxval < arr[i])
			maxval = arr[i];
	}
}

void minmax2D(csQNUMBER**arr, size_t nx, size_t ny, csQNUMBER& minval, csQNUMBER& maxval)
{
	minval = arr[0][0];
	maxval = arr[0][0];
	for (int j=0; j<ny; j++)
	{
		for (int i=0; i<nx; i++)
		{
			if(minval > arr[j][i])
				minval = arr[j][i];
			if(maxval < arr[j][i])
				maxval = arr[j][i];
		}
	}
}

void arrayBoost2(csQNUMBER*& arr, size_t n, csQNUMBER fact)
{
	for (int i=0; i<n; i++)
		arr[i] = arr[i]*fact;

}

/*void setStrRGB(char*str,   size_t pos,  csRGBA c)
{
	*(str+pos)=c.b;
	*(str+pos+1)=c.g;
	*(str+pos+2)=c.r;
	*(str+pos+3)=0;
}*/

void setStrRGB2(char*str,   size_t pos,  size_t r, size_t g, size_t b)
{
	*(str+pos)=(unsigned char)r;
	*(str+pos+1)=(unsigned char)g;
	*(str+pos+2)=(unsigned char)r;
	*(str+pos+3)=0;
}

size_t**fullUInt2D(size_t nx, size_t ny, size_t nb)
{
	size_t**arr = (size_t**)malloc(sizeof(size_t*)*ny);
	for (int j=0; j<ny; j++)
	{
		arr[j] = (size_t*)malloc(sizeof(size_t)*nx);
		for (int i=0; i<nx; i++)
		{
			arr[j][i]=nb;
		}
	}
	return arr;
}

int**fullInt2D(size_t nx, size_t ny, int nb)
{
	int**arr = (int**)malloc(sizeof(int*)*ny);
	for (int j=0; j<ny; j++)
	{
		arr[j] = (int*)malloc(sizeof(int)*nx);
		for (int i=0; i<nx; i++)
		{
			arr[j][i]=nb;
		}
	}
	return arr;
}

csQNUMBER**fullDouble2D(size_t nx, size_t ny, csQNUMBER nb)
{
	csQNUMBER**arr = (csQNUMBER**)malloc(sizeof(csQNUMBER*)*ny);
	for (int j=0; j<ny; j++)
	{
		arr[j] = csQNUMBER_PTR_ALLOC(nx);
		for (int i=0; i<nx; i++)
		{
			arr[j][i]=nb;
		}
	}
	return arr;
}


csQNUMBER* trimDouble1D(csQNUMBER*map, size_t nx, size_t ny, int x, int cx)
{
    csQNUMBER*ret=0;
    if(x>=0 && x<=nx && cx>x && cx<nx)
    {
        csQNUMBER*ret=(csQNUMBER*)malloc(cx*sizeof(csQNUMBER));
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

int* trimInt1D(int*map, size_t nx, size_t ny, int x, int cx)
{
    int*ret=0;
    if(x>=0 && x<=nx && cx>x && cx<nx)
    {
        int*ret=(int*)malloc(cx*sizeof(int));
        for(int i=0; i<cx; i++)
        {
            ret[i] = map[i+x];
        }
    }
    else
    {
        std::cout<<"From trimInt1D() : Invalid trimming sizes."<<std::endl;
        ret = map;
    }
    return ret;
}

char* trimDouble1D(char*map, size_t nx, size_t ny, int x, int cx)
{
    char*ret=0;
    if(x>=0 && x<=nx && cx>x && cx<nx)
    {
        char*ret=(char*)malloc(cx*sizeof(char));
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

csQNUMBER** trimDouble2D(csQNUMBER**map, size_t nx, size_t ny, int x, int y, int cx, int cy)
{
    csQNUMBER**ret=0;
    if(x>=0 && x<=nx && y>=0 && y<=ny && cx>x && cx<nx && cy>y && cy<=ny)
    {
        csQNUMBER**ret=(csQNUMBER**)malloc(cy*sizeof(csQNUMBER*));
        for(int j=0; j<cy; j++)
        {
            ret[j] = csQNUMBER_PTR_ALLOC(cx);
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

int** trimInt2D(int**map, size_t nx, size_t ny, int x, int y, int cx, int cy)
{
    int**ret=0;
    if(x>=0 && x<=nx && y>=0 && y<=ny && cx>x && cx<nx && cy>y && cy<=ny)
    {
        int**ret=(int**)malloc(cy*sizeof(int*));
        for(int j=0; j<cy; j++)
        {
            ret[j] = (int*)malloc(cx*sizeof(int));
            for(int i=0; i<cx; i++)
                ret[j][i] = map[j+y][i+x];
        }
    }
    else
    {
        std::cout<<"From trimInt2D() : Invalid trimming sizes."<<std::endl;
        ret = map;
    }
    return ret;
}

char** trimChar2D(char**map, size_t nx, size_t ny, int x, int y, int cx, int cy)
{
    char**ret=0;
    if(x>=0 && x<=nx && y>=0 && y<=ny && cx>x && cx<nx && cy>y && cy<=ny)
    {
        char**ret=(char**)malloc(cy*sizeof(char*));
        for(int j=0; j<cy; j++)
        {
            ret[j] = (char*)malloc(cx*sizeof(char));
            for(int i=0; i<cx; i++)
                ret[j][i] = map[j+y][i+x];
        }
    }
    else
    {
        std::cout<<"From trimInt2D() : Invalid trimming sizes."<<std::endl;
        ret = map;
    }
    return ret;
}




