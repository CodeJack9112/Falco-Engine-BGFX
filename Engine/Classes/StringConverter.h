#pragma once

#include <string>

namespace GX
{
	class StringConvert
	{
	public:
		static std::wstring s2ws(const std::string& s, int cp = 0);
		static std::string ws2s(const std::wstring& s, int cp = 0);
		static std::string cp_convert(const char* str, int from, int to);
		static std::string cp_convert(std::string str, int from, int to);

		static std::string cp_from_sys_to_unicode(std::string str);
		static std::string cp_from_unicode_to_sys(std::string str);
	};
}

#ifndef CP_UNI
#define CP_UNI GX::StringConvert::cp_from_sys_to_unicode
#endif

#ifndef CP_SYS
#define CP_SYS GX::StringConvert::cp_from_unicode_to_sys
#endif