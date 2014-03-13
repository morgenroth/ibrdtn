/*
 * TarUtils.cpp
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
 *  Created on: Sep 16, 2013
 */

#include "io/TarUtils.h"
#include "io/ObservedNormalFile.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sstream>
#include <typeinfo>

#include <archive.h>
#include <archive_entry.h>

#ifdef HAVE_LIBTFFS
extern "C"
{
#include <tffs.h>
}
#include "io/ObservedFATFile.h"
#endif

#define BUFF_SIZE 8192

namespace io
{
	TarUtils::TarUtils()
	{
	}
	TarUtils::~TarUtils()
	{
	}

	int __tar_utils_open_callback( struct archive *, void * )
	{
		//blob does not need to be opened, do nothing
		return ARCHIVE_OK;
	}

	ssize_t __tar_utils_write_callback( struct archive *, void *ostream_ptr, const void *buffer, size_t length )
	{
		ssize_t ret = 0;
		char* cast_buf = (char*) buffer;

		std::ostream &os = *(std::ostream*)ostream_ptr;
		os.write(cast_buf, length);
		ret += length;

		return ret;
	}

	ssize_t __tar_utils_read_callback( struct archive *, void *istream_ptr, const void **buffer )
	{
		char *cbuff = new char[BUFF_SIZE];

		std::istream &is = *(std::istream*)istream_ptr;
		is.read(cbuff,BUFF_SIZE);

		*buffer = cbuff;
		return BUFF_SIZE;
	}

	int __tar_utils_close_callback( struct archive *, void * )
	{
		//blob does not need to be closed, do nothing
		return ARCHIVE_OK;
	}

	void TarUtils::read( const ibrcommon::File &extract_folder, std::istream &input )
	{
		struct archive *a;
		struct archive_entry *entry;
		int ret,fd;

		a = archive_read_new();
		archive_read_support_filter_all(a);
		archive_read_support_compression_all(a);
		archive_read_support_format_tar(a);

		archive_read_open(a, (void*) &input, &__tar_utils_open_callback, &__tar_utils_read_callback, &__tar_utils_close_callback);


		while ((ret = archive_read_next_header(a, &entry)) == ARCHIVE_OK )
		{
			ibrcommon::File filename = extract_folder.get(archive_entry_pathname(entry));
			ibrcommon::File path = filename.getParent();
			ibrcommon::File::createDirectory(path);
			fd = open(filename.getPath().c_str(),O_CREAT|O_WRONLY,0600);
			if(fd < 0) throw ibrcommon::IOException("cannot open file " + path.getPath());
			archive_read_data_into_fd(a,fd);
			close(fd);
		}

		archive_read_free(a);
	}

	void TarUtils::write(std::ostream &output, const ibrcommon::File &parent, const std::list<ObservedFile*> &files_to_send)
	{
		//create new archive, set format to tar, use callbacks (above this method)
		struct archive *a;
		a = archive_write_new();
		archive_write_set_format_ustar(a);
		archive_write_open(a, &output, &__tar_utils_open_callback, &__tar_utils_write_callback, &__tar_utils_close_callback);

		for(std::list<ObservedFile*>::const_iterator of_iter = files_to_send.begin(); of_iter != files_to_send.end(); ++of_iter)
		{
			ObservedFile &of = (**of_iter);

			struct archive_entry *entry;
			entry = archive_entry_new();
			archive_entry_set_size(entry, of.size());
			if(of.isDirectory())
			{
				archive_entry_set_filetype(entry, AE_IFDIR);
				archive_entry_set_perm(entry, 0755);
			}
			else
			{
				archive_entry_set_filetype(entry, AE_IFREG );
				archive_entry_set_perm(entry, 0644);
			}

			archive_entry_set_pathname(entry, rel_filename(parent, of).c_str());

			//set timestamps
			struct timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			archive_entry_set_atime(entry, ts.tv_sec, ts.tv_nsec); //accesstime
			archive_entry_set_birthtime(entry, ts.tv_sec, ts.tv_nsec); //creationtime
			archive_entry_set_ctime(entry, ts.tv_sec, ts.tv_nsec); //time, inode changed
			archive_entry_set_mtime(entry, ts.tv_sec, ts.tv_nsec); //modification time

			archive_write_header(a, entry);

#ifdef HAVE_LIBTFFS
			//read file on vfat-image
			try {
				ObservedFATFile &ffile = dynamic_cast<ObservedFATFile&>(of);

				char buff[BUFF_SIZE];
				ssize_t ret = 0;
				size_t len = 0;

				tffs_handle_t htffs = 0;
				tfile_handle_t hfile = 0;

				//mount tffs
				byte* path = const_cast<char *>(ffile.getImageFile().getPath().c_str());
				if ((ret = TFFS_mount(path, &htffs)) != TFFS_OK)
				{
					std::stringstream ss;
					ss << "TFFS_mount " << ret;
					throw ibrcommon::IOException(ss.str());
				}

				//open file
				byte* file = const_cast<char*>(ffile.getPath().c_str());
				if ((ret = TFFS_fopen(htffs, file, "r", &hfile)) != TFFS_OK)
				{
					std::stringstream ss;
					ss << "TFFS_fopen" << ret << ", " << ffile.getPath();
					throw ibrcommon::IOException(ss.str());
				}

				memset(buff, 0, BUFF_SIZE);

				// read file
				if (( ret = TFFS_fread(hfile,BUFF_SIZE,(unsigned char*) buff)) < 0)
				{
						if( ret == ERR_TFFS_FILE_EOF)
						{
							len = 0;
						}
						else
						{
							std::stringstream ss;
							ss << "TFFS_fread" << ret << ", " << ffile.getPath();
							throw ibrcommon::IOException(ss.str());
						}
				}
				else
				{
					len = (ret >= 0) ? (size_t)ret : 0;
				}

				//write buffer to archive
				while (len > 0)
				{
					if( (ret = archive_write_data(a, buff, len)) < 0)
					{
						cout << "ERROR: archive_write_data " << ret << endl;
						return;
					}

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
					len = (ret >= 0) ? (size_t)ret : 0;
				}

				if ((ret = TFFS_fclose(hfile)) != TFFS_OK) {
					cout << "ERROR: TFFS_fclose" << ret;
					return;
				}
			} catch (const std::bad_cast&) { }
#endif

			try {
				ObservedNormalFile &ffile = dynamic_cast<ObservedNormalFile&>(of);

				char buff[BUFF_SIZE];
				ssize_t ret = 0;
				size_t len = 0;

				int fd = ::open(ffile.getPath().c_str(), O_RDONLY);
				ret = ::read(fd, buff, BUFF_SIZE);
				len = (ret >= 0) ? (size_t)ret : 0;

				//write buffer to archive
				while (len > 0)
				{
					if( (ret = archive_write_data(a, buff, len)) < 0)
					{
						std::stringstream ss;
						ss << "archive write error " << ret;
						throw ibrcommon::IOException(ss.str());
					}

					ret = ::read(fd, buff, sizeof(buff));
					len = (ret >= 0) ? (size_t)ret : 0;
				}

				// close the file
				close(fd);
			} catch (const std::bad_cast&) { }

			archive_entry_free(entry);
		}
		archive_write_close(a);
		archive_write_free(a);
	}

	std::string TarUtils::rel_filename(const ibrcommon::File &parent, ObservedFile &f)
	{
		const std::string parent_path = parent.getPath();
		const std::string file_path = f.getPath();
		return file_path.substr(parent_path.length()+1,file_path.length()-parent_path.length()-1);
	}
}
