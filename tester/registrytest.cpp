
#include "test.h"
#include "libcc\registry.h"
using namespace LibCC;

bool RegistryTest()
{
  // clean up.
  RegistryKeyW x(L"hklm\\software\\libcc\\regtest");
  std::wstring w;
  x.Delete();
  TestAssert(!x.Exists());// make sure it doesnt exist
  TestAssert(!x.ValueExists(L"x"));

  // create, and make sure exists.
  TestAssert(x.Create());
  TestAssert(x.Exists());

  // set a value
  TestAssert(!x.ValueExists(L"x"));
  TestAssert(x.SetValue(L"x", L"aoeu"));
  TestAssert(x.ValueExists(L"x"));
  w.clear();

  TestAssert(x.GetValue(L"x", w));
  TestAssert(StringEquals(w, "aoeu"));

  // set default value
  int i;
  TestAssert(!x[L""].Exists());
  TestAssert(x[L""] = -1);
  TestAssert(x.ValueExists(L""));
  TestAssert(x.GetValue(L"", i));
  TestAssert(i == -1);
  TestAssert(x.GetValue(L"").Delete());
  TestAssert(!x.GetValue(L"").Exists());

  // dword
  DWORD dw;
  TestAssert(!x[L"dw"].Exists());
  TestAssert(x[L"dw"] = 0x80000000);
  TestAssert(x[L"dw"].Exists());
  TestAssert(x.GetValue(L"dw", dw));
  TestAssert(dw == 0x80000000);

  // binary
  Blob<DWORD> buf;
  TestAssert(x.GetValue(L"dw", buf));
  TestAssert(buf[0] == 0x80000000);

  // enum keys
  // create some subkeys.
  TestAssert(x.SubKey(L"1").Create());
  TestAssert(x.SubKey(L"2").Create());
  TestAssert(x.SubKey(L"3").Create());
  TestAssert(x.SubKey(L"4").Create());
  RegistryKeyW::SubKeyIterator it;
  int n = 0;
  for(it = x.EnumSubKeysBegin(); it != x.EnumSubKeysEnd(); ++ it)
  {
    n ++;
    TestAssert(it->Exists());
  }

  return true;
}


