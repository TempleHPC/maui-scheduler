/* HEADER */

LL_element *ll_get_objs(LL_element *A1,int A2,void *A3,int *A4,int *A5) { return(FAILURE); }  /* ll_get_objs() */ 
LL_element *ll_next_obj(LL_element *A1) { return(FAILURE); }
int ll_free_objs(LL_element *A1) { return(FAILURE); }
int llfree_job_info(LL_job *A,int B) { return(FAILURE); }
LL_element *ll_query(int A1) { return(FAILURE); }
int ll_set_request(LL_element *A1,int A2,void *A3,int A4) { return(FAILURE); } 
int ll_deallocate(LL_element *A1) { return(FAILURE); }
int ll_get_data(LL_element *A1,int A2,void *A3) { return(FAILURE); } 
int ll_start_job(LL_start_job_info *A1) { return(FAILURE); } 
int ll_terminate_job(LL_terminate_job_info *A1) { return(FAILURE); } 
int ll_control() { return(FAILURE); }
int llsubmit(char *A,void *B,void *C,LL_job *D,int E) { return(FAILURE); }

/* END __MLLStub.c */

