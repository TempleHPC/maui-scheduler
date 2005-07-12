/* Generates a HTML-table from the output of 'showres -n' 

   Version: 1.3

   Copyright (C) 2001 Niclas Andersson
   National Supercomputer Centre, Sweden
*/   

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define INT_MAX ((int)(~0U>>1))

#define DEBUG 0

#if DEBUG
/* Create a stand alone WWW-page
   Convenient during testing */
# define WWWPAGE 1
# define IMGURL ""
#else
# define WWWPAGE 0
/* The path to the images */
/* # define IMGURL "/images/" */ 
# define IMGURL ""
#endif

#define NROFNODES 32

/* name of common development reservation */
#define DEVELRES "devel"

/* pixels/row */
#define RESHEIGHT 12
/* pixels = SCALE(seconds) */
#define _SCALE ((double)RESHEIGHT / 3600)
#define SCALE(x) (int)(_SCALE * (x) + .5)

/* The reservation data structures */
struct node {
  struct node *next;
  struct res *jobres;
  struct res *userres;
  char name[16];
};
struct res {
  struct res *next;
  int start;
  int end;
  char state;
  char type;
  char id[20];
};
struct node *nodes;

int mintime = 0;
int maxtime = 0;
int jobmaxtime = 0;

/* Reservation data */
enum restype { res_none, res_bg, res_running, res_idle, res_user, res_devel, 
	       res_time };
char *resimg[] = {
  "transp.gif",
  "bggrid.gif",
  "darkgreen.gif",
  "green.gif",
  "red.gif",
  "yellow.gif",
  "gray.gif"
};

#define DAYIMGTIME 5*3600 /* width of day image in scheduling time */
char *dayimg[] = { 
  "sunday.gif",
  "monday.gif",
  "tuesday.gif",
  "wednesday.gif",
  "thursday.gif",
  "friday.gif",
  "saturday.gif"
};

void initnodes()
{
  int n;
  nodes = (struct node *)malloc(NROFNODES * sizeof(struct node));
  if (!nodes) {
    perror("malloc");
    exit(1);
  }
  /* initialize fields and make a linked list of nodes */
  for (n=0; n<NROFNODES; n++) {
    sprintf(nodes[n].name, "i%d", n+1);
    nodes[n].next = nodes+n+1;
    nodes[n].jobres = NULL;
    nodes[n].userres = NULL;
  }
  nodes[NROFNODES-1].next = NULL;
}

void printreslist(struct res *list)
{
  struct res *r;

  for (r=list; r; r=r->next) {
    fprintf(stderr, "    start=%d end=%d type=%c id=%s\n",
	    r->start, r->end, r->type, r->id);
  }
}

void printallres()
{
  struct node *n;

  for (n=nodes; n; n=n->next) {
    fprintf(stderr, "Node %s\n", n->name);
    if (n->jobres) {
      fprintf(stderr, "  Job\n");
      printreslist(n->jobres);
    }
    if (n->userres) {
      fprintf(stderr, "  User\n");
      printreslist(n->userres);
    }
  }
}

void insertresinlist(struct res *new, struct res **list)
{
  struct res **cur;
  for (cur=list; *cur; cur=&(*cur)->next)
    if (new->start < (*cur)->start) break;
  new->next = *cur;
  *cur = new;
}

/* The reservations is splitted into two reservation lists, job
   reservations and advance user reservations for each node.
*/
void insertres(char *nodename, char *id, char type, char state,
	       int start, int duration)
{
  int r, i;
  struct node *n;
  struct res **cur, *prev, *new;

  /* create new reservation */
  new = (struct res *)malloc(sizeof(struct res));
  if (!new) {
    perror("malloc");
    exit(1);
  }
  new->next = NULL;
  new->start = start;
  new->end = start + duration;
  new->state = state;
  new->type = type;
  strncpy(new->id, id, 20);

  /* find node */
  n = nodes;
  for (n=nodes; n; n=n->next) {
    if (!strcmp(n->name, nodename))
      break;
  }
  /* nodename not found */
  if (!n) return;

  switch (type) {
  case 'J': /* Job */
    insertresinlist(new, &n->jobres);
    if (new->end > jobmaxtime) jobmaxtime = new->end;
    break;
  case 'U': /* User */
  case 'S': /* System */
    insertresinlist(new, &n->userres);
    break;
  }

  /* statistics */
  if (start < mintime)          mintime = start;
  if (start+duration > maxtime) maxtime = start + duration;
}

