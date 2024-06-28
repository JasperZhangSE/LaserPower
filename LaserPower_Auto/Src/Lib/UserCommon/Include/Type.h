/*
	Type.h

	Common Type Defines Head File For The Whole Project
*/

/* Copyright 2019 Shanghai BiNY Inc. */

/*
	modification history
	--------------------
	01a, 15Jul19, Karl Created
*/

#ifndef __TYPE_H__
#define __TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include <time.h>
#include <stdint.h>

/* Types */
typedef enum {
    STATUS_OK,
    STATUS_ERR
}Status_t;

typedef uint8_t Bool_t;
typedef struct tm Time_t;

typedef int SOCKET;

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /*__TYPE_H__*/
