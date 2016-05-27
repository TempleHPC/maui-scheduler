/* HEADER */

/* NOTE:  requires library 'msu' and include file 'msu.h' */

#if !defined(__G2_H) 
#define __G2_H

#define MG2_VERSION "4.2.0p0"

#if defined(__MTEST)
#define SUEMLog fprintf
#endif /* __MTEST */


/* sync w/MG2MCList[] */

enum MG2MCListEnum {
  mg2mclNONE = 0,
  mg2mclHostList,
  mg2mclState,
  mg2mclSJID };


enum G2MetaCmdEnum {
  g2mcNONE = 0,
  g2mcInitialize,
  g2mcCommit,
  g2mcList,
  g2mcRsvCreate,
  g2mcRemove,
  g2mcQuery,
  g2mcSubmit,
  g2mcModify,
  g2mcResetStats,
  g2mcRegister };

/* Callback types (sync w/MG2CBType,MG2DEF_CBMASK) */

enum MG2CallBackEnum {
  mg2cbNONE = 0,
  mg2cbRsvCreate,
  mg2cbRsvStart,
  mg2cbRsvEnd,
  mg2cbRsvDestroy,
  mg2cbJobCreate,
  mg2cbJobStart,
  mg2cbJobEnd,
  mg2cbJobDestroy };

#if !defined(__G2INTERNAL_H)

#include "msu.h" 

enum { g2ptNONE = 0, g2ptMoab, g2ptSilver };

enum { g2isNONE = 0, g2isFile };

#define __MINTERFACE /* FIXME */

#ifdef __MINTERFACE
#if !defined(__MX)
#define __MX
#endif /* __MX */

#include "moab.h"
#include "moab-proto.h"

typedef struct
  {
  char        *Name;

  mpar_t      *MPar;
  mnode_t    **MNode;
  msched_t    *Sched;
  mfsc_t      *FS;
  mjob_t     **Job;
  mres_t     **Res;
  mlog_t      *dlog;
  mrange_t    *MRange;
  mrm_t       *MRM;
  mam_t       *AM;
  mattrlist_t *AttrList;
  long        *CREndTime;
  char        *CurrentHostName;
  time_t      *PresentTime;

  int         (*G2RsvFind)();
  int         (*G2ResSetAttr)();
  int         (*G2AcctFind)();
  int         (*G2JobFind)();
  int         (*G2JobAddCreds)();
  int         (*G2JobGetResourceAvailability)();
  int         (*G2ReservationCancel)();
  int         (*G2RMJobCancel)();
  int         (*G2WikiJobLoad)();
  int         (*G2JobAllocateNodes)();
  int         (*G2JobDistributeTasks)();
  int         (*G2JobNameAdjust)();
  int         (*G2ReservationCreate)();
  int         (*G2SimJobSubmit)();
  int         (*G2RMJobSubmit)();
  } m_base_t;

#else /* __MINTERFACE */

typdef struct
  {
  char       *Name;
  } m_base_t;

#endif /* __MINTERFACE */


enum G2OEnum {
  g2oNONE,
  g2oAccount,
  g2oClass,
  g2oGroup,
  g2oJob,
  g2oNode,
  g2oQOS,
  g2oReq,
  g2oRes,
  g2oSystem,
  g2oUser };

/* suballocation policies */

enum {
  G2sapNONE = 0,
  G2sapFirstFit,
  G2sapBestFit,
  G2sapWorstFit,
  G2sapBestCRFit,
  G2sapWorstCRFit,
  G2sapBalanceAFit,
  G2sapBalanceCFit,
  G2sapBalanceARFit,
  G2sapBalanceARRFit,
  G2sapBalanceARSFit };

typedef struct
  {
  int            Type;
  int            ChargeType;
 
  unsigned long  Flags;
 
  double HourOfDayChargeFactor[24];
  double DayOfWeekChargeFactor[7];
  double QOSChargeFactor[100];
  double NodeTypeChargeFactor[100];
  } g2am_t;

typedef struct {
  su_t *su;

  m_base_t *m;

  int     SubAllocPolicy;          
  int     RMShowErrMsg;
  int     RMMaxNMThreadCount;

  g2am_t *AM;
 
  sufilebuf_t *GlobalCfgFBuf;
  sufilebuf_t *PrivateCfgFBuf;

  int   MetaUseAccountMasq;
  char  MetaAccountMasqBaseName[MAX_SUNAME];
  int   MetaAccountMasqCount;
  } G2_t;

typedef struct {
  char *UserName;
  char *GroupName;
  char *AccountName;
  } g2cred_t;

typedef struct {
  char *Arch;
  char *Features;
  int   NodeMem;
  char  NodeMemCmp;
  int   NodeProcs;
  char  NodeProcsCmp;
  char *OS;
  } g2rspec_t;

typedef struct {
  int   Disk;
  int   Mem;
  char *Network;
  int   Procs;
  int   Swap;
  } g2rreq_t;

typedef struct {
  int       Count;
  int       TPN;
  int       TPNCmp;
  g2rspec_t ResSpec;
  g2rreq_t  ResReq;
  } g2task_t;

typedef struct {
  char       *Name;
  char       *Class;
  g2cred_t    Cred;
  char       *QOSRequested;
  long        WallTime;

  int         TaskCount;
  g2task_t   *Task[16];
  suattr_t  **JobAttr;

  char       *SystemID;
  char       *SubmitScript;
  } g2job_t;

typedef struct {
  char *Name;
  } g2qos_t;

typedef struct 
  {
  char *Name;
  } g2rm_t;
 
typedef struct 
  {
  char *Name;
  } g2ocfg_t;

typedef struct
  {
  char *Name;
  } g2res_t;

typedef struct
  {
  char *Name;
  } g2tcon_t;

#define MAX_G2ACL                32

#define MAX_G2RANGE_PER_RESOURCE 32

#define MAX_G2XMLATTR  64
#define DEFAULT_G2XMLICCOUNT   16

typedef struct g2xml_s {
  char *Name;
  char *Val;

  int   ACount;
  int   ASize;

  int   CCount;
  int   CSize;

  char **AName;
  char **AVal;
 
  struct g2xml_s **C;
  } g2xml_t;


/* silver structure attributes */

/* sync w/enum SResAttr (mg2-internal.h) */

enum SResAttrEnum {
  sraNONE = 0,
  sraArchList,
  sraAttr,
  sraBandwidth,
  sraCfgNodeCount,
  sraCfgProcCount,
  sraClassList,
  sraDataDir,
  sraDataServer,
  sraDirectJobStageEnabled,
  sraFeature,
  sraFlags,
  sraFullName,
  sraKey,
  sraLocalTime,
  sraNetList,
  sraOSList,
  sraRsvOverlap,
  sraRM,
  sraSource,
  sraTimeout };



/* G2 parameter keywords */

#define G2_SUBALLOC_KEYWORD   "G2SUBALLOCPOLICY"

#endif /* !__G2INTERNAL_H */
#endif /* !__G2_H */

/* END G2.h */
