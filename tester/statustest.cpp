

#include "test.h"
#include "libcc\result.h"
using namespace LibCC;

bool StatusTest()
{
  ResultA a(S_OK);
  TestAssert(a.hr() == S_OK);
  TestAssert(a.str() == "");
  TestAssert(a.Failed() == false);
  TestAssert(a.Succeeded() == true);
  TestAssert(!a == a.Failed());

  a.Fail("omg");
  TestAssert(a.hr() == E_FAIL);
  TestAssert(a.str() == "omg");
  TestAssert(a.Failed() == true);
  TestAssert(a.Succeeded() == false);
  TestAssert(!a == a.Failed());

  a.Succeed();
  TestAssert(a.hr() == S_OK);
  TestAssert(a.str() == "");
  TestAssert(a.Failed() == false);
  TestAssert(a.Succeeded() == true);
  TestAssert(!a == a.Failed());

  a.Succeed("omg");
  TestAssert(a.hr() == S_OK);
  TestAssert(a.str() == "omg");
  TestAssert(a.Failed() == false);
  TestAssert(a.Succeeded() == true);
  TestAssert(!a == a.Failed());

  a.Fail();
  TestAssert(a.hr() == E_FAIL);
  TestAssert(a.str() == "");
  TestAssert(a.Failed() == true);
  TestAssert(a.Succeeded() == false);
  TestAssert(!a == a.Failed());

  a.Assign(S_FALSE, "omg");
  TestAssert(a.hr() == S_FALSE);
  TestAssert(a.str() == "omg");
  TestAssert(a.Failed() == false);
  TestAssert(a.Succeeded() == true);
  TestAssert(!a == a.Failed());

  a.Assign(E_INVALIDARG);
  TestAssert(a.hr() == E_INVALIDARG);
  TestAssert(a.str() == "");
  TestAssert(a.Failed() == true);
  TestAssert(a.Succeeded() == false);
  TestAssert(!a == a.Failed());

  ResultA b;
  b.Fail();
  TestAssert(b.Failed());
  b = a;

  TestAssert(b.hr() == E_INVALIDARG);
  TestAssert(b.str() == "");
  TestAssert(b.Failed() == true);
  TestAssert(b.Succeeded() == false);
  TestAssert(!b == b.Failed());

  return true;
}
