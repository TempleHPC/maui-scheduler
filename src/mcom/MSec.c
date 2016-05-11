/* HEADER */

#include "moab.h"
#include "moab-proto.h"

extern mlog_t    mlog;

extern msched_t  MSched;
extern m64_t     M64;

extern const char *MCSAlgoType[];
extern const char *MBool[];


/* DES config */

#define MAX_MDESCKSUM_ITERATION   4

unsigned short __MSecDoCRC(unsigned short,unsigned char);
int __MSecPSDES(unsigned int *,unsigned int *);

/* END DES config */



/* HMAC config */

#ifndef SHA_DIGESTSIZE
# define SHA_DIGESTSIZE  20
#endif

#ifndef SHA_BLOCKSIZE
# define SHA_BLOCKSIZE   64
#endif

typedef struct {
  MUINT4 state[5];
  MUINT4 count[2];
  unsigned char buffer[64];
  } SHA1_CTX;

typedef union
  {
  unsigned char c[64];
  MUINT4        l[16];
  } CHAR64LONG16;

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */

#if !defined(SHA1_LITTLE_ENDIAN) && !defined(SHA1_BIG_ENDIAN)
# if defined(__BYTE_ORDER)
#   if __BYTE_ORDER == __LITTLE_ENDIAN
#     define SHA1_LITTLE_ENDIAN 1
#   endif
#   if __BYTE_ORDER == __BIG_ENDIAN
#     define SHA1_BIG_ENDIAN 1
#   endif
# endif

# if !defined(SHA1_LITTLE_ENDIAN) && !defined(SHA1_BIG_ENDIAN)
#   if defined(__AIX43) || defined(__AIX51) || defined(__HPUX) || defined(__IRIX) || defined(__SOLARIS)
#     define SHA1_BIG_ENDIAN
#   else
#     define SHA1_LITTLE_ENDIAN
#   endif
# endif
#endif  /* !defined(SHA1_LITTLE_ENDIAN) && !defined(SHA1_BIG_ENDIAN) */


#if defined(SHA1_LITTLE_ENDIAN) && defined(SHA1_BIG_ENDIAN)
#error "So, which is it: big endian or little endian?"
#endif

#if defined(SHA1_LITTLE_ENDIAN)
# define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
# define SHA1_ENDIAN_KNOWN
#else
# define blk0(i) block->l[i]
# define SHA1_ENDIAN_KNOWN
#endif

#if !defined(SHA1_ENDIAN_KNOWN)
#error "ENDIANness of system not defined."
#endif

#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */

#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);


int MSecHMACGetDigest(unsigned char *,int,unsigned char *,int,char *,int,char *,mbool_t,mbool_t);

static void __MSecSHA1Transform(MUINT4 S[5],unsigned char B[64]);
void __MSecSHA1Init(SHA1_CTX *);
void __MSecSHA1Update(SHA1_CTX *,unsigned char *,MUINT4);
void __MSecSHA1Final(unsigned char D[20],SHA1_CTX *);
void __MSecHMACTruncate (char *,char *,int);

/* END HMAC config */


/* MD5 config */

#ifdef __MMD5
#include <openssl/md5.h>

#ifdef MD5_DIGEST_LENGTH
#define MMD5_DIGESTSIZE MD5_DIGEST_LENGTH
#else /* MD5_DIGEST_LENGTH */
#define MMD5_DIGESTSIZE 0
#endif /* MD5_DIGEST_LENGTH */
#else /* __MMD5 */
#define MMD5_DIGESTSIZE 0
#endif /* __MMD5 */

int MSecMD5GetDigest(char *,int,char *,int,char *,int);

/* END MD5 config */




int M64Init(

  m64_t *M)  /* I (modified) */

  {
  if (sizeof(int) == 4)
    {
    /* 32 bit operation */

    M->Is64     = FALSE;

    M->INTBITS  = M32INTBITS;
    M->INTLBITS = M32INTLBITS;
    M->INTSIZE  = M32INTSIZE;
    M->INTSHIFT = M32INTSHIFT;
    }
  else
    {
    /* 64 bit operation */

    M->Is64     = TRUE;

    M->INTBITS  = M64INTBITS;
    M->INTLBITS = M64INTLBITS;
    M->INTSIZE  = M64INTSIZE;
    M->INTSHIFT = M64INTSHIFT;
    }

  MDB(5,fSTRUCT) MLog("INFO:     64Bit enabled: %s  UINT4[%d]  UINT8[%d]\n",
    MBool[M->Is64],
    sizeof(MUINT4),
    sizeof(MUINT8));

  return(SUCCESS);
  }  /* END M64Init() */




