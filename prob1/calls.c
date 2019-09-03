#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct node {
	char* number;
	struct edge* linkHead;
	struct node* next;
	int level;
} node;

typedef struct edge {
	struct node* to;
	struct edge* next;
	int count;
} edge;

typedef struct qNode {
	struct node* element;
	struct qNode* next;
	
} qNode;

// Global variable: list head
node* head = NULL; 

// Global variable: q head
qNode* qHeadPtr = NULL;

// Global variable: error seen
int errSeen = 0;

// Function prototypes:
int readFile(FILE*);
int isPhoneNumber(char*);
void addRecord(char*, char*);
node* createNewNode(char*);
void createLink(node*, node*);
void addToEndOfList(node*);
void outOfMemory();
void getInput();
void checkRecord(char*, char*);
int bfs(node*, node*);
void resetLevels();
void freeList(node*);
void addToEndOfQ(node*, qNode*);
void printQ(qNode*);
void freeQ(qNode*);

int main(int argc, char*argv[]) {
	FILE* input = NULL;
	
	// READ FROM FILES
	
	// Set input stream/open file
	if (argc <= 1) {	// No arguments from command line
		fprintf(stderr, "At least one input file must be supplied\n");
		exit(1);
	} else {
		int i;	// iterate through all files passed as arguments
		for (i = 1; i < argc; i++) {
			input = fopen(argv[i], "r");
			if (input == NULL) {
				fprintf(stderr, "%s: No such file or directory\n", argv[i]);
				errSeen = 1;
			} else {
				readFile(input);
				fclose(input);
			}
		}
	} 
	
	// READ FROM STDIN
	readFile(stdin);

	freeList(head);
	return errSeen;
}


int readFile (FILE* input) {
	char* line = NULL;
	size_t len = 0;
	while (getline(&line, &len, input) != EOF) {
		if (line == NULL) {
			outOfMemory();
		}
		char* linePtr = line;
		char num1[13];
		char num2[13];
		char extra[2];
		int itemsRead = 0;
		while ( (itemsRead = sscanf(linePtr, "%12s %12s %1s", num1, num2, extra)) > 0 ) {
			if (itemsRead != 2) {
				fprintf(stderr, "Wrong number of strings in: %s\n", linePtr);
				errSeen = 1;
				break;
			} else if (!isPhoneNumber(num1) || !isPhoneNumber(num2)){
				fprintf(stderr, "Bad phone number in line: %s\n", linePtr);
				errSeen = 1;
				break;
			} else {
				if (strcmp(num1, num2) == 0) {
					fprintf(stderr, "Duplicate numbers %s\n", num1);
					errSeen = 1;
					break;
				}
				if (input == stdin) {
					checkRecord(num1, num2);
				} else {
					addRecord(num1, num2);
				}
				linePtr += strlen(num1) + strlen(num2) + 1;
			}			
			
			
		}
		
	}
	free(line);
	return 0;
}

void checkRecord(char* num1, char* num2) {
	node* ptr = head;
	node* node1 = NULL;
	node* node2 = NULL;
	while (ptr) {
		if (strcmp(ptr->number, num1) == 0) {
			node1 = ptr;
		} else if (strcmp(ptr->number, num2) == 0) {
			node2 = ptr;
		}
		ptr = ptr->next;
	}
	if (!node1 || !node2) {
		fprintf(stderr, "No such phone either %s or %s\n", num1, num2);
		errSeen = 1;
		return;
	}
	// Search node1 for link to node2
	edge* linkPtr = node1->linkHead;
	int found = 0;
	while(linkPtr) {
		if (strcmp(linkPtr->to->number, num2) == 0) {
			printf("Talked %d times\n", linkPtr->count);
			found = 1;
			break;
		}
		linkPtr = linkPtr->next;
	}
	// if not found, search node2 for link to node1
	if (!found) {
		linkPtr = node2->linkHead;
		while(linkPtr) {
		if (strcmp(linkPtr->to->number, num2) == 0) {
			printf("Talked %d times\n", linkPtr->count);
			found = 1;
			break;
		}
		linkPtr = linkPtr->next;
		}
	}
	
	//if still not found, run bfs
	if (!found) {
		int connected = bfs(node1, node2);
		freeQ(qHeadPtr);
		if (connected > 0) {
			printf("Connected through %d numbers\n", connected-1);
		} else {
			printf("Not connected\n");
		}
	}
	
}

