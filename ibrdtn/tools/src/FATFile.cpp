/*
 * FATFile.cpp
 *
 *  Created on: Sep 23, 2013
 *      Author: goltzsch
 */

#include "FATFile.h"

string FATFile::_img_path = "";

FATFile::FATFile() : File(), htffs(0),hdir(0),hfile(0), ret(-1)
{
	update();
}

FATFile::FATFile( string file_path) : File(file_path), htffs(0),hdir(0),hfile(0), ret(-1)
{
	update();
}

FATFile::~FATFile()
{
}

int FATFile::getFiles( list<FATFile> &files)
{
	//if(!isDirectory())
		//return -1;

	mount_tffs();
	open_tffs();

	//iterate directory
	while( 1 )
	{
		if ((ret = TFFS_readdir(hdir, &dirent)) == TFFS_OK) {

			files.push_back(FATFile(string(dirent.d_name)));
		}
		else if (ret == ERR_TFFS_LAST_DIRENTRY) { // end of directory
			break;
		}
		else {
			cout << "ERROR: TFFS_readdir" << ret << endl;
			return -1;
		}
	}
	return 0;



}

int FATFile::remove( bool recursive )
{
	//TODO not implemented yet, because not needed
}

FATFile FATFile::get( string filename )
{
	//TODO not implemented yet, because not needed
}

FATFile FATFile::getParent()
{
	//TODO not implemented yet, because not needed
}

bool FATFile::exists()
{
	mount_tffs();
	open_tffs();
	return (set_dirent_to_current() == 1);
}

void FATFile::update()
{
	mount_tffs();
	open_tffs();
	set_dirent_to_current();
	ubyte attr = dirent.dir_attr;
	if( attr & DIR_ATTR_DIRECTORY)
		_type = DT_DIR;
}

size_t FATFile::size()
{
	mount_tffs();
	open_tffs();
	set_dirent_to_current();
	return dirent.dir_file_size;
}

time_t FATFile::lastaccess()
{
	mount_tffs();
	open_tffs();
	set_dirent_to_current();

	struct tm tm;
	tm.tm_year = dirent.crttime.year - 1900;
	tm.tm_mon  = dirent.crttime.month - 1;
	tm.tm_mday = dirent.crttime.day;
	tm.tm_hour = dirent.crttime.hour;
	tm.tm_min  = dirent.crttime.min;
	tm.tm_sec  = dirent.crttime.sec / 2; //somehow, libtffs writes seconds from 0-119;
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

void FATFile::setImgPath(string img_path)
{
	_img_path = img_path;
}

int FATFile::mount_tffs()
{
	byte* path = const_cast<char *>(_img_path.c_str());
	if ((ret = TFFS_mount(path, &htffs)) != TFFS_OK) {
		cout << "ERROR: TFFS_mount" << ret << endl;
		return -1;
	}
	return 0;
}

int FATFile::open_tffs()
{
	if ((ret = TFFS_opendir(htffs, "/", &hdir)) != TFFS_OK) {
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

