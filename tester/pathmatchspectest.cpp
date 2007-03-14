

#include "test.h"
#include "libcc\pathmatchspec.h"

#pragma comment(lib, "shlwapi.lib")

bool PathMatchSpecTest()
{
  LibCC::PathMatchSpecP4A x;

  // no wildcards
  x.SetCriteria("c:\\hi");
  TestAssert(x.Match("c:\\hi"));
  TestAssert(x.Match("c:/hi"));// path separators
  TestAssert(x.Match("C:\\HI"));// case sensitivity
  TestAssert(!x.Match("c:\\hix"));
  TestAssert(!x.Match("c:\\xhi"));
  TestAssert(!x.Match("c:\\h"));
  TestAssert(!x.Match("cx:\\hi"));
  TestAssert(!x.Match("xc:\\hi"));
  TestAssert(!x.Match("d:\\hi"));
  TestAssert(!x.Match("c:\\hi\\"));
  TestAssert(!x.Match(""));

  // Question mark
  x.SetCriteria("c:\\test/test\\123.?");
  TestAssert(x.Match("c:\\test/test\\123. "));
  TestAssert(x.Match("c:\\test\\test\\123.x"));
  TestAssert(x.Match("c:\\test\\test\\123.1"));
  TestAssert(!x.Match("c:\\test\\test\\123.\\"));
  TestAssert(!x.Match("c:\\test\\test\\123.1\\"));
  TestAssert(!x.Match("c:\\test\\test\\123.\\1"));
  TestAssert(!x.Match("c:\\test\\test\\123."));// needs to match exactly 1 char
  TestAssert(!x.Match(""));

  x.SetCriteria     ("c:\\test/te?t\\123.?");
  TestAssert(x.Match("c:\\test/test\\123.x"));
  TestAssert(x.Match("c:\\test\\text\\123.x"));
  TestAssert(!x.Match("c:\\test/tet\\123.x"));
  TestAssert(!x.Match("c:\\test\\123.x"));
  TestAssert(!x.Match(""));

  x.SetCriteria("c:\\te?t/test\\123.?");
  TestAssert(x.Match("c:\\text\\test\\123.x"));
  TestAssert(!x.Match("c:\\te\\t\\test\\123.x"));
  TestAssert(!x.Match(""));

  x.SetCriteria     ("?:\\test/te?t\\123.?");
  TestAssert(x.Match("a:\\test/test\\123.*"));
  TestAssert(x.Match("b:\\test/test\\123.^"));
  TestAssert(!x.Match("ac:\\test/text\\123."));
  TestAssert(!x.Match(":\\test\\test\\123.x"));
  TestAssert(!x.Match(""));

  x.SetCriteria("????");
  TestAssert(x.Match("1234"));
  TestAssert(!x.Match("12345"));

  // star
  x.SetCriteria("c:\\hi\\a*a");
  TestAssert(x.Match("c:\\hi\\aa"));
  TestAssert(x.Match("c:\\hi\\aba"));
  TestAssert(x.Match("c:\\hi\\abba"));
  TestAssert(!x.Match("c:\\hi\\a\\a"));
  TestAssert(!x.Match("c:\\hi\\axxxa\\axxxxa"));
  TestAssert(!x.Match("c:\\hi\\axxxa\\a"));
  TestAssert(!x.Match(""));

  x.SetCriteria("c:\\*\\abc");
  TestAssert(x.Match("c:\\hi/abc"));
  TestAssert(x.Match("c:\\hiaoeuaoeu/abc"));
  TestAssert(x.Match("c:\\/abc"));
  TestAssert(!x.Match("c:\\aoeu\\aoeu/abc"));

  x.SetCriteria("*?");
  TestAssert(x.Match("x"));
  TestAssert(x.Match("xx"));
  TestAssert(x.Match("xxxxxxxxxxxxxxxxxxxxxxxx"));
  TestAssert(!x.Match("xxxxxxxxxxx\\xxxxxxxxxxxxx"));
  TestAssert(!x.Match(""));

  x.SetCriteria("***************");
  TestAssert(x.Match(""));
  TestAssert(x.Match("1"));
  TestAssert(x.Match("12"));
  TestAssert(x.Match("123"));
  TestAssert(x.Match("1234"));
  TestAssert(x.Match("12345"));
  TestAssert(x.Match("123456"));
  TestAssert(x.Match("1234567"));
  TestAssert(x.Match("12345678"));
  TestAssert(x.Match("123456789"));
  TestAssert(x.Match("123456789a"));
  TestAssert(x.Match("123456789ab"));
  TestAssert(x.Match("123456789abc"));
  TestAssert(x.Match("123456789abcd"));
  TestAssert(x.Match("123456789abcde"));
  TestAssert(x.Match("123456789abcdef"));
  TestAssert(x.Match("123456789abcdefg"));
  TestAssert(!x.Match("\\123456789abcdefg"));

  x.SetCriteria("*\\hithere");
  TestAssert(x.Match("\\hithere"));
  TestAssert(x.Match("x\\hithere"));
  TestAssert(x.Match("xx\\hithere"));
  TestAssert(!x.Match("xx\\xx\\hithere"));
  TestAssert(!x.Match(""));

  x.SetCriteria("hithere*");
  TestAssert(x.Match("hithere"));
  TestAssert(!x.Match("xhithereaoeu"));
  TestAssert(!x.Match(""));

  // ellipses
  x.SetCriteria("c:\\...log");
  TestAssert(x.Match("C:\\xlog"));
  TestAssert(x.Match("C:\\a\\o\\aoeu\\aoeu\\aoeu\\aoeu\\aoeu\\aoeu\\\\\\\\\\ulog"));
  TestAssert(x.Match("C:\\lp43\\log"));
  TestAssert(x.Match("C:\\lp43/log"));
  TestAssert(!x.Match("C:\\lp43\\logo"));
  TestAssert(!x.Match("log"));
  TestAssert(!x.Match("c:\\xlog\\"));

  x.SetCriteria("c:\\...\\log");
  TestAssert(x.Match("C:\\x\\x\\log"));
  TestAssert(x.Match("C:\\log"));
  TestAssert(!x.Match("C:\\xlog"));// 1 or more directories

  // need more tests here.

  // Unicode version
  LibCC::PathMatchSpecP4W y;
  y.SetCriteria(L"123\\456");
  TestAssert(y.Match(L"123\\456"));
  TestAssert(!y.Match(L" 23\\456"));

  return true;
}


