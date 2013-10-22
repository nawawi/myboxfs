#ifndef CFMOD_LIB_H
#define CFMOD_LIB_H

int __file_exists(const char *s);
int __is_dir(const char *s);
int __mkdir(const char *p);

int execute_action(char *actcmd, const char *modname,char *url,const char *virname,const char *ctype,const char *clientip,const char *username,int wait);
int init_hash(hash_table **hash, const char *modname);
const char * const update_file(hash_table *hash, const char *modname,const char * const tempdir,const char * const url,const char * const client_addr,const char * const buf,int len,long int **lastscannedsize);
void remove_file(hash_table *hash, const char *modname,const char * const url,const char * const client_addr);

#endif
