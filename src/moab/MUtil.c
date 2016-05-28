/* HEADER */
        
/*                                           *
 * Contains:                                 *
 *                                           *
 *                                           */


#include "moab.h"
#include "msched-proto.h"

extern mlog_t    mlog;

extern mattrlist_t MAList;
extern mclass_t    MClass[];
extern msched_t    MSched;
extern mnode_t    *MNode[];
extern mjob_t     *MJob[];
extern mres_t     *MRes[];

extern const char *MAttrType[];
extern const char *MComp[];
extern const char *MNodeState[];
extern const char *MHRObj[];
extern const char *MResourceType[];
extern m64_t       M64;

extern mx_t      X;

#define MAX_MUARG  12
 
typedef struct {
    int (*Func)();
    void *Arg[MAX_MUARG];
    long  TimeOut;
    int  *RC;
    int  *Lock;
    } mut_t;

/* global buffer for string manipulations.
   has to be kept reusable at all time. */
char temp_str[MMAX_LINE];

#include "MGUtil.c"


/* prototypes */
 
int __MUTFunc(void *);




int MUGetPair(

  char        *String,       /* I */
  const char **AttrName,     /* I */
  int         *AttrIndex,    /* O */
  char        *AttrArray,    /* O (optional) */
  int          CmpRelative,  /* I (boolean) */
  int         *CmpMode,      /* O (optional) */
  char        *ValLine,      /* O */
  int          ValSize)      /* I */

  {
  char *ptr;

  char  tmpLine[MAX_MNAME + 1];
  int   index;
  int   CIndex;

  int   vindex;

  int   SQCount;
  int   DQCount;
 
  if ((String == NULL) ||
      (String[0] == '\0') ||
      (AttrName == NULL) ||
      (AttrName[0] == NULL) ||
      (AttrIndex == NULL) ||
      (ValLine == NULL) ||
      (ValSize <= 0))
    {
    return(FAILURE);
    }

  /* FORMAT:  [<WS>]<ATTRIBUTE>[\[<INDEX>\]][<WS>]<CMP>[<WS>]<VAL>[<WS>] */
  /* FORMAT:  <CMP>: =,==,+=,-= */

  *AttrIndex = 0;

  if (AttrArray != NULL)
    AttrArray[0] = '\0';

  ptr = String;

  /* remove leading WS */

  while (isspace(ptr[0]))
    ptr++;

  /* load attribute */

  for (index = 0;index < MAX_MNAME;index++)
    {  
    if (ptr[index] == '\0')
      break;

    if (CmpRelative == TRUE)
      {
      if (isspace(ptr[index]) || 
         (ptr[index] == '=') ||
         (ptr[index] == '+') ||
	 (ptr[index] == '-'))
        {
        break;
        }
      }
    else
      {
      if (isspace(ptr[index]) ||
         (ptr[index] == '=') ||
         (ptr[index] == '>') ||
	 (ptr[index] == '<'))
        {
        break;
        }
      }

    if (ptr[index] == '[')
      {
      int aindex2 = 0;

      /* attr index located */

      tmpLine[index] = '\0';
      
      for (index = index + 1;index < MAX_MNAME;index++)
        {
        if ((ptr[index] == ']') || (ptr[index] == '\0'))
          break;

        if (AttrArray != NULL)
          {
          AttrArray[aindex2] = ptr[index];
       
          aindex2++;
          }
        }  /* END for (index) */

      if (AttrArray != NULL)
        AttrArray[aindex2] = '\0';

      index++;

      break;
      }  /* END if (ptr[index] == '[') */

    tmpLine[index] = ptr[index];
    }  /* END for (index) */

  tmpLine[index] = '\0';
  ptr += index;
 
  if ((*AttrIndex = MUGetIndex(tmpLine,AttrName,FALSE,0)) == 0)
    {
    /* cannot process attr name */

    *AttrIndex = -1;

    return(FAILURE);   
    }

  /* remove whitespace */

  while (isspace(ptr[0]))
    ptr++;

  if (CmpRelative == TRUE)
    {
    if ((ptr[0] != '=') && (ptr[0] != '+') && (ptr[0] != '-'))
      {
      return(FAILURE);
      }

    switch(ptr[0])
      {
      case '+':

        CIndex = 1;

        break;

      case '-':

        CIndex = -1;

        break;

      default:

        CIndex = 0;

        break;
      }  /* END switch(ptr[0]) */

    ptr += 1;
    }
  else
    {
    if ((ptr[0] != '=') && (ptr[0] != '<') && (ptr[0] != '>'))
      {
      return(FAILURE);
      }

    CIndex = MUCmpFromString(ptr,&index);

    ptr += index;
    }

  if (CmpMode != NULL)
    *CmpMode = CIndex;

  if (ptr[0] == '=')
    {
    ptr++;
    }
    
  /* remove whitespace */

  while (isspace(ptr[0]))
    ptr++;

  /* load value */

  SQCount = 0;
  DQCount = 0;

  vindex = 0;
 
  for (index = 0;index < ValSize - 1;index++)
    {
    if (ptr[index] == '\'')
      {
      SQCount++;

      continue;
      }
    else if (ptr[index] == '\"')
      {
      DQCount++;
   
      continue;
      }

    if ((!(SQCount % 2) && !(DQCount % 2) && isspace(ptr[index])) || 
       (ptr[index] == '\0'))
      {
      break;
      }
 
    ValLine[vindex++] = ptr[index];
    }  /* END for (index) */

  ValLine[vindex] = '\0';   
    
  return(SUCCESS);   
  }  /* END MUGetPair() */




int MUStrDup(

  char **Dst,
  char  *Src)

  {
  if (Dst == NULL)
    {
    return(FAILURE);
    }

  if ((*Dst != NULL) && 
      (Src != NULL) &&
      (Src[0] == (*Dst)[0]) &&  
      (!strcmp(Src,*Dst)))
    {
    /* strings are identical */

    return(SUCCESS);
    }

  MUFree(Dst);

  if ((Src != NULL) && (Src[0] != '\0'))
    *Dst = strdup(Src);

  return(SUCCESS);
  }  /* END MUStrDup() */




int MUStrCat(

  char *Dst,
  char *Src,
  int   DstSize)

  {
  int index;
  int DEnd;

  if ((Dst == NULL) || (DstSize <= 0))
    {
    return(FAILURE);
    }

  if ((Src == NULL) || (Src[0] == '\0'))
    {
    return(SUCCESS);
    }

  DEnd = MIN((int)strlen(Dst),DstSize);

  for (index = 0;index < DstSize - DEnd;index++)
    {
    if (Src[index] == '\0')
      break;
  
    Dst[DEnd + index] = Src[index];
    }  /* END for (index) */

  Dst[MIN(DstSize - 1,DEnd + index)] = '\0';

  return(SUCCESS);  
  }  /* END MUStrCat() */




 
int MUFree(

  char **Ptr)  /* I */

  {
  if ((Ptr == NULL) || (*Ptr == NULL))
    {
    return(SUCCESS);
    }

  free(*Ptr);

  *Ptr = NULL;

  return(SUCCESS);
  }  /* END MUFree() */




int MUSleep(

  long SleepDuration) /* I (in us) */

  {
  struct timeval timeout;              

  if ((X.XSleep != (int (*)())0) && (MSched.TimePolicy != mtpReal))
    {
    (*X.XSleep)(X.xd,SleepDuration);
    }
  else
    {
    timeout.tv_sec  = SleepDuration / 1000000;
    timeout.tv_usec = SleepDuration % 1000000;
 
    select(0,(fd_set *)NULL,(fd_set *)NULL,(fd_set *)NULL,&timeout);
    }

  return(SUCCESS);
  }  /* END MUSleep() */





int MUGetMS(

  struct timeval *TV,
  long           *MS)

  {
  struct timeval  tvp;

  if (MS == NULL)
    {
    return(FAILURE);
    }

  if (TV == NULL)
    {
    gettimeofday(&tvp,NULL);

    /* determine millisecond offset in current time interval */

    *MS = (tvp.tv_sec % 1000000) * 1000 + (tvp.tv_usec / 1000);
    }
  else
    {
    *MS = (TV->tv_sec % 1000000) * 1000 + (TV->tv_usec / 1000);
    }

  return(SUCCESS);
  }  /* END MUGetMS() */




int MUBoolFromString(

  char *Value,   /* I */
  int   Default) /* I */

  {
  int index;

  const char *MTrueString[] = {
    "1",
    "true",
    "True",
    "TRUE",
    "on"
    "On",
    "ON",
    "yes",
    "Yes",
    "YES",
    NULL };

  const char *MFalseString[] = {
    "0",
    "false",
    "False",
    "FALSE",
    "off",
    "Off",
    "OFF",
    "no",
    "No",
    "NO",
    NULL };

  if ((Value == NULL) || (Value[0] == '\0'))
    {
    return(Default);
    }

  if (Default == TRUE)
    {
    for (index = 0;MFalseString[index] != NULL;index++)
      {
      if (!strcmp(Value,MFalseString[index]))
        {
        return(FALSE);
        }
      }    /* END for (index) */
    }
  else  
    {
     for (index = 0;MTrueString[index] != NULL;index++)
      {
      if (!strcmp(Value,MTrueString[index]))
        {
        return(TRUE);
        }
      }    /* END for (index) */
    }  

  return(Default);     
  }  /* END MUBoolFromString() */

  


int MClassGetPrio(

  mjob_t *J,
  long   *CPrio)

  {
  int cindex;
  int index;

  mnuml_t *CList;

  if (CPrio != NULL)
    *CPrio = 0;

  if ((J == NULL) || (CPrio == NULL))
    {
    return(FAILURE);
    }

  if (J->Cred.C != NULL)
    *CPrio = J->Cred.C->F.Priority;

  CList = J->Req[0]->DRes.PSlot;

  for (cindex = 1;cindex < MAX_MCLASS;cindex++)
    {
    if (CList[cindex].count > 0)
      {
      for (index = 0;MClass[index].Name[0] != '\0';index++)
        {
        if (!strcmp(MAList[eClass][cindex],MClass[index].Name))
          {
          *CPrio = MClass[index].F.Priority;

          return(SUCCESS);
          }
        }    /* END for (index)                  */
      }      /* END if (CList[cindex].count > 0) */    
    }        /* END for (cindex)                 */

  return(FAILURE);
  }  /* END MClassGetPrio() */




int MNodeAdjustState(

  mnode_t             *N,      /* I */
  enum MNodeStateEnum *State)  /* I */

  {
  const char *FName = "MNodeAdjustState";

  DBG(7,fSTRUCT) DPrint("%s(%s,%s)\n",
    FName,
    (N != NULL) ? N->Name : "NULL",
    (State != NULL) ? "State" : "NULL");

  if ((N == NULL) || (State == NULL))
    {
    return(FAILURE);
    }

  switch(*State)
    {
    case mnsDown:
    case mnsNone:
    case mnsDraining:

      break;

    case mnsIdle:
    case mnsActive:   
    case mnsBusy:

      if (N->ARes.Procs == -1)
        {
        *State = mnsUnknown;
        }
      else if ((N->ARes.Procs <= 0) || (N->DRes.Procs >= N->CRes.Procs))
        {
        *State = mnsBusy;
        }
      else if ((N->ARes.Procs >= N->CRes.Procs) && (N->DRes.Procs == 0))
        {
        *State = mnsIdle;
        }
      else
        {
        *State = mnsActive;
        }

      break;

    default:

      /* do not modify state */

      /* NO-OP */

      break;
    }  /* END switch(*State) */

  DBG(7,fSTRUCT) DPrint("INFO:     node %s state set to %s\n",
    N->Name,
    MNodeState[*State]);

  return(SUCCESS);
  }  /* END MNodeAdjustState() */




int MUCmpFromString(
 
  char *Line,
  int  *Size)
 
  {
  int index;
  int CIndex;
  int Len;

  const char *FName = "MUCmpFromString";
 
  DBG(9,fSTRUCT) DPrint("%s(%s,Size)\n",
    FName,
    Line);
 
  CIndex = 0;

  if (Size != NULL)
    *Size = 0;
 
  for (index = 0;MComp[index] != '\0';index++)
    {
    Len = strlen(MComp[index]);
 
    if (strncmp(Line,MComp[index],Len) != 0)
      continue;
 
    if (Len == 2)
      {
      if (Size != NULL)
        *Size = Len;

      return(index);
      }
 
    CIndex = index;
    
    if (Size != NULL)
      *Size = Len;
    }  /* END for (index) */

  if (CIndex == mcmpEQ2)
    CIndex = mcmpEQ;
  else if (CIndex == mcmpNE2)
    CIndex = mcmpNE;
 
  return(CIndex);
  }  /* END MUCmpFromString() */




int MUMAGetBM(

  int   Attr,   /* IN:  attribute type  */
  char *Value,  /* IN:  attribute index */
  int   Mode)   /* IN:  search mode     */

  {
  /* NOTE:  only supports 32 bits regardless of MAX_MATTR setting  */

  int index;

  const char *FName = "MUMAGetBM";

  DBG(9,fSTRUCT) DPrint("%s(%s,%s,%d)\n",
    FName,
    MAttrType[Attr],
    (Value != NULL) ? Value : "NULL",
    Mode);

  if (Value == NULL)
    {
    return(FAILURE);
    }

  for (index = 1;index < MAX_MATTR;index++)
    {
    if (!strcmp(MAList[Attr][index],Value))
      {
      return(1 << index);
      }

    if (MAList[Attr][index][0] == '\0')
      break;
    }  /* END for (index) */

  if (index == MAX_MATTR)
    {
    return(FAILURE);
    }

  if (Mode == mVerify)
    {
    return(FAILURE);
    }

  /* add new value to table */

  MUStrCpy(MAList[Attr][index],Value,sizeof(MAList[0][0]));

  DBG(5,fSTRUCT) DPrint("INFO:     adding MAList[%s][%d]: '%s'\n",
    MAttrType[Attr],
    index,
    Value);

  return(1 << index);
  }  /* END MUMAGetBM() */




