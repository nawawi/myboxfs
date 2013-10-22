#include "libmybox.h"

// str_replace,str_replace_add -- Replace all occurrences of the search string with the replacement string
int str_replace_add(char **start, const char *string_add, size_t s) {
        char *tmp_ptr=NULL;
        size_t add_size;
        size_t ss;
        ss=s;
        if(ss == 0) ss=strlen(string_add) + 1;
        if(*start == NULL) {
                *start=xmalloc(ss);
                if(!(*start)) return -1;
                strncpy(*start, string_add, ss);
                if(s != 0) (*start)[ss]='\0';
                return 0;
        }
        add_size=strlen(*start);
        tmp_ptr=realloc(*start, (add_size + ss) + 1);
        if(tmp_ptr){
                *start=tmp_ptr;
                strncpy((*start) + add_size, string_add, ss);
                (*start)[add_size + ss]='\0';
                return 0;
        }
        return -1;
}
char *str_replace(const char *string, const char *search_string, const char *replace_string) {
        char *str=NULL, *pos;
        size_t spos;
        int res=0;

        if((pos=strstr(string, search_string)) == NULL) return NULL;
        spos=pos - string;
        if (spos == 0) {
                res += str_replace_add(&str, replace_string, 0);
                res += str_replace_add(&str, string + strlen(search_string), 0);
        } else{

                res += str_replace_add(&str, string, spos);
                res += str_replace_add(&str, replace_string, 0);
                res += str_replace_add(&str, string + strlen(search_string) + spos, 0);
        }

        if (res == 0) {
                return str;
        }
        free(str);
        return 0;
}
