/*
 * TarUtils.cpp
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
 *  Created on: Sep 16, 2013
 */

#include "TarUtils.h"
#include <stdlib.h>
#include <string.h>

using namespace ibrcommon;


std::string TarUtils::_img_path = "";
tffs_handle_t TarUtils::htffs = 0;
tdir_handle_t TarUtils::hdir = 0;
tfile_handle_t TarUtils::hfile = 0;
int32 TarUtils::ret = 0;
#define BUFF_SIZE 8192

#ifdef HAVE_LIBARCHIVE
TarUtils::TarUtils()
{
}
TarUtils::~TarUtils()
{
}

int TarUtils::open_callback( struct archive *, void *blob_iostream )
{
	//blob does not need to be opened, do nothing
	return ARCHIVE_OK;
}

ssize_t TarUtils::write_callback( struct archive *, void *blob_ptr, const void *buffer, size_t length )
{
	char* cast_buf = (char*) buffer;

	BLOB::Reference blob = *((BLOB::Reference*) blob_ptr);
	BLOB::iostream os = blob.iostream();

	(*os).write(cast_buf, length);

	return length;
}



ssize_t TarUtils::read_callback( struct archive *a, void *blob_ptr, const void **buffer )
{
	int len = BUFF_SIZE;
	char *cbuff = new char[len];

	BLOB::Reference *blob = (BLOB::Reference*) blob_ptr;
	BLOB::iostream is = blob->iostream();

	(*is).read(cbuff,len);

	*buffer = cbuff;
	return len;
}

int TarUtils::close_callback( struct archive *, void *blob_iostream )
{
	//blob does not need to be closed, do nothing
	return ARCHIVE_OK;
}

void TarUtils::read_tar_archive( string extract_folder, ibrcommon::BLOB::Reference *blob )
{
	struct archive *a;
	struct archive_entry *entry;


	a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_compression_all(a);
	archive_read_support_format_tar(a);

	archive_read_open(a, (void*) blob, &open_callback, &read_callback, &close_callback);

	int ret,fd;

	while ((ret = archive_read_next_header(a, &entry)) == ARCHIVE_OK )
	{
		string filename = archive_entry_pathname(entry);
		string path = extract_folder + "/" + filename;
		fd = open(path.c_str(),O_CREAT|O_WRONLY,0666);
		archive_read_data_into_fd(a,fd);
		close(fd);
	}
	archive_read_free(a);
}

void TarUtils::write_tar_archive( ibrcommon::BLOB::Reference *blob, list<ObservedFile*> files_to_send)
{

	vector<tarfile> tarfiles;
	list<ObservedFile*>::iterator of_iter;
	for(of_iter = files_to_send.begin(); of_iter != files_to_send.end(); of_iter++)
	{
		ObservedFile* of = (*of_iter);

		tarfile tf;
		tf.filename = of->getPath().c_str();

		struct archive_entry *e;
		e= archive_entry_new();
		archive_entry_set_size(e, of->size());
		if(of->isDirectory())
		{
			archive_entry_set_filetype(e, AE_IFDIR);
			archive_entry_set_perm(e, 0755);
		}
		else
		{
			archive_entry_set_filetype(e, AE_IFREG );
			archive_entry_set_perm(e, 0644);
		}


		archive_entry_set_pathname(e, rel_filename(of->getPath()).c_str());

		//set timestamps
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		archive_entry_set_atime(e, ts.tv_sec, ts.tv_nsec); //accesstime
		archive_entry_set_birthtime(e, ts.tv_sec, ts.tv_nsec); //creationtime
		archive_entry_set_ctime(e, ts.tv_sec, ts.tv_nsec); //time, inode changed
		archive_entry_set_mtime(e, ts.tv_sec, ts.tv_nsec); //modification time

		tf.entry = e;
		tarfiles.push_back(tf);
	}
	write_tar_archive(blob,tarfiles);
}

