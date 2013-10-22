//
// iswfact.h (26-NOV-1999)
//
// This file is a part of the Sophos Anti-Virus Interface (SAVI)(tm).
//
// Copyright (C) 1997,2000 Sophos Plc, Oxford, England.
// All rights reserved.
//
// This source code is only intended as a supplement to the
// SAVI(tm) Reference and related documentation for the library.
//
// Class factory interface file for SAVI 1 only.
// Use iswfact2.h for SAVI2.

#ifndef __ISWFACT_H__
#define __ISWFACT_H__

// Check that we aren't trying to mix SAVI1 and SAVI2 interfaces:
#ifdef _SOPHOS_SAVI2
#  error Attempting to mix SAVI1 and SAVI2 include files. Include only isavi2.h for SAVI2.
#endif
#define _SOPHOS_SAVI1

#include "SophType.h"
   // Include Sophos basic types
#include "iswunk.h"
   // ISweepUnknown declaration

extern "C" U32 __stdcall SophosGetClassObject( U32 CLSID, U32 IID, void** ppObject );
    // Return possibilities:
    //   SOPHOS_ERROR_SUCCESS
    //   SOPHOS_ERROR_EXCEPTION
    //   SOPHOS_ERROR_NOCLASS
    //   SOPHOS_ERROR_NOINTERFACE
    //   SOPHOS_ERROR_BAD_WRITE_POINTER


class ISweepClassFactory : public ISweepUnknown
{
public:
   // ISweepClassFactory
   virtual U32 __stdcall CreateInstance( U32 IID, void** ppObject ) = 0;
      /*
       * Return possibilities
       *   SOPHOS_ERROR_SUCCESS
       *   SOPHOS_ERROR_NOCLASS
       *   SOPHOS_ERROR_EXCEPTION
       *   SOPHOS_ERROR_INVALID_PARAMETER
       *   SOPHOS_ERROR_BAD_WRITE_POINTER
       *   SOPHOS_ERROR_NOINTERFACE
       */

   virtual U32 __stdcall LockServer( S32 Lock ) = 0;
      /*
       * Return possibilities
       *   SOPHOS_ERROR_SUCCESS
       *   SOPHOS_ERROR_EXCEPTION
       *   SOPHOS_ERROR_INVALID_PARAMETER
       */
};

#endif   //__ISWFACT_H__