int MAttrSubset(

  int  *AvlMap,
  int  *ReqMap,
  int   MapSize,
  int   Mode)

  {
  int index;

  if (Mode == tlAND)
    {
    for (index = 0;index < MapSize >> 2;index++)
      {
      if ((AvlMap[index] & ReqMap[index]) != ReqMap[index])
        {
        return(FAILURE);
        }
      }  /* END for (index) */
    }
  else
    {
    int ReqFound;

    ReqFound = FALSE;

    for (index = 0;index < MapSize >> 2;index++)
      {
      if (ReqMap[index] == 0)
        continue;

      ReqFound = TRUE;

      if ((AvlMap[index] & ReqMap[index]) != 0)
        return(SUCCESS);
      }  /* END for (index) */

    if (ReqFound == TRUE)
      {
      return(FAILURE);
      }
    }    /* END else (Mode == tlAND) */

  return(SUCCESS);
  }  /* END MAttrSubset() */





int MUGetMAttr(

  int   AttrIndex,  /* I */
  char *AttrValue,  /* I */
  int   SearchMode, /* I: (mSet, mAdd, mVerify)  */
  int  *AttrMap,    /* I/O: attr BM to evaluate (optional) */
  int   MapSize)    /* I */

  {
  int index;

  const char *FName = "MUGetMAttr";

  DBG(9,fSTRUCT) DPrint("%s(%s,%s,%s,%d)\n",
    FName,
    MAttrType[AttrIndex],
    (AttrValue != NULL) ? AttrValue : "NULL",
    (SearchMode != mVerify) ? "ADD" : "CHECK",
    MapSize);

  if ((SearchMode == mSet) && (AttrValue == NULL) && (AttrMap != NULL))
    {
    /* clear attr map */

    MUBMClear(AttrMap,(MapSize << 3));

    return(SUCCESS);
    }

  if ((AttrValue == NULL) || (MapSize < M64.INTSIZE))
    {
    return(FAILURE);
    }

  index = 0;

  /* determine if attr already set */

  for (index = 0;index < MapSize << 3;index++)
    {
    if (MAList[AttrIndex][index][0] == '\0')
      break;

    if (!strcmp(MAList[AttrIndex][index],AttrValue))
      {
      if (AttrMap != NULL)
        AttrMap[index >> M64.INTLBITS] |= 1 << (index % M64.INTBITS);

      return(SUCCESS);
      }
    }    /* END for (index) */

  if ((SearchMode == mVerify) || (index == (MapSize << 3)))
    {
    return(FAILURE);
    }

  if (AttrMap != NULL) 
    {
    /* add new value to table */

    MUStrCpy(MAList[AttrIndex][index],AttrValue,sizeof(MAList[0][0]));

    AttrMap[index >> M64.INTLBITS] |= 1 << (index % M64.INTBITS);

    DBG(5,fSTRUCT) DPrint("INFO:     added MAList[%s][%d]: '%s'\n",
      MAttrType[AttrIndex],
      index,
      AttrValue);
    }

  return(SUCCESS);
  }  /* END MUGetMAttr() */





int MUMAGetIndex(

  int   AIndex, /* IN:  Attribute Type  */
  char *Value,  /* IN:  Attribute Index */
  int   Mode)   /* IN:  Search Mode     */

  {
  /* determine index of up to MAX_MATTR attribute values */

  int index;

  const char *FName = "MUMAGetIndex";

  DBG(9,fSTRUCT) DPrint("%s(%s,%s,%s)\n",
    FName,
    MAttrType[AIndex],
    Value,
    (Mode == mVerify) ? "CHECK" : "ADD");

  for (index = 1;index < MAX_MATTR;index++)
    {
    if (!strcmp(MAList[AIndex][index],Value))
      return(index);

    if (MAList[AIndex][index][0] == '\0')
      break;
    }

  if (index == MAX_MATTR)
    {
    return(FAILURE);
    }

  if (Mode == mVerify)
    {
    return(FAILURE);
    }

  /* add new value to table */

  MUStrCpy(MAList[AIndex][index],Value,sizeof(MAList[0][0]));

  DBG(5,fCONFIG) DPrint("INFO:     adding MAList[%s][%d]: '%s'\n",
    MAttrType[AIndex],
    index,
    Value);

  return(index);
  }  /* END MUMAGetIndex() */





int MUMAFromList(

  int    Attr,   /* IN:  attribute type  */
  char **List,   /* IN:  null terminated list of char * */
  int    Mode)   /* IN:  search mode     */

  {
  int index;
  int lindex;

  int value;

  const char *FName = "MUMAFromList";

  DBG(9,fSTRUCT) DPrint("%s(%d,List,%d)\n",
    FName,
    Attr,
    Mode);

  value = 0;

  for (lindex = 0;lindex < MAX_MATTR;lindex++)
    {
    if (List[lindex] == NULL)
      break;

    if ((index = MUMAGetBM(Attr,List[lindex],Mode)) == FAILURE)
      {
      return(FAILURE);
      }

    value |= index;
    }  /* END for (lindex) */

  return(value);
  }  /* END MUMAFromList() */





int MUMAFromString(

  int   AIndex,    /* I */
  char *AttrLine,  /* I */
  int   Mode)      /* I */

  {
  char *ptr;
  char *TokPtr;

  char  Line[MAX_MLINE];

  int value;

  const char *FName = "MUMAFromString";

  /* FORMAT:  <ATTR>[:<ATTR>]... */

  DBG(6,fCONFIG) DPrint("%s(%s,'%s',%d)\n",
    FName,
    MAttrType[AIndex],
    (AttrLine != NULL) ? AttrLine : "NULL",
    Mode);

  if (AttrLine == NULL)
    {
    return(0);
    }

  value = 0;

  MUStrCpy(Line,AttrLine,sizeof(Line));

  /* terminate at ';', '\n', '#' */

  if ((ptr = strchr(Line,';')) != NULL)
    *ptr = '\0';

  if ((ptr = strchr(Line,'\n')) != NULL)
    *ptr = '\0';

  if ((ptr = strchr(Line,'#')) != NULL)
    *ptr = '\0';

  ptr = MUStrTok(Line," :\t",&TokPtr);

  while (ptr != NULL)
    {
    value |= MUMAGetBM(AIndex,ptr,Mode);

    ptr = MUStrTok(NULL," :\t",&TokPtr);
    }

  DBG(5,fCONFIG) DPrint("INFO:     %s attributes '%s' set\n",
    MAttrType[AIndex],
    MUListAttrs(AIndex,value));

  return(value);
  }  /* END MUMAFromString() */




int MUMAMAttrFromLine(

  int   AttrIndex,
  char *AttrLine,
  int   Mode,
  int  *AttrMap,
  int   MapSize)

  {
  char *ptr;
  char *TokPtr;

  char  Line[MAX_MLINE];

  const char *FName = "MUMAMAttrFromLine";

  /* FORMAT:  <ATTR>:<ATTR>:<ATTR>:... */

  DBG(6,fCONFIG) DPrint("%s(%s,'%s',%d)\n",
    FName,
    MAttrType[AttrIndex],
    AttrLine,
    Mode);

  if (AttrMap == NULL)
    {
    return(FAILURE);
    }

  memset(AttrMap,0,MapSize);

  strcpy(Line,AttrLine);

  /* terminate at ';', '\n', '#' */

  ptr = MUStrTok(Line,";#\n",&TokPtr);

  ptr = MUStrTok(ptr," :\t",&TokPtr);

  while (ptr != NULL)
    {
    MUGetMAttr(AttrIndex,ptr,Mode,AttrMap,MapSize);

    ptr = MUStrTok(NULL," :\t",&TokPtr);
    }

  DBG(5,fCONFIG) DPrint("INFO:     %s attributes '%s' set\n",
    MAttrType[AttrIndex],
    MUMAList(AttrIndex,AttrMap,MapSize));

  return(SUCCESS);
  }  /* END MUMAMAttrFromLine() */




char *MUListAttrs(

  int Attr,  /* I */
  int Value) /* I */

  {
  static char Line[MAX_MLINE];
  int         i;

  if (Value == 0)
    {
    strcpy(Line,NONE);

    return(Line);
    }

  Line[0] = '\0';

  for (i = 1;i < M64.INTBITS;i++)
    {
    if ((Value & (1 << i)) && (MAList[Attr][i][0] != '\0'))
      {
      sprintf(temp_str,"[%s]",
        MAList[Attr][i]);
      strcat(Line,temp_str);
      }
    }    /* for (i) */

  return(Line);
  }  /* END MUListAttrs() */





char *MUMAList(

  int  AttrIndex,  /* I */
  int *ValueMap,   /* I */
  int  MapSize)    /* I */

  {
  static char Line[MAX_MLINE];
  int         index;
  int         findex;

  if ((ValueMap == NULL) || (MapSize < M64.INTSIZE))
    {
    strcpy(Line,NONE);

    return(Line);
    }

  Line[0] = '\0';

  for (findex = 0;findex < (MapSize >> M64.INTSHIFT);findex++)
    {
    for (index = 0;index < M64.INTBITS;index++)
      {
      if ((ValueMap[findex] & (1 << index)) && 
          (MAList[AttrIndex][index][0] != '\0'))
        {
        sprintf(temp_str,"[%s]",
          MAList[AttrIndex][index + findex * M64.INTBITS]);
        strcat(Line,temp_str);
        }
      }    /* END for (index) */
    }      /* END for (findex) */

  if (Line[0] == '\0')
    {
    strcpy(Line,NONE);
    }

  return(Line);
  }  /* END MUMAList() */




char *MAttrFind(

  char  *SearchString,  /* I */
  int    AttrIndex,
  int   *ValueMap,
  int    MapSize,
  char **Head)
 
  {
  int   index;
  int   findex;

  int   Len;

  char  tail;

  if ((SearchString == NULL) || (SearchString[0] == '\0'))
    {
    return(NULL);
    }
 
  if ((ValueMap == NULL) || (MapSize < M64.INTSIZE))
    {
    return(NULL);
    }

  Len = strlen(SearchString);
  tail = SearchString[Len - 1];
 
  for (findex = 0;findex < (MapSize >> 2);findex++)
    {
    for (index = 0;index < M64.INTBITS;index++)
      {
      if ((ValueMap[findex] & (1 << index)) &&
          (MAList[AttrIndex][index][0] != '\0'))
        {
        if (tail != '$')
          {
          if (strncmp(MAList[AttrIndex][index],SearchString,Len))
            {
            if (Head != NULL)
              *Head = &MAList[AttrIndex][index][Len];

            return(MAList[AttrIndex][index]);
            }
          }
        else
          {
          if (strncmp(MAList[AttrIndex][index],SearchString,Len - 1) &&
              isdigit(MAList[AttrIndex][index][Len - 1]))
            {
            if (Head != NULL)
              *Head = &MAList[AttrIndex][index][Len - 1];

            return(MAList[AttrIndex][index]);
            }
          }
        }
      }
    }    /* END for (findex) */
 
  return(NULL);
  }  /* END MAttrFind() */




char *MUBListAttrs(

  int Attr,  /* I */
  int Value) /* I */

  {
  static char Line[MAX_MLINE];
  int         i;
  int         First;

  Line[0] = '\0';

  First   = TRUE;

  if (Value <= 0)
    {
    return(Line);
    }

  for (i = 1;i < M64.INTBITS;i++)
    {
    if ((Value & (1 << i)) && (MAList[Attr][i][0] != '\0'))
      {
      if (First == TRUE)
        First = FALSE;
      else
        strcat(Line,":");

      strcat(Line,MAList[Attr][i]);
      }
    }    /* END for (i) */

  return(Line);
  }  /* END MUBListAttrs() */





char *MUCAListToString(

  mnuml_t *AClass,  /* I */
  mnuml_t *CClass,  /* I */
  char    *Buf)     /* O (optional) */

  {
  static char Line[MAX_MLINE];
  int         cindex;

  char        *ptr;

  if (Buf != NULL)
    ptr = Buf;
  else
    ptr = Line;
 
  ptr[0] = '\0';

  for (cindex = 1;cindex < MAX_MCLASS;cindex++)
    {
    if (MAList[eClass][cindex][0] == '\0')
      break;

    if (CClass != NULL)
      {
      if ((CClass[cindex].count > 0) || 
          (AClass[cindex].count > 0)) 
        {
        sprintf(temp_str,"[%s %d:%d]",
          MAList[eClass][cindex],
          AClass[cindex].count,
          CClass[cindex].count); 
        }
      strcat(ptr,temp_str);
      }
    else
      {
      if (AClass[cindex].count > 0) 
        {
        sprintf(temp_str,"[%s %d]",
          MAList[eClass][cindex],
          AClass[cindex].count);
        strcat(ptr,temp_str);
        }
      }
    }   /* END for(cindex) */

  if (ptr[0] == '\0')
    strcpy(ptr,NONE);
  
  return(ptr);
  }  /* END MUCAListToString() */




int MUNumListGetCount(
 
  long      Priority,  /* I */
  mnuml_t  *Req,       /* I */
  mnuml_t  *Avail,     /* I */
  int       SLSize,    /* I (optional) */
  int      *CPtr)      /* O */
 
  {
  int index;
  int Count;
 
  if (CPtr != NULL)
    *CPtr = 0;
 
  if ((Avail == NULL) || (Req == NULL))
    {
    return(FAILURE);
    }
 
  if (Req[0].count <= 0)
    {
    if (CPtr != NULL)
      *CPtr = 99999;
 
    return(SUCCESS);
    }
 
  if (Avail[0].count == 0)
    {
    return(FAILURE);
    }
 
  Count = Avail[0].count / Req[0].count;
 
  for (index = 1;index < MAX_MCLASS;index++)
    {
    if (Req[index].count == 0)
      continue;
 
    if ((Avail[index].count == 0) || (Count == 0))             
      {
      return(FAILURE);
      }
 
    Count = MIN(Count,Avail[index].count / Req[index].count);
    }
 
  if (CPtr != NULL)
    *CPtr = Count;
 
  return(Count);
  }  /* END MUNumListGetCount() */      




