#include <libmybox.h>

#define MAX 1024 /* buffer */
#define MAX_PATH 255 /* increase this size if you need a longer path */

void usage(void) {
	fprintf_stdout("\nUsage: logtail [LOG_FILE] <offset_file>\n");
}

int check_log(char *logname, char *offset_filename) {
	FILE *input, *offset_output;
	struct stat file_stat;
	char inode_buffer[MAX], offset_buffer[MAX], buffer[MAX];
	long offset_position;
	if((input=fopen(logname, "rb")) == NULL) {
		fprintf(stderr,"File %s cannot be read.\n",logname);
		return(2);
	}
	if((offset_output=fopen(offset_filename, "rb")) != NULL) {
		if((fgets(buffer,MAX,offset_output))!=NULL) strcpy(inode_buffer,buffer);
		if((fgets(buffer,MAX,offset_output)) !=NULL) strcpy(offset_buffer,buffer);
		fclose(offset_output);
	} else {
		strcpy(inode_buffer,"0");
		offset_position=0L;
	}
	if((stat(logname,&file_stat)) != 0) {
		fprintf(stderr,"Cannot get %s file size.\n",logname);
		return(3);
	}
	if(((atol(inode_buffer)) == (file_stat.st_ino))  && (atol(offset_buffer) > (file_stat.st_size))) {
		offset_position=0L;
		fprintf(stderr,"***************\n");
		fprintf(stderr,"*** WARNING ***: Log file %s is smaller than last time checked!\n",logname);
		fprintf(stderr,"***************	 This could indicate tampering.\n");
	}
	if(((atol(inode_buffer)) != (file_stat.st_ino)) || (atol(offset_buffer) > (file_stat.st_size))) {
		offset_position=0L;
	} else {
		offset_position=atol(offset_buffer);
	}
	fseek(input, offset_position, 0);
	while((fgets(buffer,MAX,input)) !=NULL) fprintf(stderr,"%s",buffer);
	if((offset_output=fopen(offset_filename, "w")) == NULL) {
		fprintf(stderr,"File %s cannot be created. Check your permissions.\n",offset_filename);
		fclose(input);
		fclose(offset_output);
		return(4);
	} else {
		if((chmod(offset_filename,00600)) != 0) {
			fprintf(stderr,"Cannot set permissions on file %s\n",offset_filename);
			return(3);
		} else {
			offset_position=ftell(input);
			fprintf(offset_output,"%ld\n%ld",(long)file_stat.st_ino,offset_position); 
		}
	}
	fclose(input);
	fclose(offset_output);
	return(0);
}


int logtail_main(int argc, char **argv) {
	int status=1;
	char offset_filename[MAX];
	if((argc < 2) || (argc > 3)) {
		usage();
		exit(1);
	}
	if((strlen(argv[1])) > MAX_PATH - 8) {
		fprintf(stderr,"Input filename %s is too long.\n",argv[1]);
		exit(1);
	}
	if(argc == 3) {
		if((strlen(argv[2])) > MAX_PATH - 8 ) {
			fprintf(stderr,"Input filename %s is too long.\n",argv[1]);
			exit(1);
		}
		strcpy(offset_filename,argv[2]);
	} else {
		strcpy(offset_filename,argv[1]);
		strcat(offset_filename,".offset");
	}

	status=check_log(argv[1], offset_filename);
	exit(status);
}




