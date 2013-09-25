/*
 * TarUtils.h
 *
 *  Created on: Sep 16, 2013
 *      Author: goltzsch
 */

#ifndef TARUTILS_H_
#define TARUTILS_H_
#include "config.h"
#include "ibrcommon/data/BLOB.h"

#include "FATFile.h"
#include "ObservedFile.h"

#ifdef HAVE_LIBARCHIVE
#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>
extern "C"
{
#include "tffs/tffs.h"
}
#endif
struct tarfile
{
	const char *filename;
	archive_entry *entry;
};
class TarUtils
{
public:
	TarUtils();
	virtual ~TarUtils();
	/**
	 * write tar archive to payload block, FATFile version
	 */
	static void write_tar_archive( ibrcommon::BLOB::Reference* blob, list<ObservedFile<FATFile> *> files_to_send );

	/**
	 * write tar archive to payload block, FATFile version
	 */
	static void write_tar_archive( ibrcommon::BLOB::Reference *blob, list<ObservedFile<ibrcommon::File> *> files_to_send );

	/*
	 * read tar archive from payload block
	 */
	static void read_tar_archive( const char *extract_folder, ibrcommon::BLOB::Reference *blob );

	static void set_img_path(std::string img_path);
private:
	 //* write tar archive to payload block, interfal version
	static void write_tar_archive(	ibrcommon::BLOB::Reference *blob, vector<tarfile> tarfiles);

	static string rel_filename(string);

	//CALLBACKS FOR LIBARCHIVE
	static int close_callback( struct archive *, void *blob_iostream );
	static ssize_t write_callback( struct archive *, void *blob_ptr, const void *buffer, size_t length );
	static int open_callback( struct archive *, void *blob_iostream );

	static ssize_t read_callback( struct archive *a, void *client_data, const void **buff );

	static std::string _img_path;

	//handles
	static tffs_handle_t htffs;
	static tdir_handle_t hdir;
	static tfile_handle_t hfile;
	static int32 ret;

};

#endif /* TARUTILS_H_ */