int MUParseComp(

  char *CString,
  char *AName,
  int  *Cmp,
  char *ValLine)

  {
  char tmpLine[MAX_MLINE];

  char *base;

  char *ptr;

  const char *IgnCList = " \t\n()[]\'\"";
  const char *CmpCList = "<>!=";

  const char *FName = "MUParseComp";

  /* FORMAT:  <KEYWORD>[<WS>]<COMP>[<WS>]["]<VAL>[<WS>][<SYMBOL>] */

  DBG(9,fLL) DPrint("%s('%s',Cmp,ValLine)\n",
    FName,
    CString);

  ptr = CString;

  /* ignore whitespace */

  while (strchr(IgnCList,*ptr))
    {
    ptr++;
    }

  /* step over keyword */

  base = ptr;

  while (isalnum(*ptr))
    {
    AName[ptr - base] = *ptr;

    ptr++;
    }

  AName[ptr - base] = '\0';

  /* step over white space */

  while (strchr(IgnCList,*ptr))
    {
    ptr++;
    }

  /* get comparison */

  base = ptr;

  if (Cmp != NULL)
    *Cmp = mcmpEQ;

  if ((*ptr == 'e') || (*ptr == 'n'))
    {
    /* handle 'english' comparisons */

    if (!strncmp(ptr,"eq",2))
      {
      if (Cmp != NULL)
        *Cmp = mcmpEQ; 

      ptr += 2;
      }
    else if (!strncmp(ptr,"ne",2))
      {
      if (Cmp != NULL)
        *Cmp = mcmpNE;

      ptr += 2;
      }
    }
  else
    { 
    while (strchr(CmpCList,*ptr))
      {
      tmpLine[ptr - base] = *ptr;

      ptr++;
      }

    tmpLine[ptr - base] = '\0';

    if (Cmp != NULL)
      *Cmp = MUCmpFromString(tmpLine,NULL);
    }

  if (Cmp != NULL)
    {
    if (*Cmp == mcmpNE2)
      *Cmp = mcmpNE;
    }

  /* step over white space */

  while (strchr(IgnCList,*ptr))
    {
    ptr++;
    }

  /* load alphanumeric/underscore value */

  base = ptr;

  while(isalnum(*ptr) || (*ptr == '_') || (*ptr == '.'))
    {
    ValLine[ptr - base] = *ptr;

    ptr++;
    }

  ValLine[ptr - base] = '\0';

  return(SUCCESS);
  }  /* END MUParseComp() */




char *MUStrChr(

  char *Head,
  char  Delim)

  {
  char *ptr;
  int   EscapeMode;
  int   QuoteMode;

  EscapeMode = 0;
  QuoteMode  = 0;

  for (ptr = Head;*ptr != '\0';ptr++)
    {
    if (*ptr == '\\')
      {
      EscapeMode = 1;

      continue;
      }

    if ((*ptr == '\"') && (EscapeMode == 0))
      QuoteMode = 1 - QuoteMode;
      
    if ((*ptr == Delim) && (EscapeMode != 1) && (QuoteMode != 1))
      return(ptr);

    EscapeMode = FALSE;
    }

  return(NULL);
  }  /* END MUStrChr() */




char *MUStrTokE(
 
  char  *Line,   /* I */
  char  *DList,  /* I */
  char **Ptr)    /* O */
 
  {
  char *Head = NULL;
 
  int dindex;
  int ignchar;

  int SQCount = 0;
  int DQCount = 0;
 
  if (Line != NULL)
    *Ptr = Line;
 
  ignchar = FALSE;

  while (**Ptr != '\0')
    {
    /* locate SQ/DQ */

    if (**Ptr == '\'')
      {
      SQCount++;

      **Ptr = '\'';
 
      (*Ptr)++;
 
      if ((Head != NULL) && !(SQCount % 2) && !(DQCount % 2))
        {
        return(Head);
        }
      else
        {
        ignchar = TRUE;
        }
      }
    else if (**Ptr == '\"')
      {
      DQCount++;
 
      **Ptr = '\0';
 
      (*Ptr)++;

      if ((Head != NULL) && !(SQCount % 2) && !(DQCount % 2)) 
        {
        return(Head);
        }
      else
        {
        ignchar = TRUE;
        }
      }
    else if (!(SQCount % 2) && !(DQCount % 2))
      {
      /* locate delimiter */    

      for (dindex = 0;DList[dindex] != '\0';dindex++)
        {
        if (**Ptr == DList[dindex])
          {
          **Ptr = '\0';
 
          (*Ptr)++;
 
          if (Head != NULL)
            {
            return(Head);
            }
          else
            {
            ignchar = TRUE;
 
            break;
            }
          }
        }    /* END for (dindex) */
      }
 
    if ((ignchar != TRUE) && (**Ptr != '\0'))
      {
      if (Head == NULL)
        Head = *Ptr;
 
      (*Ptr)++;
      }
 
    ignchar = FALSE;
    }  /* END while (**Ptr != '\0') */
 
  return(Head);
  }  /* END MUStrTokE() */





int MUPurgeEscape(

  char *String)  /* I (modified) */

  {
  char *hptr;
  char *tptr;

  int   EscapeMode;
  int   QuoteMode;

  EscapeMode = 0;
  QuoteMode  = 0;

  tptr = String;

  for (hptr = String;*hptr != '\0';hptr++)
    {
    if ((EscapeMode == FALSE) && (*hptr == '\\'))
      {
      EscapeMode = TRUE;

      continue;
      }

    if ((*hptr == '\"') && (EscapeMode == 0))
      {
      QuoteMode = 1 - QuoteMode;

      continue;
      }

    *tptr = *hptr;

    tptr++;

    EscapeMode = FALSE;
    }

  *tptr = '\0';

  return(SUCCESS);
  }  /* END MUPurgeEscape() */




int MUNumListFromString(

  mnuml_t *NumList,  /* O */
  char    *String,   /* I */
  int      LIndex)   /* I */

  {
  char *cptr;
  char *tail;
  int   Count;
  int   CIndex;

  char *TokPtr;

  char  Buffer[MAX_MLINE];

  if ((String == NULL) || (NumList == NULL))
    {
    return(FAILURE);
    }

  /* FORMAT:  [<ATTR>[:<COUNT>]]... */

  memset(NumList,0,sizeof(mnuml_t) * MAX_MCLASS);

  if (strstr(String,NONE) != NULL)
    {
    return(FAILURE);
    }

  strcpy(Buffer,String);

  cptr = MUStrTok(Buffer,"[]",&TokPtr);

  do
    {
    if ((tail = strchr(cptr,':')) != NULL)
      {
      *tail = '\0';

      Count = (int)strtol(tail + 1,NULL,0);
      }
    else
      {
      Count = 1;
      }

    if ((CIndex = MUMAGetIndex(LIndex,cptr,mVerify)) == FAILURE)
      {
      switch(LIndex)
        {
        case eClass:
 
          {
          mclass_t *C;
 
          if (MClassAdd(cptr,&C) == SUCCESS)
            { 
            CIndex = C->Index;
            }
          }    /* END BLOCK */
 
          break;
 
        default:
 
          CIndex = MUMAGetIndex(LIndex,cptr,mAdd);
 
          break;
        }
      }     /* END if ((CIndex = MUMAGetIndex()) == FAILURE) */

    NumList[CIndex].count =  Count;
    NumList[0].count      += Count;
    }
  while ((cptr = MUStrTok(NULL,"[]",&TokPtr)) != NULL);

  return(SUCCESS);
  }  /* END MUNumListFromString() */




unsigned long MUBMFromRangeString(

  unsigned long *BM,          /* I */
  char          *RangeString)

  {
  int   rangestart;
  int   rangeend;

  int   rindex;

  char  Line[MAX_MLINE];

  char *rtok;
  char *tail;

  char *TokPtr;

  if (BM == NULL)
    {
    return(FAILURE);
    }

  *BM = 0;

  if (RangeString == NULL)
    {
    return(SUCCESS);
    }
 
  for (rindex = 0;RangeString[rindex] != '\0';rindex++)
    {
    if (isalpha(RangeString[rindex]))
      {
      /* string contains name referenced string objects */

      return(FAILURE);
      }
    }    /* END for (rindex) */
 
  MUStrCpy(Line,RangeString,sizeof(Line));

  /* FORMAT:  RANGESTRING:   <RANGE>[:<RANGE>]... */
  /*          RANGE:         <VALUE>[-<VALUE>]    */

  /* NOTE:    The following non-numeric values may appear in the string */
  /*          an should be handled: '&'                                 */

  rtok = MUStrTok(Line,",:",&TokPtr);

  while (rtok != NULL)
    {
    while (*rtok == '&')
      rtok++;

    rangestart = strtol(rtok,&tail,10);
    
    if (*tail == '-')
      rangeend = strtol(tail + 1,&tail,10);
    else
      rangeend = rangestart;

    for (rindex = rangestart;rindex <= rangeend;rindex++)
      {
      *BM |= (1 << rindex);
      }

    rtok = MUStrTok(NULL,",:",&TokPtr);
    }  /* END while (rtok) */

  return(SUCCESS);
  }  /* END MUBMFromRangeString() */ 





