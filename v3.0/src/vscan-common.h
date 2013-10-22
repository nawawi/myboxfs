int execute_action(const char* actionCmd, const char* moduleName,
		   const char* url, const char* virname,
                   const char* contentType,
		   const char* clientAddr, const char* userName,
		   int wait);

int init_hash(hash_table** hash, const char* moduleName);

const char * const update_file(hash_table* hash, const char* moduleName,
                               const char * const tempdir,
			       const char * const url,
			       const char * const client_addr,
			       const char * const buf,
			       int len,
			       long int ** lastScannedSize);

const char * const query_file(hash_table* hash, const char* moduleName,
			      const char * const url,
			      const char * const client_addr);

void remove_file(hash_table* hash, const char* moduleName,
		 const char * const url,
		 const char * const client_addr);
