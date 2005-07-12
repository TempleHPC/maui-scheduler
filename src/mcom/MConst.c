/* HEADER */

#include "moab.h"

/* repository for char arrays */

const char *MFrameProtocol[] = { 
  NONE, 
  "COUNT", 
  "SSS-0.1", 
  NULL };

const char *MSockProtocol[] = {
  NONE,
  "SUTCP",
  "SSS-HALF",
  "HTTPCLIENT",
  "HTTP",
  "SSS-CHALLENGE",
  NULL };


/* sync w/enum MSockAttrEnum */

const char *MSockAttr[] = {
  NONE,
  "LocalHost",
  "LocalPort",
  "RemoteHost",
  "RemotePort",
  NULL };


/* sync w/XXX */

const char *MS3SockProtocol[] = {
  NONE,
  NULL,
  "sssbase",
  NULL,
  NULL,
  "ssschallenge",
  NULL };


/* sync w/enum MDataFormatEnum */

const char *MDFormat[] = {
  NONE,
  "string",
  "int",
  "long",
  "double",
  "stringarray",
  "intarray",
  "longarray",
  "doublearray",
  "other",
  NULL };


/* sync w/enum MS3ActionEnum */

const char *MS3Action[] = {
  NONE,
  "Cancel",
  "Create",
  "Destroy",
  "Initialize",
  "List",
  "Modify",
  "Notify",
  "Query",
  "Start",
  NULL };


const char *MWireProtocol[] = {
  NONE,
  "AVP",
  "XML",
  "HTML",
  "SSS2",
  NULL };


/* sync w/enum MChecksumAlgoEnum */

const char *MCSAlgoType[] = {
  NONE,
  "DES",
  "HMAC",
  "HMAC64",
  "MD5",
  "PASSWD",
  "REMOTE",
  NULL };


/* sync w/enum MS3VEnum */

const char *MS3VName[] = {
  "SSSRM0.2",
  "SSS3.0",
  "SSS4.0",
  NULL };

/* END MConst.c */

