/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */
#define _GNU_SOURCE
#include "slinclud.h"

#include <ctype.h>

#include "slang.h"
#include "_slang.h"

#define DEBUG_MALLOC 0

#if DEBUG_MALLOC
# define SLREALLOC_FUN	SLdebug_realloc
# define SLMALLOC_FUN	SLdebug_malloc
# define SLFREE_FUN	SLdebug_free
#else
# define SLREALLOC_FUN	SLREALLOC
# define SLMALLOC_FUN	SLMALLOC
# define SLFREE_FUN	SLFREE
#endif

/* Version information goes here since this file is always needed. */
int SLang_Version = SLANG_VERSION;
char *SLang_Version_String = SLANG_VERSION_STRING;

char *SLmake_string(char *str)
{
   return SLmake_nstring(str, strlen (str));
}

char *SLmake_nstring (char *str, unsigned int n)
{
   char *ptr;

   if (NULL == (ptr = SLmalloc(n + 1)))
     {
	return NULL;
     }
   SLMEMCPY (ptr, str, n);
   ptr[n] = 0;
   return(ptr);
}

void SLmake_lut (unsigned char *lut, unsigned char *range, unsigned char reverse)
{
   /* register unsigned char *l = lut, *lmax = lut + 256; */
   int i, r1, r2;

   memset ((char *)lut, reverse, 256);
   /* while (l < lmax) *l++ = reverse; */
   reverse = !reverse;

   r1 = *range++;
   while (r1)
     {
	r2 = *range++;
	if ((r2 == '-') && (*range != 0))
	  {
	     r2 = *range++;
	     for (i = r1; i <= r2; i++) lut[i] = reverse;
	     r1 = *range++;
	     continue;
	  }
	lut[r1] = reverse;
	r1 = r2;
     }
}

char *SLmalloc (unsigned int len)
{
   char *p;

   p = (char *) SLMALLOC_FUN (len);
   if (p == NULL)
     SLang_Error = SL_MALLOC_ERROR;

   return p;
}

void SLfree (char *p)
{
   if (p != NULL) SLFREE_FUN (p);
}

char *SLrealloc (char *p, unsigned int len)
{
   if (len == 0)
     {
	SLfree (p);
	return NULL;
     }

   if (p == NULL) p = SLmalloc (len);
   else
     {
	p = (char *)SLREALLOC_FUN (p, len);
	if (p == NULL)
	  SLang_Error = SL_MALLOC_ERROR;
     }
   return p;
}

char *SLcalloc (unsigned int nelems, unsigned int len)
{
   char *p;

   len = nelems * len;
   p = SLmalloc (len);
   if (p != NULL) SLMEMSET (p, 0, len);
   return p;
}


char *_SLskip_whitespace (char *s)
{
   while (isspace (*s))
     s++;
   
   return s;
}


/* p and ch may point to the same buffer */
char *_SLexpand_escaped_char(char *p, char *ch)
{
   int i = 0;
   int max = 0, num, base = 0;
   char ch1;

   ch1 = *p++;

   switch (ch1)
     {
      default: num = ch1; break;
      case 'n': num = '\n'; break;
      case 't': num = '\t'; break;
      case 'v': num = '\v'; break;
      case 'b': num = '\b'; break;
      case 'r': num = '\r'; break;
      case 'f': num = '\f'; break;
      case 'E': case 'e': num = 27; break;
      case 'a': num = 7;
	break;

	/* octal */
      case '0': case '1': case '2': case '3':
      case '4': case '5': case '6': case '7':
	max = '7';
	base = 8; i = 2; num = ch1 - '0';
	break;

      case 'd':			       /* decimal -- S-Lang extension */
	base = 10;
	i = 3;
	max = '9';
	num = 0;
	break;

      case 'x':			       /* hex */
	base = 16;
	max = '9';
	i = 2;
	num = 0;
	break;
     }

   while (i--)
     {
	ch1 = *p;

	if ((ch1 <= max) && (ch1 >= '0'))
	  {
	     num = base * num + (ch1 - '0');
	  }
	else if (base == 16)
	  {
	     ch1 |= 0x20;
	     if ((ch1 < 'a') || ((ch1 > 'f'))) break;
	     num = base * num + 10 + (ch1 - 'a');
	  }
	else break;
	p++;
     }

   *ch = (char) num;
   return p;
}

/* s and t could represent the same space */
void SLexpand_escaped_string (register char *s, register char *t,
			      register char *tmax)
{
   char ch;

   while (t < tmax)
     {
	ch = *t++;
	if (ch == '\\')
	  {
	     t = _SLexpand_escaped_char (t, &ch);
	  }
	*s++ = ch;
     }
   *s = 0;
}

int SLextract_list_element (char *list, unsigned int nth, char delim,
			    char *elem, unsigned int buflen)
{
   char *el, *elmax;
   char ch;

   while (nth > 0)
     {
	while ((0 != (ch = *list)) && (ch != delim))
	  list++;

	if (ch == 0) return -1;

	list++;
	nth--;
     }

   el = elem;
   elmax = el + (buflen - 1);

   while ((0 != (ch = *list)) && (ch != delim) && (el < elmax))
     *el++ = *list++;
   *el = 0;

   return 0;
}