/* Convert from [-][dd:]hh:mm:ss to seconds. */
int time2sec(char *time)
{
  int dd, hh, mm, ss;
  int res, sign, sec;
  char tmp;

  /* a fix for INFINITE & INFINITY 
     should be handled in another way */
  if (strstr(time, "INFINIT")) return 1;

  /* get sign */
  res = sscanf(time, " -%c", &tmp);
  sign = res ? -1 : 1; 

  res = sscanf(time, "%d:%d:%d:%d", &dd, &hh, &mm, &ss);
  dd = abs(dd);
  if (res == 3) {
    /* shift rigth */
    ss = mm; mm = hh; hh = dd; dd = 0;
  }
  sec = sign * (86400 * dd + 3600 * hh + 60 * mm + ss);
  return sec;
}

void getshowres(FILE *fd)
{
  char line[2000];
  int t;
  char tok[8][30];
  char nodename[20], type[20], resid[20], jobstate[20];
  int start, duration, task;
  while (fgets(line, 2000, fd)) {
    /* Ignore some lines */
    if (strlen(line) <= 1) continue;
    if (strstr(line, "reservations on")) continue;
    if (strstr(line, "nodes reserved")) continue;

    for (t=0; t<8; t++) memset(tok[t], 0, 30);
    /* Use the fixed format */
    sscanf(line, "%20c%11c%19c%11c%5c%12c%12c%21c", 
	   tok[0], tok[1], tok[2], tok[3], 
	   tok[4], tok[5], tok[6], tok[7]);
    sscanf(tok[0], "%s", nodename);
    /* Skip title line */
    if (strstr(nodename, "NodeName")) continue;
    sscanf(tok[1], "%s", type);
    sscanf(tok[2], "%s", resid);
    sscanf(tok[3], "%s", jobstate);
    sscanf(tok[4], "%d", &task);
    start = time2sec(tok[5]);
    duration = time2sec(tok[6]);
    insertres(nodename, resid, type[0], jobstate[0], start, duration);
  }
}

/* Print one reservation. */
void printres(int start, int end, enum restype type)
{
  int height, width;
  char *img;
  int size;

  img = resimg[type];
  height = RESHEIGHT;
  /* Need to do two proper conversions to integer */
  width = SCALE(end) - SCALE(start);
  /* width=0 is ignored by Netscape */
  if (width <= 0) return;

#if 1 /* one <IMG> per "reservation" */
  printf("<img border=0 height=%d width=%d src=\"%s%s\">", 
	 height, width, IMGURL, img);
#else /* binary sized <IMG>
         a test to see if the browser caches the images differently */
  for (size=4096; size>=1; size/=2) {
    fprintf(stderr, "res size=%d width=%d\n", size, width);
    if (size > width) continue;
    printf("<img border=0 height=%d width=%d src=\"%s%s\">", 
	   height, size, IMGURL, img);
    width -= size;
  }
#endif

#if DEBUG
  fprintf(stderr, "start=%8d end=%8d type=%d\n", start, end, type);
#endif
}

