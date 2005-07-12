/*
*/

#ifndef __PBSAPI
#define __PBSAPI

/* structures */

struct attrl {
  struct attrl *next;
  char         *name;
  char         *resource;
  char         *value;
  int           op;    /* not used */
  };


struct batch_status {
  struct batch_status *next;
  char                *name;
  struct attrl        *attribs;
  char                *text;
  };

/* prototypes */

struct batch_status *pbs_statnode();
struct batch_status *pbs_statjob();
struct batch_status *pbs_statque(int, char*, attrl*, char*);

int                  pbs_statfree(batch_status*);

int                  pbs_runjob(int, char*, char*, char*);
int                  pbs_alterjob(int, char*, attrl*, char*);
int                  pbs_deljob(int, char*, char*);
int                  pbs_stagein(int,char*,char*,char*);
int                  pbs_rerunjob(int, char*, char*);

char                *pbs_geterrmsg(int);

int                  openrm(char *,int);
int                  closerm(int);

int                  pbs_connect();
int                  pbs_disconnect();

int                  addreq(int,char*);
char                *getreq(int);


/* defines */

#ifndef ND_free

#define ND_free                 "free"
#define ND_offline              "offline"
#define ND_down                 "down"
#define ND_reserve              "reserve"
#define ND_job_exclusive        "job-exclusive"
#define ND_job_sharing          "job-sharing"
#define ND_busy                 "busy"
#define ND_state_unknown        "state-unknown"
#define ND_timeshared           "time-shared"
#define ND_cluster              "cluster"

#endif /* ND_free */

#ifndef ATTR_NODE_state

#define ATTR_NODE_state         "state"
#define ATTR_NODE_np            "np"
#define ATTR_NODE_properties    "properties"
#define ATTR_NODE_ntype         "ntype"
#define ATTR_NODE_jobs          "jobs"

#define ATTR_a "Execution_Time"
#define ATTR_c "Checkpoint"
#define ATTR_e "Error_Path"
#define ATTR_g "group_list"
#define ATTR_h "Hold_Types"
#define ATTR_j "Join_Path"
#define ATTR_k "Keep_Files"
#define ATTR_l "Resource_List"
#define ATTR_m "Mail_Points"
#define ATTR_o "Output_Path"
#define ATTR_p "Priority"
#define ATTR_q "destination"
#define ATTR_r "Rerunable"
#define ATTR_u "User_List"
#define ATTR_v "Variable_List"
#define ATTR_A "Account_Name"
#define ATTR_M "Mail_Users"
#define ATTR_N "Job_Name"
#define ATTR_S "Shell_Path_List"

#define ATTR_depend   "depend"
#define ATTR_inter    "interactive"
#define ATTR_stagein  "stagein"
#define ATTR_stageout "stageout"
#define ATTR_ctime      "ctime"
#define ATTR_exechost   "exec_host"
#define ATTR_mtime      "mtime"
#define ATTR_qtime      "qtime"
#define ATTR_session    "session_id"
#define ATTR_euser      "euser"
#define ATTR_egroup     "egroup"
#define ATTR_hashname   "hashname"
#define ATTR_hopcount   "hop_count"
#define ATTR_security   "security"
#define ATTR_sched_hint "sched_hint"
#define ATTR_substate   "substate"
#define ATTR_name       "Job_Name"
#define ATTR_owner      "Job_Owner"
#define ATTR_used       "resources_used"
#define ATTR_state      "job_state"
#define ATTR_queue      "queue"
#define ATTR_server     "server"
#define ATTR_maxrun     "max_running"
#define ATTR_total      "total_jobs"
#define ATTR_comment    "comment"
#define ATTR_cookie     "cookie"
#define ATTR_qrank      "queue_rank"
#define ATTR_altid      "alt_id"
#define ATTR_etime      "etime"
#define ATTR_aclhten    "acl_host_enable"
#define ATTR_aclhost    "acl_hosts"

#endif /* ATTR_NODE_state */

enum batch_op { SET, UNSET, INCR, DECR,
                EQ, NE, GE, GT, LE, LT, DFLT
};

#endif /* __PBSAPI */

/* END maui-pbsapi.h */