#ifndef HAVE_VSNPRINTF
int _SLvsnprintf (char *buf, unsigned int buflen, char *fmt, va_list ap)
{
#if 1
   unsigned int len;

   /* On some systems vsprintf returns useless information.  So, punt */
   vsprintf (buf, fmt, ap);
   len = strlen (buf);
   if (len >= buflen)
     {
	SLang_exit_error ("\
Your system lacks the vsnprintf system call and vsprintf overflowed a buffer.\n\
The integrity of this program has been violated.\n");
	return EOF;		       /* NOT reached */
     }
   return (int)len;
#else
   int status;

   status = vsprintf (buf, fmt, ap);
   if (status >= (int) buflen)
     {
	/* If we are lucky, we will get this far.  The real solution is to
	 * provide a working version of vsnprintf
	 */
	SLang_exit_error ("\
Your system lacks the vsnprintf system call and vsprintf overflowed a buffer.\n\
The integrity of this program has been violated.\n");
	return EOF;		       /* NOT reached */
     }
   return status;
#endif
}
#endif

#ifndef HAVE_SNPRINTF
int _SLsnprintf (char *buf, unsigned int buflen, char *fmt, ...)
{
   int status;

   va_list ap;

   va_start (ap, fmt);
   status = _SLvsnprintf (buf, buflen, fmt, ap);
   va_end (ap);

   return status;
}
#endif

typedef struct _Cleanup_Function_Type
{
   struct _Cleanup_Function_Type *next;
   void (*f)(void);
}
Cleanup_Function_Type;

static Cleanup_Function_Type *Cleanup_Function_List;

static void cleanup_slang (void)
{
   while (Cleanup_Function_List != NULL)
     {
	Cleanup_Function_Type *next = Cleanup_Function_List->next;
	(*Cleanup_Function_List->f)();
	SLFREE_FUN ((char *) Cleanup_Function_List);
	Cleanup_Function_List = next;
     }
}

#ifndef HAVE_ATEXIT
# ifdef HAVE_ON_EXIT
static void on_exit_cleanup_slang (int arg_unused)
{
   (void) arg_unused;
   cleanup_slang ();
}
# endif
#endif

int SLang_add_cleanup_function (void (*f)(void))
{
   Cleanup_Function_Type *l;

   l = (Cleanup_Function_Type *) SLMALLOC_FUN (sizeof (Cleanup_Function_Type));
   if (l == NULL)
     return -1;

   l->f = f;
   l->next = Cleanup_Function_List;

   if (Cleanup_Function_List == NULL)
     {
#ifdef HAVE_ATEXIT
	(void) atexit (cleanup_slang);
#else
# ifdef HAVE_ON_EXIT
	(void) on_exit (on_exit_cleanup_slang, 0);
# endif
#endif
     }
   Cleanup_Function_List = l;
   return 0;
}


int SLang_guess_type (char *t)
{
   char *p;
   register char ch;
   int modifier;

   if (*t == '-') t++;
   p = t;

#if SLANG_HAS_FLOAT
   if (*p != '.')
     {
#endif
	modifier = 0;
	while ((*p >= '0') && (*p <= '9')) p++;
	if (t == p) return (SLANG_STRING_TYPE);
	if ((*p == 'x') && (p == t + 1))   /* 0x?? */
	  {
	     modifier |= 8;
	     p++;
	     while (ch = *p,
		    ((ch >= '0') && (ch <= '9'))
		    || (((ch | 0x20) >= 'a') && ((ch | 0x20) <= 'f'))) p++;
	  }

	/* Now look for UL, LU, UH, HU, L, H modifiers */
	while ((ch = *p) != 0)
	  {
	     ch |= 0x20;
	     if (ch == 'h') modifier |= 1;
	     else if (ch == 'l') modifier |= 2;
	     else if (ch == 'u') modifier |= 4;
	     else break;
	     p++;
	  }
	if ((1|2) == (modifier & (1|2)))	       /* hl present */
	  return SLANG_STRING_TYPE;

	if (ch == 0)
	  {
	     if ((modifier & 0x7) == 0) return SLANG_INT_TYPE;
	     if (modifier & 4)
	       {
		  if (modifier & 1) return SLANG_USHORT_TYPE;
		  if (modifier & 2) return SLANG_ULONG_TYPE;
		  return SLANG_UINT_TYPE;
	       }
	     if (modifier & 1) return SLANG_SHORT_TYPE;
	     if (modifier & 2) return SLANG_LONG_TYPE;
	     return SLANG_INT_TYPE;
	  }

	if (modifier) return SLANG_STRING_TYPE;
#if SLANG_HAS_FLOAT
     }

   /* now down to double case */
   if (*p == '.')
     {
	p++;
	while ((*p >= '0') && (*p <= '9')) p++;
     }
   if (*p == 0) return(SLANG_DOUBLE_TYPE);
   if ((*p != 'e') && (*p != 'E'))
     {
# if SLANG_HAS_COMPLEX
	if (((*p == 'i') || (*p == 'j'))
	    && (p[1] == 0))
	  return SLANG_COMPLEX_TYPE;
# endif
	if (((*p | 0x20) == 'f') && (p[1] == 0))
	  return SLANG_FLOAT_TYPE;

	return SLANG_STRING_TYPE;
     }

   p++;
   if ((*p == '-') || (*p == '+')) p++;
   while ((*p >= '0') && (*p <= '9')) p++;
   if (*p != 0)
     {
# if SLANG_HAS_COMPLEX
	if (((*p == 'i') || (*p == 'j'))
	    && (p[1] == 0))
	  return SLANG_COMPLEX_TYPE;
# endif
	if (((*p | 0x20) == 'f') && (p[1] == 0))
	  return SLANG_FLOAT_TYPE;

	return SLANG_STRING_TYPE;
     }
   return SLANG_DOUBLE_TYPE;
#else
   return SLANG_STRING_TYPE;
#endif				       /* SLANG_HAS_FLOAT */
}

