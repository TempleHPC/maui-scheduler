/* HEADER */


#if !defined(__MLL) && !defined(__MLL2) && !defined(__MLL31)

int MLLLoadModule(

  mrmfunc_t *F) 

  { 
  if (F == NULL)
    {
    return(FAILURE);
    }

  /* NO-OP stub */

  return(SUCCESS); 
  }  /* END MLLLoadModule() */

#endif /* !__MLL && !__MLL2 && !__MLL31 */

/* END MLLI.c */