int bfs(node* start, node* target) {
	resetLevels();
	start->level = 0;
	qNode* qHead = malloc(sizeof(qNode));
	qHeadPtr = qHead;
	qHead->element = start;
	qHead->next = NULL;
	node* A = NULL;
	node* C = NULL;
	while(qHead) {
		A = qHead->element;
		if (strcmp(A->number, target->number) == 0) {
			return A->level;
		}
		edge* linkPtr = A->linkHead;
		while (linkPtr) {
			C = linkPtr->to;
			
			if (C->level == -1) {
				C->level = A->level + 1;
				addToEndOfQ(C, qHead);
			}
			linkPtr = linkPtr->next;
		}
		qHead = qHead->next;
		
	}
	return -1;
}

void freeQ(qNode* qHead) {
	qNode* nextQ;
	while (qHead) {
		nextQ = qHead->next;
		free(qHead);
		qHead = nextQ;
	}
}

void printQ(qNode* qHead) {
	printf("\t\tCURRENT Q:\n");
	qNode* ptr = qHead;
	while (ptr) {
		printf("\t\t\t%s\n", ptr->element->number);
		ptr = ptr->next;
	}	
}

void addToEndOfQ(node* nodeToAdd, qNode* qHead) {
	qNode* ptr = qHead;
	while (ptr->next) {
		ptr = ptr->next;
	}
	qNode* newQ = malloc(sizeof(qNode));
	newQ->element = nodeToAdd;
	newQ->next = NULL;
	ptr->next = newQ;
}

void resetLevels() {
	node* ptr = head;
	while(ptr) {
		ptr->level = -1;
		ptr = ptr->next;
	}
}

int isPhoneNumber(char* str) {
	if (strlen(str) != 12) {
		return 0;
	}
	int i;
	for (i = 0; i < strlen(str); i++) {
		if (i == 3 || i == 7) {
			if (str[i] != '-') {
				return 0;
			}
		} else {
			if (!isdigit(str[i])) {
				return 0;
			}
		}
	}
	return 1;
}

void addRecord(char* num1, char* num2) {
	node* node1 = NULL;
	node* node2 = NULL;
	if (!head) {	// list is empty, create a new node for each number
		node1 = createNewNode(num1);
		node2 = createNewNode(num2);
		head = node1;
		head->next = node2;
	} else {
		// Search list for num1 AND num2
		node* ptr = head;
		while (ptr) {
			if (strcmp(ptr->number, num1) == 0) {
				node1 = ptr;
			} else if (strcmp(ptr->number, num2) == 0) {
				node2 = ptr;
			}
			ptr = ptr->next;
		}
		// If either number was not found, create a node for it, add to end of list
		if (!node1) {
			node1 = createNewNode(num1);
			addToEndOfList(node1);
		}
		if (!node2) {
			node2 = createNewNode(num2);
			addToEndOfList(node2);
		}
	}	
	
	// Link both nodes
	createLink(node1, node2);
	createLink(node2, node1);
}

void addToEndOfList(node* n) {
	node* ptr = head;
	while (ptr->next) {
		ptr = ptr->next;
	}
	ptr->next = n;
}

// Allocates memory for a new node, sets the number field, returns pointer
node* createNewNode(char* num) {
	node* newNode = malloc(sizeof(node));
	if (newNode == NULL) {
		outOfMemory();
	}
	newNode->number = strdup(num);
	newNode->level = -1;
	newNode->next = NULL;
	newNode->linkHead = NULL;
	return newNode;
}

// creates a link between two EXISTING nodes (does not check if either node is null)
// searches node1 links for node2. 
// if found, increments link count
// if not, creates link at end of node1 link list
void createLink(node* node1, node* node2) {
	edge* newEdge = malloc(sizeof(edge));
	if (newEdge == NULL) {
		outOfMemory();
	}
	newEdge->to = node2;
	newEdge->next = NULL;
	newEdge->count = 1;
	edge* linkPtr = node1->linkHead;
	
	if (linkPtr == NULL) {
		node1->linkHead = newEdge;
		return;
	}
	// search node1 link list for node2
	while (linkPtr) {
		if (strcmp(node2->number, linkPtr->to->number) == 0) {
			linkPtr->count++;
			free(newEdge);
			return;
		}
		linkPtr = linkPtr->next;
	}
	// not found
	linkPtr = node1->linkHead;
	while (linkPtr->next) {
		linkPtr = linkPtr->next;
	}
	linkPtr->next = newEdge;
	return;
	
}
	
void outOfMemory() {
	fprintf(stderr, "Out of memory\n");
	exit(1);
}

void freeList(node* head) {
	edge* linkPtr;
	edge* nextPtr;
	node* nextNode;
	while (head != NULL) {
		free(head->number);
		linkPtr = head->linkHead;
		while (linkPtr != NULL) {
			//free(linkPtr->to);
			nextPtr = linkPtr->next;
			free(linkPtr);
			linkPtr = nextPtr;
		}
		nextNode = head->next;
		free(head);
		head = nextNode;
	}	
}