static int hex_atoul (unsigned char *s, unsigned long *ul)
{
   register unsigned char ch;
   register unsigned long value;
   register int base;

   s++;				       /* skip the leading 0 */

   /* look for 'x' which indicates hex */
   if ((*s | 0x20) == 'x')
     {
	base = 16;
	s++;
	if (*s == 0)
	  {
	     SLang_Error = SL_SYNTAX_ERROR;
	     return -1;
	  }
     }
   else base = 8;

   value = 0;
   while ((ch = *s++) != 0)
     {
	char ch1 = ch | 0x20;
	switch (ch1)
	  {
	   default:
	     SLang_Error = SL_SYNTAX_ERROR;
	     break;

	   case 'u':
	   case 'l':
	   case 'h':
	     *ul = value;
	     return 0;

	   case '8':
	   case '9':
	     if (base != 16) SLang_Error = SL_SYNTAX_ERROR;
	     /* drop */
	   case '0':
	   case '1':
	   case '2':
	   case '3':
	   case '4':
	   case '5':
	   case '6':
	   case '7':
	     ch1 -= '0';
	     break;

	   case 'a':
	   case 'b':
	   case 'c':
	   case 'd':
	   case 'e':
	   case 'f':
	     if (base != 16) SLang_Error = SL_SYNTAX_ERROR;
	     ch1 = (ch1 - 'a') + 10;
	     break;
	  }
	value = value * base + ch1;
     }
   *ul = value;
   return 0;
}

/* Note: These routines do not check integer overflow.  I would use the C
 * library functions atol and atoul but some implementations check overflow
 * and some do not.  The following implementations provide a consistent
 * behavior.
 */
unsigned long SLatoul (unsigned char *s)
{
   int sign;
   unsigned long value;

   if (*s == '-') sign = -1;
   else
     {
	sign = 1;
	if (*s == '+') s++;
     }

   if (*s == '0')
     {
	if (-1 == hex_atoul (s, &value))
	  return (unsigned long) -1;
     }
   else
     {
	s = (unsigned char *) _SLskip_whitespace ((char *)s);

	value = 0;
	while (isdigit (*s))
	  {
	     value = value * 10 + (unsigned long) (*s - '0');
	     s++;
	  }
     }

   if (sign == -1)
     value = (unsigned long)-1L * value;

   return value;
}

long SLatol (unsigned char *s)
{
   s = (unsigned char *) _SLskip_whitespace ((char *)s);

   if (*s == '-')
     {
	long value = (long) SLatoul (s+1);
	return -value;
     }
   return (long) SLatoul (s);
}

int SLatoi (unsigned char *s)
{
   return (int) SLatol (s);
}

#if !defined(HAVE_ISSETUGID) && defined(__GLIBC__) && (__GLIBC__ >= 2)
extern int __libc_enable_secure;
# define HAVE___LIBC_ENABLE_SECURE 1
#endif

int _SLsecure_issetugid (void)
{
#ifdef HAVE_ISSETUGID
   return (1 == issetugid ());
#else
# ifdef HAVE___LIBC_ENABLE_SECURE
//   return __libc_enable_secure;
# else
#  if defined(HAVE_GETUID) && defined(HAVE_GETEUID) && defined(HAVE_GETGID) && defined(HAVE_GETEUID)
   static int enable_secure;
   if (enable_secure == 0)
    {
       if ((getuid () != geteuid ()) 
	   || (getgid () != getegid ()))
	 enable_secure = 1;
       else
	 enable_secure = -1;
    }
   return (enable_secure == 1);
#  else
   return 0;
#  endif
# endif
#endif
}

/* Like getenv, except if running as setuid or setgid, returns NULL */
char *_SLsecure_getenv (char *s)
{
   if (_SLsecure_issetugid ())
     return NULL;
   return getenv (s);
}
