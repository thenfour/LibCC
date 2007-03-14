

#include "test.h"
#include "libcc\Blob.h"
using namespace LibCC;




bool BlobTest()
{
	Blob<BYTE> a(1);
	TestAssert(a.Size() == 1);
	TestAssert(a.AllocatedSize() > 1);
	TestAssert(!a.CompletelyUnallocated());
	TestAssert(a.CurrentlyUsingStaticBuffer());
	a.Fill(78);
	TestAssert(a.Alloc(2));
	TestAssert(a.Size() == 2);
	TestAssert(a[0] == 78);
	TestAssert(a[1] != 78);
	
	TestAssert(a.Free());
	TestAssert(a.Size() == 0);

	TestAssert(a.Alloc(40000000));// 40 megs
	TestAssert(!a.CurrentlyUsingStaticBuffer());
	TestAssert(a.Size() == 4000000);
	a.Fill(7);
	a[40000000-1] = 254;
	a.GetBuffer()[40000000-2] = 255;
	TestAssert(a[40000000-1] == 254);
	TestAssert(a[40000000-2] == 255);
	TestAssert(a.Free());
	return true;
}

