#ifndef __MAIN_H__
#define __MAIN_H__

// the configured options and settings for Tutorial
#define Tutorial_VERSION_MAJOR @Tutorial_VERSION_MAJOR@
#define Tutorial_VERSION_MINOR @Tutorial_VERSION_MINOR@

#define TSET_STR "tset"

#define CLEAN_STDIN() while ( getchar() != '\n' )
#define ERR_GOTO(err, errString, goto_tab) do{\
                                              if(err){\
                                                      printf_s("%s", errString);\
                                                      goto goto_tab;\
                                                      }\
                                              }while(0)


#define DEBUG_PRING(ret,pos)do{\
printf("ret=%llu, point:%p, func:<%s>, line:%d\n",\
(size_t)ret,(void*)pos,__func__, __LINE__);\
}while(0)



#endif  // __MAIN_H__
