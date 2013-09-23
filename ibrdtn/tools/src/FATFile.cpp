/*
 * FATFile.cpp
 *
 *  Created on: Sep 23, 2013
 *      Author: goltzsch
 */

#include "FATFile.h"

//#ifdef HAVE_LIBTFFS
#define BUF_SIZE 1024 //buffer size for vfat file reads
//#endif

FATFile::FATFile() : File(), _img_path(""), htffs(0),hdir(0),hfile(0), ret(-1)
{
	update();
}

FATFile::FATFile( string img_path) : File("/"), _img_path(img_path), htffs(0),hdir(0),hfile(0), ret(-1)
{
	update();
}

FATFile::FATFile( string img_path, string file_path ) : File(file_path), _img_path(img_path), htffs(0),hdir(0),hfile(0),ret(-1)
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

			files.push_back(FATFile(_img_path,string(dirent.d_name)));
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
	return set_dirent_to_current();
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
	tm.tm_sec  = 0; //TODO
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

//void FATFile::createDirectory( File& path )
//{
	//TODO problem: static
//}

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
		if ((ret = TFFS_readdir(hdir, &dirent)) == TFFS_OK) {
			if(dirent.d_name == getBasename())
			{
				return 1;
			}
		}
		else if (ret == ERR_TFFS_LAST_DIRENTRY) { // end of directory
			return 0;
		}
		else {
			cout << "ERROR: TFFS_readdir" << ret << endl;
			return -1;
		}
	}
	return 0;
}

