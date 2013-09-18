/*
 * ToolUtils.h
 *
 *  Created on: Sep 16, 2013
 *      Author: goltzsch
 */

#ifndef TOOLUTILS_H_
#define TOOLUTILS_H_
#include "config.h"
#include "ibrcommon/data/BLOB.h"

#ifdef HAVE_LIBARCHIVE
#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>
#endif

class ToolUtils
{
public:
	ToolUtils();
	virtual ~ToolUtils();
#ifdef HAVE_LIBARCHIVE
	/**
	 * write tar archive to payload block
	 */
	static void write_tar_archive( ibrcommon::BLOB::Reference *blob, const char **filenames, size_t num_files );
	/*
	 * read tar archive from payload block
	 */
	static void read_tar_archive( const char *extract_folder, ibrcommon::BLOB::Reference *blob );
#endif
private:
#ifdef HAVE_LIBARCHIVE
	//CALLBACKS FOR LIBARCHIVE
	static int close_callback( struct archive *, void *blob_iostream );
	static ssize_t write_callback( struct archive *, void *blob_ptr, const void *buffer, size_t length );
	static int open_callback( struct archive *, void *blob_iostream );

	static ssize_t read_callback( struct archive *a, void *client_data, const void **buff );
#endif
};

#endif /* TOOLUTILS_H_ */
