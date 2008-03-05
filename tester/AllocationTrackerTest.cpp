

#include "test.h"
#include "libcc\AllocationTracker.h"
using namespace LibCC;

AllocationManager* LibCC::g_pAllocationManager = 0;

struct attest1
{
	Allocation a;
};


bool AllocationTrackerTest()
{
	g_pAllocationManager = new AllocationManager();
  //TestAssert(!b == b.Failed());
	g_pAllocationManager->Report();
	attest1 x;
	g_pAllocationManager->Report();
	delete g_pAllocationManager;
	g_pAllocationManager = 0;
  return true;
}