int MUNLFromTL(

  mnalloc_t *NL,
  short     *TL,
  int       *NCount)

  {
  int tindex;
  int nindex;

  const char *FName = "MUNLFromTL";

  DBG(5,fCONFIG) DPrint("%s(NL,TL)\n",
    FName);

  if ((NL == NULL) || (TL == NULL))
    {
    return(FAILURE);
    }

  NL[0].N = NULL;

  nindex = 0;

  for (tindex = 0;TL[tindex] != -1;tindex++)
    {
    for (nindex = 0;NL[nindex].N != NULL;nindex++)
      {
      if (nindex >= MAX_MNODE_PER_JOB)
	break;

      if (TL[tindex] == NL[nindex].N->Index)
        {
        NL[nindex].TC++;

        break;
        }
      }    /* END for (nindex) */

    if ((NL[nindex].N == NULL) && (nindex < MAX_MNODE_PER_JOB))
      {
      NL[nindex].N  = MNode[TL[tindex]];
      NL[nindex].TC = 1;

      NL[nindex + 1].N = NULL;
      }
    }     /* END for (tindex) */

  if (NCount != NULL)
    *NCount = nindex;

  if (NL[0].N == NULL)
    {
    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MUNLFromTL() */




char *MUCResToString(

  mcres_t *R,           /* I */
  long     WallTime,    /* I */
  int      DisplayMode, /* I */
  char    *Buf)         /* O (optional) */

  {
  static char LocalBuf[MAX_MLINE];

  const char *ResName[] = {
    "PROCS",
    "MEM",
    "SWAP",
    "DISK",
    NULL };

  char *Line;

  int index;

  int *ResPtr[4];

  int   Val;
  char *N;

  int tmpI;

  if (Buf != NULL)
    Line = Buf;
  else
    Line = LocalBuf;

  Line[0] = '\0';

  ResPtr[0] = &R->Procs;
  ResPtr[1] = &R->Mem;
  ResPtr[2] = &R->Swap;
  ResPtr[3] = &R->Disk;

  /* FORMAT:  <ATTR>=<VAL>[;<ATTR>=<VAL>]... */

  for (index = 0;ResName[index] != NULL;index++)
    {
    Val = *ResPtr[index];
    N   =  (char *)ResName[index];

    if (Val == 0)
      continue;

    if (Line[0] != '\0')
      {
      if (DisplayMode == 2)
        MUStrCat(Line,";",MAX_MLINE);
      else
        MUStrCat(Line,"  ",MAX_MLINE);
      }

    if (Val > 0)
      {
      tmpI = (WallTime > 0) ? Val / WallTime : Val;        
 
      if (DisplayMode == 1)
        {
        /* human readable - percent */

        sprintf(temp_str,"%s: %0.2lf",
          N,
          (double)tmpI / 100.0);
        strcat(Line,temp_str);
        }
      else if (DisplayMode == 2)
        {
        /* machine readable */

        sprintf(temp_str,"%s=%d",
          N,
          tmpI);
        strcat(Line,temp_str);
        }
      else
        {
        /* human readable - basic */

        if (index > 0)
          {
          sprintf(temp_str,"%s: %s",
            N,
            MULToRSpec((long)tmpI,mvmMega,NULL));
          strcat(Line,temp_str);
          }
        else
          {
          sprintf(temp_str,"%s: %d",
            N,
            tmpI);
          strcat(Line,temp_str);
          }
        }
      }
    else 
      {
      if (DisplayMode == 2)
        {
        sprintf(temp_str,"%s=%s",
          N,
          ALL);
        strcat(Line,temp_str);
        }
      else
        {
        sprintf(temp_str,"%s: %s",
          N,
          ALL);
        strcat(Line,temp_str);
        }
      }
    }    /* END for (index)   */

  /* check generic resources */

  for (index = 1;index < MAX_MGRES;index++)
    { 
    if (R->GRes[index].count == 0)
      continue;

    if (Line[0] != '\0')
      MUStrCat(Line,"  ",MAX_MLINE);

    sprintf(temp_str,"%s: %d",
      MAList[eGRes][index],
      R->GRes[index].count);
    strcat(Line,temp_str);
    }  /* END for (index) */

  if (Line[0] == '\0')
    strcpy(Line,NONE);
     
  return(Line); 
  }  /* END MUCResToString() */




int MUSScanF(
 
  char *StringBuffer,  /* I */
  char *Format,        /* I */
  ...)
 
  {
  char *fptr;
  char *sptr;
 
  char *tail;
 
  char *tmpS;
  long *tmpL;
  int  *tmpI;

  char  IFSList[MAX_MNAME];
 
  long  size;
  long  length;
 
  va_list VA;

  int   ArgCount;

  ArgCount = 0;
 
  /* FORMAT:  "%x%s %ld %d" */
 
  if (StringBuffer == NULL)
    {
    return(FAILURE);
    }
 
  if (Format == NULL)
    {
    return(FAILURE);
    }

  sptr = StringBuffer;

  if (!strncmp(sptr,"IFS-",strlen("IFS-")))
    {
    sptr += strlen("IFS-");

    IFSList[0] = *sptr;

    IFSList[1] = '\0';

    sptr++;
    }
  else
    {
    strcpy(IFSList," \t\n");
    }

  va_start(VA,Format);
 
  size = MAX_MNAME;
 
  for (fptr = Format;*fptr != '\0';fptr++)
    {
    if (*fptr == '%')
      {
      fptr++;

      /* remove IFS chars */

      while (strchr(IFSList,sptr[0]) && (sptr[0] != '\0'))
        sptr++;
 
      switch(*fptr)
        {
        case 'd':                                    

          /* read integer */
 
          tmpI = va_arg(VA,int *);
 
          if (tmpI != NULL)
            *tmpI = (int)strtol(sptr,&tail,10);

          sptr = tail;

          while (!strchr(IFSList,sptr[0]) && (sptr[0] != '\0'))  
            sptr++;

          ArgCount++;

          break;
 
        case 'l':
 
          tmpL = va_arg(VA,long *);
 
          if (tmpL != NULL)
            *tmpL = strtol(sptr,&tail,10);

          sptr = tail;

          while (!strchr(IFSList,sptr[0]) && (sptr[0] != '\0')) 
            sptr++; 

          ArgCount++;
 
          break;
 
        case 's':
 
          if (size == 0)
            {
            return(FAILURE);
            }
 
          tmpS = va_arg(VA,char *);
          tmpS[0] = '\0';
 
          length = 0;
 
          while ((sptr[0] != '\0') && strchr(IFSList,sptr[0]))
            sptr++;
 
          while(length < (size - 1))
            {
            if (*sptr == '\0')
              break;
 
            if (strchr(IFSList,sptr[0]))
              break;
 
            if (tmpS != NULL)
              tmpS[length] = *sptr;
 
            length++;
 
            sptr++;
            }
 
          if (tmpS != NULL)
            tmpS[length] = '\0';

          if (length > 0)
            ArgCount++;
 
          break;
 
        case 'x':
 
          size = va_arg(VA,int);                                       
 
          break;
 
        default:
 
          break;
        }  /* END switch(*fptr) */
      }    /* END if (*fptr == '%') */
    }      /* END for (fptr = Format,*fptr != '\0';fptr++) */
 
  va_end(VA);
 
  return(ArgCount);
  }  /* END MUSScanf() */                    





#ifndef __NT

#define MAX_RXCACHE   32         
 
int MUREToList(
 
  char  *Pattern,
  int    ObjType,
  int    PIndex,
  short *List,
  int   *Count,
  char  *Buffer)  /* I:  match field  O: message string */
 
  {
#ifdef LIBGEN
 
  static struct {
    char    Pattern[MAX_MBUFFER];
    char    re[MAX_MLINE];
    } rxcache[MAX_RXCACHE + 1];
 
  char *re;
 
#else /* LIBGEN */
 
  static struct {
    char    Pattern[MAX_MBUFFER];
    regex_t re;
    } rxcache[MAX_RXCACHE + 1];
 
  regex_t *re;
 
#endif /* LIBGEN */
 
  int   LIndex;
 
  int   index;
  int   rxindex;

  int   rc;
  char *ptr;
 
  char  Match[MAX_MNAME];
 
  mjob_t  *J; 
  mnode_t *N;
  mres_t  *R;
 
  static int InitRequired = TRUE;

  const char *FName = "MUREToList";
 
  DBG(3,fUI) DPrint("%s(%s,%s,%d,List,Count,Buffer)\n",
    FName,
    Pattern,
    MHRObj[ObjType],
    PIndex);
 
  if (InitRequired == TRUE)
    {
    memset(rxcache,0,sizeof(rxcache));
 
    InitRequired = FALSE;
    }
 
  if ((Buffer != NULL) && (Buffer[0] != '\0'))
    {
    MUStrCpy(Match,Buffer,sizeof(Match));
 
    DBG(4,fSTRUCT) DPrint("INFO:     checking for regex match with '%s'\n",
      Match);
    }
  else
    {
    Match[0] = '\0';
    }
 
  if (Buffer != NULL)
    Buffer[0] = '\0';
 
  /* truncate at newline */
 
  if ((ptr = strchr(Pattern,'\n')) != NULL)
    *ptr = '\0';
 
  if (ObjType == mxoNode)
    {
    for (rxindex = 0;rxindex < MAX_RXCACHE;rxindex++)
      {
      if (rxcache[rxindex].Pattern[0] == '\0')
        break; 
 
      if (!strcmp(rxcache[rxindex].Pattern,Pattern))
        break;
      }  /* END for (rxindex) */
    }
  else
    {
    rxindex = MAX_RXCACHE;
    }
 
  if (rxcache[rxindex].Pattern[0] == '\0')
    {
#ifdef LIBGEN
 
    re = rxcache[rxindex].re;
 
    ptr = regcmp(Pattern,NULL);
 
    if (ptr != NULL)
      {
      strcpy(re,ptr);
 
      free(ptr);
      }
    else
      {
      DBG(1,fUI) DPrint("ALERT:    cannot compile regular expression '%s'\n",
        Pattern);
 
      if (Buffer != NULL)
        {
        sprintf(Buffer,"ERROR:    cannot interpret regular expression '%s'\n",
          Pattern);
        }
 
      return(FAILURE);
      }
 
#else /* LIBGEN */
 
    re = &rxcache[rxindex].re; 
 
    rc = regcomp(re,Pattern,REG_EXTENDED|REG_ICASE|REG_NEWLINE|REG_NOSUB);
 
    if (rc != 0)
      {
      DBG(1,fUI) DPrint("ALERT:    cannot compile regular expression '%s', rc: %d\n",
        Pattern,
        rc);
 
      if (Buffer != NULL)
        {
        sprintf(Buffer,"ERROR:    cannot interpret regular expression '%s', rc: %d\n",
          Pattern,
          rc);
        }
 
      return(FAILURE);
      }
 
#endif /* LIBGEN */
 
    if (rxindex < MAX_RXCACHE)
      {
      strcpy(rxcache[rxindex].Pattern,Pattern);
      }
    }
  else
    {
#ifdef LIBGEN
 
    re = rxcache[rxindex].re;
 
#else /* LIBGEN */
 
    re = &rxcache[rxindex].re;
 
#endif /* LIBGEN */
    }
 
  switch(ObjType)
    {
    case mxoNode: 
 
      LIndex = 0;
 
      for (index = 0;index < MAX_MNODE;index++)
        {
        N = MNode[index];
 
        if ((N == NULL) || (N->Name[0] == '\0'))
          break;
 
        if (N->Name[0] == '\1')
          continue;
 
        if ((PIndex > 0) && (N->PtIndex != PIndex))
          continue;
 
        ptr = Pattern;
 
        while ((ptr = strstr(ptr,N->Name)) != NULL)
          {
          /* FORMAT:  <WS>+<NODENAME><WS> */
 
          if (((ptr > Pattern) && *(ptr - 1) == '+') &&
            ((*(ptr + strlen(N->Name)) == '\0') ||
                isspace(*(ptr + strlen(N->Name)))))
            {
            List[LIndex++] = index;
 
            DBG(3,fUI) DPrint("INFO:     MNode[%03d] '%s' added to regex list\n",
              LIndex,
              N->Name);
 
            if (Buffer != NULL)
              {
              sprintf(temp_str,"node '%s' found\n",
                N->Name);
              strcat(Buffer,temp_str);
              }
 
            break;
            }
          else
            { 
            ptr++;
            }
          }   /* END while (ptr) */
 
        if (ptr != NULL)
          continue;
 
#ifdef LIBGEN
        if (!strcmp(Pattern,"ALL") || 
           (regex(re,N->Name,NULL) != NULL) ||
           ((N->FullName != NULL) && (regex(re,N->FullName,NULL) != NULL)))
#else /* LIBGEN */
        if (!strcmp(Pattern,"ALL") || 
           !regexec(re,N->Name,0,NULL,0) ||
           ((N->FullName != NULL) && !regexec(re,N->FullName,0,NULL,0)))
#endif /* LIBGEN */
          {
          if (Match[0] == '\0')
            {
            List[LIndex++] = index;
 
            DBG(3,fUI) DPrint("INFO:     MNode[%03d] '%s' added to regex list\n",
              LIndex,
              N->Name);
 
            if (Buffer != NULL)
              {
              sprintf(temp_str,"node '%s' found\n",
                N->Name);
              strcat(Buffer,temp_str);
              }
            }
          else if (!strcmp(N->Name,Match))
            {
            List[LIndex++] = index;
 
            if (Buffer != NULL)
              {
              strcpy(Buffer,N->Name);
              }
 
            break;
            }
          }    /* END if (!strcmp(Pattern,"ALL") || ...) */
        }      /* for (index = 0;index < MAX_MNODE;index++) */ 
 
      List[LIndex] = -1;

      if (Count != NULL) 
        *Count = LIndex;
 
      if (rxindex == MAX_RXCACHE)
        {
#ifdef LIBGEN
 
        /* DO NOTHING */
 
#else /* LIBGEN */

        if (re != NULL)
          { 
          regfree(re);

          re = NULL;
          } 
#endif /* LIBGEN */
        }
 
      if ((LIndex == 0) && (Match[0] == '\0'))
        {
        DBG(3,fUI) DPrint("INFO:     no matches found for node expression\n");
 
        if (Buffer != NULL)
          {
          sprintf(temp_str,"ERROR:    no matches found for node expression\n");
          strcat(Buffer,temp_str);
          }
 
        return(FAILURE);
        }
 
      break;
 
    case mxoJob:
 
      LIndex = 0;
 
      for (J = MJob[0]->Next;(J != NULL) && (J != MJob[0]);J = J->Next)
        {
        DBG(6,fUI) DPrint("INFO:     comparing job '%s' against regex\n",
          J->Name);

        if (!strcmp(Pattern,"ALL") ||
            !strcmp(Pattern,"^(ALL)$") ||
#ifdef LIBGEN 
           (regex(re,J->Name,NULL) != NULL))
#else /* LIBGEN */
           !regexec(re,J->Name,0,NULL,0))
#endif /* LIBGEN */
          {
          if (Match[0] == '\0')
            {
            DBG(3,fUI) DPrint("INFO:     job '%s' added to regex list\n",
              J->Name);
 
            DBG(6,fUI) DPrint("INFO:     List[%02d]: '%s'\n",
              LIndex,
              J->Name);
 
            List[LIndex++] = J->Index;
 
            if (Buffer != NULL)
              {
              sprintf(Buffer,"%sjob '%s' found\n",
                Buffer,
                J->Name);
              strcat(Buffer,temp_str);
              }
            }
          else if (!strcmp(J->Name,Match))
            {
            if (Buffer != NULL)
              strcpy(Buffer,J->Name);
 
            List[LIndex++] = J->Index;
 
            break;
            }
          }  /* END if (!strcmp(Pattern,"ALL") || ... */
        }    /* END for (J) */
 
      List[LIndex] = -1;

      if (Count != NULL) 
        *Count = LIndex;
 
      if (rxindex == MAX_RXCACHE)
        { 
#ifdef LIBGEN
 
        /* DO NOTHING */
 
#else /* LIBGEN */

        if (re != NULL)
          { 
          regfree(re);

          re = NULL;
          } 
#endif /* LIBGEN */
        }
 
      if ((LIndex == 0) && (Match[0] == '\0'))
        {
        DBG(3,fUI) DPrint("INFO:     no matches found for job expression\n");
 
        if (Buffer != NULL)
          {
          sprintf(temp_str,"ERROR:    no matches found for job expression\n");
          strcat(Buffer,temp_str);
          }
 
        return(FAILURE);
        }
 
      break;

    case mxoRsv:

      LIndex = 0;

      for (index = 0;index < MAX_MRES;index++)
        {
        R = MRes[index];

        if ((R == NULL) || (R->Name[0] == '\0'))
          break;
        
        if (R->Name[0] == '\1')
          continue;

        if ((PIndex > 0) && (R->PtIndex != PIndex))
          continue;

#ifdef LIBGEN
        if (!strcmp(Pattern,"ALL") || (regex(re,R->Name,NULL) != NULL))
#else /* LIBGEN */
        if (!strcmp(Pattern,"ALL") || !regexec(re,R->Name,0,NULL,0))
#endif /* LIBGEN */
          {
          if (Match[0] == '\0')
            {
            List[LIndex++] = index;

            DBG(3,fUI) DPrint("INFO:     res '%s' added to regex list\n",
              R->Name);

            if (Buffer != NULL)
              {
              sprintf(temp_str,"res '%s' found\n",
                R->Name);
              strcat(Buffer,temp_str);
              }
            }
          else if (!strcmp(R->Name,Match))
            {
            if (Buffer != NULL)
              strcpy(Buffer,R->Name);

            List[LIndex++] = R->Index;

            break;
            }
          }  /* END if (!strcmp(Pattern,"ALL") || ... */
        }    /* END for (index) */

      List[LIndex] = -1;

      if (Count != NULL)
        *Count = LIndex;

      if (rxindex == MAX_RXCACHE)
        {
#ifdef LIBGEN

        /* DO NOTHING */

#else /* LIBGEN */

        if (re != NULL)
          {
          regfree(re);

          re = NULL;
          }
#endif /* LIBGEN */
        }

      if ((LIndex == 0) && (Match[0] == '\0'))
        {
        DBG(3,fUI) DPrint("INFO:     no matches found for job expression\n");

        if (Buffer != NULL)
          {
          sprintf(temp_str,"ERROR:    no matches found for job expression\n");
          strcat(Buffer,temp_str);
          }

        return(FAILURE);
        }

      break;
 
    default:
 
      DBG(0,fUI) DPrint("ERROR:    unexpected object type '%d' passed to %s()\n",
        ObjType,
	FName);
 
      if (rxindex == MAX_RXCACHE)
        {
#ifdef LIBGEN
 
        /* DO NOTHING */
 
#else /* LIBGEN */

        if (re != NULL)
          { 
          regfree(re);

          re = NULL;
          } 
#endif /* LIBGEN */ 
        }
 
      return(FAILURE);
 
      /* break; */
    }  /* END switch(ObjType) */
 
  if (rxindex == MAX_RXCACHE)
    {
#ifdef LIBGEN
 
    /* DO NOTHING */
 
#else /* LIBGEN */

    if (re != NULL) 
      {
      regfree(re);

      re = NULL;
      } 
#endif /* LIBGEN */
    }
 
  return(SUCCESS);
  }  /* MUREToList() */
 
#else /* __NT */
 
int MUREToList(
 
  char  *Pattern,
  int    ObjType,
  int    PIndex,
  short *List,
  int   *Count,
  char  *Buffer)
 
  {
  return(FAILURE);
  }  /* END MUREToList() */
 
#endif /* __NT */




long MURSpecToL(

  char             *String,    /* I */
  enum MValModEnum  Modifier,  /* I */
  enum MValModEnum  DefMod)    /* I */

  {
  long  val;
  char *ptr;
  char *ptr2;

  int   tmpMod;

  const char ModC1[] = "bwkmgt";
  const char ModC2[] = "BWKMGT";

  const int  ModVal[] = { 0, 3, 10, 20, 30, 40 };

  enum MValModEnum index;

  /* FORMAT:  HH:MM:SS || <VAL>[<MOD>] */

  val = strtol(String,&ptr,10);

  if (*ptr == ':')   /* time resource -> convert to seconds */
    {
    /* time value detected */

    /* currently drop milliseconds */

    val *= 3600;                               /* hours   */

    val += (strtol(ptr + 1,&ptr2,10) * 60);  /* minutes */

    if (*ptr2 == ':')
      val += strtol(ptr2 + 1,&ptr,10);       /* seconds */

    return(val);
    }

  tmpMod = 0;

  tmpMod -= ModVal[Modifier];

  if (!strncmp(ptr,"-1",2))
    {
    return(-1);
    }

  if ((ptr[0] != '\0') && ((ptr[1] == ModC1[mvmWord]) || (ptr[1] == ModC2[mvmWord])))
    {
    /* adjust for word */

    tmpMod += ModVal[mvmWord];
    }

  for (index = mvmByte;index <= mvmTera;index++)
    {
    if ((ptr[0] == ModC1[index]) || (ptr[0] == ModC2[index]))
      {
      tmpMod += ModVal[index];

      break;
      }
    }    /* END for (index) */

  if (index > mvmTera)
    {
    tmpMod += ModVal[DefMod];
    }

  if (tmpMod > 0)
    {
    val <<= tmpMod;
    }
  else
    {
    tmpMod *= -1;

    val >>= tmpMod;
    }

  return(val);
  }  /* END MURSpecToL() */




char *MULToRSpec(

  long  LVal,    /* I */
  int   BaseMod, /* I */
  char *Buf)     /* O (optional) */

  {
  int index;

  long tmpL;

  static char tmpBuf[MAX_MNAME];

  const char ModC[] = "\0KMGTPF";

  char *ptr;

  if (Buf != NULL)
    ptr = Buf;
  else
    ptr = tmpBuf;

  ptr[0] = '\0';

  tmpL = LVal;

  for (index = 0;index < 6;index++)
    {
    LVal = tmpL;

    tmpL >>= 10;

    if (tmpL < 10)
      { 
      sprintf(ptr,"%ld%c",
        LVal,
        (LVal > 0) ? ModC[index + MAX(0,BaseMod - 1)] : '\0');

      return(ptr);
      }
    }    /* END for (index) */

  sprintf(ptr,"%ld%c",
    tmpL,
    (tmpL > 0) ? ModC[index + MAX(0,BaseMod - 1)] : '\0');

  return(ptr);
  }  /* END MULToRSpec() */





int MUCResFromString(

  mcres_t *R,      /* O */
  char    *String) /* I */

  {
  char *ptr;

  if ((String == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  /* NOTE:  do not initialize R */           

  /* FORMAT:  <RES>{:=}<VAL>[{,+;}<RES>{:=}<VAL>]... */

  if ((ptr = strstr(String,"PROCS")) != NULL)
    {
    ptr += strlen("PROCS") + 1;

    if (!strcmp(ptr,"ALL") || !strcmp(ptr,ALL))
      R->Procs = -1;
    else
      R->Procs = (int)strtol(ptr,NULL,0);
    }
  else if ((ptr = strstr(String,MResourceType[mrProc])) != NULL)
    {
    ptr += strlen(MResourceType[mrProc]) + 1;

    if (!strcmp(ptr,"ALL"))
      R->Procs = -1;
    else 
      R->Procs = (int)strtol(ptr,NULL,0);
    }
 
  if ((ptr = strstr(String,MResourceType[mrMem])) != NULL)
    {
    ptr += strlen(MResourceType[mrMem]) + 1;

    if (!strcmp(ptr,"ALL"))
      R->Mem = -1;
    else 
      R->Mem = (int)MURSpecToL(ptr,mvmMega,mvmMega);
    }
 
  if ((ptr = strstr(String,MResourceType[mrDisk])) != NULL)
    {
    ptr += strlen(MResourceType[mrDisk]) + 1;

    if (!strcmp(ptr,"ALL"))
      R->Disk = -1;
    else 
      R->Disk = (int)MURSpecToL(ptr,mvmMega,mvmMega);
    }
 
  if ((ptr = strstr(String,MResourceType[mrSwap])) != NULL)
    {
    ptr += strlen(MResourceType[mrSwap]) + 1;

    if (!strcmp(ptr,"ALL"))
      R->Swap = -1;
    else 
      R->Swap = (int)MURSpecToL(ptr,mvmMega,mvmMega);
    }

  /* load classes */

  /* NYI */

  /* load gres */

  /* NYI */

  return(SUCCESS);
  }  /* END MUCResFromString() */




char *MULToTString(

  long iTime)    /* I:  epoch time   */

  {
  static char String[MAX_MNAME];
  long        Time;
  int         Negative = FALSE;

  int         index;

  /* FORMAT:  [DD:]HH:MM:SS */

  if (iTime >= 8640000)
    {
    strcpy(String,"  INFINITY");

    return(String);
    }
  else if (iTime <= -864000)
    {
    strcpy(String," -INFINITY");

    return(String);
    }

  /* determine if number is negative */

  if (iTime < 0)
    {
    Negative = TRUE;

    Time = -iTime;
    }
  else
    {
    Time = iTime;
    }

  String[11] = '\0';

  /* setup seconds */

  String[10] = (Time)    % 10 + '0';    
  String[9]  = (Time/10) %  6 + '0';   
  String[8]  = ':';

  Time /= 60;

  /* setup minutes */

  String[7] = (Time)    % 10 + '0';
  String[6] = (Time/10) %  6 + '0'; 
  String[5] = ':';

  /* setup hours */

  Time /= 60;

  String[4] = (Time % 24) % 10 + '0';
  String[3] = (Time/10) ? (((Time % 24)/10) % 10 + '0') : ' ';

  if ((String[4] == '0') && (String[3] == ' '))
    String[3] = '0';

  /* setup days */

  Time /= 24;

  if (Time > 0)
    {
    String[2] = ':';
    String[1] = (Time) % 10 + '0';
    String[0] = (Time/10) ? ((Time/10) % 10 + '0') : ' ';
    }
  else
    {
    String[2] = ' ';
    String[1] = ' ';
    String[0] = ' ';
    }

  if (Negative == TRUE)
    {
    if (String[3] == ' ')
      {
      String[3] = '-';
      }
    else if (String[2] == ' ')
      {
      String[2] = '-';
      }
    else if (String[1] == ' ')
      {
      String[1] = '-';
      }
    else if (String[0] == ' ')
      {
      String[0] = '-';
      }
    else
      {
      String[1] = '9';
      String[0] = '-';
      }
    }

  for (index = 3;index >= 0;index--)
    {
    if (String[index] == ' ')
      return(&String[index + 1]);
    }

  return(String);
  }  /* END MULToTString() */






char *MUBStringTime(

  long iTime)  /* I:  epoch time */

  {
  static char String[MAX_MNAME];
  long        Time;

  Time = iTime;

  String[8] = '\0';

  /* setup seconds */

  String[7] = (Time)    % 10 + '0';
  String[6] = (Time/10) %  6 + '0';
  String[5] = ':';

  Time /= 60;

  /* setup minutes */

  String[4] = (Time)    % 10 + '0';
  String[3] = (Time/10) %  6 + '0';
  String[2] = ':';

  /* setup hours */

  Time /= 60;

  String[1] = (Time)    % 10 + '0';

  String[0] = (Time/10) ? ((Time/10) % 10 + '0') : ' ';

  return(String);
  }  /* END MUBStringTime() */



char *MULToDString(

  time_t *Time) /* I */

  {
  static char String[MAX_MNAME];

  strncpy(String,ctime(Time),19);

  String[19] = '\n';
  String[20] = '\0';

  return(String);
  }  /* END MULToDString() */





char *MUSNCTime(

  long *Time)  /* I */

  {
  time_t tmpTime;

  static char String[MAX_MNAME];

  tmpTime = (time_t)*Time;
  
  strncpy(String,ctime(&tmpTime),19);

  String[19] = '\0';

  return(String);
  }  /* END MUSNCTime() */





int MUGetOpt(

  int   *ArgC,      /* I:    Address of argc */
  char **ArgV,      /* I:    argv            */
  char  *ParseLine, /* I:    parse string    */
  char **OptArg,
  int   *Tok)

  {
  int   flag;
  int   index;
  int   aindex;

  int   AStart;

  char *ptr;

  static char ArgVal[MAX_MLINE];

  const char *FName = "MUGetOpt";

  DBG(3,fCONFIG) DPrint("%s(%d,ArgV,%s,OptArg)\n",
    FName,
    *ArgC,
    ParseLine);

  /* NOTE:  extract requested args, ignore others */

  flag = -1;

  *OptArg = NULL;

  if (*ArgC == 0)
    {
    return(flag);
    }

  if (Tok != NULL)
    AStart = *Tok;
  else
    AStart = 0;

  for (aindex = AStart;ArgV[aindex] != NULL;aindex++)
    {
    /* if arg is not flag, ignore */

    if (ArgV[aindex][0] != '-')
      continue;

    /* if flag is not in ParseLine, return '?' */

    if (!(ptr = strchr(ParseLine,ArgV[aindex][1])))
      {
      strncpy(ArgVal,ArgV[aindex],sizeof(ArgVal));
      ArgVal[sizeof(ArgVal) - 1] = '\0';

      *OptArg = ArgVal;

      if (Tok != NULL)
        (*Tok)++;

      return('?');
      } 

    flag = (int)ArgV[aindex][1];

    DBG(3,fCONFIG) DPrint("INFO:     flag '%c' detected\n",
      (char)flag);

    /* mark arg for removal */

    ArgV[aindex][0] = '\1';

    (*ArgC)--;
 
    if (*(ptr + 1) == ':')
      {
      /* flag value expected */

      if (ArgV[aindex][2] != '\0')
        {
        /* if flag value contained in flag argument */

        DBG(3,fCONFIG) DPrint("INFO:     arg '%s' found for flag '%c'\n",
          &ArgV[aindex][2],
          (char)flag);

        strncpy(ArgVal,&ArgV[aindex][2],sizeof(ArgVal));
        ArgVal[sizeof(ArgVal) - 1] = '\0';

        *OptArg = ArgVal;
        }
      else if ((aindex < *ArgC) && 
               (ArgV[aindex + 1] != NULL) &&
               (ArgV[aindex + 1][0] != '-'))
        {
        /* if flag value contained in next arg */

        DBG(3,fCONFIG) DPrint("INFO:     arg '%s' found for flag '%c'\n",
          ArgV[aindex + 1],
          (char)flag);

        strncpy(ArgVal,ArgV[aindex + 1],sizeof(ArgVal));
        ArgVal[sizeof(ArgVal) - 1] = '\0';

        *OptArg = ArgVal;

        ArgV[aindex + 1][0] = '\1';

        (*ArgC)--;
        }
      else
        {
        /* if flag option not supplied */

        DBG(3,fCONFIG) DPrint("INFO:     expected arg not supplied for flag '%c'\n",
          flag);

        ArgVal[0] = '\0';

        *OptArg = ArgVal;
        }
      }    /* END if (*(ptr + 1) == ':') */ 

    /* remove all marked args */

    for (index = aindex + 1;ArgV[index] != NULL;index++)
      {
      if (ArgV[index][0] == '\1')
        continue;

      ArgV[aindex] = ArgV[index];

      aindex++;
      }

    /* terminate arg list */

    ArgV[aindex] = NULL;  
    
    break;
    }  /* END for (aindex) */

  return(flag);
  }  /* END MUGetOpt() */






char *MUUIDToName(

  uid_t UID)  /* I */

  {
  struct passwd *bufptr;
  static char    Line[MAX_MNAME];

  const char *FName = "MUUIDToName";

  DBG(10,fSTRUCT) DPrint("%s(%d)\n",
    FName,
    UID);

  if (UID == ~0U)
    {
    strcpy(Line,NONE);

    return(Line);
    }

  if ((bufptr = getpwuid(UID)) == NULL)
    {
    sprintf(Line,"UID%d",UID);
    }
  else
    {
    strcpy(Line,bufptr->pw_name);
    }

  return(Line);
  }  /* END MUUIDToName() */





char *MUGIDToName(

  gid_t GID)  /* I */

  {
  struct group *bufptr;

  static char   Line[MAX_MNAME];

  const char *FName = "MUGIDToName";

  DBG(10,fSTRUCT) DPrint("%s(%d)\n",
    FName,
    GID);

  if (GID == ~0U)
    {
    strcpy(Line,NONE);

    return(Line);
    }

  if ((bufptr = getgrgid(GID)) == NULL)
    {
    sprintf(Line,"GID%d",GID);
    }
  else
    {
    strcpy(Line,bufptr->gr_name);
    }

  return(Line);
  }  /* END MUGIDToName() */




int MUGNameFromUName(

  char *UName,  /* I */
  char *GName)  /* O */

  {
  int UID;
  int GID;

  char tmpLine[MAX_MLINE];

  if ((UName == NULL) || (GName == NULL))
    {
    return(FAILURE);
    }

  if ((UID = MUUIDFromName(UName)) < 0)
    {
    return(FAILURE);
    }

  if ((GID = MUGIDFromUID(UID)) < 0)
    {
    return(FAILURE);
    }

  strcpy(tmpLine,MUGIDToName(GID));

  if (!strcmp(tmpLine,NONE) || !strncmp(tmpLine,"GID",3))
    {
    return(FAILURE);
    }

  strcpy(GName,tmpLine);

  return(SUCCESS);
  }  /* END MUGNameFromUName() */





gid_t MUGIDFromUID(

  uid_t UID)  /* I */

  {
  struct passwd *bufptr;

  const char *FName = "MUGIDFromUID";

  DBG(10,fSTRUCT) DPrint("%s(%d)\n",
    FName,
    UID);

  if ((bufptr = getpwuid(UID)) != NULL)
    {
    return(bufptr->pw_gid);
    }
  else
    {
    return(-1);
    }

  }  /* END MUGIDFromUID() */





uid_t MUUIDFromName(

  char *Name)  /* I */

  {
# define __UTILUIDHEADER "UID"

  struct passwd *buf;

  const char *FName = "MUUIDFromName";

  DBG(10,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    Name);

  if (!strcmp(Name,NONE))
    {
    return(-1);
    }

  buf = getpwnam(Name);

  if (buf == NULL)
    {
    /* look for UID??? format */

    if (!strncmp(Name,__UTILUIDHEADER,strlen(__UTILUIDHEADER)))
      return(atoi(Name + strlen(__UTILUIDHEADER)));

    return(-1);
    }

  return(buf->pw_uid);
  }  /* END MUUIDFromName() */





gid_t MUGIDFromName(

  char *Name)  /* I */

  {
  struct group *buf;

  const char *FName = "MUGIDFromName";

  DBG(10,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    Name);

  if (!strcmp(Name,NONE))
    {
    return(-1);
    }

  buf = getgrnam(Name);

  if (buf == NULL)
    {
    /* look for GID??? format */

    if (!strncmp(Name,"GID",3))
      {
      return(atoi(Name + 3));
      }

    return(-1);
    }
  else
    {
    return(buf->gr_gid);
    }
  }    /* END MUGIDFromName() */




char *MUPrintBuffer(

  char *Buf,     /* I */
  int   BufSize) /* I */

  {
  int  bindex;
  int  lindex;

  int  bcount;

  static char Line[MAX_MLINE + 1];

  lindex = 0;

  bcount = (MAX_MLINE < BufSize) ? MAX_MLINE : BufSize;

  for (bindex = 0;bindex < bcount;bindex++)
    {
    if (!isprint(Buf[bindex]) && !isspace(Buf[bindex]))
      break;
    else
      Line[lindex++] = Buf[bindex];
    }  /* END for (bindex) */

  Line[lindex] = '\0';
   
  return(Line);
  }  /* END MUPrintBuffer() */





char *MUFindEnv(

  char *Name,   /* I */
  int  *Offset) /* O */

  {
  extern char **environ;
  int           len;
  char         *np;
  char        **p;
  char         *c;

  if ((Name == NULL) || (environ == NULL))
    return(NULL);

  /* determine length of variable name */

  for (np = Name;((*np != '\0') && (*np != '='));np++);

  len = np - Name;

  for (p = environ;(c = *p) != NULL;p++)
    {
    if ((strncmp(c,Name,len) == 0) && (c[len] == '='))
      {
      *Offset = p - environ;

      return (c + len + 1);
      }
    }

  return (NULL);
  }  /* END MUFindEnv() */




int MUUnsetEnv(

  char *Name) /* I */

  {
  extern char **environ;

  char **ptr;

  int Offset;

  if (MUFindEnv(Name,&Offset) != NULL)
    {
    /* free(environ[Offset]); */

    for (ptr = &environ[Offset];*ptr != NULL;ptr++)
      {
      *ptr = *(ptr + 1);
      }
  
    return(SUCCESS);
    }

  return(FAILURE);
  }  /* END MUUnsetEnv() */





int MUSetEnv(

  char *Var,   /* I */
  char *Value) /* I */

  {
  static char EnvVal[MAX_MENVVAR][2][MAX_MLINE];
  int         index;

  const char *FName = "MUSetEnv";

  DBG(4,fSTRUCT) DPrint("%s(%s,%s)\n",
    FName,
    Var,
    Value);

  if ((Value == NULL) || (Value[0] == '\0'))
    {
    /* unset env */

    MUUnsetEnv(Var);

    return(SUCCESS);
    }

  for (index = 0;EnvVal[index][0][0] != '\0';index++)
    {
    if (!strcmp(Var,EnvVal[index][0]))
      {
      sprintf(EnvVal[index][1],"%s=%s",
        Var,
        Value);

      putenv(EnvVal[index][1]);
      
      return(SUCCESS);
      }
    }    /* END for (index) */

  if (index < MAX_MENVVAR)
    {
    strcpy(EnvVal[index][0],Var);

    sprintf(EnvVal[index][1],"%s=%s",
      Var,
      Value);

    putenv(EnvVal[index][1]);

    return(SUCCESS);
    }

  return(FAILURE);
  }  /* END MUSetEnv() */




int MUGetTokens(

  char  **Line,
  short  *TokList,
  char   *TypeList,
  char  **ValList)

  {
  char *ptr;
  int   tindex;

  tindex = 0;

  ptr = *Line;

  while ((*ptr != ';') && (*ptr != '\0'))
    {
    TypeList[tindex] = *ptr;

    ptr++;

    TokList[tindex]  = (short)strtol(ptr,NULL,0);
 
    while (isdigit(*ptr))
      ptr++;

    ptr++;

    ValList[tindex] = ptr;

    while ((*ptr != ':') && (*ptr != ';') && (*ptr != '\0'))
      ptr++;

    if (*ptr == ':')
      {
      /* end of attribute */

      *ptr = '\0';

      ptr++;

      tindex++;

      continue;
      }
    else
      {
      /* end of object */

      *ptr = '\0';

      TokList[tindex] = 0;

      *Line = ptr;

      return(SUCCESS);
      }
    }    /* END while ((*ptr != ';') && (*ptr != '\0')) */

  *ptr = '\0';

  TokList[tindex] = 0;

  *Line = ptr;

  if (tindex == 0)
    {
    return(FAILURE);
    }

  return(SUCCESS);
  }  /* END MUGetTokens() */





int MUCompare(

  int A,   /* I */
  int Cmp, /* I */
  int B)   /* I */

  { 
  int val;

  switch(Cmp)
    {
    case mcmpGT:

      val = (A > B);

      break;

    case mcmpGE:

      val = (A >= B);

      break;

    case mcmpEQ:

      val = (A == B);

      break;

    case mcmpLE:

      val = (A <= B);

      break;

    case mcmpLT:

      val = (A < B);

      break;

    case mcmpNE:

      val = (A != B);

      break;

    case mcmpNONE:

      val = 1;

      break;
   
    default:

      val = 0;

      break;
    }  /* END switch(Cmp) */

  return(val);
  }  /* END MUCompare() */




int MUDStatInitialize(

  dstat_t *D,     /* I */
  int      DSize) /* I */

  {
  D->DSize = DSize;
  D->Count = 0;

  if ((D->Data = (char *)calloc(DSTAT_STEPSIZE, DSize)) == NULL)
    {
    DBG(0,fCORE) DPrint("ALERT:    cannot calloc memory for dstat\n");

    return(FAILURE);
    }

  D->Size = DSTAT_STEPSIZE;

  return(SUCCESS);
  }  /* END MUDStatInitialize() */




int MUDStatIsEnabled(

  dstat_t *D)  /* I */

  {
  if ((D == NULL) || (D->Data == NULL))
    {
    return(FAILURE);
    }

  if (D->Size <= D->Count)
    {
    if ((D->Data = (char *)realloc(
           D->Data,
           ((D->Size + DSTAT_STEPSIZE) * D->DSize))) == NULL)
      {
      DBG(0,fCORE) DPrint("ALERT:    cannot realloc memory for dstat\n");

      return(FAILURE);
      }

    D->Size += DSTAT_STEPSIZE;
    }
 
  return(SUCCESS); 
  }  /* END MUDStatIsEnabled() */




int MUDStatAdd(

  dstat_t *D,    /* I */
  char    *Data) /* I */

  {
  if (D == NULL)
    {
    return(FAILURE);
    }

  memcpy(&D->Data[D->Count * D->DSize],Data,D->DSize);

  D->Count++;

  return(SUCCESS);
  }  /* END MUDStatAdd() */




int MUGetPeriodStart(

  long  BaseTime,      /* I */
  long  PeriodOffset,  /* I */
  int   DIndex,        /* I */
  int   PeriodType,    /* I */
  long *PeriodStart)   /* O */

  {
  time_t     tmpTime;
  struct tm *Time;

  long       tmpPeriodStart;
  long       Offset;

  /* returns start time of period[DIndex] (including expired) */

  /* NOTE:  must incorporate daylight savings time            */
  /*        return PeriodStart DIndex periods in the future   */

  if (PeriodStart == NULL)
    {
    return(FAILURE);
    }

  tmpTime = (time_t)BaseTime;

  Time = localtime(&tmpTime);

#ifdef __LINUX
  Offset = (BaseTime + Time->tm_gmtoff) % 86400;
#else /* __LINUX */
  Offset = Time->tm_hour * 3600 +
           Time->tm_min  * 60 +
           Time->tm_sec;
#endif /* __LINUX */

  switch (PeriodType)
    {
    case mpDay:

      /* no adjustment needed */
     
      break;

    case mpWeek:

      Offset += Time->tm_wday * 86400;

      break;

    case mpInfinity:
    default:

      Offset = BaseTime;

      break;
    }
  
  tmpPeriodStart = BaseTime - Offset;

  switch (PeriodType)
    {
    case mpDay:

      tmpPeriodStart += (DIndex * DAY_LEN);

      break;

    case mpWeek:

      tmpPeriodStart += (DIndex * WEEK_LEN);

      break;

    case mpInfinity:
    default:

      /* no adjustment needed */

      break;
    }  /* END switch (PeriodType) */

  tmpTime = (time_t)tmpPeriodStart;

  Time = localtime(&tmpTime);

  /* handle daylight savings */

  if (Time->tm_hour == 23)
    {
    tmpPeriodStart += 3600;
    }
  else if (Time->tm_hour == 1)
    {
    tmpPeriodStart -= 3600;
    }

  *PeriodStart = MAX(0,tmpPeriodStart);

  return(SUCCESS);
  }  /* END MUGetPeriodStart() */




char *MUBMToString(
 
  unsigned long  BM,       /* I */
  const char    *AList[],  /* I */
  char           Delim,    /* I */
  char          *Buffer,   /* O */
  char          *EString)  /* I: Empty string */
 
  {
  int         i;

  static char Line[MAX_MLINE];

  char *ptr;

  char  DS[2];

  if (Buffer == NULL)
    ptr = Line;
  else
    ptr = Buffer;

  DS[0] = Delim;
  DS[1] = '\0';

  ptr[0] = '\0';

  for (i = 1;i < M64.INTBITS;i++)
    {
    if ((BM & (1 << i)) && (AList[i] != NULL) && (AList[i][0] != '\0'))
      {
      if (Delim == '[')
        {
        sprintf(temp_str,"[%s]",
          AList[i]);
        strcat(ptr,temp_str);
        }
      else
        {
        if (ptr[0] != '\0')
          strcat(ptr,DS);
  
        strcat(ptr,AList[i]);
        }
      }
    }  /* END for (i) */

  if ((ptr[0] == '\0') && (EString != NULL))
    strcpy(ptr,EString);
    
  return(ptr);
  }  /* END MUBMToString() */




int MUBMFromString(

  char          *Line,
  const char    *StringArray[],
  unsigned long *BM)

  {
  int index;

  char *TokPtr;
  char *ptr;

  if ((Line == NULL) || 
      (Line[0] == '\0') || 
      (BM == NULL) || 
      (StringArray == NULL))
    {
    return(FAILURE);
    }

  *BM = 0;

  ptr = MUStrTok(Line,",[] \t\n",&TokPtr);

  while (ptr != NULL)
    {
    if ((index = MUGetIndex(ptr,StringArray,FALSE,0)) > 0)
      {
      /* ignore attributes not listed */

      *BM |= (1 << index);
      }

    ptr = MUStrTok(NULL,",[] \t\n",&TokPtr);   
    }  /* END while (ptr != NULL) */

  return(SUCCESS);
  }  /* END MUBMFromString() */




int MOSSetUID(
 
  uid_t UID)  /* I */
 
  {
  if (MOSGetEUID() == UID)
    {
    return(0);
    }
 
  #ifdef __OLDHPUX
    return(seteuid(UID));
  #else /* __OLDHPUX */
    return(setuid(UID));
  #endif /* __OLDHPUX */
  }  /* END MOSSetUID() */

 
 
 
int MOSSetGID(
 
  gid_t GID)  /* I */
 
  {
  if (getgid() == GID)
    {
    return(0);
    }
 
    return(setgid(GID));
  }  /* END MOSSetGID() */




int MOSGetPID()

  {
  return(getpid());
  }  /* END MOSGetPID() */





int MUBMOR(

  int *DstMap,  /* I/O */
  int *SrcMap,  /* I */
  int  MapSize) /* I */

  {
  int mindex;
  int len;

  len = MAX(1,(MapSize >> M64.INTLBITS));

  for (mindex = 0;mindex < len;mindex++)
    {
    DstMap[mindex] |= SrcMap[mindex];
    }  /* END for (mindex) */

  return(SUCCESS);
  }  /* END MUBMOR() */




int MUBMAND(
 
  int *DstMap,   /* I/O */
  int *SrcMap,   /* I */
  int  MapSize)  /* I */

  {
  int mindex;
  int len;

  len = MAX(1,(MapSize >> M64.INTLBITS));
 
  for (mindex = 0;mindex < len;mindex++)
    {
    DstMap[mindex] &= SrcMap[mindex];
    }  /* END for (mindex) */

  return(SUCCESS);
  }  /* END MUBMAND() */





unsigned long MUGetHash(
 
  char *Name)  /* I */
 
  {
  const int     x[] = { 7, 11, 17, 23, 31, 37, 43, 47, 51, 53, 57 };
  int           i   = 0;
  unsigned long key = 0;

  const char *FName = "MUGetHash";
 
  DBG(8,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    Name);
 
  while (Name[i] != '\0')
    {
    key += (0x0f0f * x[i % 11] * (Name[i] - '.')) << (i % 6);

    i++;
    }  /* END while (Name[i] != '\0') */
 
  DBG(8,fSTRUCT) DPrint("INFO:     hash '%s' --> %6ld\n",
    Name,
    key);
 
  return(key);
  }  /* END MUGetHash() */



int MUGetHash2(
 
  char *Name)  /* I */
 
  {
  int index;
  int val;

  const char *FName = "MUGetHash2";
 
  DBG(5,fSTRUCT) DPrint("%s(%s)\n",
    FName,
    Name);
 
  val = 0;
 
  for (index = 0;Name[index] != '\0';index++)
    {
    val += (0x10101 * Name[index]);
 
    val <<= 1;
 
    val += (val / 0x1000000);
 
    val = val % 0x1000000;
    }  /* END for (index) */
 
  DBG(5,fSTRUCT) DPrint("INFO:     hash '%s' -> %x\n",
    Name,
    val);
 
  return(val);
  }  /* END MUGetHash2() */




char *MUCResRatioToString(

  mcres_t *DRes,
  mcres_t *BRes,
  mcres_t *CRes,
  int      RC)

  {
  static char Line[MAX_MLINE];

  const char *ResName[] = {
    "Procs",
    "Mem",
    "Swap",
    "Disk",
    NULL };

  char *N;
 
  int index;
 
  int *DResPtr[4];
  int *BResPtr[4];      
  int *CResPtr[4];      

  int  DVal;
  int  BVal;
  int  CVal;

  int TotalRes;

  Line[0] = '\0';

  DResPtr[0] = &DRes->Procs;
  DResPtr[1] = &DRes->Mem;
  DResPtr[2] = &DRes->Swap;
  DResPtr[3] = &DRes->Disk;

  BResPtr[0] = &BRes->Procs;
  BResPtr[1] = &BRes->Mem;
  BResPtr[2] = &BRes->Swap;
  BResPtr[3] = &BRes->Disk;

  CResPtr[0] = &CRes->Procs;
  CResPtr[1] = &CRes->Mem;
  CResPtr[2] = &CRes->Swap;
  CResPtr[3] = &CRes->Disk;

  for (index = 0;ResName[index] != NULL;index++)
    {
    DVal = *DResPtr[index];
    BVal = *BResPtr[index];      
    CVal = *CResPtr[index];      

    N   =  (char *)ResName[index];

    TotalRes = RC * ((DVal != -1) ? DVal : CVal);

    if (BVal == -1)
      BVal = TotalRes;
 
    if (TotalRes > 0)
      {
      if (Line[0] != '\0')
        MUStrCat(Line,"  ",sizeof(Line));

      sprintf(temp_str,"%s: %d/%d (%.2f%c)",
        N,
        BVal,
        TotalRes,
        (double)BVal * 100.0 / TotalRes,
        '%');
      strcat(Line,temp_str);
      }
    }  /* END for (index) */
 
  if (Line[0] == '\0')
    strcpy(Line,NONE);

  return(Line);
  }  /* END ShowCResRatio() */




int MUBMIsClear(

  int *Map,
  int  Size)

  {
  int index;

  if (Map == NULL)
    {
    return(FAILURE);
    }

  for (index = 0;index < (Size >> 5) + 1;index++)
    {
    if (Map[index] != 0)
      {
      return(FAILURE);
      }
    }  /* END for (index) */

  return(SUCCESS);
  }  /* END MUBMIsClear() */




int MUThread(

  int  (*F)(),    /* I */
  long   TimeOut, /* I */
  int   *RC,      /* O */
  int    ACount,  /* I */
  int   *Lock,    /* O */
  ...)            /* I */

  {
  int rc;

  int index;

  int MyLock;

  mut_t D;

  va_list Args;

  /* NOTE:  if Lock specified, function is non-blocking */

  if (F == NULL)
    {
    return(FAILURE);
    }

  if (Lock != NULL)
    D.Lock = Lock;
  else
    D.Lock = &MyLock;

  *D.Lock = TRUE;

  D.Func = (int (*)())F;

  if (RC != NULL)
    D.RC = RC;
  else
    D.RC = &rc;

  va_start(Args,Lock);

  for (index = 0;index < MIN(ACount,MAX_MUARG);index++)
    {
    D.Arg[index] = va_arg(Args,void *);
    }  /* END for (index) */

  va_end(Args);

  rc = __MUTFunc(&D);

  return rc;
  }  /* END MUThread() */




int __MUTFunc(

  void *V)  /* I */

  {
  mut_t *D;

  if (V == NULL)
    {
    return(FAILURE);
    }

  D = (mut_t *)V;

  *D->RC = ((int (*)(void *,void *,void *,void *,void *,void *,void *))D->Func)(
     D->Arg[0],
     D->Arg[1],
     D->Arg[2],
     D->Arg[3],
     D->Arg[4],
     D->Arg[5],
     D->Arg[6]);

  *D->Lock = FALSE;

  return(SUCCESS);
  }  /* END __MUTFunc() */


int MUMemCCmp(

  char *Data,
  char  Value,
  int   Size)

  {
  int index;

  if (Data == NULL)
    {
    return(FAILURE);
    }

  for (index = 0;index < Size;index++)
    {
    if (Data[index] != Value)
      {
      return(FAILURE);
      }
    }  /* END for (index) */

  return(SUCCESS);
  }  /* END MUMemCCmp() */




char *MUShowIArray(
 
  const char *Parm,
  int         PIndex,
  int         Val)
 
  {
  static char Line[MAX_MLINE];
 
  sprintf(Line,"%s[%1d]%-.*s  %d\n",
    Parm,
    PIndex,
    32 - 3 - (int)strlen(Parm),
    "                                ",
    Val);
 
  return(Line);
  }  /* END MUShowIArray() */
 
 
 
 
char *MUShowLArray(
 
  const char *Parm,
  int         PIndex,
  long        Val)
 
  {
  static char Line[MAX_MLINE];
 
  sprintf(Line,"%s[%1d]%-.*s  %ld\n",
    Parm,
    PIndex,
    32 - 3 - (int)strlen(Parm),
    "                                ",
    Val); 
 
  return(Line);
  }  /* END MUShowLArray() */
 
 
 
 
char *MUShowSArray(
 
  const char *Parm,
  int         PIndex,
  char       *Val)
 
  {
  static char Line[MAX_MLINE];
 
  sprintf(Line,"%s[%1d]%-.*s  %s\n",
    Parm,
    PIndex,
    32 - 3 - (int)strlen(Parm),
    "                                ",
    (Val != NULL) ? Val : "");
 
  return(Line);
  }  /* END MUShowSArray() */
 
 
 
#define MCFG_HDR_LEN 32
 
int MUShowSSArray(
 
  const char *Param,     /* I */
  char       *IndexName, /* I */
  char       *ValLine,   /* I */
  char       *Buffer)    /* I/O */
 
  { 
  sprintf(temp_str,"%s[%s]%-.*s  %s\n",
    Param,
    IndexName,
    MCFG_HDR_LEN - 2 - (int)(strlen(Param) + strlen(IndexName)),
    "                                        ",
    (ValLine != NULL) ? ValLine : "");
  strcat(Buffer,temp_str);
 
  return(SUCCESS);
  }  /* END MUShowSSArray() */
 
 
 
 
char *MUShowFArray(
 
  const char *Parm,
  int         PIndex,
  double      Val)
 
  {
  static char Line[MAX_MLINE];
 
  sprintf(Line,"%s[%1d]%-.*s  %6.2f\n",
    Parm,
    PIndex,
    32 - 3 - (int)strlen(Parm),
    "                                ",
    Val);
 
  return(Line);
  }  /* END MUShowFArray() */ 




int MUCResGetMin(
 
  mcres_t *R,
  mcres_t *Res1,
  mcres_t *Res2)
 
  {
  int index;
 
  R->Procs = MIN(Res1->Procs,Res2->Procs);
  R->Mem   = MIN(Res1->Mem,  Res2->Mem  );
  R->Disk  = MIN(Res1->Disk, Res2->Disk );
  R->Swap  = MIN(Res1->Swap, Res2->Swap );
 
  R->PSlot[0].count = 0;
 
  for (index = 1;index < MAX_MCLASS;index++)
    {
    if (MAList[eClass][index][0] == '\0')
      break;
 
    R->PSlot[index].count =  MIN(Res1->PSlot[index].count,Res2->PSlot[index].count);
    R->PSlot[0].count     += R->PSlot[index].count;
    }  /* END for (index) */
 
  for (index = 1;index < MAX_MCLASS;index++)
    {
    if (MAList[eGRes][index][0] == '\0')
      break;
 
    R->GRes[index].count =  MIN(Res1->GRes[index].count,Res2->GRes[index].count);
    R->GRes[0].count     += R->GRes[index].count;
    }  /* END for (index) */
 
  return(SUCCESS);
  }  /* END MUCResGetMin() */




int MUCResGetMax(
 
  mcres_t *R,
  mcres_t *Res1,
  mcres_t *Res2)
 
  {
  int index;
  int tmp1,tmp2;
 
  R->Procs = MAX(Res1->Procs,Res2->Procs);
  R->Mem   = MAX(Res1->Mem  ,Res2->Mem  );
  R->Disk  = MAX(Res1->Disk ,Res2->Disk );
  R->Swap  = MAX(Res1->Swap ,Res2->Swap );
 
  R->PSlot[0].count = 0;
 
  for (index = 1;index < MAX_MCLASS;index++)
    {
    if (MAList[eClass][index][0] == '\0')
      break;
 
    tmp1 = (Res1->PSlot[index].count < 32000) ? Res1->PSlot[index].count : 0;
    tmp2 = (Res2->PSlot[index].count < 32000) ? Res2->PSlot[index].count : 0;
 
    R->PSlot[index].count =  MAX(tmp1,tmp2);
    R->PSlot[0].count     += R->PSlot[index].count;
    }  /* END for (index) */
 
  for (index = 1;index < MAX_MGRES;index++)
    {
    if (MAList[eGRes][index][0] == '\0')
      break;
 
    R->GRes[index].count  = MAX(Res1->GRes[index].count,Res2->GRes[index].count);
    R->GRes[0].count     += R->GRes[index].count;
    }  /* END for (index) */
 
  return(SUCCESS);
  }  /* END MUCResGetMax() */




int MUCResIsNeg(

  mcres_t *R)  /* I */
 
  {
  if ((R->Procs < 0) ||
      (R->Mem   < 0) ||
      (R->Disk  < 0) ||
      (R->Swap  < 0))
    {
    return(SUCCESS);
    }
 
  if ((R->PSlot[0].count < 0) ||
      (R->GRes[0].count < 0))
    {
    return(SUCCESS);
    }
 
  return(FAILURE);
  }  /* END MUCResIsNeg() */




int MUBuildPList(

  mcfg_t  *C,
  char   **PList)

  {
  int cindex;

  if ((C == NULL) || (PList == NULL))
    {
    return(FAILURE);
    }

  for (cindex = 0;C[cindex].Name != NULL;cindex++)
    {
    PList[C[cindex].PIndex] = C[cindex].Name;
    }

  return(SUCCESS);
  }  /* END MUBuildPList() */




int MUShowCopy()

  {
  fprintf(stderr,"This software utilizes the Moab Scheduling Library, version %s\n",
    MOAB_VERSION);

  fprintf(stderr,"Copyright 2000-2010 Cluster Resources, Inc, All Rights Reserved\n");

  return(SUCCESS);
  }  /* END MUShowCopy() */




int MUStringUnpack(

  char *SString,  /* I */
  char *DString,  /* O */
  int   DStrLen)  /* I */

  {
  int index;

  char *SPtr;

  char  c;

  char tmpString[MAX_MLINE];

  if ((SString == NULL) || (DString == NULL))
    {
    return(FAILURE);
    }

  if (SString == DString)
    {
    if (DStrLen > MAX_MLINE)
      {
      return(FAILURE);
      }

    MUStrCpy(tmpString,SString,MAX_MLINE);

    SPtr = tmpString;
    }
  else
    {
    SPtr = SString;
    }

  if (strncmp(SPtr,"\\START",strlen("\\START")))
    {
    /* string not packed */

    MUStrCpy(DString,SPtr,DStrLen);

    return(SUCCESS);
    }
  else
    {
    SPtr += strlen("\\START");
    }
 
  index = 0;

  for (;*SPtr != '\0';SPtr++)
    {
    if (SPtr[0] != '\\')
      {
      /* perform direct copy */

      DString[index++] = *SPtr;

      if (index > DStrLen - 2)
        break;

      continue;
      }

    /* translate character */

    if (SPtr[1] >= 'a')
      c = (SPtr[1] - 'a' + 10) << 4;
    else
      c = (SPtr[1] - '0') << 4;

    if (SPtr[2] >= 'a')
      c += (SPtr[2] - 'a' + 10);
    else
      c += (SPtr[2] - '0');

    DString[index++] = c;

    SPtr += 2;
    }  /* END for (SPtr) */

  DString[index] = '\0';

  return(SUCCESS);
  }  /* END MUStringUnpack() */




int MUStrNCmpL(

  char *String,  /* I */
  char *Pattern, /* I */
  int   StrLen)  /* I */

  {
  int pindex;

  if ((String == NULL) || (Pattern == NULL))
    {
    return(FAILURE);
    }

  for (pindex = 0;Pattern[pindex] != '\0';pindex++)
    {
    if (pindex >= StrLen)
      break;

    if (tolower(String[pindex]) != tolower(Pattern[pindex]))
      {
      return(FAILURE);
      }
    }  /* END for (pindex) */

  return(SUCCESS);
  }  /* END MUStrCmpL() */




char *MUStrStrL(

  char *String,  /* I */
  char *Pattern) /* I */

  {
  int sindex;
  int pindex;

  int slen;

  char sc;
  char pc;

  if ((String == NULL) || (Pattern == NULL))
    {
    return(NULL);
    }

  slen = strlen(String); 

  for (sindex = 0;sindex < slen;sindex++)
    {
    for (pindex = 0;Pattern[pindex] != '\0';pindex++)
      {
      sc = String[sindex + pindex];
      pc = Pattern[pindex];

      if (isalpha(sc))
        { 
        if (tolower(sc) != tolower(pc))
          break;
        }
      else if (sc != pc)
        {
        break;
        }
      }  /* END for (pindex) */

    if (Pattern[pindex] == '\0')
      return(&String[sindex]);
    }  /* END for (index) */

  return(NULL);
  }  /* END MUStrStrL() */




int MUSNPrintF(

  char **BPtr,
  int   *BSpace,
  char  *Format,
  ...)

  {
  int len;
  const char *FName = "MUSNPrintF";
  va_list Args;

  if ((BPtr == NULL) || 
      (BSpace == NULL) || 
      (Format == NULL) || 
      (*BSpace <= 0))
    {
	DBG(4,fCORE) DPrint("ALERT:    Memory Error in %s\n",
	        FName);
    return(FAILURE);
    }

  va_start(Args,Format);
  
  len = vsnprintf(*BPtr,*BSpace,Format,Args);

  va_end(Args);

  if (len <= 0)
    {
	DBG(4,fCORE) DPrint("ALERT:    vsnprintf Error in %s\n",
	        FName);
    return(FAILURE);
    }

#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 1
  /* XXX: The following is true for glibc > 2.1 */

  /* if vsnprintf returns the same value as size or greater */
  /* then the output is truncated. see man vsnprintf. */
  if (len == *BSpace || len > *BSpace) {
	  DBG(1,fCORE) DPrint("ALERT:    Possible vsnprintf truncation in %s\n",
	  	        FName);
	  return(FAILURE);
  }
#endif

  *BPtr += len;
  *BSpace -= len;

  return(SUCCESS);
  }  /* END MUSNPrintf() */



int MUSNCat(char **BPtr,int *BSpace,char *Src) { return(MUStrNCat(BPtr,BSpace,Src)); }

int MUStrNCat(

  char **BPtr,
  int   *BSpace,
  char  *Src)

  {
  int index;

  if ((BPtr == NULL) || (BSpace == NULL) || (*BSpace <= 0))
    {
    return(FAILURE);
    }

  if ((Src == NULL) || (Src[0] == '\0'))
    {
    return(SUCCESS);
    }

  for (index = 0;index < *BSpace - 1;index++)
    {
    if (Src[index] == '\0')
      break;

    (*BPtr)[index] = Src[index];
    }  /* END for (index) */

  (*BPtr)[index] = '\0';

  *BPtr   += index;
  *BSpace -= index;

  return(SUCCESS);
  }  /* END MUStrNCat() */




int MUTMToHostList(

  short   *TM,       /* I */
  char   **HostList, /* O */
  mrm_t   *R)

  {
  int index;
  int tindex;

  const char *FName = "MUTMToHostList";

  DBG(4,fLL) DPrint("%s(TM,HostList,%s)\n",
    FName,
    (R != NULL) ? R->Name : "NULL");

  if ((TM == NULL) || (HostList == NULL) || (R == NULL))
    {
    return(FAILURE);
    }

  tindex = 0;

  for (index = 0;TM[index] != -1;index++)
    {
    if (MNode[TM[index]]->RM == R)
      {
      HostList[tindex] = MNode[TM[index]]->Name;

      DBG(6,fLL) DPrint("INFO:     TM[%02d] %s\n",
        tindex,
        HostList[tindex]);

      tindex++;
      }
    }    /* END for (index) */

  HostList[tindex] = NULL;

  return(SUCCESS);
  }  /* END MUTMToHostList() */




int MUIXMLSetStatus(

  mxml_t *E,       /* I (modified) */
  int      Status,  /* I */
  char    *Msg,     /* I */
  int      Code)    /* I */

  {
  mxml_t *C = NULL;

  if (E == NULL)
    {
    return(FAILURE);
    }

  if ((MXMLGetChild(E,"status",NULL,&C) == FAILURE) &&
     ((MXMLCreateE(&C,"status") == FAILURE) ||
      (MXMLAddE(E,C) == FAILURE)))
    {
    /* cannot add status child */

    return(FAILURE);
    }

  if (Status == SUCCESS)
    MXMLSetVal(C,"success",mdfString);
  else
    MXMLSetVal(C,"failure",mdfString);

  if (Msg != NULL)
    MXMLSetAttr(C,"message",(void **)Msg,mdfString);

  MXMLSetAttr(C,"code",(void **)&Code,mdfInt);
    
  return(SUCCESS);
  }  /* END MUIXMLSetStatus() */




char *MUURLCreate(

  char *Protocol,  /* I (optional) */
  char *HostName,  /* I */
  char *Directory, /* I (optional) */
  int   Port,      /* I */
  char *Buf,       /* O (optional) */
  int   BufSize)

  {
  char *Head;

  char *BPtr;
  int   BSpace;

  if (HostName == NULL)
    {
    return(NULL);
    }

  if (Buf != NULL)
    {
    BPtr   = Buf;
    BSpace = BufSize;
    }
  else if ((BPtr = calloc(MAX_MLINE, 1)) != NULL)
    {
    BSpace = BufSize;
    }
  else
    {
    return(NULL);
    }

  Head = BPtr;

  BPtr[0] = '\0';

  /* FORMAT:  [<PROTO>://]<HOST>[:<PORT>][/<DIR>] */

  if ((Protocol != NULL) && (Protocol[0] != '\0'))
    {
    MUSNPrintF(&BPtr,&BSpace,"%s://%s",
      Protocol,
      HostName);
    }
  else
    {
    MUSNPrintF(&BPtr,&BSpace,"%s",
      HostName);
    }

  if (Port > 0)
    {
    MUSNPrintF(&BPtr,&BSpace,":%d",
      Port);
    }

  if ((Directory != NULL) && (Directory[0] != '\0'))
    {
    MUSNPrintF(&BPtr,&BSpace,"%s%s",
      (Directory[0] != '/') ? "/" : "",
      Directory);
    }

  return(Head);
  }  /* END MUURLCreate() */





int MUURLParse(

  char    *URL,           /* I (modified) */
  char    *Protocol,      /* I (optional) */
  char    *HostName,      /* I */
  char    *Directory,     /* I (optional) */
  int      DirSize,       /* I */
  int     *Port,          /* I */
  mbool_t  DoInitialize)  /* I (boolean) */

  {
  char *head;
  char *tail;

  if (URL == NULL)
    {
    return(FAILURE);
    }

  if (DoInitialize == TRUE)
    {
    if (Protocol != NULL)
      Protocol[0] = '\0';

    if (HostName != NULL)
      HostName[0] = '\0';

    if (Directory != NULL)
      Directory[0] = '\0';

    if (Port != NULL)
      *Port = 0;
    }  /* END if (DoInitialize == TRUE) */

  /* FORMAT:  [<PROTO>://]<HOST>[:<PORT>][/<DIR>] */

  head = URL;

  if ((tail = strstr(head,"://")))
    {
    if (Protocol != NULL)
      MUStrCpy(Protocol,head,MIN(tail - head + 1,MAX_MNAME));
  
    head = tail + strlen("://");
    }

  if (!(tail = strchr(head,'/')) && 
      !(tail = strchr(head,':')))
    {
    tail = head + strlen(head);
    }

  if (HostName != NULL)
    MUStrCpy(HostName,head,MIN(tail - head + 1,MAX_MNAME));

  head = tail;

  if (*head == ':')
    {
    /* extract port */

    if (Port != NULL)
      *Port = (int)strtol(head + 1,&tail,0);

    head = tail;
    }
  
  if (*head == '/')
    {
    /* extract directory */

    tail = head + strlen(head);
 
    if (Directory != NULL)
      MUStrCpy(Directory,head,MIN(tail - head + 1,DirSize));

    head = tail; 
    }

  return(SUCCESS);
  }  /* END MUURLParse() */



char *MUMAToString(

  int                AttrIndex,  /* I */
  char               Delim,      /* I */
  int               *ValueMap,   /* I */
  int                MapSize)    /* I */

  {
  static char  Line[MMAX_LINE];
  int          index;
  int          findex;

  char        *ptr;

  if ((ValueMap == NULL) || (MapSize < M64.INTSIZE))
    {
    strcpy(Line,NONE);

    return(Line);
    }

  Line[0] = '\0';

  for (findex = 0;findex < (MapSize >> M64.INTSHIFT);findex++)
    {
    for (index = 0;index < M64.INTBITS;index++)
      {
      if ((ValueMap[findex] & (1 << index)) &&
          (MAList[AttrIndex][index][0] != '\0'))
        {
        ptr = MAList[AttrIndex][index + findex * M64.INTBITS];

        if (Delim != '\0')
          {
          if (Line[0] != '\0')
            {
            sprintf(temp_str,"%c%s",
              Delim,
              ptr);
            strcat(Line,temp_str);
            }
          else
            {
            strcpy(Line,ptr);
            }
          }
        else
          {
          sprintf(temp_str,"[%s]",
            ptr);
          strcat(Line,temp_str);
          }
        }
      }    /* END for (index) */
    }      /* END for (findex) */

  if (Line[0] == '\0')
    {
    strcpy(Line,NONE);
    }

  return(Line);
  }  /* END MUMAToString() */


char *MUStrStr(

  char    *String,            /* I */
  char    *Pattern,           /* I */
  int      TIndex,            /* I (optional) */
  mbool_t  IsCaseInsensitive, /* I */
  mbool_t  GetLast)           /* I */

  {
  int sindex;
  int pindex;

  int slen;
  int plen;

  char sc;
  char pc;

  if ((String == NULL) || (Pattern == NULL))
    {
    return(NULL);
    }

  /* TIndex is starting tail index when GetLast is true */

  slen = strlen(String);

  if (GetLast == FALSE)
    {
    for (sindex = 0;sindex < slen;sindex++)
      {
      for (pindex = 0;Pattern[pindex] != '\0';pindex++)
        {
        sc = String[sindex + pindex];
        pc = Pattern[pindex];

        if ((IsCaseInsensitive == TRUE) && (isalpha(sc)))
          {
          if (tolower(sc) != tolower(pc))
            break;
          }
        else if (sc != pc)
          {
          break;
          }
        }  /* END for (pindex) */

      if (Pattern[pindex] == '\0')
        {
        return(&String[sindex]);
        }
      }  /* END for (sindex) */
    }
  else
    {
    /* GetLast == TRUE */

    plen = strlen(Pattern);

    sindex = (TIndex > 0) ? TIndex - plen : slen - plen;

    for (;sindex >= 0;sindex--)
      {
      for (pindex = 0;Pattern[pindex] != '\0';pindex++)
        {
        sc = String[sindex + pindex];
        pc = Pattern[pindex];

        if ((IsCaseInsensitive == TRUE) && (isalpha(sc)))
          {
          if (tolower(sc) != tolower(pc))
            break;
           }
        else if (sc != pc)
          {
          break;
          }
        }  /* END for (pindex) */

      if (Pattern[pindex] == '\0')
        {
        return(&String[sindex]);
        }
      }  /* END for (sindex) */
    }    /* END else (GetLast == TRUE) */

  return(NULL);
  }  /* END MUStrStr() */



int MUStrNCmpCI(

  char *String,  /* I */
  char *Pattern, /* I */
  int   SStrLen) /* I (optional) */

  {
  int StrLen;

  int pindex;

   if ((String == NULL) || (Pattern == NULL))
    {
    return(FAILURE);
    }

  StrLen = (SStrLen > 0) ? SStrLen : MMAX_BUFFER;

  for (pindex = 0;pindex < StrLen;pindex++)
    {
    if (Pattern[pindex] == '\0')
      {
      if (String[pindex] != '\0')
        {
        return(FAILURE);
        }

      break;
      }

    if (tolower(String[pindex]) != tolower(Pattern[pindex]))
      {
      return(FAILURE);
      }
    }  /* END for (pindex) */

  return(SUCCESS);
  }  /* END MUStrNCmpCI() */




char *MUNumListToString(

  mnuml_t *ANL,     /* I */
  mnuml_t *CNL,     /* I (optional) */
  char    *DString, /* I (optional) */
  char    *Buf,     /* O (optional) */
  int      BufSize) /* I */

  {
  static char Line[MMAX_LINE];
  int         cindex;

  char       *Head;

  char       *BPtr;
  int         BSpace;

  if (Buf != NULL)
    {
    BPtr   = Buf;
    BSpace = BufSize;
    }
  else
    {
    BPtr   = Line;
    BSpace = sizeof(Line);
    }

  /* NOTE:  if delimiter specified, only show class name, not count */
  /* NOTE:  DString only enabled for 'no CNL specification' */

  Head    = BPtr;

  BPtr[0] = '\0';

  for (cindex = 1;cindex < MMAX_CLASS;cindex++)
    {
    if (MAList[meClass][cindex][0] == '\0')
      break;

    if (CNL != NULL)
      {
      if ((CNL[cindex].count > 0) ||
        (ANL[cindex].count > 0))
        {
        /* NOTE:  DString is ignored */

        MUSNPrintF(&BPtr,&BSpace,"[%s %d:%d]",
          MAList[meClass][cindex],
          ANL[cindex].count,
          CNL[cindex].count);
        }
      }
    else
      {
      if (ANL[cindex].count > 0)
        {
        if (DString != NULL)
          {
          /* NOTE:  do not display class count */

          if (Head[0] != '\0')
            MUStrNCat(&BPtr,&BSpace,DString);

          MUStrNCat(&BPtr,&BSpace,MAList[meClass][cindex]);
          }
        else
          {
          MUSNPrintF(&BPtr,&BSpace,"[%s %d]",
            MAList[meClass][cindex],
            ANL[cindex].count);
          }
        }
      }
    }   /* END for (cindex) */

  if (Head[0] == '\0')
    strcpy(Head,NONE);

  return(Head);
  }  /* END MUNumListToString() */




/* END MUtil.c */