int MSecGetChecksum(

  char *Buf,         /* I */
  int   BufSize,     /* I */
  char *Checksum,    /* O */
  char *Digest,      /* O (optional) */
  enum MChecksumAlgoEnum Algo, /* I */
  char *CSKeyString) /* I */

  {
#ifndef __MPROD
  const char *FName = "MSecGetChecksum";

  MDB(5,fSTRUCT) MLog("%s(Buf,%d,Checksum,%s,CSKey)\n",
    FName,
    BufSize,
    MCSAlgoType[Algo]);
#endif /* !__MPROD */

  if ((Buf == NULL) || (Checksum == NULL) || (CSKeyString == NULL))
    {
    return(FAILURE);
    }

  switch(Algo)
    {
    case mcsaHMAC:

      {
      MSecHMACGetDigest(
        (unsigned char *)CSKeyString,
        strlen(CSKeyString),
        (unsigned char *)Buf,
        strlen(Buf),
        Checksum,
        SHA_DIGESTSIZE,
        NULL,
        FALSE,
        FALSE);
      }    /* END BLOCK */

      break;

    case mcsaHMAC64:

      {
      MSecHMACGetDigest(
        (unsigned char *)CSKeyString,
        strlen(CSKeyString),
        (unsigned char *)Buf,
        strlen(Buf),
        Checksum,
        SHA_DIGESTSIZE,
        Digest,
        TRUE,
        TRUE);
      }  /* END BLOCK */

      break;

    case mcsaMD5:

      {
      MSecMD5GetDigest(
        CSKeyString,
        strlen(CSKeyString),
        Buf,
        strlen(Buf),
        Checksum,
        MMD5_DIGESTSIZE);
      }    /* END BLOCK */

      break;

    case mcsaRemote:

      {
      char CmdLine[MMAX_LINE];

      sprintf(CmdLine,"mauth \"%s\"",
        Buf);

#ifndef __M32COMPAT
      if (MUReadPipe(CmdLine,NULL,Checksum,MMAX_LINE) == FAILURE)
        {
        return(FAILURE);
        }
#endif /* __M32COMPAT */
      }  /* END BLOCK */

      break;

    case mcsaDES:
    default:

      {
      unsigned int crc;
      unsigned int lword;
      unsigned int rword;

      int          index;

      unsigned int SKey;

      SKey = (unsigned int)strtoul(CSKeyString,NULL,0);

      crc = 0;

      for (index = 0;index < BufSize;index++)
        {
        crc = (unsigned int)__MSecDoCRC((unsigned short)crc,Buf[index]);
        }

      lword = crc;
      rword = SKey;

      __MSecPSDES(&lword,&rword);

      Checksum[0] = '\0';

      sprintf(Checksum,"%08x%08x",
        (int)lword,
        (int)rword);
      }  /* END BLOCK */

      break;
    }  /* END switch(Algo) */

/* NOTE:  enable logging only for unit testing.  MUST DISABLE FOR PRODUCTION */

/*
   MDB(8,fSTRUCT) MLog("INFO:     checksum '%s' calculated for %d byte buffer '%s' using seed %x\n",
     Checksum,
     BufSize,
     Buf,
     SKey);
*/

  return(SUCCESS);
  }  /* END MSecGetChecksum() */




int MSecGetChecksum2(

  char          *Buf1,         /* I */
  int            BufSize1,     /* I */
  char          *Buf2,         /* I */
  int            BufSize2,     /* I */
  char          *Checksum,     /* O */
  char          *Digest,       /* O (optional) */
  enum MChecksumAlgoEnum Algo, /* I */
  char          *CSKeyString)  /* I */

  {
#ifndef __MPROD
  const char *FName = "MSecGetChecksum2";

  MDB(5,fSTRUCT) MLog("%s(Buf1,%d,Buf2,%d,Checksum,%s,CSKey)\n",
    FName,
    BufSize1,
    BufSize2,
    MCSAlgoType[Algo]);
#endif /* !__MPROD */

  if ((Buf1 == NULL) || (Buf2 == NULL) || (Checksum == NULL))
    {
    return(FAILURE);
    }

  switch(Algo)
    {
    case mcsaHMAC:

      {
      char *ptr;

      /* NOTE:  merge header and data */

      ptr = (char *)calloc(BufSize1 + BufSize2, 1);

      strcpy(ptr,Buf1);
      strcpy(ptr + BufSize1,Buf2);

      MSecHMACGetDigest(
        (unsigned char *)CSKeyString,
        strlen(CSKeyString),
        (unsigned char *)ptr,
        BufSize1 + BufSize2,
        Checksum,
        SHA_DIGESTSIZE,
        NULL,
        FALSE,
        FALSE);

      free(ptr);
      }  /* END BLOCK */

      break;

    case mcsaHMAC64:

      {
      char *ptr;

      /* NOTE:  merge header and data */


      ptr = (char *)calloc(BufSize1 + BufSize2, 1);

      strcpy(ptr,Buf1);
      strcpy(ptr + BufSize1,Buf2);

      MSecHMACGetDigest(
        (unsigned char *)CSKeyString,
        strlen(CSKeyString),
        (unsigned char *)ptr,
        strlen(ptr),
        Checksum,
        SHA_DIGESTSIZE,
        Digest,
        TRUE,
        TRUE);

      free(ptr);
      }  /* END BLOCK */

      break;

    case mcsaMD5:

      {
      char *ptr;

      /* NOTE:  merge header and data */


      ptr = (char *)calloc(BufSize1 + BufSize2, 1);

      strcpy(ptr,Buf1);
      strcpy(ptr + BufSize1,Buf2);

      MSecMD5GetDigest(
        CSKeyString,
        strlen(CSKeyString),
        ptr,
        BufSize1 + BufSize2,
        Checksum,
        MMD5_DIGESTSIZE);

      free(ptr);
      }  /* END BLOCK */

      break;

    case mcsaDES:
    default:

      {
      unsigned int crc;
      unsigned int lword;
      unsigned int rword;

      unsigned int Seed;

      int index;

      Seed = (unsigned int)strtoul(CSKeyString,NULL,0);

      crc = 0;

      for (index = 0;index < BufSize1;index++)
        {
        crc = (unsigned int)__MSecDoCRC((unsigned short)crc,Buf1[index]);
        }  /* END for (index) */

      for (index = 0;index < BufSize2;index++)
        {
        crc = (unsigned int)__MSecDoCRC((unsigned short)crc,Buf2[index]);
        }  /* END for (index) */

      lword = crc;
      rword = Seed;

      __MSecPSDES(&lword,&rword);

      sprintf(Checksum,"%08x%08x",
        (int)lword,
        (int)rword);
      }  /* END BLOCK */

      break;
    }    /* END switch(Algo) */

  return(SUCCESS);
  }  /* END MSecGetChecksum2() */




