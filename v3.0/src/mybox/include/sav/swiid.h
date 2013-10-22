/*
 * swiid.h (26-NOV-1999)
 *
 * This file is a part of the Sophos Anti-Virus Interface (SAVI)(tm).
 *
 * Copyright (C) 2002 Sophos Plc, Oxford, England.
 * All rights reserved.
 *
 * This source code is only intended as a supplement to the
 * SAVI(tm) Reference and related documentation for the library.
 *
 * SAVI class and interface ID definitions.
 */

#ifndef __SWIID_DOT_H__
#define __SWIID_DOT_H__

#include "compute.h"
#include "savitype.h"


#ifndef __SOPHOS_WIN32__
 /*
  * Define GUIDs and associated macros when not on Windows platform
  */
 typedef struct _GUID
 {
   U32   Data1;
   U16   Data2;
   U16   Data3;
   U08   Data4[8];
 } GUID;

 typedef GUID          IID;
 typedef const IID *   REFIID;
 typedef GUID          CLSID;
 typedef const CLSID * REFCLSID;

# ifndef INITGUID
#  define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
      extern const GUID name
# else /* INITGUID */
#  define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
      const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
# endif /* INITGUID */

 /*
  * Define IID_IUnknown and IID_IClassFactory to the same values as Microsoft
  */
 DEFINE_GUID(IID_IUnknown,       0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
 DEFINE_GUID(IID_IClassFactory,  0x00000001, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

 /*
  * Define comparison utilities.
  */
# define IsEqualIID(a,b)                       \
   ( ((a)!=NULL && (b)!=NULL)   &&             \
     ( ((a) == (b)) ||                         \
       ((a)->Data1 == (b)->Data1 &&            \
        (a)->Data2 == (b)->Data2 &&            \
        (a)->Data3 == (b)->Data3 &&            \
        memcmp((a)->Data4, (b)->Data4, 8)==0) )\
   )

# define IsEqualCLSID(a,b) IsEqualIID(a,b)
#endif /* !__SOPHOS_WIN32__ */


/*          */
/* Classes. */
/*          */

/*          
 * Generic SAVI class identifier 
 */          
DEFINE_GUID(SOPHOS_CLASSID_SAVI, 0x91c4c540, 0x9fdd, 0x11d2, 0xaf, 0xaa, 0x00, 0x10, 0x5a, 0x30, 0x5a, 0x2b);
   /* {91C4C540-9FDD-11d2-AFAA-00105A305A2B} */

/*
 * For backward compatibility
 */
#define   SOPHOS_CLSID_SAVI              0x10000000
#define   SOPHOS_CLSID_SWAPI             SOPHOS_CLSID_SAVI

#define   SOPHOS_CLSID_SAVI2             SOPHOS_CLASSID_SAVI


/*             */
/* Interfaces. */
/*             */

/*
 * SAVI 1 only
 */
#define   SOPHOS_IID_UNKNOWN             0x00000000   
#define   SOPHOS_IID_CLASSFACTORY        0x00000001
#define   SOPHOS_IID_SWAPI_0_00          0x10000000
#define   SOPHOS_IID_SAVI1               SOPHOS_IID_SWAPI_0_00

/* 
 * SAVI 2 only
 */
#define   SOPHOS_IID_UNKNOWN2            IID_IUnknown          /* Use Windows' definition. */
#define   SOPHOS_IID_CLASSFACTORY2       IID_IClassFactory     /* Use Windows' definition. */
DEFINE_GUID(SOPHOS_IID_SAVI2, 0x2d12c871, 0x7ac, 0x11d3, 0xbe, 0x8d, 0x0, 0x10, 0x5a, 0x30, 0x5d, 0x2f);
   /* {2D12C871-07AC-11d3-BE8D-00105A305D2F} */

/* 
 * SAVI 3 only
 */
DEFINE_GUID(SOPHOS_IID_SAVI3, 0xff4e3eaa, 0x9380, 0x4a82, 0x82, 0x85, 0x2f, 0x17, 0xf2, 0xda, 0x8c, 0xfa);
   /* {FF4E3EAA-9380-4a82-8285-2F17F2DA8CFA} */

/*
 * Sophos internal.
 */
DEFINE_GUID(SOPHOS_IID_SWEEPRESULTS, 0x91c4c542, 0x9fdd, 0x11d2, 0xaf, 0xaa, 0x00, 0x10, 0x5a, 0x30, 0x5a, 0x2b);
   /* {91C4C542-9FDD-11d2-AFAA-00105A305A2B} */

DEFINE_GUID(SOPHOS_IID_ENUM_SWEEPRESULTS, 0x3a2fcc2, 0xa0a8, 0x11d2, 0xaf, 0xac, 0x00, 0x10, 0x5a, 0x30, 0x5a, 0x2b);
   /* {03A2FCC2-A0A8-11d2-AFAC-00105A305A2B} */

DEFINE_GUID(SOPHOS_IID_SWEEPERROR, 0x12d7b270, 0xc65f, 0x11d2, 0xaf, 0xcf, 0x00, 0x10, 0x5a, 0x30, 0x5a, 0x2b);
   /* {12D7B270-C65F-11d2-AFCF-00105A305A2B} */

DEFINE_GUID(SOPHOS_IID_IDEDETAILS, 0xbf8ea561, 0x7a7, 0x11d3, 0xbe, 0x8d, 0x0, 0x10, 0x5a, 0x30, 0x5d, 0x2f);
   /* {BF8EA561-07A7-11d3-BE8D-00105A305D2F} */

DEFINE_GUID(SOPHOS_IID_ENUM_IDEDETAILS, 0x3a2fcc3, 0xa0a8, 0x11d2, 0xaf, 0xac, 0x00, 0x10, 0x5a, 0x30, 0x5a, 0x2b);
   /* {03A2FCC3-A0A8-11d2-AFAC-00105A305A2B} */

DEFINE_GUID(SOPHOS_IID_SWEEPNOTIFY, 0x3a2fcc0, 0xa0a8, 0x11d2, 0xaf, 0xac, 0x00, 0x10, 0x5a, 0x30, 0x5a, 0x2b);
   /* {03A2FCC0-A0A8-11d2-AFAC-00105A305A2B} */

DEFINE_GUID(SOPHOS_IID_SWEEPNOTIFY2, 0x179fe890, 0x59, 0x11d6, 0x95, 0x28, 0xaa, 0x0, 0x4, 0x0, 0x12, 0x4);
   /* {179FE890-0059-11d6-9528-AA0004001204} */

DEFINE_GUID(SOPHOS_IID_SWEEPDISKCHANGE, 0x2a6e01c6, 0x72df, 0x4469, 0x9b, 0x37, 0x1b, 0xa, 0x3b, 0xe4, 0x2b, 0xa6);
   /* {2A6E01C6-72DF-4469-9B37-1B0A3BE42BA6} */

DEFINE_GUID(SOPHOS_IID_ENGINECONFIG, 0x3a2fcc1, 0xa0a8, 0x11d2, 0xaf, 0xac, 0x00, 0x10, 0x5a, 0x30, 0x5a, 0x2b);
   /* {03A2FCC1-A0A8-11d2-AFAC-00105A305A2B} */

DEFINE_GUID(SOPHOS_IID_ENUM_ENGINECONFIG, 0x3a2fcc4, 0xa0a8, 0x11d2, 0xaf, 0xac, 0x00, 0x10, 0x5a, 0x30, 0x5a, 0x2b);
   /* {03A2FCC4-A0A8-11d2-AFAC-00105A305A2B} */

DEFINE_GUID(SOPHOS_IID_GENERIC_LIST_HEAD, 0x2110ac80, 0xf999, 0x11d2, 0xbe, 0x88, 0x0, 0x10, 0x5a, 0x30, 0x5d, 0x2f);
   /* {2110AC80-F999-11d2-BE88-00105A305D2F} */

DEFINE_GUID(SOPHOS_IID_GENERIC_LIST_ITEM, 0x858c4ee0, 0xf999, 0x11d2, 0xbe, 0x88, 0x0, 0x10, 0x5a, 0x30, 0x5d, 0x2f);
   /* {858C4EE0-F999-11d2-BE88-00105A305D2F} */

DEFINE_GUID(SOPHOS_IID_SAVISTREAM, 0x341c886d, 0x7558, 0x47ea, 0xb5, 0x77, 0x85, 0xe4, 0x7f, 0xeb, 0xb7, 0xc7);
   /* {341C886D-7558-47ea-B577-85E47FEBB7C7} */

#endif   /* __SWIID_DOT_H__ */
