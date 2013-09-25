/*
 * FATFile.h
 *
 *  Created on: Sep 23, 2013
 *      Author: goltzsch
 */

#ifndef FATFILE_H_
#define FATFILE_H_

#include "ibrcommon/data/File.h"
#include <list>
#include <dirent.h>
//#ifdef HAVE_LIBTFFS
extern "C" //libtffs does not support c++
{
#include "tffs/tffs.h"
}
//#endif

using namespace std;

class FATFile: public ibrcommon::File
{
public:
	FATFile();
	FATFile(const string img_path);
	FATFile(const string img_path,const string file_path);
	virtual ~FATFile();

	int getFiles(list<FATFile> &files);
	int remove(bool recursive);
	FATFile get(string filename);
	FATFile getParent();
	bool exists();
	void update();
	size_t size();
	time_t lastaccess();
	time_t lastmodify();
	time_t laststatchange();

	static void setImgPath(string img_path);


private:
	static string _img_path;

	//handles
	tffs_handle_t htffs;
	tdir_handle_t hdir;
	tfile_handle_t hfile;

	int32 ret;
	dirent_t dirent;

	//tffs methods
	int mount_tffs();
	int opendir_tffs();
	int set_dirent_to_current();


};

#endif /* FATFILE_H_ */
