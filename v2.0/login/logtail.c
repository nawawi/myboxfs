/* ------------------------------------------------------------------*/
/* logtail.c -- ASCII file tail program that remembers last position.*/
/* 								     */
/* Author:							     */
/* Craig H. Rowland <crowland@psionic.com> 15-JAN-96		     */
/*		    <crowland@vni.net>				     */
/*								     */
/* Please send me any hacks/bug fixes you make to the code. All      */
/* comments are welcome!					     */
/*								     */
/* Idea for program based upon the retail utility featured in the    */
/* Gauntlet(tm) firewall protection package published by Trusted     */
/* Information Systems Inc. <info@tis.com>			     */
/*								     */ 
/* This program will read in a standard text file and create an      */
/* offset marker when it reads the end. The offset marker is read    */
/* the next time logtail is run and the text file pointer is moved   */
/* to the offset location. This allows logtail to read in the next   */
/* lines of data following the marker. This is good for marking log  */
/* files for automatic log file checkers to monitor system events.   */
/*								     */
/* This program covered by the GNU License. This program is free to  */
/* use as long as the above copyright notices are left intact. This  */
/* program has no warranty of any kind.				     */
/*								     */
/* VERSION 1.1: Initial release					     */
/*								     */
/*         1.11: Minor typo fix. Fixed NULL comparison.		     */
/* ------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


#define MAX 1024 /* buffer */
#define MAX_PATH 255 /* increase this size if you need a longer path */
#define VERSION "1.11" 


/* Prototypes */
void usage(void);
int check_log(char *logname, char *offset_filename);

void main(int argc, char *argv[])
{

int status=1; /* Set status flag to error */
char offset_filename[MAX];


	/* Check args */
	if((argc < 2) || (argc > 3))
		{
		usage();
		exit(EX_USAGE);
		}

	/* Do sanity check on all user supplied data */
	if ((strlen(argv[1])) > MAX_PATH - 8) /* longer than MAX_PATH characters? */
		{
		printf("Input filename %s is too long.\n",argv[1]);
		exit(EX_DATAERR);
		}

	if (argc == 3) /* check user supplied alternate filename */
		{
		if ((strlen(argv[2])) > MAX_PATH - 8 ) /* longer than MAX_PATH characters? */
			{
			printf("Input filename %s is too long.\n",argv[1]);
			exit(EX_DATAERR);
			}
		strcpy(offset_filename,argv[2]);
		}
	else  /* If no alternate filename given, make our own */
		{
		strcpy(offset_filename,argv[1]);
		strcat(offset_filename,".offset");
		}

	status=check_log(argv[1], offset_filename); /* check the logs */

	if(status == 0)
		exit(EX_OK);
	else if(status == 1)
		exit(EX_SOFTWARE);
	else if(status == 2)
		exit(EX_NOINPUT);
	else if(status == 3)
		exit(EX_DATAERR);
	else if(status == 4)
		exit(EX_CANTCREAT);
	else
		{
		printf("An unknown error has occurred\n\n");
		exit(EX_SOFTWARE);
		}
}


