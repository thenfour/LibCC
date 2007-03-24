

#include "test.h"
#include "libcc\stringutil.h"
#include <vector>
using namespace LibCC;


// tests if a unicode char belongs to a given codepage.
bool IsValidCharacter(UINT codepage, wchar_t unicodeChar)
{
	SetLastError(ERROR_SUCCESS);

	CPINFO cpinfo;
	if(0 == GetCPInfo(codepage, &cpinfo))
	{
		return false;
	}

	BOOL usedDefaultChar = FALSE;
	int length = WideCharToMultiByte(codepage, 0, &unicodeChar, 1, 0, 0, 0, 0);

	Blob<BYTE> b(length);
	WideCharToMultiByte(codepage, 0, &unicodeChar, 1, (LPSTR)b.GetBuffer(), length, 0, 0);

	if(length == 1 && b[0] == cpinfo.DefaultChar[0] && unicodeChar != cpinfo.DefaultChar[0]) return false;
	return length > 0;
}


// attempts to generate a unicode string that contains all characters in the given codepage.
std::wstring GenerateString(UINT codepage)
{
	// method 1, part a, which is not that accurate. some are hard-coded lists of characters. this ensures that we are getting characters that are truly in the code pages,
	// and trying to eliminate mis-testing from our own bugs or bugs in windows.
	//// main reference: http://www.kostis.net/charsets/
	//// http://www.kostis.net/charsets/ebc1026.htm
	//const WCHAR cp1026[] = { 32,160,226,228,224,225,227,229,123,241,199,46,60,40,43,33,38,233,234,235,232,237,238,239,236,223,286,304,42,41,59,94,45,47,194,196,192,193,195,197,91,209,351,44,37,95,62,63,248,201,202,203,200,205,206,207,204,305,58,214,350,39,61,220,216,97,98,99,100,101,102,103,104,105,171,187,125,96,166,177,176,106,107,108,109,110,111,112,113,114,170,186,230,184,198,164,181,246,115,116,117,118,119,120,121,122,161,191,93,36,64,174,162,163,165,183,169,167,182,188,189,190,172,124,175,168,180,215,231,65,66,67,68,69,70,71,72,73,173,244,126,242,243,245,287,74,75,76,77,78,79,80,81,82,185,251,92,249,250,255,252,247,83,84,85,86,87,88,89,90,178,212,35,210,211,213,48,49,50,51,52,53,54,55,56,57,179,219,34,217,218, 0 };
	//// http://www.kostis.net/charsets/cp1250.htm
	//const WCHAR cp1250[] = { 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,8364,8218,8222,8230,8224,8225,8240,352,8249,346,356,381,377,8216,8217,8220,8221,8226,8211,8212,8482,353,8250,347,357,382,378,160,711,728,321,164,260,166,167,168,169,350,171,172,173,174,379,176,177,731,322,180,181,182,183,184,261,351,187,376,733,317,380,340,193,194,258,196,313,262,199,268,201,280,203,282,205,206,270,272,323,327,211,212,336,214,215,344,366,218,368,220,221,354,223,341, 0};
	//// http://www.kostis.net/charsets/cp1251.htm
	//const WCHAR cp1251[] = { 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,1026,1027,8218,1107,8222,8230,8224,8225,8364,8240,1033,8249,1034,1036,1035,1039,1106,8216,8217,8220,8221,8226,8211,8212,8482,1113,8250,1114,1116,1115,1119,160,1038,1118,1032,164,1168,166,167,1025,169,1028,171,172,173,174,1031,176,177,1030,1110,1169,181,182,183,1105,8470,1108,187,1112,1029,1109,1111,1040,1041,1042,1043,1044,1045,1046,1047,1048,1049,1050,1051,1052,1053,1054,1055,1056,1057,1058,1059,1060,1061,1062,1063,1064,0};
	//// http://www.kostis.net/charsets/cp1252.htm
	//const WCHAR cp1252[] = { 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,8364,8218,402,8222,8230,8224,8225,710,8240,352,8249,338,381,8216,8217,8220,8221,8226,8211,8212,732,8482,353,8250,339,382,376,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,0};
	//// http://www.kostis.net/charsets/cp1253.htm
	//const WCHAR cp1253[] = { 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,8364,8218,402,8222,8230,8224,8225,8240,8249,8216,8217,8220,8221,8226,8211,8212,8482,8250,160,901,902,163,164,165,166,167,168,169,171,172,173,174,8213,176,177,178,179,900,181,182,183,904,905,906,187,908,189,910,911,912,913,914,915,916,917,918,919,920,921,922,923,924,925,926,927,928,929,931,932,933,934,935,936,937,938,939,940,941,942,943,944,945,946,947,948,949,950,951,952,953,954,955,956,957,958,959,960,961,962,963,964,965,966,967,968,969,970,971,972,973,974,248,249,250,251,252,253,254,255,0};
	//// http://www.kostis.net/charsets/cp1254.htm
	//const WCHAR cp1254[] = { 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,8364,8218,402,8222,8230,8224,8225,710,8240,352,8249,338,8216,8217,8220,8221,8226,8211,8212,732,8482,353,8250,339,376,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,286,209,210,211,212,213,214,215,216,217,218,219,220,304,350,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,287,241,242,243,244,245,246,247,248,249,250,251,252,305,351,255,254,255,0};
	//// http://www.kostis.net/charsets/cp1255.htm
	//const WCHAR cp1255[] = { 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,8364,8218,402,8222,8230,8224,8225,8240,8249,8216,8217,8220,8221,8226,8211,8212,8482,8250,160,162,163,164,165,166,167,168,169,215,171,172,173,174,781,176,177,178,179,180,181,182,183,184,185,247,187,188,189,190,8215,1488,1489,1490,1491,1492,1493,1494,1495,1496,1497,1498,1499,1500,1501,1502,1503,1504,1505,1506,1507,1508,1509,1510,1511,1512,1513,1514,211,212,213,214,215,216,217,218,219,220,304,350,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,287,241,242,243,244,245,246,247,248,249,250,251,252,305,351,255,254,255,0};
	//// http://www.kostis.net/charsets/cp1256.htm
	//const WCHAR cp1256[] = { 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,1548,8218,8222,8230,8224,8225,8240,8249,8216,8217,8220,8221,8226,8211,8212,1563,8482,1567,8250,1569,1570,1571,376,160,1572,1573,163,164,1574,166,167,1575,169,1576,171,172,173,174,176,177,1577,1578,1579,181,182,183,1580,1581,187,1582,1583,1584,1585,192,1586,194,1587,1588,1589,199,200,201,202,203,1590,1591,206,207,1592,1593,1594,1600,212,1601,1602,215,1603,217,1711,219,220,1604,1605,1606,224,1607,226,1608,1609,1610,231,232,233,234,235,1611,1612,238,239,1613,1614,1615,1616,244,1617,1618,247,249,251,252,255,246,247,248,249,250,251,252,305,351,255,254,255,0};
	//// http://www.kostis.net/charsets/cp1257.htm
	//const WCHAR cp1257[] = { 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,8364,8218,8222,8230,8224,8225,8240,8249,168,711,184,8216,8217,8220,8221,8226,8211,8212,8482,8250,175,731,160,162,163,164,166,167,216,169,342,171,172,173,174,198,176,177,178,179,180,181,182,183,248,185,343,187,188,189,190,230,260,302,256,262,196,197,280,274,268,201,377,278,290,310,298,315,352,323,325,211,332,213,214,215,370,321,340,362,220,379,381,223,261,303,257,263,228,229,281,275,269,233,378,279,291,311,299,316,353,324,326,243,333,245,246,247,371,322,347,363,252,380,382,729,251,252,305,351,255,254,255,0};
	//// http://www.kostis.net/charsets/cp1258.htm
	//const WCHAR cp1258[] = { 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,8364,8218,402,8222,8230,8224,8225,710,8240,8249,338,8216,8217,8220,8221,8226,8211,8212,732,8482,8250,339,376,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,258,196,197,198,199,200,201,202,203,832,205,206,207,272,209,777,211,212,416,214,215,216,217,218,219,220,431,771,223,224,225,226,259,228,229,230,231,232,233,234,235,833,237,238,239,273,241,803,243,244,417,246,247,248,249,250,251,252,432,8363,255,351,255,254,255,0};

	//switch(codepage)
	//{
	//case 1026: return cp1026;
	//case 1250: return cp1250;
	//case 1251: return cp1251;
	//case 1252: return cp1252;
	//case 1253: return cp1253;
	//case 1254: return cp1254;
	//case 1255: return cp1255;
	//case 1256: return cp1256;
	//case 1257: return cp1257;
	//case 1258: return cp1258;
	//}
	// method 1, part b, attempt to get windows to tell us the same information in an automated way.
	//// default... try every char in the unicode set, testing them if they fit in the codepage.
	//std::wstring ret;
	//for(WCHAR i = 0; i < 0xffff; i ++)
	//{
	//	if(IsValidCharacter(codepage, i))
	//	{
	//		ret.push_back(i);
	//	}
	//}

	// method 2: do our best given the winapi + libcc. it will resutl in some mis-testing though because it relies on libcc working.
	// make a string of all unicode characters
	std::wstring ws;
	for(WCHAR i = 1; i < 0xfffe; i ++)
	{
		if(iswgraph(i) != 0)
		{
			ws.push_back(i);
		}
	}
	// convert it to the codepage
	std::string a;
	LibCC::ConvertString(ws, a, codepage);
	// convert it back to unicode so it's easy to handle
	LibCC::ConvertString(a, ws, codepage);
	CPINFOEX cpi;
	GetCPInfoEx(codepage, 0, &cpi);
	// extract all non-default chars. yes we will miss 1 character here, but that's ok.
	std::wstring filtered;
	for(std::wstring::iterator it = ws.begin(); it != ws.end(); ++ it)
	{
		if(*it != cpi.UnicodeDefaultChar)
		{
			filtered.push_back(*it);
		}
	}

	return filtered;
}

