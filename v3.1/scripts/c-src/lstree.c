/*
* 
* lstree
* Directory Tree :-)
*
* h.januschka 2004
* Usage: lstree [OPTION] DIRNAME
*	compile:
*		gcc -Wall -o lstree lstree.c
*    pstree like programm for files :-)
*    
*       -f, --hide-files         suppress File output (usefull if many files are in a directory)
*       -d, --hide-directories   suppress Directory output 
*       -p, --show-stat-per-dir  after last file in dir print a summary of directorie (files/size)
*       -r, --humanreadable      Format all size output in B/KB/MB/GB format instead of plain bytes 
*       -s, --follow-symlinks    Follow Symlinks
*    	-c, --output-csv	 Output CSV Line instead of per dir
*
*
*       -h, --help           show this help
*    --------------------------------------------------------------------------
*    
*    lstree has been written by h.januschka <mail@klewan.at>
*    Report Bugs to <bugs@klewan.at>
*    
*
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

#include <getopt.h>

#include <unistd.h>
#include <sys/stat.h>



//*Global options*//
int print_dirs=1;
int print_files=1;
int print_stat_perdir=0;
int follow_symlinks=0;
int human_readable=0;

int output_csv=0;

/*Global Vars */
int trav_deep=-1;
int max_dd=20;


/*Types*/
typedef struct {
	int files;
	long size;
	time_t  maxtime;
	
} travor;


/*Makro coz i am a bad bitch*/
#define SPACKO for(tmp_x=0; tmp_x<trav_deep; tmp_x++) { \
			printf("%2c|", ' '); \
		}


char * dispSize(int size);
travor travDir(char * path);
void parse_options(int argc, char ** argv);
void help(void);





void parse_options(int argc, char ** argv) {
	static struct option longopts[] = {
		{ "help",	0, NULL, 'h'},
		{ "hide-files",	0, NULL, 'f'},
		{ "max-deep", 0, NULL, 'm'},
		{ "output-csv",	0, NULL, 'c'},
		{ "hide-directories",	0, NULL, 'd'},
		{ "show-stat-per-dir",	0, NULL, 'p'},
		{ "humanreadable",	0, NULL, 'r'},
		{ "follow-symlinks",	0, NULL, 's'},
		{ NULL,		0, NULL, 0}
	};
	int c;
	
	
	if(argc < 2) {
		help();
			
	}

	for (;;) {
		c = getopt_long(argc, argv, "hfdcprsm:", longopts, (int *) 0);
		if (c == -1)
			break;
		switch (c) {
		case 'h':  /* --help */
			help();
		break;
		case 'm':
			max_dd=atoi(optarg);
		break;
		case 'f':
			print_files=0;
		break;
		case 'd':
			print_dirs=0;
		break;
		case 'p':
			print_stat_perdir=1;
		break;
		
		case 'r':
			//optarg
			human_readable=1;
			
		break;
		case 's':
			follow_symlinks=1;
		break;
		
		case 'c':
			output_csv=1;
			print_stat_perdir=1;
			print_dirs=0;
			print_files=0;
		break;
		
		
		default:
			help();
		}
	}
	
	
}

char * dispSize(int size) {
	char * buf;
	float result;
	
	
	
	buf=malloc(sizeof(char)*(2+20));
	
	if(!human_readable) {
		sprintf(buf, "%d", size);	
		return buf;
	}
	
	result=size;
	sprintf(buf, "%.2f B",result);
	
	if(result > 1024) {
		result=size/1024;//KB
		sprintf(buf, "%.2f KB",result);	
	}
	if(result > 1024) {
		
		result=size/1024/1024;//MB
		sprintf(buf, "%.2f MB",result);	
	}
	
	if(result > 1024) {
		result=size/1024/1024/1024;//GB
		sprintf(buf, "%.2f GB",result);	
	}
	
	
	
	return buf;	
}

