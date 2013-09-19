/*
 * tffs.h
 *
 * Interface definitions of Tiny Fat File System.
 * head file.
 *
 * knightray@gmail.com
 * 10/28 2008
 */

#ifndef _TFFS_H
#define _TFFS_H

#include "comdef.h"

/*
 * Structure definitions
 */
typedef struct {}* tffs_handle_t;
typedef struct {}* tdir_handle_t;
typedef struct {}* tfile_handle_t;

#define DIR_ATTR_READ_ONLY  0x01
#define DIR_ATTR_HIDDEN     0x02
#define DIR_ATTR_SYSTEM     0x04
#define DIR_ATTR_VOLUME_ID  0x08
#define DIR_ATTR_DIRECTORY  0x10
#define DIR_ATTR_ARCHIVE    0x20

#define DNAME_MAX			64
#define DNAME_SHORT_MAX		13

typedef struct _tffs_time {
	int32 year;
	int32 month;
	int32 day;
	int32 hour;
	int32 min;
	int32 sec;
}tffs_time_t;

typedef struct _dirent {
	byte		d_name[DNAME_MAX];
	byte		d_name_short[DNAME_SHORT_MAX];
	ubyte		dir_attr;
	uint32		dir_file_size;
	tffs_time_t crttime;
}dirent_t;

/*
 * Error code definitions
 */
#define TFFS_OK						(1)
#define ERR_TFFS_INVALID_PARAM 		(-1)
#define ERR_TFFS_DEVICE_FAIL 		(-2)
#define ERR_TFFS_BAD_BOOTSECTOR		(-3)
#define ERR_TFFS_BAD_FAT			(-4)
#define ERR_TFFS_INVALID_PATH		(-5)
#define ERR_TFFS_LAST_DIRENTRY		(-6)
#define ERR_TFFS_INVALID_OPENMODE	(-7)
#define ERR_TFFS_FILE_NOT_EXIST		(-8)
#define ERR_TFFS_FILE_OPEN_FAIL		(-9)
#define ERR_TFFS_NO_FREE_SPACE		(-10)
#define ERR_TFFS_READONLY			(-11)
#define ERR_TFFS_FILE_EOF			(-12)
#define ERR_TFFS_FAT				(-13)
#define ERR_TFFS_DIR_ALREADY_EXIST	(-14)
#define ERR_TFFS_INITIAL_DIR_FAIL	(-15)
#define ERR_TFFS_NO_SUCH_FILE		(-16)
#define ERR_TFFS_IS_NOT_A_FILE		(-17)
#define ERR_TFFS_REMOVE_FILE_FAIL	(-18)
#define ERR_TFFS_IS_NOT_A_DIRECTORY	(-19)
#define ERR_TFFS_NOT_EMPTY_DIR		(-20)
#define ERR_TFFS_REMOVE_DIR_FAIL	(-21)


/*
 * Interface
 */
int32
TFFS_mount(
	IN	byte * dev,
	OUT	tffs_handle_t * phtffs
);

int32
TFFS_opendir(
	IN	tffs_handle_t hfs,
	IN	byte * path,
	OUT	tdir_handle_t * phdir
);

int32
TFFS_readdir(
	IN	tdir_handle_t hdir,
	OUT	dirent_t * pdirent
);

int32
TFFS_closedir(
	IN	tdir_handle_t hdir
);

int32 
TFFS_fopen(
	IN	tffs_handle_t hfs,
	IN	byte * file_path,
	IN	byte * open_mode,
	OUT	tfile_handle_t * phfile
);

int32
TFFS_fclose(
	IN	tfile_handle_t hfile
);

int32
TFFS_fread(
	IN	tfile_handle_t hfile,
	IN	uint32 buflen,
	OUT	ubyte * ptr
);

int32
TFFS_fwrite(
	IN	tfile_handle_t hfile,
	IN	uint32 buflen,
	IN	ubyte * ptr
);

int32
TFFS_mkdir(
	IN	tffs_handle_t hfs,
	IN	byte * dir_path
);

int32
TFFS_chdir(
	IN	tffs_handle_t hfs,
	IN	byte * dir_path
);

int32
TFFS_rmdir(
	IN	tffs_handle_t hfs,
	IN	byte * dir_path
);

int32
TFFS_rmfile(
	IN	tffs_handle_t hfs,
	IN	byte * file_path
);

int32
TFFS_umount(
	IN	tffs_handle_t  htffs
);

#endif

