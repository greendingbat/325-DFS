/* File: calls.c
 * Author: Eric Anson
 * Purpose: To read in logs of calls and then answer questions about
 *          them.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct n {
  char number[13];
  struct e *calls;
  struct n *next;
  int level;
  struct n *qptr;      //used for implementing queue in bfs
} phone;

typedef struct e {
  phone *to;
  int cnt;
  struct e *next;
} call;

int errSeen = 0;
phone *hd = NULL;             //we'll use a global list for this program

/* printGraph() prints the graph pointed to by hd. It's used for program
   checking
*/

void printGraph() {
  phone *ptr;
  call *cptr;

  for (ptr = hd; ptr; ptr = ptr->next) {
    printf("%s\n", ptr->number);
    for (cptr = ptr->calls; cptr; cptr = cptr->next) {
      printf("    %s : %d\n", cptr->to->number, cptr->cnt);
    }
  }
}

/* addPhone(p1) adds a node to the head of the list pointed to by hd
   for phone number p1
*/

phone *addPhone(char *p1) {
  phone *newPhone;
  
  newPhone = malloc(sizeof(phone));
  if (newPhone == NULL) {
    fprintf(stderr,"Out of memory\n");
    exit(1);
  }
  newPhone->next = hd;
  strcpy(newPhone->number, p1);
  newPhone->calls = NULL;
  hd = newPhone;
  return hd;
}

/* findPhone(p1) looks through the list pointed to by hd for a node with
   number == p1. It returns a pointer to the node if found, and NULL
   otherwise
*/

phone *findPhone(char *p1) {
  phone *ptr;
  for (ptr = hd; ptr; ptr = ptr->next)
    if (strcmp(p1, ptr->number) == 0)
      return ptr;
  
  return NULL;
}

/* addCall(p1, p2) adds the single call struct to p1 pointing to p2*/
void addCall(phone *p1, phone *p2) {
  call *newCall;

  newCall = malloc(sizeof(call));
  if (newCall == NULL) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }
  newCall->to = p2;
  newCall->cnt = 1;
  newCall->next = p1->calls;
  p1->calls = newCall;
}

/* addCalls(p1, p2) adds a call from p1 to p2 to the structure */

void addCalls(char *p1, char *p2) {
  phone *phone1, *phone2;
  call *cptr;

  phone1 = findPhone(p1);
  if (phone1 == NULL)
    phone1 = addPhone(p1);
  phone2 = findPhone(p2);
  if (phone2 == NULL)
    phone2 = addPhone(p2);

  for (cptr = phone1->calls; cptr; cptr = cptr->next) {
    if (cptr->to == phone2) {       //already calls between these
      ++cptr->cnt;
      for (cptr = phone2->calls; cptr->to != phone1; cptr = cptr->next) ;
      ++cptr->cnt;   // must exist since semetric
      return;
    }
  }
  addCall(phone1, phone2);
  addCall(phone2, phone1);
}

/* goodNumber(str) returns 1 if str is a good phone number and 0 otherwise */

int goodNumber(char *str) {
  int i;
  for (i = 0; i < 3; ++i)
    if (!isdigit(str[i]))
      return 0;
  if (str[i++] != '-')
    return 0;
  while (i < 7)
    if (!isdigit(str[i++]))
      return 0;
  if (str[i++] != '-')
    return 0;
  while (i < 12)
    if (!isdigit(str[i++]))
      return 0;
  return (str[i] == '\0');
}

/* readFile(in) reads the phone numbers from the stream in and adds to the
   graph structure being built */

void readFile(FILE *in) {
  char *line = NULL;
  size_t sz;
  char p1[14], p2[14], check[2];
  int numWords;

  while (getline(&line, &sz, in) > 0) {
    numWords = sscanf(line, "%13s%13s%1s", p1, p2, check);
    if (numWords == 2) {
      if (goodNumber(p1) && goodNumber(p2)) {
        if (strcmp(p1,p2) == 0) {
          fprintf(stderr, "Duplicate numbers %s\n", p1);
          errSeen = 1;
        } else {
          addCalls(p1, p2);
        }
      } else {
        fprintf(stderr,"Bad phone number in line: %s", line);
        errSeen = 1;
      }
    } else if (numWords > 0) {
      fprintf(stderr, "Wrong number of strings in: %s", line);
      errSeen = 1;
    }
  }
  free(line);
}