void printrow(struct node *n)
{
  struct res *jobres, *userres;
  enum restype type;
  int dur, t;
  int curtime;

  printf("<tr><td class=\"nodename\">%s</td><td>&nbsp;</td>"
	 "<td class=\"reservations\">", 
	 n->name);
#if DEBUG
  fprintf(stderr, "Node %s\n", n->name);
#endif

  /* This is where we figure out what reservation to output */
  /* It traverse two linked list and handles all events (start, end) that
     occurs. Job reservation are always visible. User reservations "under"
     job reservations are not visible. */
  curtime = 0;
  jobres = n->jobres;
  userres = n->userres;
  while (jobres || userres) {
    if (jobres && jobres->start <= curtime) {
      if (jobres->end > curtime) {
	switch (jobres->state) {
	case 'R':
	  type = res_running;
	  break;
	case 'I':
	  type = res_idle;
	  break;
	}
	printres(curtime, jobres->end, type);
	curtime = jobres->end;
      }
      jobres = jobres->next;
      continue;
    }
    if (userres && userres->start <= curtime) {
      if (userres->end > curtime) {
	/* Use another color on development reservations */
	type = (strstr(userres->id, DEVELRES)) ? res_devel : res_user;
	if (jobres && jobres->start < userres->end) {
	  printres(curtime, jobres->start, type);
	  curtime = jobres->start;
	  /* The user_res may contine after the job_res */
	  continue;
	}
	/* limit the length of user reservations */
	t = userres->end;
	/* t = MIN(userres->end, jobmaxtime); */
	printres(curtime, t, type);
	curtime = userres->end;  
      }
      userres = userres->next;
      continue;
    }
    /* The node is currently free. 
       Figuring out when next reservation occurs */
    t = INT_MAX;
    if (jobres)  t = MIN(t, jobres->start);
    if (userres) t = MIN(t, userres->start);
    printres(curtime, t, res_none);
    curtime = t;
  }
  /* Unallocated nodes needs something between <td> and </td>.
     (&nbsp; adds extra vertical space ???) */
  printres(0, 3600, res_none);
  printf("</td></tr>\n");
}

void printdays() 
{
  int curtime, tim, day, diff;
  struct tm midnight;
  time_t t;
  int i;
  struct tm *now;

  curtime = 0;
  diff = time(&t);
  now = localtime(&t);
  midnight = *now;
  midnight.tm_hour = midnight.tm_min = midnight.tm_sec = 0;
  tim = mktime(&midnight) - diff;
  day = now->tm_wday;
#if DEBUG
  fprintf(stderr, "seconds to midnight=%d day=%d\n", tim-curtime, day);
#endif
  printf("<tr><td>&nbsp;</td><td>&nbsp;</td><td class=\"reservations\">");
  for (;;) {
    /* next day */
    day = (day+1)%7;
    tim += 24*3600;
    /* continue 'til end of row */
    if (tim+DAYIMGTIME > maxtime) break;
    printres(curtime, tim, res_none);
    printf("<img src=\"%s%s\">", IMGURL, dayimg[day]);
    curtime = tim+DAYIMGTIME;
  }      
  printf("</td></tr>");
}

void printlegend()
{
  int end = 3600;

  /* empty row */
  printf("<tr><td>&nbsp;</td></tr>\n");

  printf("<!-- legend -->\n"
	 "<tr><td>&nbsp;</td><td>&nbsp;</td>"
	 "<td><div class=\"legend\">");

  printres(0, end, res_bg);
  printf(" = 1 node-hour ");
  printres(0, end, res_running);
  printf(" = running job ");
  printres(0, end, res_idle);
  printf(" = advance job reservation ");
  printres(0, end, res_user);
  printf(" = static reservation ");
  printres(0, end, res_devel);
  printf(" = development reservation (max walltime: 1 h) ");

  printf("</div></td></tr>\n");
}

void printtable() 
{
  struct node *n;

#if DEBUG
  fprintf(stderr, "mintime=%d maxtime=%d\n", mintime, maxtime);
#endif
  printf("<table class=\"nodealloc\" cellpadding=0 cellspacing=0 border=0>\n");
  /* reservations */
  for (n=nodes; n; n=n->next) 
    printrow(n);
  printdays();
  printlegend();
  printf("</table>\n");
}


int main(int argc, char ** argv) 
{
  initnodes();
  getshowres(stdin);
#if DEBUG
  printallres();
#endif

#if WWWPAGE
  printf("<html>\n"
	 "<head>\n"
	 " <title>%s</title>\n"
	 " <style>\n"
	 "  td { font-family: Arial,Helvetica; font-size: 10px}\n"
	 "  .nodename { text-align: right; }\n"
	 "  .reservations { white-space:pre; background: url(%s%s); background-color: #ffffa0; font-size: 10px;}"
	 " </style>\n"
	 "</head>\n"
	 "<body>\n",
	 "Showres");
#endif

  printtable();

#if WWWPAGE
  printf("</body>\n</html>\n");
#endif
}
