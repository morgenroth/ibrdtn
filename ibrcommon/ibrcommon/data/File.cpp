/*
 * File.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
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

#if !defined(HAVE_FEATURES_H) || defined(ANDROID)
#include <libgen.h>
#endif

#ifdef WIN32
#include <io.h>
#define FILE_DELIMITER_CHAR '\\'
#define FILE_DELIMITER "\\"
#else
#define FILE_DELIMITER_CHAR '/'
#define FILE_DELIMITER "/"
#endif

namespace ibrcommon
{
	File::File()
	 : _path(), _type(DT_UNKNOWN)
	{
	}

	File::File(const std::string &path, const FILE_TYPE t)
	 : _path(path), _type(t)
	{
		resolveAbsolutePath();
		removeSlash();
	}

	File::File(const std::string &path)
	 : _path(path), _type(DT_UNKNOWN)
	{
		resolveAbsolutePath();
		removeSlash();
		update();
	}

	void File::removeSlash()
	{
		std::string::iterator iter = _path.end(); --iter;

		if ((*iter) == FILE_DELIMITER_CHAR)
		{
			_path.erase(iter);
		}
	}

	void File::resolveAbsolutePath()
	{
		std::string::iterator iter = _path.begin();

		if ((*iter) != FILE_DELIMITER_CHAR)
		{
			_path = "." + std::string(FILE_DELIMITER) + _path;
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

		if ( stat(_path.c_str(), &s) == 0 )
		{
			int type = s.st_mode & S_IFMT;

			switch (type)
			{
				case S_IFREG:
					_type = DT_REG;
					break;

#ifndef WIN32
				case S_IFLNK:
					_type = DT_LNK;
					break;
#endif

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

	File::FILE_TYPE File::getType() const
	{
		return _type;
	}

	int File::getFiles(std::list<File> &files) const
	{
		if (!isDirectory()) return -1;

		DIR *dp;
		struct dirent dirp_data;
		struct dirent *dirp;
		if((dp = opendir(_path.c_str())) == NULL) {
			return errno;
		}

#if WIN32
		while ((dirp = ::readdir(dp)) != NULL)
#else
		while (::readdir_r(dp, &dirp_data, &dirp) == 0)
#endif
		{
			if (dirp == NULL) break;

			// TODO: check if the name is always limited by a zero
			// std::string name = std::string(dirp->d_name, dirp->d_reclen);

			std::string name = std::string(dirp->d_name);
			std::stringstream ss; ss << getPath() << FILE_DELIMITER_CHAR << name;
#if WIN32
			File file(ss.str());
#else
			File file(ss.str(), dirp->d_type);
#endif
			files.push_back(file);
		}
		closedir(dp);

		return 0;
	}

	bool File::isSystem() const
	{
		return ((getBasename() == "..") || (getBasename() == "."));
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
#if !defined(ANDROID) && defined(HAVE_FEATURES_H)
		return std::string(basename(_path.c_str()));
#else
		char path[_path.length()+1];
		::memcpy(&path, _path.c_str(), _path.length()+1);

		return std::string(basename(path));
#endif
	}

	File File::get(string filename) const
	{
		stringstream ss; ss << getPath() << FILE_DELIMITER_CHAR << filename;
		File file(ss.str());

		return file;
	}

	int File::remove(bool recursive)
	{
		if (isSystem()) return -1;
		if (_type == DT_UNKNOWN) return -1;

		if (isDirectory())
		{
			if (recursive)
			{
				int ret = 0;

				// container for all files
				list<File> files;

				// get all files in this directory
				if ((ret = getFiles(files)) < 0)
					return ret;

				for (list<File>::iterator iter = files.begin(); iter != files.end(); ++iter)
				{
					ibrcommon::File &file = (*iter);
					if (!file.isSystem())
					{
						if ((ret = file.remove(recursive)) < 0)
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
#ifdef WIN32
			::mkdir(path.getPath().c_str());
#else
			::mkdir(path.getPath().c_str(), 0700);
#endif

			// update file information
			path.update();
		}
	}

	size_t File::size() const
	{
		struct stat filestatus;
		stat( getPath().c_str(), &filestatus );
		return static_cast<size_t>(filestatus.st_size);
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
		// check if path is a directory
		if (!path.isDirectory())
		{
			throw ibrcommon::IOException("given path is not a directory.");
		}

		std::string pattern = path.getPath() + FILE_DELIMITER_CHAR + prefix + "XXXXXX";

		std::vector<char> name(pattern.length() + 1);
		::strcpy(&name[0], pattern.c_str());

#ifdef WIN32
		// create temporary file - win32 style
		int fd = -1;
		if (_mktemp_s(&name[0], pattern.length() + 1) == 0) {
			fd = open(&name[0], O_CREAT, S_IREAD | S_IWRITE);
		}
#else
		int fd = mkstemp(&name[0]);
#endif
		if (fd == -1) throw ibrcommon::IOException("Could not create a temporary name.");
		::close(fd);

		return std::string(name.begin(), name.end());
	}
}