int MSecHMACGetDigest(

  unsigned char *CSKeyString,      /* I */
  int            CSKeyLen,         /* I */
  unsigned char *DataString,       /* I */
  int            DataLen,          /* I */
  char          *CSString,         /* O */
  int            MaxCSStringLen,   /* I */
  char          *DigestString,     /* O (optional) */
  mbool_t        TwoPass,          /* I */
  mbool_t        Use64BitEncoding) /* I */

  {
  SHA1_CTX ictx;
  SHA1_CTX octx;

  unsigned char isha[SHA_DIGESTSIZE];
  unsigned char osha[SHA_DIGESTSIZE];
  unsigned char key[SHA_DIGESTSIZE];
  unsigned char buf[SHA_BLOCKSIZE];
  unsigned char osha2[SHA_DIGESTSIZE];

  int     index;

  char    tmpBuf[SHA_DIGESTSIZE << 1];

  unsigned char *DPtr;

  int     DLen;

#ifndef __MPROD
  const char *FName = "MSecHMACGetDigest";

  MDB(6,fSTRUCT) MLog("%s(%s,%d,%s,%d,CSString,%d,DigestString,%s,%s)\n",
    FName,
    (CSKeyString != NULL) ? (char *)CSKeyString : "NULL",
    CSKeyLen,
    (DataString != NULL) ? (char *)DataString : "NULL",
    DataLen,
    MaxCSStringLen,
    MBool[TwoPass],
    MBool[Use64BitEncoding]);
#endif /* !__MPROD */

  /* create hash */

  if (TwoPass == TRUE)
    {
    SHA1_CTX tctx;

    __MSecSHA1Init(&tctx);
    __MSecSHA1Update(&tctx,DataString,DataLen);
    __MSecSHA1Final(osha2,&tctx);

    if (DigestString != NULL)
      {
      /* encode digest */

      if (Use64BitEncoding == TRUE)
        {
        MSecBufTo64BitEncoding(
          (char *)osha2,
          SHA_DIGESTSIZE,
          DigestString);
        }
      else
        {
        MSecBufToHexEncoding(
          (char *)osha2,
          SHA_DIGESTSIZE,
          DigestString);
        }
      }    /* END if (DigestValue != NULL) */

    DPtr = osha2;
    DLen = SHA_DIGESTSIZE;
    }   /* END if (CSKeyString == NULL) */
  else
    {
    DPtr = DataString;
    DLen = DataLen;
    }

  if (CSKeyLen > SHA_BLOCKSIZE)
    {
    SHA1_CTX tctx;

    __MSecSHA1Init(&tctx);
    __MSecSHA1Update(&tctx,CSKeyString,CSKeyLen);
    __MSecSHA1Final(key,&tctx);

    CSKeyString = key;
    CSKeyLen    = SHA_DIGESTSIZE;
    }

  /**** inner digest ****/

  __MSecSHA1Init(&ictx) ;

  /* pad the key for inner digest */

  for (index = 0;index < CSKeyLen;index++)
    {
    buf[index] = CSKeyString[index] ^ 0x36;
    }

  for (index = CSKeyLen;index < SHA_BLOCKSIZE;index++)
    {
    buf[index] = 0x36;
    }

  __MSecSHA1Update(&ictx,buf,SHA_BLOCKSIZE);

  __MSecSHA1Update(&ictx,DPtr,DLen);

  __MSecSHA1Final(isha,&ictx);

  /**** outer digest ****/

  __MSecSHA1Init(&octx) ;

  /* pad the key for outer digest */

  for (index = 0;index < CSKeyLen;index++)
    {
    buf[index] = CSKeyString[index] ^ 0x5C;
    }

  for (index = CSKeyLen;index < SHA_BLOCKSIZE;index++)
    {
    buf[index] = 0x5C;
    }

  __MSecSHA1Update(&octx,buf,SHA_BLOCKSIZE);
  __MSecSHA1Update(&octx,isha,SHA_DIGESTSIZE);

  __MSecSHA1Final(osha,&octx);

  /* truncate and encode digest */

  __MSecHMACTruncate(
    (char *)osha,
    tmpBuf,
    MIN(SHA_DIGESTSIZE,MaxCSStringLen));

  if (Use64BitEncoding == TRUE)
    {
    MSecBufTo64BitEncoding(
      tmpBuf,
      MIN(SHA_DIGESTSIZE,MaxCSStringLen),
      CSString);
    }
  else
    {
    MSecBufToHexEncoding(
      tmpBuf,
      MIN(SHA_DIGESTSIZE,MaxCSStringLen),
      CSString);
    }

  return(SUCCESS);
  }  /* END MSecHMACGetDigest() */




void __MSecHMACTruncate(

  char *d1,   /* data to be truncated */
  char *d2,   /* truncated data */
  int   len)  /* length in bytes to keep */

  {
  int i;

#ifndef __MPROD
  const char *FName = "__MSecHMACTruncate";

  MDB(9,fSTRUCT) MLog("%s(d1,d2,%d)\n",
    FName,
    len);
#endif /* !__MPROD */

  for (i = 0;i < len;i++)
    {
    d2[i] = d1[i];
    }  /* END for (i) */

  return;
  }  /* END __MSecHMACTruncate() */





static void __MSecSHA1Transform(

  MUINT4        state[5],    /* I */
  unsigned char buffer[64])  /* I */

  {
  /* hash a single 512-bit block. This is the core of the algorithm. */

  MUINT4 a,b,c,d,e;

  unsigned char workspace[64];

  CHAR64LONG16 *block = (CHAR64LONG16*)workspace;

#ifndef __BMPROD
  const char *FName = "__MSecSHA1Transform";

  MDB(9,fSTRUCT) MLog("%s(context)\n",
    FName);
#endif /* !__BMPROD */

  memcpy(block,buffer,64);

  /* copy context->state[] to working vars */

  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];

  /* 4 rounds of 20 operations each. Loop unrolled. */

  R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
  R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
  R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
  R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
  R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
  R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
  R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
  R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
  R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
  R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
  R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
  R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
  R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
  R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
  R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
  R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
  R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
  R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
  R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
  R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

  /* add the working vars back into context.state[] */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;

  /* clear variables */

  a = b = c = d = e = 0;

  memset(block,0,64);

  return;
  }  /* END __MSecSHA1Transform() */




