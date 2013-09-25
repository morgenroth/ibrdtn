/*
 * TarUtils.cpp
 *
 *  Created on: Sep 16, 2013
 *      Author: goltzsch
 */

#include "TarUtils.h"
#include <stdlib.h>
#include <string.h>
using namespace ibrcommon;


std::string TarUtils::_img_path = "";
tffs_handle_t TarUtils::htffs = 0;
tdir_handle_t TarUtils::hdir = 0;
tfile_handle_t TarUtils::hfile = 0;
int32 TarUtils::ret= 0;
#define BUFF_SIZE 8192

#ifdef HAVE_LIBARCHIVE
TarUtils::TarUtils()
{
}
TarUtils::~TarUtils()
{
}
struct path_and_blob
{
	const char *path;
	BLOB::Reference *blob;
};
int TarUtils::open_callback( struct archive *, void *blob_iostream )
{
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



ssize_t TarUtils::read_callback( struct archive *a, void *client_data, const void **buffer )
{
	struct path_and_blob *data = (path_and_blob*) client_data;

	BLOB::iostream is = data->blob->iostream();


	int len = 750;
	char mbuf[len];
	(*is).read(mbuf,len);
	buffer = (const void**) &mbuf;

	int i = 0;
	while(i<len)
	{
		cout << mbuf[i++];
	}
	cout << endl;

	return len;
}

int TarUtils::close_callback( struct archive *, void *blob_iostream )
{
	return ARCHIVE_OK;
}

void TarUtils::read_tar_archive( const char *extract_folder, ibrcommon::BLOB::Reference *blob )
{
	void *void_blob = (void*) blob;
	struct archive *a;
	struct archive_entry *entry;

	path_and_blob *data = new path_and_blob;
	data->path = extract_folder;
	data->blob = blob;

	a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_compression_all(a); //TODO gebraucht?
	archive_read_support_format_tar(a);

	archive_read_open(a, data, &open_callback, &read_callback, &close_callback);

	while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
	{
		printf("%s\en", archive_entry_pathname(entry));
		archive_read_data_skip(a);
	}
	archive_read_free(a);
}

void TarUtils::write_tar_archive( ibrcommon::BLOB::Reference *blob, const char **filenames, size_t num_files )
{
	struct archive *a;
	struct archive_entry *entry;
	struct stat st;
	char buff[BUFF_SIZE];
	int len;
	int fd;

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	//create new archive, set format to tar, use callbacks (above this method)
	a = archive_write_new();
	archive_write_set_format_ustar(a);
	archive_write_open(a, blob, &open_callback, &write_callback, &close_callback);

	while (num_files != 0)
	{

		if (_img_path == "")
		{
			stat(*filenames, &st);
			entry = archive_entry_new();
			archive_entry_set_size(entry, st.st_size);
			archive_entry_set_filetype(entry, AE_IFREG );
			archive_entry_set_perm(entry, 0644);
		}
		else
		{
			entry = archive_entry_new();
			archive_entry_set_size(entry, 1337); //TODO echte size setzen
			archive_entry_set_filetype(entry, AE_IFREG );
			archive_entry_set_perm(entry, 0644);
		}

		//set filename in archive to relative paths
		std::string path(*filenames);
		unsigned slash_pos = path.find_last_of('/', path.length());
		std::string rel_name = path.substr(slash_pos + 1, path.length() - slash_pos);
		archive_entry_set_pathname(entry, rel_name.c_str());

		//set timestamps
		archive_entry_set_atime(entry, ts.tv_sec, ts.tv_nsec); //accesstime
		archive_entry_set_birthtime(entry, ts.tv_sec, ts.tv_nsec); //creationtime
		archive_entry_set_ctime(entry, ts.tv_sec, ts.tv_nsec); //time, inode changed
		archive_entry_set_mtime(entry, ts.tv_sec, ts.tv_nsec); //modification time

		archive_write_header(a, entry);

		if(_img_path == "")
		{
			fd = open(*filenames, O_RDONLY);
			len = read(fd, buff, BUFF_SIZE);
		}
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
			char* file = const_cast<char *>(*filenames);
			cout << file << endl;
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
				{
					len = 0;
					break;
				}
				cout << "ERROR: TFFS_fread" << ret << endl;
				return;
			}
			len = ret;
		}
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
						break;
					}
					cout << "ERROR: TFFS_fread" << ret << endl;
					return;
				}
				len = ret;
			}

		}
		if(_img_path == "")
			close(fd);
		else
		{
			if ((ret = TFFS_fclose(hfile)) != TFFS_OK) {
				cout << "ERROR: TFFS_fclose" << ret;
				return;
			}
		}
		archive_entry_free(entry);
		filenames++;
		num_files--;
	}
	archive_write_close(a);
	archive_write_free(a);
}

void TarUtils::set_img_path( std::string img_path )
{
	_img_path = img_path;
}

#endif
