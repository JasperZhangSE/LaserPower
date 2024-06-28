/*
	Define.h

	Common Macro Defines Head File For The Whole Project
*/

/* Copyright 2019 Shanghai BiNY Inc. */

/*
	modification history
	--------------------
	01a, 15Jul19, Karl Created
*/

#ifndef __DEFINE_H__
#define __DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Defines */
#ifndef TRUE
#define TRUE		        (1)
#endif
#ifndef FALSE
#define FALSE		        (0)
#endif
#ifndef NULL
#define NULL			    (0)
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET		(-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR		(-1)
#endif

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef INOUT
#define INOUT
#endif

#ifndef SECTION
#define SECTION(x)  __attribute__((section(x)))
#endif
#ifndef USED
#define USED        __attribute__((used))
#endif
#ifndef ALIGN
#define ALIGN(n)    __attribute__((aligned(n)))
#endif
#ifndef WEAK
#define WEAK        __attribute__((weak))
#endif
#ifndef INLINE
#define INLINE      static __inline
#endif
#ifndef API
#define API
#endif

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /*__DEFINE_H__*/
