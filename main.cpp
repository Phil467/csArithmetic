#include "csARITHMETIC.h"
#include "csParallelTask/csPERF_MEASUREMENT.h"
#include "csSQRT.h"

using namespace std;
using namespace CSARITHMETIC;

int main()
{
    CSARITHMETIC_API CSARITHMETIC::init();

    csPERF_MEASUREMENT pm(CSTIME_UNIT_MILLISECOND);

    csSQRTFAST sf(75,20);
    csRNUMBER rn, err;
    rn.random(1000);
    sf.rn = &rn;
    sf.getBlocks();

    pm.start();
    sf.makeSqrt();
    pm.stop();
    sf.printNumber("Number : \n");
    sf.printRoot("\nRoot : \n");
    sf.printError("\nError : \n");
    pm.printReport(" fast (csSQRT) : ");

    sf.releaseBlocks();
    sf.releaseNumber();
}
