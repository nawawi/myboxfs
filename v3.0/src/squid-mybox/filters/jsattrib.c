/* C code produced by gperf version 2.7 */
/* Command-line: gperf -k1,5,$ -o -l -C -NisJSAttrib  */
/* gperf output needs postprocessing for case-insensitivity! */

#define TOTAL_KEYWORDS 18
#define MIN_WORD_LENGTH 6
#define MAX_WORD_LENGTH 11
#define MIN_HASH_VALUE 6
#define MAX_HASH_VALUE 26
/* maximum key range = 21, duplicates = 0 */

#ifdef __GNUC__
__inline
#endif
static unsigned int
hash (str, len)
     register const char *str;
     register unsigned int len;
{
  static const unsigned char asso_values[] =
    {
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 10,  5, 10,
      10,  5, 27, 27, 27,  0, 27, 10,  0, 27,
      10,  0,  0, 27,  0,  5,  0,  0, 27, 27,
      27,  0, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
      27, 27, 27, 27, 27, 27
    };
  return len + asso_values[tolower((unsigned char)str[4])] + asso_values[tolower((unsigned char)str[0])] + asso_values[tolower((unsigned char)str[len - 1])];
}

#ifdef __GNUC__
__inline
#endif
const char *
isJSAttrib (str, len)
     register const char *str;
     register unsigned int len;
{
  static const unsigned char lengthtable[] =
    {
       0,  0,  0,  0,  0,  0,  6,  7,  8,  9, 10, 11,  7,  8,
       0, 10, 11,  7,  8,  9, 10, 11,  7,  8,  0,  0,  6
    };
  static const char * const wordlist[] =
    {
      "", "", "", "", "", "",
      "onblur",
      "onkeyup",
      "onselect",
      "onmouseup",
      "onmouseout",
      "onmouseover",
      "onreset",
      "onsubmit",
      "",
      "onkeypress",
      "onmousemove",
      "onclick",
      "onunload",
      "onkeydown",
      "ondblclick",
      "onmousedown",
      "onfocus",
      "onchange",
      "", "",
      "onload"
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        if (len == lengthtable[key])
          {
            register const char *s = wordlist[key];

            if (!strcasecmp(str, s))
              return s;
          }
    }
  return 0;
}
