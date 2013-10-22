#include <libmybox.h>
sqlite *db_connect(const char *database) {
	char *errmsg=NULL;
	sqlite *db_id=NULL;
	db_id=sqlite_open(database, 0, &errmsg);
        if(!db_id) {
		printf("%s\n",errmsg);
                free(errmsg);
                return 0;
        }
	return db_id;
}

void db_clean_buffer(void) {
	memset(SQL_RESULT->name,0x0,sizeof(SQL_RESULT->name));
	memset(SQL_RESULT->value,0x0,sizeof(SQL_RESULT->value));
	SQL_NUMROWS=0;
}

void db_flush(sqlite *db_id) {
	db_query(db_id,"VACUUM;");
}

void db_close(sqlite *db_id) {
	db_clean_buffer();
	db_flush(db_id);
	sqlite_close(db_id);
}
int db_callback(void *args,int numrows, char **colresults, char **colnames) {
	int x=0;
	for(x=0;x<numrows;x++) {
		sprintf(SQL_RESULT[SQL_NUMROWS].name,"%s",colnames[x]);
		sprintf(SQL_RESULT[SQL_NUMROWS].value,"%s",colresults[x]);
		SQL_NUMROWS++;
        }
        return 0;
}

int db_query(sqlite *db_id,const char *sql) {
	char *errmsg=NULL;
	int ret=0;
	ret=sqlite_exec(db_id, sql, &db_callback, NULL, &errmsg);
	if(ret==1) {
		printf("%s\n",errmsg);
                free(errmsg);
                return 0;
	}
	return 1;
}

char *db_get_single(sqlite *db_id,const char *sql) {
        db_query(db_id,sql);
        return (char *) SQL_RESULT[0].value;
}
