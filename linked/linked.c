#include <stdio.h>
#include <stdlib.h>
#include "linked.h"

struct List list_init() {
	struct List l;
	l.head = NULL;
	l.next = NULL;
	l.id = NULL;
	l.size = 0;
	return l;
}

void task_add(struct List* head, char* id, int status) {
	if (head->head == NULL) {
		struct Task* node = malloc(sizeof(struct Task));
		node->id = id; node->status = status; node->next = NULL;
		head->head = node;
	} else {
		struct Task* curr = head->head;
		while (curr->next != NULL) {curr = curr->next;}
		struct Task* node = malloc(sizeof(struct Task));
		node->id = id; node->status = status; node->next = NULL;
		curr->next = node;
	}
}
void list_add(struct Lists* head, char* id) {
	if (head->head == NULL) {
		struct List* node = malloc(sizeof(struct List));
		node->id = id; node->next = NULL;
		head->head = node;
	} else {
		struct List* curr = head->head;
		while (curr->next != NULL) {curr = curr->next;}
		struct List* node = malloc(sizeof(struct Task));
		node->id = id; node->next = NULL;
		curr->next = node;
	}
}

void task_ladd(struct List* head, char* id, int status) {
	if (head->head == NULL) {
		return;
	}
	struct Task* node = head->head;
	struct Task* new = malloc(sizeof(struct Task));
	new->id = id; new->status = status; new->next = node;
	head->head = new;
}
void list_ladd(struct Lists* head, char* id) {
	if (head->head == NULL) {
		return;
	}
	struct List* node = head->head;
	struct List* new = malloc(sizeof(struct Task));
	new->id = id; new->next = node;
	head->head = new;
}

void task_print(struct List* head) {
	printf("[");
	if (head->head == NULL) {printf("]\n");return;}
	struct Task* curr = head->head;
	while (curr->next != NULL) {
		printf("%s, ", curr->id);
		curr = curr->next;
	}
	printf("%s]\n", curr->id);
}
void list_print(struct Lists* head) {
	printf("[");
	if (head->head == NULL) {printf("]\n");return;}
	struct List* curr = head->head;
	while (curr->next != NULL) {
		printf("%s, ", curr->id);
		curr = curr->next;
	}
	printf("%s]\n", curr->id);
}

void task_pop(struct List* head) {
	if (head->head == NULL) return;
	struct Task* curr = head->head;
	if (curr->next == NULL) {free(head->head);head->head = NULL;return;}
	while (curr->next != NULL) {
		if (curr->next->next == NULL) break;
		curr = curr->next;
	}
	free(curr->next);
	curr->next=NULL;
}
void list_pop(struct Lists* head) {
	if (head->head == NULL) return;
	struct List* curr = head->head;
	if (curr->next == NULL) {free(head->head);head->head = NULL;return;}
	while (curr->next != NULL) {
		if (curr->next->next == NULL) break;
		curr = curr->next;
	}
	free(curr->next);
	curr->next=NULL;
}

int task_popat(struct List* head, int index) {
	if (head->head == NULL) return 0;
	if (!index) {task_rpop(head);return 1;}
	struct Task* curr = head->head;
	/*if (!curr->next && !index) {
		task_pop(head);
		return 1;
	}*/
	while (index && index-1) {
		if (curr->next == NULL) return 0;
		curr = curr->next;
		index--;
	}
	struct Task* tmp = curr->next->next;
	free(curr->next);
	curr->next = tmp;
	return 1;
}
int list_popat(struct Lists* head, int index) {
	if (head->head == NULL) return 0;
	if (!index) {list_rpop(head);return 1;}
	struct List* curr = head->head;
	/*if (!curr->next && !index) {
		task_pop(head);
		return 1;
	}*/
	while (index && index-1) {
		if (curr->next == NULL) return 0;
		curr = curr->next;
		index--;
	}
	struct List* tmp = curr->next->next;
	free(curr->next);
	curr->next = tmp;
	return 1;
}

void task_rpop(struct List* head) {
	if (head->head == NULL) return;
	struct Task* next = head->head->next;
	free(head->head);
	head->head = next;
}
void list_rpop(struct Lists* head) {
	if (head->head == NULL) return;
	struct List* next = head->head->next;
	free(head->head);
	head->head = next;
}

struct Task* task_get(struct List* head, int index) {
	struct Task* curr = head->head;
	int indx = 0;
	while (curr->next != NULL) {
		if (indx == index) return curr;
		indx++;
		curr = curr->next;
	}
	if (indx == index) return curr;
	else return NULL;
}
struct List* list_get(struct Lists* head, int index) {
	struct List* curr = head->head;
	int indx = 0;
	while (curr->next != NULL) {
		if (indx == index) return curr;
		indx++;
		curr = curr->next;
	}
	if (indx == index) return curr;
	else return NULL;
}

int task_search(struct List* head, char* id) {
	struct Task* curr = head->head;
	int pos = 0;
	while (curr->next != NULL) {
		if (curr->id == id) return pos;
		pos++;
		curr = curr->next;
	}
	if (curr->id == id) return pos;
	else return -1;
}
int list_search(struct Lists* head, char* id) {
	struct List* curr = head->head;
	int pos = 0;
	while (curr->next != NULL) {
		if (curr->id == id) return pos;
		pos++;
		curr = curr->next;
	}
	if (curr->id == id) return pos;
	else return -1;
}

int task_size(struct List* head) {
	struct Task* curr =head->head;
	int size=0;
	if (curr == NULL) return size;
	while (curr->next != NULL) {curr=curr->next;size++;}
	size++;
	return size;
}
int list_size(struct Lists* head) {
	struct List* curr =head->head;
	int size=0;
	if (curr == NULL) return size;
	while (curr->next != NULL) {curr=curr->next;size++;}
	size++;
	return size;
}
