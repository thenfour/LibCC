

#include "test.h"
#include "stringutil-711.hpp"
#include "libcc\timer.hpp"
//using namespace LibCC;

#include <sstream>
#pragma warning(disable:4996)// warning C4996: 'wcscpy' was declared deprecated  -- uh, i know how to use this function just fine, thanks.


bool FormatBenchmark()
{
  LibCC::Timer t;
  char crap[100];
  const int MaxNum = 100000;

	////////////////////////////////
	std::cout << std::endl << "Converting an integer (base 16):" << std::endl;

  t.Tick();
  for(int n = 0; n < MaxNum; n ++)
  {
    DoNotOptimize(_itoa(n, crap, 16));
  }
  t.Tick();
  std::cout << "itoa    : " << t.GetLastDelta() << std::endl;


  t.Tick();
  for(int n = 0; n < MaxNum; n ++)
  {
		DoNotOptimize(LibCC::Format().ul<16>(n));
  }
  t.Tick();
  std::cout << "Format(templated): " << t.GetLastDelta() << std::endl;


  t.Tick();
  for(int n = 0; n < MaxNum; n ++)
  {
    DoNotOptimize(LibCC::Format().ul(n, 16));
  }
  t.Tick();
  std::cout << "Format(runtime): " << t.GetLastDelta() << std::endl;


  t.Tick();
  for(int n = 0; n < MaxNum; n ++)
  {
		DoNotOptimize(LibCC_711::Format().ul<16>(n));
  }
  t.Tick();
  std::cout << "Format711(templated): " << t.GetLastDelta() << std::endl;


  t.Tick();
  for(int n = 0; n < MaxNum; n ++)
  {
    DoNotOptimize(LibCC_711::Format().ul(n, 16));
  }
  t.Tick();
  std::cout << "Format711(runtime): " << t.GetLastDelta() << std::endl;


  t.Tick();
  for(int n = 0; n < MaxNum; n ++)
  {
    DoNotOptimize(sprintf(crap, "%x", n));
  }
  t.Tick();
  std::cout << "sprintf(): " << t.GetLastDelta() << std::endl;


  t.Tick();
  std::string crap2;
  for(int n = 0; n < MaxNum; n ++)
  {
    std::stringstream ss;
    ss << std::hex << n;
    ss >> crap2;
    DoNotOptimize(crap2);
  }
  t.Tick();
  std::cout << "stringstream(): " << t.GetLastDelta() << std::endl;

	////////////////////////////////
	std::cout << std::endl << "Converting a double:" << std::endl;

	t.Tick();
	for(double n = 0.0; n < MaxNum; n += 0.98)
	{
		DoNotOptimize(_gcvt(n, 7, crap));
	}
	t.Tick();
	std::cout << "gcvt    : " << t.GetLastDelta() << std::endl;


	t.Tick();
	for(double n = 0.0; n < MaxNum; n += 0.98)
	{
		DoNotOptimize(LibCC::Format().d<7,7,'0',false, 10>(n));
	}
	t.Tick();
	std::cout << "Format(templated): " << t.GetLastDelta() << std::endl;


	t.Tick();
	for(double n = 0.0; n < MaxNum; n += 0.98)
	{
		DoNotOptimize(LibCC::Format().d(n, 7, 7, '0', false, 10));
	}
	t.Tick();
	std::cout << "Format(runtime): " << t.GetLastDelta() << std::endl;


	t.Tick();
	for(double n = 0.0; n < MaxNum; n += 0.98)
	{
		DoNotOptimize(LibCC_711::Format().d(n, 7, 7, '0', false, 10));
	}
	t.Tick();
	std::cout << "Format711(runtime): " << t.GetLastDelta() << std::endl;


	t.Tick();
	for(double n = 0.0; n < MaxNum; n += 0.98)
	{
		DoNotOptimize(LibCC_711::Format().d<7,7,'0',false, 10>(n));
	}
	t.Tick();
	std::cout << "Format711(templated): " << t.GetLastDelta() << std::endl;


	t.Tick();
	for(double n = 0.0; n < MaxNum; n += 0.98)
	{
		DoNotOptimize(sprintf(crap, "%e07", n));
	}
	t.Tick();
	std::cout << "sprintf(): " << t.GetLastDelta() << std::endl;


	t.Tick();
	for(double n = 0.0; n < MaxNum; n += 0.98)
	{
		std::stringstream ss;
		ss << std::hex << n;
		ss >> crap2;
		DoNotOptimize(crap2);
	}
	t.Tick();
	std::cout << "stringstream(): " << t.GetLastDelta() << std::endl;

	////////////////////////////////
	std::cout << std::endl << "Concating strings ('01' + '02' etc) as a new string:" << std::endl;

	char left[100];
	char right[100];

	{
		t.Tick();
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
		t.Tick();
		std::cout << "strcat    : " << t.GetLastDelta() << std::endl;
	}


	t.Tick();
  for(int n = 0; n < MaxNum; n ++)
	{
		right[5] = left[0] = '0' + ((n / 1) % 10);
		right[4] = left[1] = '0' + ((n / 10) % 10);
		right[3] = left[2] = '0' + ((n / 100) % 10);
		right[2] = left[3] = '0' + ((n / 1000) % 10);
		right[1] = left[4] = '0' + ((n / 10000) % 10);
		right[0] = left[5] = '0' + ((n / 100000) % 10);
		right[6] = left[6] = 0;
		DoNotOptimize(LibCC::FormatA("%%")(left)(right));
	}
	t.Tick();
	std::cout << "Format: " << t.GetLastDelta() << std::endl;

	t.Tick();
  for(int n = 0; n < MaxNum; n ++)
	{
		right[5] = left[0] = '0' + ((n / 1) % 10);
		right[4] = left[1] = '0' + ((n / 10) % 10);
		right[3] = left[2] = '0' + ((n / 100) % 10);
		right[2] = left[3] = '0' + ((n / 1000) % 10);
		right[1] = left[4] = '0' + ((n / 10000) % 10);
		right[0] = left[5] = '0' + ((n / 100000) % 10);
		right[6] = left[6] = 0;
		DoNotOptimize(LibCC_711::FormatA("%%")(left)(right));
	}
	t.Tick();
	std::cout << "Format711: " << t.GetLastDelta() << std::endl;


	t.Tick();
  for(int n = 0; n < MaxNum; n ++)
	{
		DoNotOptimize(sprintf(crap, "%s%s", left, right));
	}
	t.Tick();
	std::cout << "sprintf(): " << t.GetLastDelta() << std::endl;


	t.Tick();
  for(int n = 0; n < MaxNum; n ++)
	{
		std::stringstream ss;
		ss << left << right;
		ss >> crap2;
		DoNotOptimize(crap2);
	}
	t.Tick();
	std::cout << "stringstream(): " << t.GetLastDelta() << std::endl;

	return true;
}


