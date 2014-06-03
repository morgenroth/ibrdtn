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
#include "io/ObservedFile.h"
#include <ibrcommon/Logger.h>

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sstream>
#include <typeinfo>
#include <fstream>

#include <archive.h>
#include <archive_entry.h>

#ifdef HAVE_LIBTFFS
#include "io/FatImageReader.h"
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

	char readbuf[BUFF_SIZE];

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
		std::istream &is = *(std::istream*)istream_ptr;
		is.read((char*)&readbuf, BUFF_SIZE);

		*buffer = &readbuf;
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

	void TarUtils::write(std::ostream &output, const io::ObservedFile &root, const std::set<ObservedFile> &files_to_send)
	{
		bool processed = false;

		//create new archive, set format to tar, use callbacks (above this method)
		struct archive *a;
		a = archive_write_new();
		archive_write_set_format_ustar(a);
		archive_write_open(a, &output, &__tar_utils_open_callback, &__tar_utils_write_callback, &__tar_utils_close_callback);

		for(std::set<ObservedFile>::const_iterator of_iter = files_to_send.begin(); of_iter != files_to_send.end(); ++of_iter)
		{
			const ObservedFile &of = (*of_iter);
			const ibrcommon::File &file = of.getFile();

			struct archive_entry *entry;
			entry = archive_entry_new();
			archive_entry_set_size(entry, file.size());

			if(file.isDirectory())
			{
				archive_entry_set_filetype(entry, AE_IFDIR);
				archive_entry_set_perm(entry, 0755);
			}
			else
			{
				archive_entry_set_filetype(entry, AE_IFREG);
				archive_entry_set_perm(entry, 0644);
			}

			archive_entry_set_pathname(entry, rel_filename(root, of).c_str());

			//set timestamps
			struct timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			archive_entry_set_atime(entry, ts.tv_sec, ts.tv_nsec); //accesstime
			archive_entry_set_birthtime(entry, ts.tv_sec, ts.tv_nsec); //creationtime
			archive_entry_set_ctime(entry, ts.tv_sec, ts.tv_nsec); //time, inode changed
			archive_entry_set_mtime(entry, ts.tv_sec, ts.tv_nsec); //modification time

			archive_write_header(a, entry);

			try {
#ifdef HAVE_LIBTFFS
				//read file on vfat-image
				try {
					const FATFile &ffile = dynamic_cast<const FATFile&>(file);
					processed = true;

					// get image reader
					const FatImageReader &reader = ffile.getReader();

					// open fat file
					io::FatImageReader::FileHandle fh = reader.open(ffile);

					char buff[BUFF_SIZE];
					ssize_t ret = 0;
					size_t len = 0;

					// read file
					len = fh.read((unsigned char*)&buff, BUFF_SIZE);

					//write buffer to archive
					while (len > 0)
					{
						if( (ret = archive_write_data(a, buff, len)) < 0)
						{
							IBRCOMMON_LOGGER_TAG("TarUtils", error) << "archive_write_data failed" << IBRCOMMON_LOGGER_ENDL;
							break;
						}

						// read next chunk
						len = fh.read((unsigned char*)&buff, BUFF_SIZE);
					}
				} catch (const std::bad_cast&) { };
#endif

				if (!processed)
				{
					char buff[BUFF_SIZE];
					ssize_t ret = 0;

					// open file for reading
					std::ifstream fs(file.getPath().c_str());

					// write buffer to archive
					while (fs.good())
					{
						// read bytes
						fs.read(buff, BUFF_SIZE);

						// write bytes to archive
						if( (ret = archive_write_data(a, buff, fs.gcount())) < 0)
						{
							IBRCOMMON_LOGGER_TAG("TarUtils", error) << "archive write failed" << IBRCOMMON_LOGGER_ENDL;
							break;
						}
					}
				}
			} catch (const ibrcommon::IOException &e) {
				// write failed
				IBRCOMMON_LOGGER_TAG("TarUtils", error) << "archive write failed: " << e.what() << IBRCOMMON_LOGGER_ENDL;

				archive_entry_free(entry);
				archive_write_close(a);
				archive_write_free(a);

				throw;
			}

			archive_entry_free(entry);
		}
		archive_write_close(a);
		archive_write_free(a);
	}

	std::string TarUtils::rel_filename(const ObservedFile &parent, const ObservedFile &f)
	{
		// get file path
		const std::string file_path = f.getFile().getPath();

		// get wrapping path
		const std::string parent_path = parent.getFile().getPath();

		// special case: if parent is root return the full path
		if (parent.getFile().isRoot()) return file_path.substr(parent_path.length(), file_path.length() - parent_path.length());

		// if path is sub-path
		return file_path.substr(parent_path.length() + 1, file_path.length() - parent_path.length() - 1);
	}
}
