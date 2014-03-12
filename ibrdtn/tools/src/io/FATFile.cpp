/*
 * FATFile.h
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
 *
 * Written-by: David Goltzsche <goltzsch@ibr.cs.tu-bs.de>
 *             Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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
 *  Created on: Sep 23, 2013
 */

#include "FATFile.h"
#ifdef HAVE_LIBTFFS


FATFile::FATFile(const ibrcommon::File &image_file, const std::string &file_path)
 : ibrcommon::File(file_path), _image_file(image_file), htffs(0), hdir(0), hfile(0), ret(-1)
{
	update();
}

FATFile::~FATFile()
{
}

const ibrcommon::File& FATFile::getImageFile() const
{
	return _image_file;
}

int FATFile::getFiles( list<FATFile> &files)
{
	mount_tffs();
	opendir_tffs();

	std::string path = getPath();
	while( 1 )
	{
		if ((ret = TFFS_readdir(hdir, &dirent)) == TFFS_OK)
		{
			std::string newpath = path + "/" + dirent.d_name;

			FATFile f(_image_file, newpath);
			files.push_back(f);
		}
		else if (ret == ERR_TFFS_LAST_DIRENTRY) { // end of directory
			break;
		}
		else {
			cout << "ERROR: TFFS_readdir" << ret << endl;
			return -1;
		}
	}
	closedir_tffs();
	umount_tffs();
	return 0;



}

int FATFile::remove( bool recursive )
{
	cout << "deleting" << getPath() << endl;
	if (isSystem()) return -1;
	//if (_type == DT_UNKNOWN) return -1;

	if(isDirectory())
	{
		if(recursive)
		{
			int ret = 0;

			// container for all files
			list<FATFile> files;

			// get all files in this directory
			if ((ret = getFiles(files)) < 0)
				return ret;

			for (list<FATFile>::iterator iter = files.begin(); iter != files.end(); ++iter)
			{
			FATFile &file = (*iter);
			if (!file.isSystem())
			{
				if ((ret = file.remove(recursive)) < 0)
					return ret;
				}
			}
			//remove dir
			mount_tffs();
			opendir_tffs();
			std::string path = getPath();
			byte* byte_path = const_cast<char *>(path.c_str());
			if ((ret = TFFS_rmdir(htffs, byte_path)) != TFFS_OK)
			{
				cout << "ERROR: TFFS_rmdir" << ret << endl;
				return -1;
			}
			closedir_tffs();
			umount_tffs();
		}
	}
	else
	{
		//remove file
		mount_tffs();
		opendir_tffs();
		std::string path = getPath();
		byte* byte_path = const_cast<char *>(path.c_str());
		if ((ret = TFFS_rmfile(htffs, byte_path)) != TFFS_OK)
		{
			cout << "ERROR: TFFS_rmfile" << ret << endl;
			return -1;
		}
		closedir_tffs();
		umount_tffs();
	}
	return 0;
}

FATFile FATFile::get( std::string filename )
{
	throw ibrcommon::IOException("not implemented yet");
}

FATFile FATFile::getParent()
{
	throw ibrcommon::IOException("not implemented yet");
}

bool FATFile::exists()
{
	mount_tffs();
	opendir_tffs();
	//remove leading "./"
	std::string path = getPath();
	if(path == "/" || path.empty())
	{
		closedir_tffs();
		umount_tffs();
		return true; //root always exists
	}

	if( path.length() > 2 && path.substr(0,2) == "./")
			path = path.substr(2);

	byte* byte_path = const_cast<char *>(path.c_str());
	if ((ret = TFFS_fopen(htffs, byte_path, "r", &hfile)) != TFFS_OK) {
		return false;
	}
	closedir_tffs();
	umount_tffs();
	return true;
}

void FATFile::update()
{
	mount_tffs();
	opendir_tffs();
	set_dirent_to_current();
	ubyte attr = dirent.dir_attr;
	if( attr & DIR_ATTR_DIRECTORY)
		_type = DT_DIR;
	closedir_tffs();
	umount_tffs();
}

size_t FATFile::size()
{
	mount_tffs();
	opendir_tffs();
	set_dirent_to_current();
	closedir_tffs();
	umount_tffs();
	return dirent.dir_file_size;
}

time_t FATFile::lastaccess()
{
	mount_tffs();
	opendir_tffs();
	set_dirent_to_current();

	struct tm tm;
	tm.tm_year = dirent.crttime.year - 1900;
	tm.tm_mon  = dirent.crttime.month - 1;
	tm.tm_mday = dirent.crttime.day;
	tm.tm_hour = dirent.crttime.hour;
	tm.tm_min  = dirent.crttime.min;
	tm.tm_sec  = dirent.crttime.sec / 2; //somehow, libtffs writes seconds from 0-119;
	closedir_tffs();
	umount_tffs();
	return mktime(&tm);
}

time_t FATFile::lastmodify()
{
	return lastaccess();
}

time_t FATFile::laststatchange()
{
	return lastaccess();
}

int FATFile::mount_tffs()
{
	byte* path = const_cast<char *>(_image_file.getPath().c_str());
	if ((ret = TFFS_mount(path, &htffs)) != TFFS_OK) {
		std::cerr << "ERROR: TFFS_mount" << ret << " in " << _image_file.getPath() << std::endl;
		return -1;
	}
	return 0;
}

int FATFile::umount_tffs()
{
	if ((ret = TFFS_umount(htffs)) != TFFS_OK) {
		cout << "ERROR: TFFS_mount" << ret << endl;
		return -1;
	}
	return 0;
}

int FATFile::opendir_tffs()
{

	std::string path = getPath();
	std::string cur_dir;
	if(path.length() == 0)
		cur_dir = "/";
	else if(isDirectory())
		cur_dir = path.substr(1);
	else
	{
		size_t slashpos = path.find_last_of("/");
		cur_dir = path.substr(1,slashpos);
	}
	byte* byte_cur_dir = const_cast<char *>(cur_dir.c_str());
	if ((ret = TFFS_opendir(htffs, byte_cur_dir, &hdir)) != TFFS_OK)
	{
		cout << "ERROR: TFFS_opendir" << ret << cur_dir << endl;
		return -1;
	}
	return 0;
}
int FATFile::closedir_tffs()
{
	if ((ret = TFFS_closedir(hdir)) != TFFS_OK)
	{
		cout << "ERROR: TFFS_opendir" << ret << endl;
		return -1;
	}
	return 0;
}

int FATFile::set_dirent_to_current()
{
	//iterate directory
	while( 1 )
	{
		ret = TFFS_readdir(hdir, &dirent);
		switch (ret)
		{
			case TFFS_OK:
				if(dirent.d_name == getBasename())
					return 1;
				break;

			case ERR_TFFS_LAST_DIRENTRY:
				return 0;

			default:
				return -1;
		}
	}
	return -1;
}
#endif
