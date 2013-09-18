/*
 * ToolUtils.cpp
 *
 *  Created on: Sep 16, 2013
 *      Author: goltzsch
 */

#include "ToolUtils.h"

using namespace ibrcommon;

#ifdef HAVE_LIBARCHIVE
ToolUtils::ToolUtils()
{
}
ToolUtils::~ToolUtils()
{
}
struct path_and_blob
{
	const char *path;
	BLOB::Reference *blob;
};
int ToolUtils::open_callback( struct archive *, void *blob_iostream )
{
	return ARCHIVE_OK;
}

ssize_t ToolUtils::write_callback( struct archive *, void *blob_ptr, const void *buffer, size_t length )
{
	char* cast_buf = (char*) buffer;
	BLOB::Reference blob = *((BLOB::Reference*) blob_ptr);
	BLOB::iostream os = blob.iostream();
	(*os).write(cast_buf, length);
	return length;
}

ssize_t ToolUtils::read_callback( struct archive *a, void *client_data, const void **buffer )
{
	cout << "read_callback" << endl;
	struct path_and_blob *data = (path_and_blob*) client_data;

	cout << data->path << endl;
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

int ToolUtils::close_callback( struct archive *, void *blob_iostream )
{
	return ARCHIVE_OK;
}

void ToolUtils::read_tar_archive( const char *extract_folder, ibrcommon::BLOB::Reference *blob )
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
void ToolUtils::write_tar_archive( ibrcommon::BLOB::Reference *blob, const char **filenames, size_t num_files )
{
	struct archive *a;
	struct archive_entry *entry;
	struct stat st;
	char buff[8192];
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

		stat(*filenames, &st);
		entry = archive_entry_new();
		archive_entry_set_size(entry, st.st_size);
		archive_entry_set_filetype(entry, AE_IFREG );
		archive_entry_set_perm(entry, 0644);

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
		fd = open(*filenames, O_RDONLY);
		len = read(fd, buff, sizeof(buff));
		while (len > 0)
		{
			archive_write_data(a, buff, len);
			len = read(fd, buff, sizeof(buff));
		}
		close(fd);
		archive_entry_free(entry);
		filenames++;
		num_files--;
	}
	archive_write_close(a);
	archive_write_free(a);
}

#endif