travor travDir(char * path) {
	DIR * dir_handle;
	struct dirent * file_name;
	char * next_path;
	char * size_displayer;
	
	char resolved_link[1024];
	
	struct stat  file_info;
	
	
	travor rc_travor;
	travor tmp_tr;
	
	
	int tmp_x;
	
	trav_deep++;
	
	rc_travor.maxtime=0;
	rc_travor.files=0;
	rc_travor.size=0;


	tmp_tr.maxtime=0;
	tmp_tr.files=0;
	tmp_tr.size=0;

	
	//printf("======= 'Deepth: %d---'%s' =========\n", trav_deep, path);
	

	
	if((dir_handle=opendir(path)) == NULL) {
		perror(path);
		trav_deep--;
		return tmp_tr;
		 	
	}
	
	
	
	if(path[strlen(path)-1] == '/') path[strlen(path)-1]='\0';
	
	while((file_name=readdir(dir_handle)) != NULL) {
		/*
			Skip our self
		*/
		if(file_name->d_name[0] == '.' && strlen(file_name->d_name) == 2) continue;
		if(file_name->d_name[0] == '.' && strlen(file_name->d_name) == 1) continue;
		
		//Trailing slash fix
		
			
		
		next_path=malloc(sizeof(char)*(strlen(file_name->d_name)+strlen(path)+2));
		sprintf(next_path, "%s/%s", path, file_name->d_name);
		if(lstat(next_path, &file_info) < 0)
			continue;
		
		
		
		//printf("File: `%s/%s' ^ %d ---> %d lnk: %d\n", path,  file_name->d_name, trav_deep, file_name->d_type, S_ISLNK(file_info.st_mode));
		
		if(S_ISDIR(file_info.st_mode)) file_name->d_type=DT_DIR;
		if(S_ISLNK(file_info.st_mode)) file_name->d_type=DT_LNK;
		if(S_ISREG(file_info.st_mode)) file_name->d_type=DT_REG;
		
		switch(file_name->d_type) {
			case DT_REG:
				if(file_info.st_mtime > rc_travor.maxtime)
					rc_travor.maxtime=file_info.st_mtime;
				//printf("File: `%s/%s' ^ %d ---> %d\n", path,  file_name->d_name, trav_deep, file_name->d_type);
				rc_travor.files++;				
				//stat(next_path, &file_info);
				rc_travor.size += file_info.st_size;
				if(print_files) {
					SPACKO
					size_displayer=dispSize(file_info.st_size);
					if(trav_deep > 0) {
						printf("---");
					}
					printf("[%s](%s)\n", next_path, size_displayer);
					free(size_displayer);

				}
				
				break;
			case DT_LNK:
				//Symlink :-)
				
				if(follow_symlinks == 1) {
					tmp_x=readlink(next_path, resolved_link, 1024);
					resolved_link[tmp_x]='\0';

					if(stat(resolved_link, &file_info) < 0)
						continue;
					
					if (S_ISREG(file_info.st_mode)) {
						rc_travor.files++;				
						stat(resolved_link, &file_info);
						rc_travor.size += file_info.st_size;
						
						if(file_info.st_mtime > rc_travor.maxtime)
							rc_travor.maxtime=file_info.st_mtime;
						
						if(print_files) {
							SPACKO
							size_displayer=dispSize(file_info.st_size);
							if(trav_deep > 0) 
								printf("---");
							printf("[%s -> %s] (%s)\n", next_path, resolved_link, size_displayer);
							free(size_displayer);
						}
						
						
					} else if ( S_ISDIR(file_info.st_mode)) {
						
						if(print_dirs) {
							SPACKO
							if(trav_deep > 0)
                                                                printf("---");

							printf("[%s/%s -> %s/]\n", path, file_name->d_name, resolved_link);
						}
										
						tmp_tr=travDir(resolved_link);
						rc_travor.files += tmp_tr.files;
						rc_travor.size  += tmp_tr.size;	
						rc_travor.maxtime = tmp_tr.maxtime;
					} else {
						printf("Readlink `%s' Type detection failed!\n", resolved_link);
							
					}
					
					
				} else {
					rc_travor.files++; //Give it a file count
				}
			break;
						
			case DT_DIR:
				//printf("Dir: `%s' ^ %d\n", next_path, trav_deep);
				
				if(print_dirs) {
					SPACKO
					if(trav_deep > 0)
                                            printf("---");

					printf("[%s/%s/]\n", path, file_name->d_name);
				}
			        									
				if(trav_deep <= max_dd) {
					tmp_tr=travDir(next_path);
					rc_travor.files += tmp_tr.files;
					rc_travor.size  += tmp_tr.size;
					
				}
				
				break;
			default:
				
				printf("unnkown `%s'=%d --> %d\n", file_name->d_name, file_name->d_type, file_info.st_mode);
				
			
		}
		free(next_path);
	}
	
	
	
	if(print_stat_perdir)  {
		if(!output_csv)  {
			SPACKO;
			size_displayer=dispSize(rc_travor.size);
			printf(">>%s %d Files %s | %s",path, rc_travor.files, size_displayer, ctime(&rc_travor.maxtime));
			free(size_displayer);
		} else {
			printf("%s;%d;%ld;%ld\n", path, rc_travor.files,rc_travor.size, rc_travor.maxtime);
			fflush(stdout);
		}
	}
	
	closedir(dir_handle);
	trav_deep--;
	return rc_travor;
}

int main(int argc, char ** argv ) {
	travor rc_travor;
	char * size_displayer;
	
	rc_travor.files=0;
	rc_travor.size=0;
	
	parse_options(argc, argv);
	
	if(argv[optind] == NULL) {
		printf("No dir supplied");
		exit(1);
	}
	
	
	rc_travor=travDir(argv[optind]);	
	
	size_displayer=dispSize(rc_travor.size);
	
	
	printf("Finished: %s %d Files %s\n",argv[optind], rc_travor.files, size_displayer);
	free(size_displayer);
	return 1;
}

/*i really hate it to write help stuff :-D*/

void help(void) {
/*
	printf(" lstree \n\
 Directory Tree :-)\n\
\n\
 h.januschka 2004\n\
 Usage: lstree [OPTION] DIRNAME\n\
	compile:\n\
		gcc -Wall -o lstree lstree.c\n\
    pstree like programm for files :-)\n\
    \n\
       -f, --hide-files         suppress File output (usefull if many files are in a directory)\n\
       -d, --hide-directories   suppress Directory output \n\
       -p, --show-stat-per-dir  after last file in dir print a summary of directorie (files/size)\n\
       -r, --humanreadable      Format all size output in B/KB/MB/GB format instead of plain bytes \n\
       -s, --follow-symlinks    Follow Symlinks\n\
       -c, --output-csv         Needed it for work\n\
       -m, --max-deepth         \
    \n\
\n\
\n\
       -h, --help           show this help\n\
    --------------------------------------------------------------------------\n\
    
    lstree has been written by h.januschka <mail@klewan.at>\n\
    Report Bugs to <bugs@klewan.at>\n\
    \n");
    */
	exit(1);	
}
