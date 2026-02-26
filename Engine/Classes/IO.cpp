#include "IO.h"

#ifdef _WIN32
#include "dirent.h"
#include <filesystem>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <fstream>
#include <stdio.h>
#include <iostream>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <boost/iostreams/device/mapped_file.hpp>
#include "StringConverter.h"

namespace GX
{
	IO::IO()
	{
	}

	IO::~IO()
	{
	}

	bool IO::isDir(std::string dir)
	{
	#ifdef _WIN32
		std::wstring wdir = StringConvert::s2ws(dir, GetACP()).c_str();
	#else
		std::wstring wdir = StringConvert::s2ws(dir, 65001).c_str();
	#endif
		return boost::filesystem::is_directory(wdir);
	}

	bool IO::isDir(std::wstring dir)
	{
		return boost::filesystem::is_directory(dir);
	}

	void IO::listFiles(std::string baseDir, bool recursive, std::function<bool(std::string)> callback_d, std::function<bool(std::string, std::string)> callback)
	{
	#ifdef _WIN32
		_WDIR *dp;
		struct _wdirent *dirp;
		int cp = GetACP();
		const char* _dir = baseDir.c_str();
		std::wstring wstr = StringConvert::s2ws(_dir, cp);
		std::wstring _wbasedir = StringConvert::s2ws(baseDir, cp);

		if ((dp = _wopendir(wstr.c_str())) == NULL)
		{
			std::cout << "[ERROR: " << errno << " ] Couldn't open " << baseDir << "." << std::endl;
			return;
		}
		else
		{
			while ((dirp = _wreaddir(dp)) != NULL)
			{
				if (dirp->d_name != std::wstring(L".") && dirp->d_name != std::wstring(L".."))
				{
					std::string _wname = StringConvert::ws2s(dirp->d_name, cp);
					std::string _str = baseDir + _wname;

					if (isDir(_wbasedir + dirp->d_name) == true && recursive == true)
					{
						if (callback_d != NULL)
						{
							if (!callback_d(_str))
								break;
						}

						listFiles(_str + "/", true, callback_d, callback);
					}
					else
					{
						if (callback != NULL)
						{
							if (!callback(baseDir, _wname))
								break;
						}
					}
				}
			}

			_wclosedir(dp);
		}
	#else
		DIR *dp;
		struct dirent *dirp;
		const char* _dir = baseDir.c_str();

		if ((dp = opendir(_dir)) == NULL)
		{
			std::cout << "[ERROR: " << errno << " ] Couldn't open " << baseDir << "." << std::endl;
			return;
		}
		else
		{
			while ((dirp = readdir(dp)) != NULL)
			{
				if (dirp->d_name != std::string(".") && dirp->d_name != std::string(".."))
				{
					std::string _wname = dirp->d_name;
					std::string _str = baseDir + _wname;

					if (isDir(baseDir + dirp->d_name) == true && recursive == true)
					{
						if (callback_d != NULL)
						{
							if (!callback_d(_str))
								break;
						}

						listFiles(_str + "/", true, callback_d, callback);
					}
					else
					{
						if (callback != NULL)
						{
							if (!callback(baseDir, _wname))
								break;
						}
					}
				}
			}

			closedir(dp);
		}
	#endif
	}

	std::string IO::GetFileExtension(const std::string& FileName)
	{
		if (FileName.length() > 0)
		{
			size_t i = FileName.find_last_of(".");

			if (i != std::string::npos)
			{
				if (FileName.length() > i)
				{
					std::string ext = FileName.substr(i + 1);
					ext = boost::algorithm::to_lower_copy(ext);
					
					return ext;
				}
			}
		}

		return "";
	}

	std::string IO::GetFileName(const std::string & FileName)
	{
		if (FileName.find_last_of("/") != std::string::npos)
			return FileName.substr(FileName.find_last_of("/") + 1, FileName.find_last_of(".") - FileName.find_last_of("/") - 1);
		else if (FileName.find_last_of("\\") != std::string::npos)
			return FileName.substr(FileName.find_last_of("\\") + 1, FileName.find_last_of(".") - FileName.find_last_of("\\") - 1);
		else if (FileName.find_last_of(".") != std::string::npos)
			return FileName.substr(0, FileName.find_last_of("."));
		else return FileName;

		return "";
	}

	std::string IO::GetFileNameWithExt(const std::string & FileName)
	{
		if (FileName.find_last_of("/") != std::string::npos)
			return FileName.substr(FileName.find_last_of("/") + 1);
		else if (FileName.find_last_of("\\") != std::string::npos)
			return FileName.substr(FileName.find_last_of("\\") + 1);
		else
			return FileName;

		return "";
	}

	std::string IO::GetFilePath(const std::string & FileName)
	{
		boost::filesystem::wpath p(FileName);
		boost::filesystem::wpath dir = p.remove_filename();
		std::string _fpath = dir.generic_string();

		if (!_fpath.empty())
			_fpath += "/";

		return _fpath;
	}

	std::string IO::RemovePart(const std::string& FileName, const std::string& part)
	{
		std::string result = boost::replace_all_copy(FileName, part, "");

		return result;
	}

