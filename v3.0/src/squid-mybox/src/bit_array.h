/*  */

#ifndef __BIT_ARRAY_H__
#define __BIT_ARRAY_H__

#if !BIT_ARRAY_ACTIVE

#define BIT_ARRAY          fd_set
#define BA_INIT(arr,len)
#define BA_FREE(arr)       

#define BA_ZERO(arr)          FD_ZERO(&arr)
#define BA_SET(fd,arr)        FD_SET(fd,&arr)
#define BA_CLR(fd,arr)        FD_CLR(fd,&arr)
#define BA_ISSET(fd,arr)      FD_ISSET(fd,&arr)

#else

typedef struct _BIT_ARRAY {

  int *p_pool;
  int  len;
  int  offset_mask;
  int  base_shift;

} BIT_ARRAY;

#define BA_INIT(arr,lenght)                                                 \
{                                                                           \
  int i,tmp = sizeof((arr).p_pool[0])*8;                                    \
                                                                            \
  for(i = 2, (arr).base_shift = 1; i < tmp; i *= 2)                         \
    (arr).base_shift++;                                                     \
  (arr).offset_mask = (i-1);                                                \
                                                                            \
  (arr).len = lenght / (sizeof((arr).p_pool[0])*8) + 1;                     \
  (arr).p_pool = xmalloc(sizeof((arr).p_pool[0])*(arr).len);                \
}                                                                           \


#define BA_FREE(arr)         \
{                            \
  if((arr).p_pool) {         \
    xfree((arr).p_pool);     \
    (arr).p_pool = NULL;     \
  }                          \
  (arr).len = 0;             \
}                            \


#define BA_ZERO(arr)        { memset((arr).p_pool, 0, sizeof((arr).p_pool[0])*(arr).len); }
#define BA_SET(fdsp, arr)   { (arr).p_pool[(fdsp)>>(arr).base_shift]|=(0x1<<((fdsp)&(arr).offset_mask)); }
#define BA_CLR(fdsp, arr)   { (arr).p_pool[(fdsp)>>(arr).base_shift]&=(~(0x1<<((fdsp)&(arr).offset_mask))); }
#define BA_ISSET(fdsp, arr) ((arr).p_pool[(fdsp)>>(arr).base_shift]&(0x1<<((fdsp)&(arr).offset_mask)))

#endif // BIT_ARRAY_ACTIVE

#endif //__BIT_ARRAY_H__
