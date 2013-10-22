// iswunk.h
/*
 * iswunk.h (26-NOV-1999)
 *
 * This file is a part of the Sophos Anti-Virus Interface (SAVI)(tm).
 *
 * Copyright (C) 1997,2000 Sophos Plc, Oxford, England.
 * All rights reserved.
 *
 * This source code is only intended as a supplement to the
 * SAVI(tm) Reference and related documentation for the library.
 *
 * Sophos ISweepUnknown declaration
 */

#ifndef __ISWUNK_H__
#define __ISWUNK_H__

#include "SophType.h"
   // Include Sophos basic types

// Check that we aren't trying to mix SAVI1 and SAVI2 interfaces:
#ifdef _SOPHOS_SAVI2
#  error Attempting to mix SAVI1 and SAVI2 include files. Include only isavi2.h for SAVI2.
#endif
#define _SOPHOS_SAVI1

class ISweepUnknown
{
public:
   virtual U32 __stdcall QueryInterface( U32 IID, void** ppObject ) = 0;
      /*
       * Return possibilities
       *   SOPHOS_ERROR_SUCCESS
       *   SOPHOS_ERROR_EXCEPTION
       *   SOPHOS_ERROR_INVALID_PARAMETER
       *   SOPHOS_ERROR_BAD_WRITE_POINTER
       *   SOPHOS_ERROR_NOINTERFACE
       */

   virtual U32 __stdcall AddRef( void ) = 0;
      /*
       * Return possibilities
       *   New reference count
       *   SOPHOS_ERROR_EXCEPTION (==0xffffffff) on error
       */

   virtual U32 __stdcall Release( void ) = 0;
      /*
       * Return possibilities
       *   New reference count
       *     (0         => This was the last refernce to the object.
       *                   It was destroyed)
       *     (+ve value => There were still outstanding references
       *                   to the object - it was not destroyed)
       *   SOPHOS_ERROR_EXCEPTION (==0xffffffff) on error
       */
};


#endif   //__ISWUNK_H__

