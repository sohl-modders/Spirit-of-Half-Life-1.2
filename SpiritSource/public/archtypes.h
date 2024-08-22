//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

/***
 *	Changelog:
 *	HL25 SDK Update (Half-Life's 25th-anniversary update) - [17.11.2023]
 *****/

//
// Word size dependent definitions
// DAL 1/03
//
#ifndef ARCHTYPES_H
#define ARCHTYPES_H

#include "steam/steamtypes.h"

#ifndef _WIN32
#define MAX_PATH PATH_MAX
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <stddef.h>
#define _S_IREAD S_IREAD
#define _S_IWRITE S_IWRITE
typedef long unsigned int ulong;
#endif

#if HL_SDK25
#include "minmax.h"
#endif

#endif // ARCHTYPES_H