int check_log(char *logname, char *offset_filename)
{

FILE *input,  /* Value user supplies for input file */ 
     *offset_output; /* name of the offset output file */

struct stat file_stat;

char inode_buffer[MAX],	    /* Inode temp storage */
     offset_buffer[MAX],    /* Offset temp storage */ 
     buffer[MAX];	    /* I/O Buffer */

long offset_position;	/* position in the file to offset */

	/* Check if the file exists in specified directory */
	/* Open as a binary in case the user reads in non-text files */
	if((input=fopen(logname, "rb")) == NULL)
		{
		printf("File %s cannot be read.\n",logname);
		return(2);
		}

	/* see if we can open an existing offset file and read in the inode */
	/* and offset */
	if((offset_output=fopen(offset_filename, "rb")) != NULL)
		{ /* read in the saved inode number */
		if((fgets(buffer,MAX,offset_output)) !=NULL) /* nested if()...yuch */
			strcpy(inode_buffer,buffer); /* copy in inode */

		/* read in the saved decimal offset */
		if((fgets(buffer,MAX,offset_output)) !=NULL) /* nested if()...yuch */
			strcpy(offset_buffer,buffer); /* copy in offset */

		fclose(offset_output); /* We're done, clean up */
		}
	else /* can't read the file? then assume no offset file exists */
		{
		strcpy(inode_buffer,"0"); /* this inode will be set later */
		offset_position=0L; /* if the file doesn't exist, assume */
				    /* offset of 0 because we've never */
				    /* tailed it before */
		}


	if((stat(logname,&file_stat)) != 0) /* load struct */ 
		{
		printf("Cannot get %s file size.\n",logname);
		return(3);
		}					

	/* if the current file inode is the same, but the file size has */
	/* grown SMALLER than the last time we checked, then something  */
	/* suspicous has happened (log file edited) and we'll report it */
	if(((atol(inode_buffer)) == (file_stat.st_ino)) 
	   && (atol(offset_buffer) > (file_stat.st_size)))
		{
 	 	offset_position=0L; /* reset offset and report everything */
		printf("***************\n");
		printf("*** WARNING ***: Log file %s is smaller than last time checked!\n",logname);
		printf("***************	 This could indicate tampering.\n");
		}

	/* if the current file inode or size is different than that in the */
	/* offset file then assume it has been rotated and set offset to zero */
	if(((atol(inode_buffer)) != (file_stat.st_ino)) 
	   || (atol(offset_buffer) > (file_stat.st_size)))
 	 offset_position=0L;
	else /* If the file inode is the same as old inode set the new offset */
	 offset_position=atol(offset_buffer); /*get value and convert */

#ifdef DEBUG
printf("inodebuf: %s offsetbuf: %s offsetpos: %ld\n",inode_buffer,offset_buffer,offset_position); 
#endif

	fseek(input, offset_position, 0); /* set the input file stream to */
					  /* the offset position */
	/* Print the file */
	while ((fgets(buffer,MAX,input)) !=NULL)
	  printf("%s",buffer);

	/* after we are done we need to write the new offset */
	if((offset_output=fopen(offset_filename, "w")) == NULL)
		{
		printf("File %s cannot be created. Check your permissions.\n",offset_filename);
		fclose(input);
		fclose(offset_output);
		return(4);
		}
	else
		{
		if ((chmod(offset_filename,00600)) != 0) /* Don't let anyone read offset */
		 	{
		 	printf("Cannot set permissions on file %s\n",offset_filename);
			return(3);
		 	}
		else
		 	{
			offset_position=ftell(input); /* set new offset */
			fprintf(offset_output,"%ld\n%ld",(long)file_stat.st_ino,offset_position); 
			/* write it */
			}
		}
	
	fclose(input); /* clean up */
	fclose(offset_output);

return(0); /* everything A-OK */
}


/* Tell them how to use this */
void usage(void)
{
printf("\nlogtail: version %s \n\n",VERSION);
printf("Written by Craig H. Rowland <crowland@psionic.com>\n");
printf("Based upon original utility: retail (c)Trusted Information Systems\n");
printf("This program is covered by the GNU license.\n");
printf("\nUsage: logtail [LOG_FILE] <offset_file>\n");
printf("\nlogtail will read in a file and output to stdout.\n\n");
printf("After outputing the file, logtail will create a file called\n");
printf("[LOG_FILE].offset in the same directory that will contain the\n"); 
printf("decimal offset and inode of the file in ASCII format. \n\n");
printf("Next time logtail is run on FILE the offset file is read and\n"); 
printf("output begins at the saved offset.\n\n"); 
printf("Rotated log files will be automatically accounted for by having\n");
printf("the offset reset to zero.\n\n");
printf("The optional <offset_file> parameter can be used to specify your\n");
printf("own name for the offset file. \n\n");
}
