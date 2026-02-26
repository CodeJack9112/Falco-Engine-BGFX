#include "Helpers.h"

#include "../Engine/Classes/StringConverter.h"
#include "../Engine/Classes/IO.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string.hpp>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <libgen.h>
#include <unistd.h>
#include <linux/limits.h>
#endif

namespace GX
{
	std::string Helper::ExePath()
	{
	#ifdef _WIN32
		HMODULE hmod = GetModuleHandle(NULL);

		wchar_t fullPath[MAX_PATH];
		DWORD pathLen = ::GetModuleFileNameW(hmod, fullPath, MAX_PATH); // hmod of zero gets the main EXE
		std::wstring _fullPath(fullPath);
		_fullPath = boost::replace_all_copy(_fullPath, std::string("\\"), std::string("/"));
		std::string path = StringConvert::ws2s(_fullPath);
		path = IO::GetFilePath(path);

		return path;
	#else
		char result[PATH_MAX];
		ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
		std::string path = "/";
		std::string str = result;
		str.resize(count);
		path = IO::GetFilePath(str);
		
		return path;
	#endif
	}

	std::string Helper::ExeName()
	{
	#ifdef _WIN32
		HMODULE hmod = GetModuleHandle(NULL);

		wchar_t fullPath[MAX_PATH];
		DWORD pathLen = ::GetModuleFileNameW(hmod, fullPath, MAX_PATH); // hmod of zero gets the main EXE
		std::wstring _fullPath(fullPath);
		_fullPath = boost::replace_all_copy(_fullPath, std::string("\\"), std::string("/"));
		std::string path = StringConvert::ws2s(_fullPath);

		return path;
	#else
		char fullPath[PATH_MAX];
		ssize_t count = readlink("/proc/self/exe", fullPath, PATH_MAX);
		std::string _fullPath = fullPath;
		_fullPath.resize(count);
		_fullPath = boost::replace_all_copy(_fullPath, std::string("\\"), std::string("/"));
		std::string path = _fullPath;

		return path;
	#endif
	}

	std::string Helper::WorkingDir()
	{
		boost::filesystem::path full_path(boost::filesystem::current_path());

		return full_path.generic_string() + "/";
	}

	std::string Helper::getVersion()
	{
	#ifdef _WIN32
		std::string csRet = "";
		HMODULE hLib = GetModuleHandle(NULL);

		HRSRC hVersion = FindResource(hLib, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
		if (hVersion != NULL)
		{
			HGLOBAL hGlobal = LoadResource(hLib, hVersion);
			if (hGlobal != NULL)
			{
				LPVOID versionInfo = LockResource(hGlobal);
				if (versionInfo != NULL)
				{
					UINT length;
					VS_FIXEDFILEINFO* verInfo = NULL;

					// Query the version information for neutral language
					if (TRUE == VerQueryValueW(
						versionInfo,
						L"\\",
						reinterpret_cast<LPVOID*>(&verInfo),
						&length))
					{
						//  Pull the version values.
						int major = HIWORD(verInfo->dwProductVersionMS);
						int minor = LOWORD(verInfo->dwProductVersionMS);
						int build = HIWORD(verInfo->dwProductVersionLS);
						int revision = LOWORD(verInfo->dwProductVersionLS);
						csRet = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(build) + "." + std::to_string(revision);
					}
				}
			}

			UnlockResource(hGlobal);
			FreeResource(hGlobal);
		}

		return csRet;
	#else
		return "3.8.0.9 (linux)";
	#endif
	}
}