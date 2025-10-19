#include <csARITHMETIC.h>

using namespace __mem_man;
using namespace __ar_man;

extern csBIDIGITS** addTable;

void CSARITHMETIC_API CSARITHMETIC::createAdditionTable()
{
  int** tab = csAlloc<int*>(58);

  for(int j=0; j<58; j++)
  {
    tab[j] = csAlloc<int>(58,0);
  }

  for(int j=48; j<58; j++)
  {
    for(int i=48; i<58; i++)
    {
      tab[j][i] = (i-48)+(j-48);
    }
  }
  addTable = toBIDIGITS(tab);

  for(int j=0; j<58; j++)
    free(tab[j]);

  free(tab);

}

void CSARITHMETIC_API CSARITHMETIC::makeAddition(char*a, char*b, char*&result, size_t opSize, size_t& resSize)
{
  csBIDIGITS prevCarry={'0','0'};
  size_t m=opSize-1;
  //for(i=m; i>=0; i--)
  //cout<<" a = "<<a<< " \n";
  //cout<<" b = "<<b<< " \n";

  for(size_t i=m,j=0; j<opSize; j++, i-=1)
  {
    csBIDIGITS bd1 = addTable[int(a[i])][int(b[i])];

    csBIDIGITS bd2 = addTable[bd1.units][prevCarry.units];
    prevCarry = addTable[bd1.tens][bd2.tens];
    *(result+i+1) = bd2.units;
    //cout<<int(a[i])<<" "<<int(b[i])<< " " <<addTable[55][48].tens<<addTable[55][48].units<<", "<<prevCarry.units<<" "<<prevCarry.tens<<"\n";
  }
  result[0] = prevCarry.units;//important !!
  removeFrontZeros(result, resSize);

  //result[0] = prevCarry.tens;
}

char* CSARITHMETIC_API CSARITHMETIC::makeAddition(char*a,char*b)
{
  size_t opSize;
  char *a1=0, *b1=0;
  //cout<<" a = "<<a<<"  fghj\n";
  //cout<<" b = "<<b<<"  fghj\n";

  getReady2(a,b,a1,b1,opSize);
  size_t resSize = opSize+1;
  
  char*result = csAllocCharPtr(resSize, '0');
  makeAddition(a1,b1,result,opSize, resSize);
  
  free(a1);
  free(b1);
  return result;
}

char* CSARITHMETIC_API CSARITHMETIC::makeAddition(char*a,char*b, size_t&resSize)
{
  size_t opSize;
  char *a1=0, *b1=0;

  getReady2(a,b,a1,b1,opSize);
  resSize = opSize+1;

  char*result = csAllocCharPtr(resSize, '0');
  //sprintf(result,"");
  makeAddition(a1,b1,result,opSize, resSize);
  //cout<<" res = "<<result<<"  fghj\n";
  free(a1);
  free(b1);
  return result;
}

char* CSARITHMETIC_API CSARITHMETIC::makeAddition(const char*a,const char*b)
{

  size_t opSize;
  char *a1=0, *b1=0;
  //cout<<"fghj\n";

  getReady2(a,b,a1,b1,opSize);
  size_t resSize = opSize+1;
  //cout<<" "<<addTable[55][48].tens<< " sza \n";
  char*result = csAllocCharPtr(resSize, '0');
  //sprintf(result,"");
  makeAddition(a1,b1,result,opSize, resSize);

  free(a1);
  free(b1);
  return result;
}