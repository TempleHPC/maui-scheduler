/* HEADER */

/* socket util object */

int MSUInitialize(msocket_t *,char *,int,long,long);
int MSUIPCInitialize(void);
int MSUListen(msocket_t *);
int MSUConnect(msocket_t *,mbool_t,char *);
int MSUDisconnect(msocket_t *);
int MSUClose(msocket_t *);
int MSUFree(msocket_t *);
int MSUAcceptClient(msocket_t *,msocket_t *,char *,int);
int MSUSendData(msocket_t *,long,mbool_t,mbool_t);
int MSUSendPacket(int,char *,long,long,enum MStatusCodeEnum *);
int MSURecvData(msocket_t *,long,mbool_t,enum MStatusCodeEnum *,char *);
int MSURecvPacket(int,char **,long,char *,long,enum MStatusCodeEnum *);
int MSUSelectWrite(int,unsigned long);
int MSUSelectRead(int,unsigned long);
int MSUCreate(msocket_t **);
int MSUAdjustSBuffer(msocket_t *,int,mbool_t);
int MSUSetAttr(msocket_t *,enum MSocketAttrEnum,void *);
int MSUDup(msocket_t **,msocket_t *);

int MUISCreateFrame(msocket_t *,mbool_t,mbool_t);



/* sec object */

int MSecGetChecksum(char *,int,char *,char *,enum MChecksumAlgoEnum,char *);
int MSecGetChecksum2(char *,int,char *,int,char *,char *,enum MChecksumAlgoEnum,char *);
int MSecTestChecksum(char *);
int MSecBufTo64BitEncoding(char *,int,char *);
int MSecCompBufTo64BitEncoding(char *,int,char *);
int MSecComp64BitToBufDecoding(char *,int,char *,int *);
int MSecBufToHexEncoding(char *,int,char *);
int MSecCompressionGetMatch(unsigned char *,unsigned int,unsigned int,int *,unsigned int *,int *);
int MSecCompress(unsigned char *,unsigned int,unsigned char *,char *);
int MSecDecompress(unsigned char *,unsigned int,unsigned char *,unsigned int,unsigned char **,char *);
int MSecEncryption(char *,char *,int);



/* XML object */

int MXMLCreateE(mxml_t **,char *);
int MXMLDestroyE(mxml_t **);
int MXMLExtractE(mxml_t *,mxml_t *,mxml_t **);
int MXMLMergeE(mxml_t *,mxml_t *,char);
int MXMLSetAttr(mxml_t *,char *,void *,enum MDataFormatEnum);
int MXMLAppendAttr(mxml_t *,char *,char *,char);
int MXMLSetVal(mxml_t *,void *,enum MDataFormatEnum);
int MXMLAddE(mxml_t *,mxml_t *);
int MXMLSetChild(mxml_t *,char *,mxml_t **);
int MXMLToString(mxml_t *,char *,int,char **,mbool_t);
int MXMLGetAttr(mxml_t *,char *,int *,char *,int);
int MXMLGetAttrF(mxml_t *,char *,int *,void *,enum MDataFormatEnum,int);
int MXMLGetChild(mxml_t *,char *,int *,mxml_t **);
int MXMLGetChildCI(mxml_t *,char *,int *,mxml_t **);
int MXMLFromString(mxml_t **,char *,char **,char *);
int MXMLDupE(mxml_t *,mxml_t **);
mbool_t MXMLStringIsValid(char *);
int MXMLToXString(mxml_t *,char **,int *,int,char **,mbool_t);



/* sss interface object */

int MS3LoadModule(mrmfunc_t *);
int MS3DoCommand(mpsi_t *,char *,char **,mxml_t **,int *,char *);
int MS3Setup(int);
int MS3InitializeLocalQueue(mrm_t *,char *);
int MS3AddLocalJob(mrm_t *,char *);
int MS3RemoveLocalJob(mrm_t *,char *);



/* sss convenience functions */

int MS3AddSet(mxml_t *,char *,char *,mxml_t **);
int MS3AddWhere(mxml_t *,char *,char *,mxml_t **);
int MS3GetSet(mxml_t *,mxml_t **,int *,char *,int,char *,int);
int MS3GetWhere(mxml_t *,mxml_t **,int *,char *,int,char *,int);
int MS3SetObject(mxml_t *,char *,char *);
int MS3GetObject(mxml_t *,char *);
int MS3AddGet(mxml_t *,char *,mxml_t **);
int MS3GetGet(mxml_t *,mxml_t **,int *,char *,int);
int MS3SetStatus(mxml_t *,char *,enum MSFC,char *);
int MS3CheckStatus(mxml_t *,enum MSFC *,char *);

/* sss object functions */

int MS3JobToXML(mjob_t *,mxml_t **,enum MJobAttrEnum *,enum MReqAttrEnum *,char *);

/* END mcom-proto.h */

