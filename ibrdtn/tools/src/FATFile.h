/*
 * FATFile.h
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
 *
 * Written-by: David Goltzsche <goltzsch@ibr.cs.tu-bs.de>
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
 *
 *  This class provides the same methods as ibrcommon::File, but for a file
 *  on a FAT image.
 *  A few methods are not implemented, because not needed;
 */

#ifndef FATFILE_H_
#define FATFILE_H_

#define HAVE_LIBTFFS 1 //TODO
#ifdef HAVE_LIBTFFS
#include "ibrcommon/data/File.h"
#include <list>
#include <dirent.h>
extern "C" //libtffs does not support c++
{
#include "tffs/tffs.h"
}

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
	int umount_tffs();
	int opendir_tffs();
	int closedir_tffs();
	int set_dirent_to_current();


};
#endif /* HAVE_LIBTFFS */
#endif /* FATFILE_H_ */
