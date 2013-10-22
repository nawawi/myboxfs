/*
 * swerror.h (26-NOV-1999)
 *
 * This file is a part of the Sophos Anti-Virus Interface (SAVI)(tm).
 *
 * Copyright (C) 1997,2000 Sophos Plc, Oxford, England.
 * All rights reserved.
 *
 * This source code is only intended as a supplement to the
 * SAVI(tm) Reference and related documentation for the library.
 *
 * NB Only use these error codes with SAVI2.
 *
 */

#ifndef __SWERROR_DOT_H__
#define __SWERROR_DOT_H__

/* Check that we aren't trying to mix SAVI1 and SAVI2 interfaces: */
#ifdef _SOPHOS_SAVI2
   #error Attempting to mix SAVI1 and SAVI2 include files. Include only isavi2.h for SAVI2.
#endif
#define _SOPHOS_SAVI1

/*
 * SAVI return codes.
 */

/*
 * SOPHOS_IID_SWAPI_0_00 return codes.
 */
enum  {SOPHOS_ERROR_SUCCESS          = 0,     /* No error */
/* 1*/ SOPHOS_ERROR_NOINTERFACE,              /* Invalid interface */
/* 2*/ SOPHOS_ERROR_NOCLASS,                  /* Invalid class requested */
/* 3*/ SOPHOS_ERROR_INITIALISING,             /* DLL failed to intialise */
/* 4*/ SOPHOS_ERROR_TERMINATING,              /* Error while unloading */
/* 5*/ SOPHOS_ERROR_SWEEPFAILURE,             /* The virus scan failed */
/* 6*/ SOPHOS_ERROR_VIRUSPRESENT,             /* A virus was detected */

/*
 * SOPHOS_IID_SAVI additional return values.
 */
       /* -- Compatibility errors -- */
/* 7*/ SOPHOS_ERROR_NOT_INITIALISED,          /* Attempt to use virus engine without initialising it */
/* 8*/ SOPHOS_ERROR_IC_INCOMPATIBLE_VERSION,  /* The installed version of SAV is running an */
                                              /* incompatible version of the InterCheck client */
/* 9*/ SOPHOS_ERROR_IC_ACCESS_DENIED,         /* The process does not have sufficient rights */
                                              /* to disable the InterCheck client */
/*10*/ SOPHOS_ERROR_IC_SCAN_PREVENTED,        /* The InterCheck client could not be disabled.  */
                                              /* The request to scan the file has been denied. */
       /* -- disinfection specific errors -- */
/*11*/ SOPHOS_ERROR_DISINFECTION_FAILED,      /* The disinfection failed */
/*12*/ SOPHOS_ERROR_DISINFECTION_UNAVAILABLE, /* Disinfection was attempted on an uninfected file. */

       /* -- threading errors -- */
/*13*/ SOPHOS_ERROR_LOCK_FAILED,              /* The library could not respond to the request */
                                              /* in a timely fashion */
/*14*/ SOPHOS_ERROR_RELEASE_FAILED,           /* Failed to release exclusive access to internal */
                                              /* library data */
          /* -- upgrade errors -- */
/*15*/ SOPHOS_ERROR_UPGRADE_IN_PROGRESS,      /* The virus engine is being upgraded the request */
                                              /* cannot be serviced */
/*16*/ SOPHOS_ERROR_UPGRADE_FAILED,           /* An attempted upgrade to the virus engine failed */
       SOPHOS_ERROR_SAV_NOT_INSTALLED,        /* Sophos Anti Virus has been removed from this */
                                              /* machine */
          /* -- caller argument errors -- */
/*17*/ SOPHOS_ERROR_BAD_WRITE_POINTER,        /* Invalid write argument passed to function */
/*18*/ SOPHOS_ERROR_BAD_READ_POINTER,         /* Invalid read argument passed to function */
/*19*/ SOPHOS_ERROR_INVALID_PARAMETER,        /* Invalid parameter */
/*20*/ SOPHOS_ERROR_INVALID_CONFIG_NAME,      /* Attempt to get/set SAVI configuration with incorrect name */
/*21*/ SOPHOS_ERROR_INVALID_CONFIG_TYPE,      /* Attempt to get/set SAVI configuration with incorrect type */

          /* -- internal library errors -- */
/*22*/ SOPHOS_ERROR_INIT_CONFIGURATION,       /* Could not configure SAVI */
/*23*/ SOPHOS_ERROR_NOT_SUPPORTED,            /* Not supported in this SAVI implementation */

          /* -- general error -- */
/*24*/ SOPHOS_ERROR_COULD_NOT_OPEN,           /* File couldn't be accessed */
/*25*/ SOPHOS_ERROR_FILE_COMPRESSED,          /* File was compressed, but no virus was found on the outer level */
/*26*/ SOPHOS_ERROR_FILE_ENCRYPTED,           /* File was encrypted */
/*27*/ SOPHOS_ERROR_END_OF_LIST,              /* No more list entries */
/*28*/ SOPHOS_ERROR_STUB,                     /* SAVI is a stub */
/*29*/ SOPHOS_ERROR_INFORMATION_NOT_AVAILABLE,/* Additional virus location is unavailable*/
/*30*/ SOPHOS_ERROR_ALREADY_INIT,             /* Attempt to initialise when already initialised */
       SOPHOS_ERROR_EXCEPTION = 0xfffffff};   /* An exception occured within the library */

#endif   /*__SWERROR_DOT_H__*/
