/*
 * TarUtils.cpp
 *
 *  Created on: Sep 16, 2013
 *      Author: goltzsch
 */

#include "TarUtils.h"
#include <stdlib.h>
#include <string.h>

#include "FATFile.cpp" //TODO doof!
#include "ObservedFile.cpp" //TODO doof!
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

void TarUtils::write_tar_archive( ibrcommon::BLOB::Reference *blob, list<ObservedFile<FATFile> *> files_to_send)
{

	vector<tarfile> tarfiles;
	list<ObservedFile<FATFile> *>::iterator of_ptr_iter;
	for(of_ptr_iter = files_to_send.begin(); of_ptr_iter != files_to_send.end(); of_ptr_iter++)
	{
		ObservedFile<FATFile> of = (**of_ptr_iter);

		tarfile tf;
		tf.filename = of.getPath().c_str();

		struct archive_entry *e;
		e= archive_entry_new();
		archive_entry_set_size(e, of.size());
		if(of.isDirectory())
		{
			archive_entry_set_filetype(e, AE_IFDIR);
			archive_entry_set_perm(e, 0755);
		}
		else
		{
			archive_entry_set_filetype(e, AE_IFREG );
			archive_entry_set_perm(e, 0644);
		}


		archive_entry_set_pathname(e, rel_filename(of.getPath()).c_str());

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

void TarUtils::write_tar_archive( ibrcommon::BLOB::Reference *blob, list<ObservedFile<File> *> files_to_send)
{
	vector<tarfile> tarfiles;
	list<ObservedFile<File> *>::iterator of_ptr_iter;
	struct stat st;
	for(of_ptr_iter = files_to_send.begin(); of_ptr_iter != files_to_send.end(); of_ptr_iter++)
	{
		ObservedFile<File> of = (**of_ptr_iter);

		tarfile tf;
		tf.filename = of.getPath().c_str();

		struct archive_entry *e;
		e= archive_entry_new();

		stat(of.getPath().c_str(),&st);
		archive_entry_copy_stat(e,&st);

		archive_entry_set_pathname(e, rel_filename(of.getPath()).c_str());

		//set timestamps
		/*struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		archive_entry_set_atime(e, ts.tv_sec, ts.tv_nsec); //accesstime
		archive_entry_set_birthtime(e, ts.tv_sec, ts.tv_nsec); //creationtime
		archive_entry_set_ctime(e, ts.tv_sec, ts.tv_nsec); //time, inode changed
		archive_entry_set_mtime(e, ts.tv_sec, ts.tv_nsec); //modification time*/

		tf.entry = e;
		tarfiles.push_back(tf);
	}
	write_tar_archive(blob,tarfiles);

}


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
	for(iter = tarfiles.begin(); iter <= tarfiles.end(); iter++)
	{
		tarfile tf = (*iter);

		archive_write_header(a, tf.entry);

		if(_img_path == "")
		{
			fd = open(tf.filename, O_RDONLY);
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
					{
						len = 0;
						break;
					}
					cout << "ERROR: TFFS_fread" << ret << endl;
					return;
			}
			len = ret;
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
						break;
					}
					cout << "ERROR: TFFS_fread" << ret << endl;
					return;
				}
				len = ret;
			}

		}

		//close files
		if(_img_path == "")
			close(fd);
		else
		{
			if ((ret = TFFS_fclose(hfile)) != TFFS_OK) {
				cout << "ERROR: TFFS_fclose" << ret;
				return;
			}
		}
		archive_entry_free(tf.entry); //TODO nötig?
	}
	archive_write_close(a);
	archive_write_free(a);

}


void TarUtils::set_img_path( std::string img_path )
{
	_img_path = img_path;
}

std::string TarUtils::rel_filename(std::string n)
{
	unsigned slash_pos = n.find_last_of('/', n.length());
	return n.substr(slash_pos + 1, n.length() - slash_pos);
}

#endif