void __MSecSHA1Init(

  SHA1_CTX* context)  /* O */

  {
#ifndef __MPROD
  const char *FName = "__MSecSHA1Init";

  MDB(9,fSTRUCT) MLog("%s(context)\n",
    FName);
#endif /* !__MPROD */

  /* initialize new context */

  /* SHA1 initialization constants */

  context->state[0] = 0x67452301;
  context->state[1] = 0xEFCDAB89;
  context->state[2] = 0x98BADCFE;
  context->state[3] = 0x10325476;
  context->state[4] = 0xC3D2E1F0;
  context->count[0] = context->count[1] = 0;

  return;
  }  /* END __MSecSHA1Init() */





void __MSecSHA1Update(

  SHA1_CTX      *Context,
  unsigned char *Data,
  MUINT4         DataLen)

  {
  MUINT4 i,j;

#ifndef __MPROD
  const char *FName = "__MSecSHA1Update";

  MDB(10,fSTRUCT) MLog("%s(Context,Data,DataLen)\n",
    FName);
#endif /* !__MPROD */

  j = (Context->count[0] >> 3) & 63;

  if ((Context->count[0] += DataLen << 3) < (DataLen << 3)) 
    Context->count[1]++;

  Context->count[1] += (DataLen >> 29);

  if ((j + DataLen) > 63)
    {
    i = 64 - j;

    memcpy(&Context->buffer[j],Data,i);

    __MSecSHA1Transform(Context->state,Context->buffer);

    for (;i + 63 < DataLen;i += 64)
      {
      __MSecSHA1Transform(Context->state, &Data[i]);
      }  /* END for() */

    j = 0;
    }
  else
    {
    i = 0;
    }

  memcpy(&Context->buffer[j],&Data[i],DataLen - i);

  return;
  }  /* END __MSecSHA1Update() */





void __MSecSHA1Final(

  unsigned char  digest[20],
  SHA1_CTX      *context)

  {
  /* add padding and return the message digest. */

  MUINT4 i;
  unsigned char finalcount[8];

#ifndef __MPROD
  const char *FName = "__MSecSHA1Final";

  MDB(9,fSTRUCT) MLog("%s(digest,context)\n",
    FName);
#endif /* !__MPROD */

  for (i = 0;i < 8;i++)
    {
    finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
       >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
    }  /* END for (i) */

  __MSecSHA1Update(context,(unsigned char *)"\200",1);

  while ((context->count[0] & 504) != 448)
    {
    __MSecSHA1Update(context, (unsigned char *)"\0", 1);
    }

  __MSecSHA1Update(context,finalcount,8);  

  for (i = 0;i < 20;i++)
    {
    digest[i] = (unsigned char)
         ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
    }  /* END for (i) */

  /* reset variables */

  i = 0;

  memset(context->buffer,0,64);
  memset(context->state,0,20);
  memset(context->count,0,8);
  memset(finalcount,0,8);

  return;
  }  /* END __MSecSHA1Final() */




int MSecMD5GetDigest(

  char *CSKeyString,    /* I */
  int   CSKeyLen,       /* I */
  char *DataString,     /* I */
  int   DataLen,        /* I */
  char *CSString,       /* O */
  int   MaxCSStringLen) /* O */

  {
  int index;

  char tmpBuf[MMAX_BUFFER];

  char MD5Sum[MMD5_DIGESTSIZE + 2];

  char *ptr = NULL;

#ifndef __MPROD
  const char *FName = "MSecMD5GetDigest";

  MDB(2,fSTRUCT) MLog("%s(CSKeyString,CSKeyLen,DString,DLen,CSString,MaxCSLen)\n",
    FName);
#endif /* !__MPROD */

  if ((CSKeyString == NULL) ||
      (DataString == NULL) ||
      (CSString == NULL))
    {
    return(FAILURE);
    }

  /* NOTE:  MD5(SrcBuf,strlen(SrcBuf),tmpLine); */

  CSString[0] = '\0';

  sprintf(tmpBuf,"%s%s",
    DataString,
    CSKeyString);

#ifdef __MMD5
  ptr = MD5(
    (unsigned char *)tmpBuf,
    (unsigned long)strlen(tmpBuf),
    MD5Sum);
#endif /* __MMD5 */

  if (ptr == NULL)
    {
    return(FAILURE);
    }

  for (index = 0;index < MMD5_DIGESTSIZE;index++)
    {
    sprintf(temp_str,"%02x",
      (unsigned char)MD5Sum[index]);
    strcat(CSString,temp_str);
    }  /* END for (index) */

  strcat(CSString,"\n");

  return(SUCCESS);
  }  /* END MSecMD5GetDigest() */




int __MSecPSDES(

  unsigned int *lword,
  unsigned int *irword)

  {
  int index;

  unsigned int ia;
  unsigned int ib;
  unsigned int iswap;
  unsigned int itmph;
  unsigned int itmpl;

  static unsigned int c1[MAX_MDESCKSUM_ITERATION] = {
    (unsigned int)0xcba4e531,
    (unsigned int)0x537158eb,
    (unsigned int)0x145cdc3c,
    (unsigned int)0x0d3fdeb2 };

  static unsigned int c2[MAX_MDESCKSUM_ITERATION] = {
    (unsigned int)0x12be4590,
    (unsigned int)0xab54ce58,
    (unsigned int)0x6954c7a6,
    (unsigned int)0x15a2ca46 };

  itmph = 0;
  itmpl = 0;

  for (index = 0;index < MAX_MDESCKSUM_ITERATION;index++)
    {
    iswap = *irword;

    ia    = iswap ^ c1[index];

    itmpl = ia & 0xffff;
    itmph = ia >> 16;

    ib    = (itmpl * itmpl) + ~(itmph * itmph);
    ia    = (ib >> 16) | ((ib & 0xffff) << 16);

    *irword = (*lword) ^ ((ia ^ c2[index]) + (itmpl * itmph));

    *lword = iswap;
    }

  return(SUCCESS);
  }  /* END __MSecPSDES() */




