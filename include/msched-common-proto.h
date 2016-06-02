/*
*/
        
#ifndef __M_COMMON_PROTO_H__
#define __M_COMMON_PROTO_H__


int DStatInitialize( dstat_t *D, int      DSize);
int DStatIsEnabled( dstat_t *D); 
int DStatAdd( dstat_t *D, char    *Data); 

char *BStringTime(long iTime);  /* IN:  epoch time */
char *snctime(long *Time);

char *PrintBuffer(char *,int);

int SystemF(char *,int,int *);

unsigned short DoCRC(unsigned short,unsigned char);
int PSDES(unsigned int *,unsigned int *);


int OSGetHostName(

  char          *HostName,
  char          *FullHostName,
  unsigned long *Address);

 
int GetTokens(

  char  *Line[],
  short  TokList[],
  char   TypeList[],
  char  *ValList[]);


char *GetShortName(char *);

int OSGetPID(void);
int OSSetGID(gid_t);
int OSSetUID(uid_t);

char *ShowIArray(const char *,int,int); 
char *ShowLArray(const char *,int,long); 
char *ShowSArray(const char *,int,char *); 
char *ShowFArray(const char *,int,double); 
int ShowSSArray(
 
  const char *Param,     /* I */
  char       *IndexName, /* I */
  char       *ValLine,   /* I */
  char       *Buffer);   /* I/O */


int UHProcessRequest(msocket_t *,char *);


#endif /* __M_COMMON_PROTO_H__ */
