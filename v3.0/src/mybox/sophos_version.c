#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/* SAVI includes */
#include "compute.h"
#include "csavi3c.h"


int main(void) {
	CISavi3 *pSAVI;
	HRESULT hr;
	U32 ide_string_length  = 100;
	CIEnumIDEDetails *IDElist = NULL;
	OLECHAR ide_version_string[101];
	SYSTEMTIME ide_date;
	U32 ide_detects_viruses;
	U32 engine_version; 
	const char *ClientName = "tracenetwork";
	CISweepClassFactory2 *pFactory;
	hr=DllGetClassObject(
		(REFIID)&SOPHOS_CLASSID_SAVI,
		(REFIID)&SOPHOS_IID_CLASSFACTORY2,
		(void **) &pFactory);

	if(!(hr==SOPHOS_S_OK)) {
		printf("Could not initialize SAVI class/object\n");
                exit(1);
        } else {
                hr=pFactory->pVtbl->CreateInstance(pFactory, NULL, &SOPHOS_IID_SAVI3, (void **) &pSAVI );
                pFactory->pVtbl->Release(pFactory);
                if (hr == SOPHOS_S_OK) {
                        hr=pSAVI->pVtbl->InitialiseWithMoniker(pSAVI, ClientName);
                        if(SOPHOS_FAILED(hr)) {
                                printf("%s Failed to initialize SAVI [%ld]\n",(long) hr);
				pSAVI->pVtbl->Release(pSAVI);
				pSAVI = NULL;
				exit(1);
			}
		}
	}

	hr = pSAVI->pVtbl->GetVirusEngineVersion(pSAVI,
						&engine_version,
						(LPOLESTR) ide_version_string,
						ide_string_length,
						&ide_date,
						&ide_detects_viruses,
						NULL,
						(REFIID) &SOPHOS_IID_ENUM_IDEDETAILS,
						(void **) &IDElist);

	if(SOPHOS_FAILED(hr)) {
		printf("Failed to obtain version information for engine/IDE\n");
		exit(1);
	} else {
		printf("Engine version\t: %u.%u\n", (unsigned int) ((engine_version & 0xFFFF0000) >> 16), (unsigned int) (engine_version & 0x0000FFFF));
		printf("IDE version\t: %s \n", ide_version_string);
		printf("Virus detection\t: %u\n", (unsigned int) ide_detects_viruses);

	}
	exit(0);
}
