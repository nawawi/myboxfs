/*
 * iswapi.h (30-NOV-1999)
 *
 * This file is a part of the Sophos Anti-Virus Interface (SAVI)(tm).
 *
 * Copyright (C) 1997,2000 Sophos Plc, Oxford, England.
 * All rights reserved.
 *
 * This source code is only intended as a supplement to the
 * SAVI(tm) Reference and related documentation for the library.
 *
 * Sophos ISavi1 declaration
 * WARNING - don't include this for SAVI2, include isavi2.h instead.
 */

#ifndef __ISWAPI_H__
#define __ISWAPI_H__

/* Check that we aren't trying to mix SAVI1 and SAVI2 interfaces: */
#ifdef _SOPHOS_SAVI2
#  error Attempting to mix SAVI1 and SAVI2 include files. Include only isavi2.h for SAVI2.
#endif
#define _SOPHOS_SAVI1

#include "iswunk.h"
#include "savitype.h"
#include "swerror.h"
#include "savichar.h"

/* Interface version 1 */

class ISavi1 : public ISweepUnknown
{
public:
   virtual U32 __stdcall SweepFile( const char* pFile, const char* pReport ) = 0;
      /*
       * Return possibilities
       *   SOPHOS_ERROR_SUCCESS
       *   SOPHOS_ERROR_NOINTERFACE
       *   SOPHOS_ERROR_SWEEPFAILURE
       *   SOPHOS_ERROR_INITIALISING
       *   SOPHOS_ERROR_VIRUSPRESENT
       */

   virtual U32 __stdcall SweepInit( void ) = 0;
      /*
       * Return possibilities
       *   SOPHOS_ERROR_SUCCESS
       *   SOPHOS_ERROR_INITIALISING
       */

   virtual U32 __stdcall SweepTerm( void ) = 0;
      /*
       * Return possibilities
       *   SOPHOS_ERROR_SUCCESS
       *   SOPHOS_ERROR_TERMINATING
       */
};

typedef ISavi1 ISweepAPI_0_00;      /* For backward source code compatibility. */

#endif   /* __ISWAPI_H__ */
