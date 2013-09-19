/*
 * comdef.h
 *
 * Common definiation of TFFS.
 * head file.
 *
 * knightray@gmail.com
 * 10/27 2008
 */

#ifndef _COM_DEF_H
#define _COM_DEF_H

/*
 * type redefiniation
 */
typedef char	byte;
typedef short	int16;
typedef int		int32;

typedef unsigned char	ubyte;
typedef unsigned short	uint16;
typedef unsigned int	uint32;

#define IN
#define OUT
#define INOUT

#ifndef NULL
#define NULL	(0)
#endif

#ifndef TRUE
#define TRUE	(1)
#endif

#ifndef FALSE
#define FALSE	(0)
#endif

#ifndef BOOL
#define BOOL int32
#endif

#endif