void TarUtils::set_img_path( std::string img_path )
{
	_img_path = img_path;
}

/*void TarUtils::write_tar_archive( ibrcommon::BLOB::Reference *blob, list<ObservedFile<File> *> files_to_send)
{
	vector<tarfile> tarfiles;
	list<ObservedFile<File> *>::iterator of_ptr_iter;
	for(of_ptr_iter = files_to_send.begin(); of_ptr_iter != files_to_send.end(); of_ptr_iter++)
	{
		ObservedFile<File> of = (**of_ptr_iter);

		tarfile tf;
		tf.filename = of.getPath().c_str();

		struct archive_entry *e;
		e= archive_entry_new();

		//get stat and copy to archive
		struct stat st;
		stat(of.getPath().c_str(),&st);
		archive_entry_copy_stat(e,&st);

		archive_entry_set_pathname(e, rel_filename(of.getPath()).c_str());

		tf.entry = e;
		tarfiles.push_back(tf);
	}
	write_tar_archive(blob,tarfiles);

}
*/

void TarUtils::write_tar_archive(	ibrcommon::BLOB::Reference *blob, vector<tarfile> tarfiles)
{
	char buff[BUFF_SIZE];
	size_t len;
	int fd;
	//create new archive, set format to tar, use callbacks (above this method)
	struct archive *a;
	a = archive_write_new();
	archive_write_set_format_ustar(a);
	archive_write_open(a, blob, &open_callback, &write_callback, &close_callback);

	//iterate tarfiles
	vector<tarfile>::iterator iter;
	for(iter = tarfiles.begin(); iter < tarfiles.end(); iter++)
	{
		tarfile tf = (*iter);

		archive_write_header(a, tf.entry);

		//write normal file
		if(_img_path == "")
		{
			fd = open(tf.filename, O_RDONLY);
			len = read(fd, buff, BUFF_SIZE);
		}
		//write file on vfat-image
		else
		{
			//mount tffs
			byte* path = const_cast<char *>(_img_path.c_str());
			if ((ret = TFFS_mount(path, &htffs)) != TFFS_OK)
			{
				cout << "ERROR: TFFS_mount" << ret << endl;
				return;
			}

			//open file
			byte* file = const_cast<char *>(tf.filename);
			if ((ret = TFFS_fopen(htffs, file, "r", &hfile)) != TFFS_OK)
			{
				cout << "ERROR: TFFS_fopen" << ret << endl;
				return;
			}

			memset(buff, 0, BUFF_SIZE);
			//read file
			if (( ret = TFFS_fread(hfile,BUFF_SIZE,(unsigned char*) buff)) < 0)
			{
					if( ret == ERR_TFFS_FILE_EOF)
						len = 0;

					else
					{
						cout << "ERROR: TFFS_fread" << ret << endl;
						return;
					}

			}
			else
			{
					len = ret;
			}
		}

		//write buffer to archive
		while (len > 0)
		{
			archive_write_data(a, buff, len);
			if(_img_path == "")
				len = read(fd, buff, sizeof(buff));
			else
			{
				if (( ret  = TFFS_fread(hfile,sizeof(buff),(unsigned char*) buff)) < 0)
				{
					if( ret == ERR_TFFS_FILE_EOF)
					{
						len = 0;
						continue;
					}
					cout << "ERROR: TFFS_fread" << ret << endl;
					return;
				}
				len = ret;
			}

		}

		//close file
		if(_img_path == "")
			close(fd);
		else
		{
			if ((ret = TFFS_fclose(hfile)) != TFFS_OK) {
				cout << "ERROR: TFFS_fclose" << ret;
				return;
			}
		}
		archive_entry_free(tf.entry);
	}
	archive_write_close(a);
	archive_write_free(a);

}

std::string TarUtils::rel_filename(std::string n)
{
	unsigned slash_pos = n.find_last_of('/', n.length());
	return n.substr(slash_pos + 1, n.length() - slash_pos);
}

#endif