unsigned short __MSecDoCRC(

  unsigned short crc,
  unsigned char  val)

  {
  int          index;

  unsigned int ans;

  ans = (crc ^ val << 8);

  for (index = 0;index < 8;index++)
    {
    if (ans & 0x8000)
      ans = (ans << 1) ^ 4129;
    else
      ans <<= 1;
    }  /* END for (index) */

  return((unsigned short)ans);
  }  /* END __MSecDoCRC() */




int MSecTestChecksum(

  char *TLine)  /* I */

  {
  char *CSKey;
  char *Msg;
  char *AType;

  char *TokPtr;

  char CSBuf[MMAX_LINE];
  char Digest[MMAX_LINE];

  int  rc;

  enum MChecksumAlgoEnum AIndex;

  if (TLine == NULL)
    {
    return(FAILURE);
    }

  /* FORMAT:  <KEY>,<ALGO>,<MSG> */

  CSKey = MUStrTok(TLine,",",&TokPtr);
  AType = MUStrTok(NULL,",",&TokPtr);
  Msg   = MUStrTok(NULL,"\n",&TokPtr);

  AIndex = (enum MChecksumAlgoEnum)MUGetIndex(AType,MCSAlgoType,FALSE,0);

  Digest[0] = '\0';

  rc = MSecGetChecksum(
    Msg,
    strlen(Msg),
    CSBuf,
    Digest,
    AIndex,
    CSKey);

  fprintf(stderr,"CS: '%s'/'%s'\n",
    CSBuf,
    Digest);

  exit(rc);

  /*NOTREACHED*/

  return(SUCCESS);
  }  /* END MSecTestChecksum() */




int MSecBufToHexEncoding(

  char *IBuf,     /* I */
  int   IBufLen,  /* I */
  char *OBuf)     /* O */

  {
  int index;

  if ((IBuf == NULL) || (OBuf == NULL))
    {
    return(SUCCESS);
    }

  OBuf[0] = '\0';

  for (index = 0;index < IBufLen;index++)
    {
    sprintf(temp_str,"%02x",
      (unsigned char)IBuf[index]);
    strcat(OBuf,temp_str);
    }  /* END for (index) */

  return(SUCCESS);
  }  /* END MSecBufToHexEncoding() */




