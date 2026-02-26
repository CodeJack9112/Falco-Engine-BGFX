#pragma once

#include <string>
#include <vector>
#include <functional>
#include <boost/filesystem.hpp>

namespace GX
{
	class IO
	{
	public:
		IO();
		~IO();

		static bool isDir(std::string dir);
		static bool isDir(std::wstring dir);
		static void listFiles(std::string baseDir, bool recursive, std::function<bool(std::string)> callback_d, std::function<bool(std::string, std::string)> callback);
		static std::string GetFileExtension(const std::string& FileName);
		static std::string GetFileName(const std::string& FileName);
		static std::string GetFileNameWithExt(const std::string& FileName);
		static std::string GetFilePath(const std::string& FileName);
		static std::string RemovePart(const std::string& FileName, const std::string& part);
		static bool FileExists(const std::string& name);
		static bool DirExists(const std::string& name);
		static std::string ReadText(std::string path);
		static void WriteText(std::string path, std::string text);
		static void AppendText(std::string path, std::string text);
		static void FileCopy(std::string from, std::string to);
		static void DirCopy(std::string from, std::string to, bool recursive = true);
		static void DirDeleteRecursive(std::string path);
		static boost::system::error_code FileRename(std::string from, std::string to);
		static std::string ReplaceBackSlashes(std::string input);
		static std::string ReplaceFrontSlashes(std::string input);
		static std::string Replace(std::string input, std::string search, std::string format);
		static std::string RemoveLastLocation(std::string input);
		static std::string RemoveLastSlash(std::string input);
		static void CreateDir(std::string path, bool recursive = false);
		static void FileDelete(std::string path);
		static std::vector<unsigned char> ReadBinaryFile(const std::string filename);
		static void WriteBinaryFile(const std::string filename, char* data, int size);
	};
}