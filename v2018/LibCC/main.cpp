

#include "libcc/format.hpp"
#include "libcc/winutils.hpp"


int main()
{

  std::wstring ws;
  cc::_FormatMessage(ws, ERROR_SUCCESS);
  return 0;
}