static char CList[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char CDList[256] = {
  00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,
  00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,
  00,00,00,00, 00,00,00,00, 00,00,00,62, 00,00,00,63,
  52,53,54,55, 56,57,58,59, 60,61,00,00, 00,00,00,00,
  0, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
  15,16,17,18, 19,20,21,22, 23,24,25,00, 00,00,00,00,
  00,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
  41,42,43,44, 45,46,47,48, 49,50,51,00, 00,00,00,00,
  00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,
  00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,
  00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,
  00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,
  00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,
  00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,
  00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,
  00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00, };


int MSecBufTo64BitEncoding(

  char *IBuf,     /* I */
  int   IBufLen,  /* I */
  char *OBuf)     /* O */

  {
  int index;
  int cindex;

  int block;
  int slack;
  int end;

  char tmpBuf[5];

  short val;

  if ((IBuf == NULL) || (OBuf == NULL))
    {
    return(SUCCESS);
    }

  tmpBuf[4] = '\0';

  OBuf[0] = '\0';

  for (index = 0;index < IBufLen;index += 3)
    {
    block = 0;
    slack = IBufLen - index - 1;
    end   = MIN(2,slack);

    for (cindex = 0;cindex <= end;cindex++)
      {
      block += 
        ((unsigned char)IBuf[index + cindex] << (8 * (2 - cindex)));
      }  /* END for (cindex) */

    for (cindex = 0;cindex < 4;cindex++)
      {
      val = (block >> (6 * (3 - cindex))) & 0x003f;

      tmpBuf[cindex] = CList[val];
      }  /* END for (cindex) */

    if (slack < 2)
      {
      tmpBuf[3] = '=';

      if (slack < 1) 
        tmpBuf[2] = '=';
      }

    strcat(OBuf,tmpBuf);
    }  /* END for (index) */

  return(SUCCESS);
  }  /* END MSecBufTo64BitEncoding() */





int MSecCompBufTo64BitEncoding(

  char *IBuf,
  int   IBufLen,
  char *OBuf)     /* size = X */

  {
  int IIndex = 0;
  int OIndex = 0;

  if ((OBuf == NULL) || (IBuf == NULL))
    {
    return(FAILURE);
    }

  do
    {
    OBuf[OIndex++] = CList[(IBuf[IIndex] >> 2) & 0x3f];

    OBuf[OIndex++] = CList[((IBuf[IIndex] << 4) & 0x30) | ((IBuf[IIndex + 1] >> 4) & 0x0f)];

    if (IIndex + 1 < IBufLen)
      OBuf[OIndex++] = CList[((IBuf[IIndex + 1] << 2) & 0x3c) | ((IBuf[IIndex + 2] >> 6) & 0x03)];
    else
      OBuf[OIndex++] = '=';

    if (IIndex + 2 < IBufLen)
      OBuf[OIndex++] = CList[((IBuf[IIndex + 2]) & 0x3f)];
    else
      OBuf[OIndex++] = '=';

    IIndex += 3;
    } while (IIndex < IBufLen);

  OBuf[OIndex++] = '\0';

  return(SUCCESS);
  }  /* MSecCompBufTo64BitEncoding() */




int MSecComp64BitToBufDecoding(

  char *IBuf,
  int   IBufLen,
  char *OBuf,
  int  *OBufLen)

  {
  int IIndex = 0;
  int OIndex = 0;

  if ((OBuf == NULL) || (IBuf == NULL))
    {
    return(FAILURE);
    }

  do
    {
    OBuf[OIndex++] = ((CDList[(int)IBuf[IIndex]] << 2 ) & 0xfc ) | ((CDList[(int)IBuf[IIndex+1]] >> 4 ) & 0x03 );

    OBuf[OIndex++] = ((CDList[(int)IBuf[IIndex+1]] << 4 ) & 0xf0 ) | ((CDList[(int)IBuf[IIndex+2]] >> 2 ) & 0x0f );

    OBuf[OIndex++] = ((CDList[(int)IBuf[IIndex+2]] << 6 ) & 0xc0 ) | ((CDList[(int)IBuf[IIndex+3]]));

    IIndex += 4;
    } while (IIndex < IBufLen);

  if (OBufLen != NULL)
    *OBufLen = OIndex;

  return(SUCCESS);
  }   /* MSecComp64BitToBufDecoding() */




#define N               16
#define noErr            0
#define DATAERROR         -1
#define KEYBYTES         8
#define subkeyfilename   "Blowfish.dat"

unsigned long P[N + 2];
unsigned long S[4][256];
FILE*         SubkeyFile;

short __MSecBlowfishOpenSubkeyFile(void) /* read only */

  {
  short error;

  error = noErr;

  if ((SubkeyFile = fopen(subkeyfilename,"rb")) == NULL) 
    {
    error = DATAERROR;
    }

  return(error);
  }  /* END __MSecBlowfishOpenSubkeyFile() */





unsigned long __MSecBlowfishF(

  mulong x)  /* I */

  {
  unsigned short a;
  unsigned short b;
  unsigned short c;
  unsigned short d;
  unsigned long  y;

  d = x & 0x00FF;
  x >>= 8;

  c = x & 0x00FF;
  x >>= 8;

  b = x & 0x00FF;
  x >>= 8;

  a = x & 0x00FF;

  /* y = ((S[0][a] + S[1][b]) ^ S[2][c]) + S[3][d]; */

  y  = S[0][a] + S[1][b];
  y ^= S[2][c];
  y += S[3][d];

  return(y);
  }  /* END __MSecBlowfishF() */




void MSecBlowfishEncipher(

  mulong *xl,  /* I (modified) */
  mulong *xr)  /* I (modified) */

  {
  unsigned long  Xl;
  unsigned long  Xr;
  unsigned long  temp;
  short          i;

  Xl = *xl;
  Xr = *xr;

  for (i = 0;i < N;++i) 
    {
    Xl = Xl ^ P[i];
    Xr = __MSecBlowfishF(Xl) ^ Xr;

    temp = Xl;
    Xl = Xr;
    Xr = temp;
    }  /* END for (i) */

  temp = Xl;
  Xl = Xr;
  Xr = temp;

  Xr = Xr ^ P[N];
  Xl = Xl ^ P[N + 1];

  *xl = Xl;
  *xr = Xr;

  return;
  }  /* END MSecBlowfishEncipher() */




void MSecBlowfishDecipher(

  mulong *xl,  /* I (modified) */
  mulong *xr)  /* I (modified) */

  {
  unsigned long  Xl;
  unsigned long  Xr;
  unsigned long  temp;
  short          i;

  Xl = *xl;
  Xr = *xr;

  for (i = N + 1; i > 1; --i) 
    {
    Xl = Xl ^ P[i];
    Xr = __MSecBlowfishF(Xl) ^ Xr;

    /* Exchange Xl and Xr */

    temp = Xl;
    Xl = Xr;
    Xr = temp;
    }  /* END for (i) */

  /* Exchange Xl and Xr */

  temp = Xl;
  Xl = Xr;
  Xr = temp;

  Xr = Xr ^ P[1];
  Xl = Xl ^ P[0];

  *xl = Xl;
  *xr = Xr;

  return;
  }  /* END MSecBlowfishDecipher() */




short MSecBlowfishInitialize(

  char *Key,      /* I */
  short KeyBytes) /* I */

  {
  short          i;
  short          j;
  short          k;
  short          error;
  short          numread;
  unsigned long  data;
  unsigned long  datal;
  unsigned long  datar;

  /* First, open the file containing the array initialization data */

  error = __MSecBlowfishOpenSubkeyFile();

  if (error == noErr) 
    {
    for (i = 0;i < N + 2;++i) 
      {
      numread = fread(&data,4,1,SubkeyFile);

#ifdef little_endian      /* Eg: Intel   We want to process things in byte   */
                          /*   order, not as rearranged in a longword          */
      data = ((data & 0xFF000000) >> 24) |
             ((data & 0x00FF0000) >>  8) |
             ((data & 0x0000FF00) <<  8) |
             ((data & 0x000000FF) << 24);
#endif

      if (numread != 1) 
        {
        return(DATAERROR);
        } 
      else 
        {
        P[i] = data;
        }
      }    /* END for (i) */

    for (i = 0;i < 4;++i) 
      {
      for (j = 0;j < 256;++j) 
        {
        numread = fread(&data,4,1,SubkeyFile);

#ifdef little_endian      /* Eg: Intel   We want to process things in byte   */
                          /*   order, not as rearranged in a longword          */
        data = ((data & 0xFF000000) >> 24) |
               ((data & 0x00FF0000) >>  8) |
               ((data & 0x0000FF00) <<  8) |
               ((data & 0x000000FF) << 24);
#endif

        if (numread != 1) 
          {
          return(DATAERROR);
          } 
        else 
          {
          S[i][j] = data;
          }
        }
      }

    fclose(SubkeyFile);

    j = 0;

    for (i = 0;i < N + 2;++i) 
      {
      data = 0x00000000;

      for (k = 0;k < 4;++k) 
        {
        data = (data << 8) | Key[j];

        j = j + 1;

        if (j >= KeyBytes) 
          {
          j = 0;
          }
        }

      P[i] = P[i] ^ data;
      }  /* END for (i) */

    datal = 0x00000000;
    datar = 0x00000000;

    for (i = 0;i < N + 2;i += 2) 
      {
      MSecBlowfishEncipher(&datal,&datar);

      P[i] = datal;
      P[i + 1] = datar;
      }

    for (i = 0;i < 4;++i) 
      {
      for (j = 0;j < 256;j += 2) 
        {
        MSecBlowfishEncipher(&datal,&datar);

        S[i][j] = datal;
        S[i][j + 1] = datar;
        }
      }
    } 
  else 
    {
    printf("Unable to open subkey initialization file : %d\n", 
      error);
    }

  return(error);
  }  /* END MSecBlowfishInitialize() */





#define   FLAG_Copied         0x80
#define   FLAG_Compress       0x40

int MSecCompressionGetMatch(

  unsigned char *Source,
  unsigned int   X,
  unsigned int   SourceSize,
  int           *Hash,
  unsigned int  *Size,
  int           *Pos)  /* O */

  {
  unsigned int HashValue = (40543L*((((Source[X] << 4) ^ Source[X+1]) << 4) ^ Source[X+2]) >> 4) & 0xfff;

  *Pos = Hash[HashValue];

  Hash[HashValue] = X;

  if ((*Pos != -1) && ((X - *Pos) < 4096))
    {
    for (*Size = 0;
       ((*Size < 18) && 
        (Source[X + *Size] == Source[*Pos + *Size]) && 
       ((X + *Size) < SourceSize)); 
        (*Size)++);
      
    return(*Size >= 3);
    }

  return(FALSE);
  }  /* END MSecCompressionGetMatch() */





int MSecCompress(

  unsigned char *SrcString,        /* I */
  unsigned int   SrcSize,          /* I */
  unsigned char *DstString,        /* O (optional,if NULL, populate SrcString) */
  char          *EKey)             /* I (optional - enables encryption when present) */

  {
  static char *LocalDstString = NULL;
  static int   LocalDstSize   = 0;

  char *Dest;

  int Hash[4096];

  int NewLength;

  unsigned int Key;
  unsigned int Size;
  unsigned int Pos;
  unsigned int Command = 0;
  unsigned int X = 0;
  unsigned int Y = 9;
  unsigned int Z = 3;

  unsigned char Bit = 0;

#ifndef __MPROD
  const char *FName = "MSecCompress";

  MDB(2,fSTRUCT) MLog("%s(SrcString,%d,DstString,%s)\n",
    FName,
    SrcSize,
    (EKey != NULL) ? "EKey" : "NULL");
#endif  /* !__MPROD */

  NewLength = (int)(ceil(SrcSize / 3.0) * 4) + 1;

  MDB(7,fSTRUCT) MLog("INFO:     compressing data (Original Size - %d)\n",
    SrcSize);

  NewLength = (int)(ceil(SrcSize / 3.0) * 4) + 1;

  if (DstString == NULL)
    {
    /* use static local buffer */

    if (LocalDstString == NULL)
      {
      LocalDstString = (char *)calloc(NewLength, 1);

      LocalDstSize   = NewLength;
      }
    else if (LocalDstSize < NewLength)
      {
      LocalDstString = (char *)realloc(LocalDstString,NewLength);

      LocalDstSize = NewLength;
      }

    Dest = LocalDstString;
    }
  else
    {
    Dest = (char *)DstString;
    }

  for (Key = 0;Key < 4096;Key++)
    Hash[Key] = -1;

  Dest[0] = FLAG_Compress;

  /* Y = 9, initialize Dest up Y */

  Dest[1] = '\0';
  Dest[2] = '\0';
  Dest[3] = '\0';
  Dest[4] = '\0';
  Dest[5] = '\0';
  Dest[6] = '\0';
  Dest[7] = '\0';
  Dest[8] = '\0';
  Dest[9] = '\0';

  for (;(X < SrcSize) && (Y <= SrcSize);)
    {
    if (Bit > 15)
      {
      Dest[Z++] = (Command >> 8) & 0x00ff;
      Dest[Z] = Command & 0x00ff;
      Z = Y;
      Bit = 0;
      Y += 2;
      }

    for (Size = 1;
        (SrcString[X] == SrcString[X + Size]) && (Size < 0x0fff) && (X + Size < SrcSize); 
         Size++);

    if (Size >= 16)
      {
      Dest[Y++] = 0;
      Dest[Y++] = ((Size - 16) >> 8) & 0x00ff;
      Dest[Y++] = (Size - 16) & 0x00ff;
      Dest[Y++] = SrcString[X];
      X += Size;
      Command = (Command << 1) + 1;
      }
    else if (MSecCompressionGetMatch(SrcString,X,SrcSize,Hash,&Size,(int *)&Pos))
      {
      Key = ((X-Pos) << 4) + (Size - 3);
      Dest[Y++] = (Key >> 8) & 0x00ff;
      Dest[Y++] = Key & 0x00ff;
      X += Size;
      Command = (Command << 1) + 1;
      }
    else
      {
      Dest[Y++] = SrcString[X++];
      Command = (Command << 1);
      }

    Bit++;
    }  /* END for (X) */

  if (Dest == LocalDstString)
    {
    /* pad end of data */

    Dest[Y] = '\0';
    Dest[Y + 1] = '\0';
    }

  Command <<= (16 - Bit);

  Dest[Z++] = (Command >> 8) & 0x00ff;
  Dest[Z] = Command & 0x00ff;

  if (Y > SrcSize)
    {
    for (Y = 0;Y < SrcSize;Dest[Y + 1] = SrcString[Y++]);

    Dest[0] = FLAG_Copied;

    return(SrcSize + 1);
    }

  if (EKey != NULL)
    {
    MSecEncryption(Dest,EKey,Y);
    }

  MSecCompBufTo64BitEncoding(
    Dest,
    Y,
    (DstString == NULL) ? (char *)SrcString : (char *)DstString);

  MDB(7,fSTRUCT) MLog("INFO:     compressed data size - %d\n",
    (DstString == NULL) ? strlen((char *)SrcString) : strlen((char *)DstString));
 
  return(SUCCESS);
  }  /* END MSecCompress() */





int MSecDecompress(

  unsigned char  *SrcString,           /* I */
  unsigned int    SrcSize,             /* I */
  unsigned char  *SDstBuf,             /* I (optional) */
  unsigned int    SDstSize,            /* I (required if DstBuf set) */
  unsigned char **DDstBuf,             /* O (optional) */
  char           *EKey)                /* I (optional - encryption enabled when present) */

  {
  static unsigned char *tmpBuf = NULL;
  static unsigned char *OutBuf = NULL;

  static int OutBufSize = 0;

  unsigned int DstSize;

  unsigned int NewLength;

  unsigned int X = 9;
  unsigned int Y = 0;
  unsigned int Pos;
  unsigned int Size;
  unsigned int K;
  int Command;

  unsigned char *DstString;

  unsigned char  Bit = 16;

  if ((SrcString == NULL) || (SrcSize <= 0))
    {
    return(FAILURE);
    }

  if ((SDstBuf != NULL) && (SDstSize > 0))
    {
    DstString = SDstBuf;
    DstSize   = SDstSize;
    }
  else if (DDstBuf != NULL)
    {
    if ((OutBuf == NULL) && 
       ((OutBuf = (unsigned char *)calloc(MMAX_BUFFER, 1)) == NULL))
      {
      return(FAILURE);
      }
    else
      {
      OutBufSize = MMAX_BUFFER;
      }

    *DDstBuf = NULL;

    DstString = OutBuf;
    DstSize   = OutBufSize;
    }
  else
    {
    return(FAILURE);
    }

  DstString[0] = '\0';

  NewLength = SrcSize;

  if (tmpBuf == NULL)
    {
    if ((tmpBuf = (unsigned char *)calloc(NewLength, 1)) == NULL)
      {
      return(FAILURE);
      }
    }
  else
    {
    if (sizeof(tmpBuf) < NewLength)
      {
      if ((tmpBuf = (unsigned char *)realloc(tmpBuf,NewLength)) == NULL)
        {
        return(FAILURE);
        }
      }
    }

  tmpBuf[0] = '\0';

  MSecComp64BitToBufDecoding((char *)SrcString,(int)SrcSize,(char *)tmpBuf,(int *)&SrcSize);

  if (EKey != NULL)
    MSecEncryption((char *)tmpBuf,EKey,SrcSize);

  Command = (tmpBuf[3] << 8) + tmpBuf[4];

/*
  if (tmpBuf[0] == FLAG_Copied)
    {
    for (Y = 1; Y < SrcSize; DstString[Y-1] = tmpBuf[Y++]);

    return(SrcSize-1);
    }
*/

  for (;X < SrcSize;)
    {
    if (Bit == 0)
      {
      Command = (tmpBuf[X++] << 8);

      Command += tmpBuf[X++];
      Bit = 16;
      }

    if (Command & 0x8000)
      {
      Pos = (tmpBuf[X++] << 4);
      Pos += (tmpBuf[X] >> 4);

      if (Pos)
        {
        Size = (tmpBuf[X++] & 0x0f) + 3;

        if ((Y + Size) > DstSize)
          {
          /* bounds check */

          if (DDstBuf != NULL)
            {
            DstSize <<= 1;

            if ((OutBuf = (unsigned char *)realloc(OutBuf,DstSize)) == NULL)
              {
              return(FAILURE);
              }

            DstString = OutBuf;
            }
          else
            {
            return(FAILURE);
            }
          }

        for (K = 0; K < Size;K++)
          {
          DstString[Y + K] = DstString[Y - Pos + K];
          }

        Y += Size;
        }
      else
        {
        Size = (tmpBuf[X++] << 8);
        Size += tmpBuf[X++] + 16;

        if ((Y + Size) > DstSize)
          {
          /* bounds check */

          if (DDstBuf != NULL)
            {
            DstSize <<= 1;

            if ((OutBuf = (unsigned char *)realloc(OutBuf,DstSize)) == NULL)
              {
              return(FAILURE);
              }

            DstString = OutBuf;
            }
          else
            {
            return(FAILURE);
            }
          }

        for (K = 0;K < Size;DstString[Y + K++] = tmpBuf[X]);

        X++;
        Y += Size;
        }
      }
    else
      {
      if (Y >= DstSize)
        {
        /* bounds check */

        if (DDstBuf != NULL)
          {
          DstSize <<= 1;

          if ((OutBuf = (unsigned char *)realloc(OutBuf,DstSize)) == NULL)
            {
            return(FAILURE);
            }

          DstString = OutBuf;
          }
        else
          {
          return(FAILURE);
          }
        }

      DstString[Y++] = tmpBuf[X++];
      }  /* END else () */

    Command <<= 1;
    Bit--;
    }  /* END for() */

  /* terminate buffer */

  if (Y < DstSize)
    OutBuf[Y] = '\0';

  if (DDstBuf != NULL)
    {
    *DDstBuf = OutBuf;
    }

  return(SUCCESS);
  }   /* END MSecDecompress() */




int  MSecEncryption(

  char *SrcString,
  char *Key,
  int SrcSize)

  {
  int  r;
  char  *cp_val;
  char  *cp_key;
  char  result;

  r = 0;

  if((SrcString != NULL) && (Key != NULL))
    {
    cp_val = SrcString;
    cp_key = Key;

    while(r < SrcSize)
      {
      result = *cp_val ^ *cp_key;
      *cp_val = result;
      cp_val++;
      cp_key++;
      if(*cp_key == '\0') cp_key = Key;
      r++;
      }
    }

  return(SUCCESS);
  }     /* END MSecEncryption() */

/* END MSec.c */