/*freeGraph frees the memory used by the graph pointed to by hd */

void freeGraph() {
  phone *pptr, *tptr;
  call *cptr, *tcptr;

  pptr = hd;
  while (pptr) {
    cptr = pptr->calls;
    while (cptr) {
      tcptr = cptr->next;
      free(cptr);
      cptr = tcptr;
    }
    tptr = pptr->next;
    free(pptr);
    pptr = tptr;
  }
}

/* bfs does a breadth first search to find the shortest path length from
   p1 to p2. It returns -1 if no path found */

int bfs(phone *p1, phone *p2) {
  phone *qhd, *qtail;
  call *cptr;

  p1->level = 0;
  qhd = qtail = p1;
  p1->qptr = NULL;
  while (qhd) {
    if (qhd == p2)
      return p2->level;
    for (cptr = qhd->calls; cptr; cptr = cptr->next) {
      if (cptr->to->level == -1) {
        cptr->to->level = qhd->level + 1;
        qtail->qptr = cptr->to;
        cptr->to->qptr = NULL;
        qtail = cptr->to;
      }
    }
    qhd = qhd->qptr;
  }

  return -1;
}

/* query(p1, p2) processes a query for phone p1 and p2 */
void query(char *p1, char *p2) {
  phone *phone1, *phone2, *ptr;
  int level;
  call *cptr;

  phone1 = findPhone(p1);
  phone2 = findPhone(p2);

  if (phone1 == NULL || phone2 == NULL) {
    fprintf(stderr, "No such phone either %s or %s\n", p1, p2);
    errSeen = 1;
    return;
  }

  // First check to see if they've called each other
  for (cptr = phone1->calls; cptr; cptr = cptr->next) {
    if (cptr->to == phone2) {
      printf("Talked %d times\n", cptr->cnt);
      return;
    }
  }

  // Didn't talk directly, so do dfs
  for (ptr = hd; ptr; ptr = ptr->next)
    ptr->level = -1;
  level = bfs(phone1, phone2);
  if (level == -1) 
    printf("Not connected\n");
  else
    printf("Connected through %d numbers\n", level - 1);
}

/* readQueries() responds to the queries that come from stdin */

void readQueries() {
  char *line = NULL;
  size_t sz;
  char p1[14], p2[14], check[2];
  int numWords;

  while (getline(&line, &sz, stdin) > 0) {
    numWords = sscanf(line, "%13s%13s%1s", p1, p2, check);
    if (numWords == 2) {
      if (goodNumber(p1) && goodNumber(p2)) {
        if (strcmp(p1,p2) == 0) {
          fprintf(stderr, "Duplicate numbers %s\n", p1);
          errSeen = 1;
        } else {
          query(p1, p2);
        }
      } else {
        fprintf(stderr,"Bad phone number in line: %s", line);
        errSeen = 1;
      }
    } else if (numWords > 0) {
      fprintf(stderr, "Wrong number of strings in: %s", line);
      errSeen = 1;
    }
  }
  free(line);
}

int main(int argc, char **argv) {
  FILE *inFile;
  int i;

  if (argc < 2) {
    fprintf(stderr, "At least one input file must be supplied\n");
    return 1;
  }
  for (i = 1; i < argc; ++i) {
    inFile = fopen(argv[i], "r");
    if (inFile == NULL) {
      perror(argv[i]);
      errSeen = 1;
      continue;    //get next file
    }
    readFile(inFile);
    if (fclose(inFile) == EOF) {
      perror(argv[i]);
      errSeen = 1;
    }
  }
//  printGraph();

  //Finished reading the files to set up the graph, now read queries
  readQueries();

  freeGraph();
  return errSeen;
}
