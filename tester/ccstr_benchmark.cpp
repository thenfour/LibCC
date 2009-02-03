

#include "test.h"
#include "stringutil-711.hpp"
#include "libcc\timer.hpp"
//using namespace LibCC;

#include <sstream>
#pragma warning(disable:4996)// warning C4996: 'wcscpy' was declared deprecated  -- uh, i know how to use this function just fine, thanks.

namespace Test
{

}

void StartBenchmark(LibCC::Timer& t)
{
	t.Tick();
}
void ReportBenchmark(LibCC::Timer& t, const std::string& name)
{
	const int nameColumn = 50;
	t.Tick();
	std::cout << LibCC::FormatA("%%: %\r\n").s<nameColumn>(name).c('.', nameColumn - name.size()).d<2>(t.GetLastDelta()).Str();
}

bool FormatBenchmark()
{
  LibCC::Timer t;
  char crap[100];
#ifdef _DEBUG
  const int MaxNum = 10000;
#else
  const int MaxNum = 1000000;
#endif
	const int StdHandicap = 10;


	std::cout << std::endl << "LibCC::Format benchmarks, " << MaxNum << " passes." << std::endl;


	//////////////////////////////////
	std::cout << std::endl << "create a string list with 5 elements:" << std::endl;
	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
		std::vector<std::wstring> v;
		v.push_back(L"1");
		v.push_back(L"12");
		v.push_back(L"123");
		v.push_back(L"1234");
		v.push_back(L"12345");
		DoNotOptimize(v);
  }
	ReportBenchmark(t, "std::vector");

	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
		LibCC::QuickStringList<wchar_t> v;
		v.push_back(L"1");
		v.push_back(L"12");
		v.push_back(L"123");
		v.push_back(L"1234");
		v.push_back(L"12345");
		DoNotOptimize(v);
  }
	ReportBenchmark(t, "LibCC::QuickStringList");

	//////////////////////////////////
	std::cout << std::endl << "create a string list with 1 element:" << std::endl;
	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
		std::vector<std::wstring> v;
		v.push_back(L"1");
		DoNotOptimize(v);
  }
	ReportBenchmark(t, "std::vector");

	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
		LibCC::QuickStringList<wchar_t> v;
		v.push_back(L"1");
		DoNotOptimize(v);
  }
	ReportBenchmark(t, "LibCC::QuickStringList");



	////////////////////////////////
	std::cout << std::endl << "Converting an integer (base 16):" << std::endl;

	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
    DoNotOptimize(_itoa(n, crap, 16));
  }
	ReportBenchmark(t, "itoa");

	StartBenchmark(t);
  for(int n = 0; n < MaxNum; n ++)
  {
    DoNotOptimize(sprintf(crap, "%x", n));
  }
	ReportBenchmark(t, "sprintf");

	StartBenchmark(t);
  std::string crap2;
  for(int n = 0; n < MaxNum / StdHandicap; n ++)
  {
    std::stringstream ss;
    ss << std::hex << n;
    ss >> crap2;
    DoNotOptimize(crap2);
  }
	ReportBenchmark(t, "stringstream");

	StartBenchmark(t);
  for(int n = 0; n < MaxNum; n ++)
  {
		DoNotOptimize(LibCC_711::Format().ul<16>(n));
  }
	ReportBenchmark(t, "Format711(templated)");

	StartBenchmark(t);
  for(int n = 0; n < MaxNum; n ++)
  {
		DoNotOptimize(LibCC::Format().ul<16>(n));
  }
	ReportBenchmark(t, "Format(templated)");

	StartBenchmark(t);
  for(int n = 0; n < MaxNum; n ++)
  {
    DoNotOptimize(LibCC_711::Format().ul(n, 16));
  }
	ReportBenchmark(t, "Format711(runtime)");

	StartBenchmark(t);
  for(int n = 0; n < MaxNum; n ++)
  {
    DoNotOptimize(LibCC::Format().ul(n, 16));
  }
	ReportBenchmark(t, "Format(runtime)");



	////////////////////////////////
	std::cout << std::endl << "Converting a double:" << std::endl;

	StartBenchmark(t);
	for(double n = 0.0; n < MaxNum; n += 0.98)
	{
		DoNotOptimize(_gcvt(n, 7, crap));
	}
	ReportBenchmark(t, "gcvt");

	StartBenchmark(t);
	for(double n = 0.0; n < MaxNum; n += 0.98)
	{
		DoNotOptimize(sprintf(crap, "%e07", n));
	}
	ReportBenchmark(t, "sprintf");

	StartBenchmark(t);
	for(double n = 0.0; n < MaxNum / StdHandicap; n += 0.98)
	{
		std::stringstream ss;
		ss << std::hex << n;
		ss >> crap2;
		DoNotOptimize(crap2);
	}
	ReportBenchmark(t, "stringstream");

	StartBenchmark(t);
	for(double n = 0.0; n < MaxNum; n += 0.98)
	{
		DoNotOptimize(LibCC_711::Format().d<7,7,'0',false, 10>(n));
	}
	ReportBenchmark(t, "Format711(templated)");

	StartBenchmark(t);
	for(double n = 0.0; n < MaxNum; n += 0.98)
	{
		DoNotOptimize(LibCC::Format().d<7,7,'0',false, 10>(n));
	}
	ReportBenchmark(t, "Format(templated)");

	StartBenchmark(t);
	for(double n = 0.0; n < MaxNum; n += 0.98)
	{
		DoNotOptimize(LibCC_711::Format().d(n, 7, 7, '0', false, 10));
	}
	ReportBenchmark(t, "Format711(runtime)");

	StartBenchmark(t);
	for(double n = 0.0; n < MaxNum; n += 0.98)
	{
		DoNotOptimize(LibCC::Format().d(n, 7, 7, '0', false, 10));
	}
	ReportBenchmark(t, "Format(runtime)");




	////////////////////////////////
	std::cout << std::endl << "Concating strings ('01' + '02' etc) as a new string:" << std::endl;

	char left[100];
	char right[100];

	{
		StartBenchmark(t);
	  for(int n = 0; n < MaxNum; n ++)
		{
			right[5] = left[0] = '0' + ((n / 1) % 10);
			right[4] = left[1] = '0' + ((n / 10) % 10);
			right[3] = left[2] = '0' + ((n / 100) % 10);
			right[2] = left[3] = '0' + ((n / 1000) % 10);
			right[1] = left[4] = '0' + ((n / 10000) % 10);
			right[0] = left[5] = '0' + ((n / 100000) % 10);
			right[6] = left[6] = 0;

			strcpy(crap, left);
			DoNotOptimize(strcat(crap, right));
		}
		ReportBenchmark(t, "strcat");
	}

	StartBenchmark(t);
  for(int n = 0; n < MaxNum; n ++)
	{
		right[5] = left[0] = '0' + ((n / 1) % 10);
		right[4] = left[1] = '0' + ((n / 10) % 10);
		right[3] = left[2] = '0' + ((n / 100) % 10);
		right[2] = left[3] = '0' + ((n / 1000) % 10);
		right[1] = left[4] = '0' + ((n / 10000) % 10);
		right[0] = left[5] = '0' + ((n / 100000) % 10);
		right[6] = left[6] = 0;
		DoNotOptimize(sprintf(crap, "%s%s", left, right));
	}
	ReportBenchmark(t, "sprintf");


	StartBenchmark(t);
  for(int n = 0; n < MaxNum / StdHandicap; n ++)
	{
		right[5] = left[0] = '0' + ((n / 1) % 10);
		right[4] = left[1] = '0' + ((n / 10) % 10);
		right[3] = left[2] = '0' + ((n / 100) % 10);
		right[2] = left[3] = '0' + ((n / 1000) % 10);
		right[1] = left[4] = '0' + ((n / 10000) % 10);
		right[0] = left[5] = '0' + ((n / 100000) % 10);
		right[6] = left[6] = 0;
		std::stringstream ss;
		ss << left << right;
		ss >> crap2;
		DoNotOptimize(crap2);
	}
	ReportBenchmark(t, "stringstream");


	StartBenchmark(t);
  for(int n = 0; n < MaxNum; n ++)
	{
		right[5] = left[0] = '0' + ((n / 1) % 10);
		right[4] = left[1] = '0' + ((n / 10) % 10);
		right[3] = left[2] = '0' + ((n / 100) % 10);
		right[2] = left[3] = '0' + ((n / 1000) % 10);
		right[1] = left[4] = '0' + ((n / 10000) % 10);
		right[0] = left[5] = '0' + ((n / 100000) % 10);
		right[6] = left[6] = 0;
		DoNotOptimize(LibCC_711::FormatA()(left)(right));
	}
	ReportBenchmark(t, "Format711");

	StartBenchmark(t);
  for(int n = 0; n < MaxNum; n ++)
	{
		right[5] = left[0] = '0' + ((n / 1) % 10);
		right[4] = left[1] = '0' + ((n / 10) % 10);
		right[3] = left[2] = '0' + ((n / 100) % 10);
		right[2] = left[3] = '0' + ((n / 1000) % 10);
		right[1] = left[4] = '0' + ((n / 10000) % 10);
		right[0] = left[5] = '0' + ((n / 100000) % 10);
		right[6] = left[6] = 0;
		DoNotOptimize(LibCC::FormatA()(left)(right));
	}
	ReportBenchmark(t, "Format");

	return true;
}