BOOL CALLBACK EnumCodePagesProc(LPWSTR lpCodePageString)
{
	UINT codepage = (UINT)wcstoul(lpCodePageString, 0, 10);
	std::wstring ws = GenerateString(codepage);
	TestMessage(LibCC::Format("Testing codepage: %, string length %")(lpCodePageString)((UINT)ws.length()).Str());
	Blob<BYTE> blob;
	TestAssert(SUCCEEDED(ConvertString(ws, blob, codepage)));
	std::wstring w2;
	TestAssert(SUCCEEDED(ConvertString(blob.GetBuffer(), blob.Size(), w2, codepage)));
	TestAssert(w2 == ws);
	return TRUE;
}




bool StringTest()
{
	{
		// ****  StringSplit
		std::wstring w = L",a,b,,aoeu,";
		std::wstring bw = L",";
		std::vector<std::wstring> v;
		StringSplit(w, bw, std::back_inserter(v));
		TestAssert(v.size() == 6);
		TestAssert(v[0].empty());
		TestAssert(v[1] == L"a");
		TestAssert(v[2] == L"b");
		TestAssert(v[3].empty());
		TestAssert(v[4] == L"aoeu");
		TestAssert(v[5].empty());

		v.clear();
		StringSplit(L",a,b,,aoeu,", bw, std::back_inserter(v));
		TestAssert(v.size() == 6);

		v.clear();
		StringSplit(w, L",", std::back_inserter(v));
		TestAssert(v.size() == 6);

		std::vector<std::string> va;
		StringSplit("---a---b------", "---", std::back_inserter(va));
		TestAssert(va.size() == 5);
		TestAssert(va[0].empty());
		TestAssert(va[1] == "a");
		TestAssert(va[2] == "b");
		TestAssert(va[3].empty());
		TestAssert(va[4].empty());

		v.clear();
		// using unicode chars
		w[0] = 9674;
		w[2] = 9674;
		w[4] = 9674;
		w[5] = 9674;
		w[10] = 9674;
		bw[0] = 9674;
		StringSplit(w, bw, std::back_inserter(v));
		TestAssert(v.size() == 6);
		TestAssert(v[0].empty());
		TestAssert(v[1] == L"a");
		TestAssert(v[2] == L"b");
		TestAssert(v[3].empty());
		TestAssert(v[4] == L"aoeu");
		TestAssert(v[5].empty());
		
		// other random tests
		v.clear();
		StringSplit(L"carl", L"a", std::back_inserter(v));
		TestAssert(v.size() == 2);
		TestAssert(v[0] == L"c");
		TestAssert(v[1] == L"rl");
		
		// other random tests
		v.clear();
		StringSplit(L"", L"", std::back_inserter(v));
		TestAssert(v.size() == 0);
		v.clear();
		StringSplit(L"", L"aoeu", std::back_inserter(v));
		TestAssert(v.size() == 0);
		v.clear();
		StringSplit(L"12345", L"", std::back_inserter(v));
		TestAssert(v.size() == 1);
		v.clear();
		StringSplit(L"12345", L"aoeu", std::back_inserter(v));
		TestAssert(v.size() == 1);
	}

	{ // *** StringJoin
		std::vector<std::string> va;
		va.push_back("tube");
		va.push_back("bowl");
		va.push_back("porcelain");
		std::string a = StringJoin(va.begin(), va.end(), "");
		TestAssert(a == "tubebowlporcelain");
		a = StringJoin(va.begin(), va.end(), "--");
		TestAssert(a == "tube--bowl--porcelain");

		va.clear();		
		va.push_back("tube");
		va.push_back("bowl");
		va.push_back("porcelain");
		va.push_back("");
		std::string b = ", ";
		a = StringJoin(va.begin(), va.end(), b);
		TestAssert(a == "tube, bowl, porcelain, ");
		
		va.clear();
		a = StringJoin(va.begin(), va.end(), b);
		TestAssert(a.empty());
		
		// unicode
		std::vector<std::wstring> vw;
		vw.push_back(L"-");
		vw.push_back(L"--");
		vw.push_back(L"blarg");
		vw[0][0] = 9674;
		vw[1][0] = 9674;
		vw[1][1] = 9674;
		std::wstring w = StringJoin(vw.begin(), vw.end(), L",");
		std::wstring compare = L"-,--,blarg";
		compare[0] = 9674;
		compare[2] = 9674;
		compare[3] = 9674;
		TestAssert(w == compare);
	}

	{ // **** StringTrim
		std::string a = StringTrim("", "");
		TestAssert(a.empty());
		
		a = StringTrim("aoeu", "aoeu");
		TestAssert(a.empty());
		
		a = StringTrim("aoeu", "uoea");
		TestAssert(a.empty());
		
		a = StringTrim("aoxeu", "uoea");
		TestAssert(a == "x");
		
		a = StringTrim("123x123x", "213");
		TestAssert(a == "x123x");
		
		a = StringTrim("x123x321", "213");
		TestAssert(a == "x123x");
		
		a = StringTrim("123x123x321", "213");
		TestAssert(a == "x123x");
		
		a = StringTrim("hi", "");
		TestAssert(a == "hi");
		
		std::wstring w1 = L"   \t omg\t    ";
		std::wstring w2 = L" \t";
		std::wstring w3 = StringTrim(w1, w2);
		TestAssert(w3 == L"omg");
	}
	
	{ // **** StringReplace
		std::string a1, a2, a3, a4;
		std::wstring w1, w2, w3, w4;
		
		a1 = "fredfred fredfred frefre fred";
		a2 = "fred";
		a3 = "sally";
		a4 = StringReplace(a1, a2, a3);
		TestAssert(a4 == "sallysally sallysally frefre sally");
		
		w1 = L"";
		w2 = L"";
		w3 = L"";
		w4 = StringReplace(w1, w2, w3);
		TestAssert(w1.empty());
		
		w1 = L"aoeu";
		w2 = L"";
		w3 = L"";
		w4 = StringReplace(w1, w2, w3);
		TestAssert(w1 == L"aoeu");
		
		w1 = L"aoeu";
		w2 = L"1234";
		w3 = L"";
		w4 = StringReplace(w1, w2, w3);
		TestAssert(w1 == L"aoeu");
		
		w1 = L"hi, fred";
		w2 = L"fred";
		w3 = L"sally";
		TestAssert(StringReplace(w1, w2, w3) == L"hi, sally");
		TestAssert(StringReplace(w1, w2, L"sally") == L"hi, sally");
		TestAssert(StringReplace(w1, L"fred", w3) == L"hi, sally");
		TestAssert(StringReplace(w1, L"fred", L"sally") == L"hi, sally");
		TestAssert(StringReplace(L"hi, fred", w2, w3) == L"hi, sally");
		TestAssert(StringReplace(L"hi, fred", w2, L"sally") == L"hi, sally");
		TestAssert(StringReplace(L"hi, fred", L"fred", w3) == L"hi, sally");
		TestAssert(StringReplace(L"hi, fred", L"fred", L"sally") == L"hi, sally");
	}
	
	{	// **** StringToUpper
		std::string a1;
		std::wstring w1;

		std::string srcA = "ABCDE123!@#abcdefghijklm˙·ÈÛÔ";
		std::wstring srcW = L"ABCDE123!@#abcdefghijklm˙·ÈÛÔ";
		std::basic_string<DWORD> srcX;
		ConvertString(srcW, srcX);
		
		std::string correctA = "ABCDE123!@#ABCDEFGHIJKLM⁄¡…”œ";
		std::wstring correctW = L"ABCDE123!@#ABCDEFGHIJKLM⁄¡…”œ";
		std::basic_string<DWORD> correctX;
		ConvertString(correctW, correctX);
		
		a1 = StringToUpper("ABCDE123!@#abcdefghijklm˙·ÈÛÔ");
		TestAssert(a1 == correctA);
		
		w1 = StringToUpper(L"ABCDE123!@#abcdefghijklm˙·ÈÛÔ");
		TestAssert(w1 == correctW);

		w1 = StringToUpper(srcW);
		TestAssert(w1 == correctW);
		
		a1 = StringToUpper(srcA);
		TestAssert(w1 == correctW);
		
		std::basic_string<DWORD> x1;
		x1 = StringToUpper(srcX);
		TestAssert(x1 == correctX);
	}
	
	{	// **** StringToLower
		std::string a1;
		std::wstring w1;

		std::string srcA = "aeousnt234@#$//¡»‘œÁAEXL>I<TTT";
		std::wstring srcW = L"aeousnt234@#$//¡»‘œÁAEXL>I<TTT";
		std::basic_string<DWORD> srcX;
		ConvertString(srcW, srcX);
		
		std::string correctA = "aeousnt234@#$//·ËÙÔÁaexl>i<ttt";
		std::wstring correctW = L"aeousnt234@#$//·ËÙÔÁaexl>i<ttt";
		std::basic_string<DWORD> correctX;
		ConvertString(correctW, correctX);
		
		a1 = StringToLower("aeousnt234@#$//¡»‘œÁAEXL>I<TTT");
		TestAssert(a1 == correctA);
		
		w1 = StringToLower(L"aeousnt234@#$//¡»‘œÁAEXL>I<TTT");
		TestAssert(w1 == correctW);

		w1 = StringToLower(srcW);
		TestAssert(w1 == correctW);
		
		a1 = StringToLower(srcA);
		TestAssert(w1 == correctW);
		
		std::basic_string<DWORD> x1;
		x1 = StringToLower(srcX);
		TestAssert(x1 == correctX);
	}

	{ // StringEquals
		std::string a1, a2;
		std::wstring w1, w2;

		a1 = "aoeu";
		a2 = "AOEU";
		TestAssert(!StringEquals(a1, a2));

		a1 = "aoeu";
		a2 = "AOEUx";
		TestAssert(!StringEquals(a1, a2));

		a1 = "aoeu";
		a2 = "aoeu";
		TestAssert(StringEquals(a1, a2));

		a1 = "aoeu";
		a2 = "aoeux";
		TestAssert(!StringEquals(a1, a2));

		a1 = "aoeu";
		a2 = "";
		TestAssert(!StringEquals(a1, a2));

		a1 = "";
		a2 = "";
		TestAssert(StringEquals(a1, a2));

		a1 = "";
		a2 = "a";
		TestAssert(!StringEquals(a1, a2));

		w1 = L"aoeu";
		w2 = L"aoeu";
		TestAssert(StringEquals(w1, w2));

		w1 = L"aoeu";
		w2 = L"aoeux";
		TestAssert(!StringEquals(w1, w2));

		a1 = "aoeu";
		TestAssert(StringEquals("aoeu", a1));

		a1 = "aoeu";
		TestAssert(StringEquals(a1, "aoeu"));

		TestAssert(StringEquals(L"HI THERE", L"HI THERE"));
	}
	
	{ // XStringEquals
		std::string a1;
		std::wstring w1;
		
		a1 = "aoeu";
		TestAssert(!XStringEquals(a1, "AOEU"));

		a1 = "aoeu";
		TestAssert(!XStringEquals(a1, "AOEUx"));

		w1 = L"aoeu";
		TestAssert(XStringEquals(w1, "aoeu"));

		w1 = L"aoeu";
		TestAssert(!XStringEquals(w1, "aoeux"));

		w1 = L"aoeu";
		TestAssert(!XStringEquals(w1, ""));

		w1 = L"";
		TestAssert(XStringEquals(w1, ""));

		a1 = "";
		TestAssert(!XStringEquals(a1, "a"));
	}
	
	{ // XStringContains
		TestAssert(XStringContains("aoeu", L'a'));
		TestAssert(!XStringContains("aoeu", L'x'));
		TestAssert(!XStringContains("aoeU", L'u'));
		TestAssert(XStringContains("aoeU", L'U'));
		TestAssert(!XStringContains("", L'a'));
		TestAssert(!XStringContains("aoeu", (char)0));
		TestAssert(!XStringContains("aoeu", (DWORD)0));
		TestAssert(!XStringContains("", (DWORD)0));
	}
	
	{ // XStringFindFirstOf
		std::string a1 = "1aoeu555555aoeu57";
		std::wstring w1 = L"1aoeu555555aoeu57";
		TestAssert(1 == XStringFindFirstOf(a1, "ueoa"));
		TestAssert(std::string::npos == XStringFindFirstOf(a1, "9342999"));
		TestAssert(5 == XStringFindFirstOf(a1, "5"));
		TestAssert(16 == XStringFindFirstOf(a1, "7"));
		TestAssert(16 == XStringFindFirstOf(w1, "7"));
		TestAssert(std::string::npos == XStringFindFirstOf(a1, ""));
		a1.clear();
		TestAssert(std::string::npos == XStringFindFirstOf(a1, "a"));
		TestAssert(std::string::npos == XStringFindFirstOf(a1, ""));
	}
	
	{ // XStringFindLastOf
		std::string a1 = "1aoeu555555aoeu57";
		std::wstring w1 = L"1aoeu555555aoeu57";
		TestAssert(14 == XStringFindLastOf(a1, "ueoa"));
		TestAssert(std::string::npos == XStringFindLastOf(a1, "9342999"));
		TestAssert(15 == XStringFindLastOf(a1, "5"));
		TestAssert(16 == XStringFindLastOf(a1, "7"));
		TestAssert(16 == XStringFindLastOf(w1, "7"));
		TestAssert(0 == XStringFindLastOf(w1, "1"));
		TestAssert(std::string::npos == XStringFindLastOf(a1, ""));
		a1.clear();
		TestAssert(std::string::npos == XStringFindLastOf(a1, "a"));
		TestAssert(std::string::npos == XStringFindLastOf(a1, ""));
	}

	{// StringConvert
		// first check that conversions work for different codepages. this test will check all installed codepages. for some reason this test does fail. it should be investigated.
		EnumCodePagesProc(L"20261");
		EnumCodePagesProc(L"10082");
		EnumCodePagesProc(L"65000");// utf-7
		EnumCodePagesProc(L"65001");// utf-8
		
		//EnumSystemCodePagesW(EnumCodePagesProc, CP_INSTALLED);
		// now check code scenarios.

		Blob<BYTE> b;
		Blob<BYTE> b2;
		std::string a;
		std::string a2;
		std::wstring w;
		std::wstring w2;
		std::basic_string<DWORD> x;
		std::basic_string<DWORD> x2;

		// different dimensions to test here:
		// 1) (5 options) from char (wchar / achar / blob / BYTE* / xchar )
		// 2) (4 options) to char (wchar / achar / blob / xchar)
		//
		// 3) (2 options) codepage / codepage excluded.
		// 4) (2 options) string literal vs. std::string variable
		// for a total of 80 tests, plus probably others

		// TODO: go through these and actually verify codepage translations.


		// WCHAR -> BLOB
		TestAssert(SUCCEEDED(ConvertString(L"hi there", b, 1252)));
		TestAssert(b.Size() == 8);// it should not null-terminate.
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString(L"hi there", b)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		w = L"hi there";
		TestAssert(SUCCEEDED(ConvertString(w, b, 1252)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString(w, b)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		// WCHAR -> char
		TestAssert(SUCCEEDED(ConvertString(L"hi there", a, 1252)));
		TestAssert(a == "hi there");

		TestAssert(SUCCEEDED(ConvertString(L"hi there", a)));
		TestAssert(a == "hi there");

		w = L"hi there";
		TestAssert(SUCCEEDED(ConvertString(w, a, 1252)));
		TestAssert(a == "hi there");

		TestAssert(SUCCEEDED(ConvertString(w, a)));
		TestAssert(a == "hi there");

		// WCHAR -> WCHAR
		TestAssert(SUCCEEDED(ConvertString(L"hi there", w2, 1252)));
		TestAssert(w2 == L"hi there");

		TestAssert(SUCCEEDED(ConvertString(L"hi there", w2)));
		TestAssert(w2 == L"hi there");

		w = L"hi there";
		TestAssert(SUCCEEDED(ConvertString(w, w2, 1252)));
		TestAssert(w2 == L"hi there");

		TestAssert(SUCCEEDED(ConvertString(w, w2)));
		TestAssert(w2 == L"hi there");

		// WCHAR -> Xchar
		TestAssert(SUCCEEDED(ConvertString(L"hi there", x, 1252)));
		TestAssert(XStringEquals(x, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(L"hi there", x)));
		TestAssert(XStringEquals(x, "hi there"));

		w = L"hi there";
		TestAssert(SUCCEEDED(ConvertString(w, x, 1252)));
		TestAssert(XStringEquals(x, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(w, x)));
		TestAssert(XStringEquals(x, "hi there"));

		// char -> Blob
		TestAssert(SUCCEEDED(ConvertString("hi there", b, 1252, 1250)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString("hi there", b, 1252, 1251)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString("hi there", b, 1252)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString("hi there", b)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		a = "hi there";
		TestAssert(SUCCEEDED(ConvertString(a, b, 1252, 1250)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString(a, b, 1252, 1251)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString(a, b, 1252)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString(a, b)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		// char -> char
		TestAssert(SUCCEEDED(ConvertString("hi there", a2, 1252)));
		TestAssert(XStringEquals(a2, "hi there"));

		TestAssert(SUCCEEDED(ConvertString("hi there", a2)));
		TestAssert(XStringEquals(a2, "hi there"));

		a = "hi there";
		TestAssert(SUCCEEDED(ConvertString(a, a2, 1252)));
		TestAssert(XStringEquals(a2, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(a, a2)));
		TestAssert(XStringEquals(a2, "hi there"));

		// char -> wchar
		TestAssert(SUCCEEDED(ConvertString("hi there", w, 1252)));
		TestAssert(XStringEquals(w, "hi there"));

		TestAssert(SUCCEEDED(ConvertString("hi there", w)));
		TestAssert(XStringEquals(w, "hi there"));

		a = "hi there";
		TestAssert(SUCCEEDED(ConvertString(a, w, 1252)));
		TestAssert(XStringEquals(w, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(a, w)));
		TestAssert(XStringEquals(w, "hi there"));

		// char -> xchar
		TestAssert(SUCCEEDED(ConvertString("hi there", x, 1252)));
		TestAssert(XStringEquals(x, "hi there"));

		TestAssert(SUCCEEDED(ConvertString("hi there", x)));
		TestAssert(XStringEquals(x, "hi there"));

		a = "hi there";
		TestAssert(SUCCEEDED(ConvertString(a, x, 1252)));
		TestAssert(XStringEquals(x, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(a, x)));
		TestAssert(XStringEquals(x, "hi there"));

		// xchar -> blob
		DWORD px[10];
		XLastDitchStringCopy("hi there", px);
		TestAssert(SUCCEEDED(ConvertString(px, b, 1252)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString(px, b)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		XLastDitchStringCopy("hi there", x);
		TestAssert(SUCCEEDED(ConvertString(x, b, 1252)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString(x, b)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		// xchar -> char
		XLastDitchStringCopy("hi there", px);
		TestAssert(SUCCEEDED(ConvertString(px, a, 1252)));
		TestAssert(XStringEquals(a, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(px, a)));
		TestAssert(XStringEquals(a, "hi there"));

		XLastDitchStringCopy("hi there", x);
		TestAssert(SUCCEEDED(ConvertString(x, a, 1252)));
		TestAssert(XStringEquals(a, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(x, a)));
		TestAssert(XStringEquals(a, "hi there"));

		// xchar -> wchar
		XLastDitchStringCopy("hi there", px);
		TestAssert(SUCCEEDED(ConvertString(px, w, 1252)));
		TestAssert(XStringEquals(w, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(px, w)));
		TestAssert(XStringEquals(w, "hi there"));

		XLastDitchStringCopy("hi there", x);
		TestAssert(SUCCEEDED(ConvertString(x, w, 1252)));
		TestAssert(XStringEquals(w, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(x, w)));
		TestAssert(XStringEquals(w, "hi there"));

		// xchar -> xchar
		XLastDitchStringCopy("hi there", px);
		TestAssert(SUCCEEDED(ConvertString(px, x2, 1252)));
		TestAssert(XStringEquals(x2, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(px, x2)));
		TestAssert(XStringEquals(x2, "hi there"));

		XLastDitchStringCopy("hi there", x);
		TestAssert(SUCCEEDED(ConvertString(x, x2, 1252)));
		TestAssert(XStringEquals(x2, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(x, x2)));
		TestAssert(XStringEquals(x2, "hi there"));

		// xchar -> ychar
		std::basic_string<__int64> y;// whoa this is really getting crazy :P

		XLastDitchStringCopy("hi there", px);
		TestAssert(SUCCEEDED(ConvertString(px, y, 1252)));
		TestAssert(XStringEquals(y, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(px, y)));
		TestAssert(XStringEquals(y, "hi there"));

		XLastDitchStringCopy("hi there", x);
		TestAssert(SUCCEEDED(ConvertString(x, y, 1252)));
		TestAssert(XStringEquals(y, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(x, y)));
		TestAssert(XStringEquals(y, "hi there"));

		// blob -> Blob
		ConvertString("hi there", b, 1252);// get a blob of the string.
		TestAssert(SUCCEEDED(ConvertString(b, b2, 1252, 1251)));
		TestAssert(b2.Size() == 8);
		TestAssert(0 == strncmp((char*)b2.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString(b, b2)));
		TestAssert(b2.Size() == 8);
		TestAssert(0 == strncmp((char*)b2.GetBuffer(), "hi there", 8));

		// blob -> char
		ConvertString("hi there", b, 1252);// get a blob of the string.
		TestAssert(SUCCEEDED(ConvertString(b, a, 1252)));
		TestAssert(a == "hi there");

		TestAssert(SUCCEEDED(ConvertString(b, a)));
		TestAssert(a == "hi there");

		// blob -> wchar
		ConvertString("hi there", b, 1252);// get a blob of the string.
		TestAssert(SUCCEEDED(ConvertString(b, w, 1252)));
		TestAssert(w == L"hi there");

		TestAssert(SUCCEEDED(ConvertString(b, w)));
		TestAssert(w == L"hi there");

		// blob -> xchar
		ConvertString("hi there", b, 1252);// get a blob of the string.
		TestAssert(SUCCEEDED(ConvertString(b, x, 1252)));
		TestAssert(XStringEquals(x, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(b, x)));
		TestAssert(XStringEquals(x, "hi there"));

		// BYTE* -> Blob
		BYTE pb[10];
		XLastDitchStringCopy("hi there", pb);
		TestAssert(SUCCEEDED(ConvertString(pb, 8, b, 1252, 1252)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString(pb, 8, b, 1252, 1251)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString(pb, 8, b, 1252)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		TestAssert(SUCCEEDED(ConvertString(pb, 8, b)));
		TestAssert(b.Size() == 8);
		TestAssert(0 == strncmp((char*)b.GetBuffer(), "hi there", 8));

		// BYTE* -> char
		TestAssert(SUCCEEDED(ConvertString(pb, 8, a, 1252, 1252)));
		TestAssert(a == "hi there");

		TestAssert(SUCCEEDED(ConvertString(pb, 8, a, 1252, 1251)));
		TestAssert(a == "hi there");

		TestAssert(SUCCEEDED(ConvertString(pb, 8, a, 1252)));
		TestAssert(a == "hi there");

		TestAssert(SUCCEEDED(ConvertString(pb, 8, a)));
		TestAssert(a == "hi there");

		// BYTE* -> wchar
		TestAssert(SUCCEEDED(ConvertString(pb, 8, w, 1252)));
		TestAssert(w == L"hi there");

		TestAssert(SUCCEEDED(ConvertString(pb, 8, w)));
		TestAssert(w == L"hi there");

		// BYTE* -> xchar
		TestAssert(SUCCEEDED(ConvertString(pb, 8, x, 1252)));
		TestAssert(XStringEquals(x, "hi there"));

		TestAssert(SUCCEEDED(ConvertString(pb, 8, x)));
		TestAssert(XStringEquals(x, "hi there"));
	}

	{// ToUTF8()
		std::string a;
		std::wstring w;
		std::string a2;

		w = L"aoeu";
		w[0] = 9674;
		std::wstring w2 = w;// make a copy for comparing later
		TestAssert(SUCCEEDED(ToUTF8(w, a2)));
		// now that string in UTF8 is E2 97 8A 6F 65 75 (verified with Uedit32)
		TestAssert(a2.length() == 6);
		TestAssert((BYTE)a2[0] == 0xe2);
		TestAssert((BYTE)a2[1] == 0x97);
		TestAssert((BYTE)a2[2] == 0x8a);
		TestAssert(a2[3] == 0x6f);
		TestAssert(a2[4] == 0x65);
		TestAssert(a2[5] == 0x75);

		// convert back to unicode
		w = L"12312312";// clean the palette
		TestAssert(SUCCEEDED(ConvertString(a2, w, CP_UTF8)));
		TestAssert(w2 == w);

		// test other compile scenarios
		ToUTF8(a, a);
		ToUTF8("aoeu", a);
		ToUTF8(w, a);
		ToUTF8(L"aoeu", a);
	}

	// TODO: add tests for libcc::Format

  return true;
}