	bool IO::FileExists(const std::string & name)
	{
	#ifdef _WIN32
		struct _stat buffer;
		return (_wstat(StringConvert::s2ws(name, GetACP()).c_str(), &buffer) == 0) && !isDir(name);
	#else
		struct stat buffer;
		return (stat(name.c_str(), &buffer) == 0) && !isDir(name);
	#endif
	}

	bool IO::DirExists(const std::string & name)
	{
	#ifdef _WIN32
		struct _stat buffer;
		return (_wstat(StringConvert::s2ws(name, GetACP()).c_str(), &buffer) == 0) && isDir(name);
	#else
		struct stat buffer;
		return (stat(name.c_str(), &buffer) == 0) && isDir(name);
	#endif
	}

	std::string IO::ReadText(std::string path)
	{
		std::string result = "";

		std::ifstream in(path, std::ios::in);

		if (!in.is_open()) {
			return "";
		}

		std::string line;
		while (std::getline(in, line))
		{
			result += line + "\n";
		}

		in.close();

		result = IO::Replace(result, "\r\n", "\n");

		return result;
	}

	void IO::WriteText(std::string path, std::string text)
	{
		std::ofstream myfile;
		myfile.open(path);
		myfile << text;
		myfile.close();
	}

	void IO::AppendText(std::string path, std::string text)
	{
		std::ofstream myfile;
		myfile.open(path, std::ios::out | std::ios::app);
		myfile << text;
		myfile.close();
	}

	void IO::FileCopy(std::string from, std::string to)
	{
		std::ifstream  src(from, std::ios::binary);
		std::ofstream  dst(to, std::ios::binary);

		dst << src.rdbuf();
	}

	void IO::DirCopy(std::string from, std::string to, bool recursive)
	{
	#ifdef _WIN32
		if (recursive)
			std::filesystem::copy(std::filesystem::path(from), std::filesystem::path(to), std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
		else
			std::filesystem::copy(std::filesystem::path(from), std::filesystem::path(to), std::filesystem::copy_options::overwrite_existing);
	#else
		if (recursive)
			boost::filesystem::copy(boost::filesystem::path(from), boost::filesystem::path(to), boost::filesystem::copy_options::recursive | boost::filesystem::copy_options::overwrite_existing);
		else
			boost::filesystem::copy(boost::filesystem::path(from), boost::filesystem::path(to), boost::filesystem::copy_options::overwrite_existing);
	#endif
	}

	boost::system::error_code IO::FileRename(std::string from, std::string to)
	{
		boost::system::error_code err;
		boost::filesystem::rename(from, to, err);

		return err;
	}

	std::string IO::ReplaceBackSlashes(std::string input)
	{
		std::string result = boost::replace_all_copy(input, std::string("\\"), std::string("/"));

		return result;
	}

	std::string IO::ReplaceFrontSlashes(std::string input)
	{
		std::string result = boost::replace_all_copy(input, std::string("/"), std::string("\\"));

		return result;
	}

	std::string IO::Replace(std::string input, std::string search, std::string format)
	{
		return boost::replace_all_copy(input, search, format);
	}

	void IO::CreateDir(std::string path, bool recursive)
	{
	#ifdef _WIN32
		if (!recursive)
		{
			CreateDirectoryW(StringConvert::s2ws(path, GetACP()).c_str(), NULL);
		}
		else
		{
			boost::filesystem::create_directories(path);
		}
	#else
		if (!recursive)
		{
			mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}
		else
		{
			boost::filesystem::create_directories(path);
		}
	#endif
	}

	void IO::FileDelete(std::string path)
	{
		std::remove(path.c_str());
	}

	std::vector<unsigned char> IO::ReadBinaryFile(const std::string filename)
	{
		// binary mode is only for switching off newline translation
		std::ifstream file(filename, std::ios::binary);
		file.unsetf(std::ios::skipws);

		std::streampos file_size;
		file.seekg(0, std::ios::end);
		file_size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<unsigned char> vec(file_size);
		vec.insert(vec.begin(),
			std::istream_iterator<unsigned char>(file),
			std::istream_iterator<unsigned char>());
		return (vec);
	}

	void IO::WriteBinaryFile(const std::string filename, char* data, int size)
	{
		std::ofstream file;
		file.open(filename.c_str(), std::ios_base::binary);
		assert(file.is_open());

		file.write((const char*)data, size);

		file.close();
	}

	void IO::DirDeleteRecursive(std::string path)
	{
		try
		{
			if (boost::filesystem::exists(path))
			{
				boost::filesystem::remove_all(path);
			}
		}
		catch (boost::filesystem::filesystem_error const& e)
		{
			//display error message 
		}
	}

	std::string IO::RemoveLastLocation(std::string input)
	{
		std::string str = input;

		if (str.length() > 0)
		{
			str = RemoveLastSlash(str);

			return GetFilePath(str);
		}

		return "";
	}

	std::string IO::RemoveLastSlash(std::string input)
	{
		std::string str = input;

		if (str.length() > 0)
		{
			if (str[str.length() - 1] == '/')
				str = str.substr(0, str.length() - 2);

			if (str[str.length() - 1] == '\\')
				str = str.substr(0, str.length() - 2);

			return str;
		}

		return "";
	}
}