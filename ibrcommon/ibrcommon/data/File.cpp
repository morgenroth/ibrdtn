/*
 * File.cpp
 *
 *  Created on: 20.11.2009
 *      Author: morgenro
 */

#include "ibrcommon/config.h"
#include "ibrcommon/data/File.h"
#include <sstream>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

#ifndef HAVE_FEATURES_H
#include <libgen.h>
#endif

namespace ibrcommon
{
	File::File()
	 : _path(), _type(DT_UNKNOWN)
	{
	}

	File::File(const string path, const unsigned char t)
	 : _path(path), _type(t)
	{
		resolveAbsolutePath();
		removeSlash();
	}

	File::File(const string path)
	 : _path(path), _type(DT_UNKNOWN)
	{
		resolveAbsolutePath();
		removeSlash();
		update();
	}

	void File::removeSlash()
	{
		std::string::iterator iter = _path.end(); iter--;

		if ((*iter) == '/')
		{
			_path.erase(iter);
		}
	}

	void File::resolveAbsolutePath()
	{
		std::string::iterator iter = _path.begin();

		if ((*iter) != '/')
		{
			_path = "./" + _path;
		}
	}

	bool File::exists() const
	{
		struct stat st;
		if( stat(_path.c_str(), &st ) == 0)
			return true;

		return false;
	}

	void File::update()
	{
		struct stat s;
		int type;

		if ( stat(_path.c_str(), &s) == 0 )
		{
			type = s.st_mode & S_IFMT;

			switch (type)
			{
				case S_IFREG:
					_type = DT_REG;
					break;

				case S_IFLNK:
					_type = DT_LNK;
					break;

				case S_IFDIR:
					_type = DT_DIR;
					break;

				default:
					_type = DT_UNKNOWN;
					break;
			}
		}
	}

	File::~File()
	{}

	unsigned char File::getType() const
	{
		return _type;
	}

	int File::getFiles(list<File> &files) const
	{
		if (!isDirectory()) return -1;

		DIR *dp;
		struct dirent *dirp;
		if((dp = opendir(_path.c_str())) == NULL) {
			return errno;
		}

		while ((dirp = readdir(dp)) != NULL)
		{
			string name = string(dirp->d_name);
			stringstream ss; ss << getPath() << "/" << name;
			File file(ss.str(), dirp->d_type);
			files.push_back(file);
		}
		closedir(dp);

		return 0;
	}

	bool File::isSystem() const
	{
		try {
			if ((_path.substr(_path.length() - 2, 2) == "..") || (_path.substr(_path.length() - 1, 1) == ".")) return true;
		} catch (const std::out_of_range&) {
			return false;
		}
		return false;
	}

	bool File::isDirectory() const
	{
		if (_type == DT_DIR) return true;
		return false;
	}

	string File::getPath() const
	{
		return _path;
	}

	std::string File::getBasename() const
	{
#ifdef HAVE_FEATURES_H
		return std::string(basename(_path.c_str()));
#else
		char path[_path.length()];
		::memcpy(&path, _path.c_str(), _path.length());

		return std::string(basename(path));
#endif
	}

	File File::get(string filename) const
	{
		stringstream ss; ss << getPath() << "/" << filename;
		File file(ss.str());

		return file;
	}

	int File::remove(bool recursive)
	{
		int ret;

		if (isSystem()) return -1;
		if (_type == DT_UNKNOWN) return -1;

		if (isDirectory())
		{
			if (recursive)
			{
				// container for all files
				list<File> files;

				// get all files in this directory
				if ((ret = getFiles(files)) < 0)
					return ret;

				for (list<File>::iterator iter = files.begin(); iter != files.end(); iter++)
				{
					if (!(*iter).isSystem())
					{
						if ((ret = (*iter).remove(recursive)) < 0)
							return ret;
					}
				}
			}

			::rmdir(getPath().c_str());
		}
		else
		{
			::remove(getPath().c_str());
		}

		return 0;
	}

	File File::getParent() const
	{
		size_t pos = _path.find_last_of('/');
		return File(_path.substr(0, pos));
	}

	void File::createDirectory(File &path)
	{
		if (!path.exists())
		{
			File parent = path.getParent();
			File::createDirectory(parent);

			// create path
			::mkdir(path.getPath().c_str(), 0700);

			// update file information
			path.update();
		}
	}

	size_t File::size() const
	{
		struct stat filestatus;
		stat( getPath().c_str(), &filestatus );
		return filestatus.st_size;
	}

	time_t File::lastaccess() const
	{
		struct stat filestatus;
		stat( getPath().c_str(), &filestatus );
		return filestatus.st_atime;
	}

	time_t File::lastmodify() const
	{
		struct stat filestatus;
		stat( getPath().c_str(), &filestatus );
		return filestatus.st_mtime;
	}

	time_t File::laststatchange() const
	{
		struct stat filestatus;
		stat( getPath().c_str(), &filestatus );
		return filestatus.st_ctime;
	}

	bool File::operator==(const ibrcommon::File &other) const
	{
		return (other._path == _path);
	}

	bool File::operator<(const ibrcommon::File &other) const
	{
		return (_path < other._path);
	}

	TemporaryFile::TemporaryFile(const File &path, const std::string prefix)
	 : File(tmpname(path, prefix))
	{
	}

	TemporaryFile::~TemporaryFile()
	{
	}

	std::string TemporaryFile::tmpname(const File &path, const std::string prefix)
	{
		std::string pattern = path.getPath() + "/" + prefix + "XXXXXX";
		char name[pattern.length()];
		::strcpy(name, pattern.c_str());

		int fd = mkstemp(name);
		if (fd == -1) throw ibrcommon::IOException("Could not create a temporary name.");
		::close(fd);

		return std::string(name);
	}
}
