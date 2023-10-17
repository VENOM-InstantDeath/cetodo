#include <stdio.h>
#include <stdlib.h>
#include "linked.h"

void list_init(struct List* head) {
	head->head = NULL;
}

void list_add(struct List* head, char* id, int status) {
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

void list_ladd(struct List* head, char* id, int status) {
	if (head->head == NULL) {
		return;
	}
	struct Task* node = head->head;
	struct Task* new = malloc(sizeof(struct Task));
	new->id = id; new->status = status; new->next = node;
	head->head = new;
}

void list_print(struct List* head) {
	printf("[");
	if (head->head == NULL) {printf("]\n");return;}
	struct Task* curr = head->head;
	while (curr->next != NULL) {
		printf("%s, ", curr->id);
		curr = curr->next;
	}
	printf("%s]\n", curr->id);
}

void list_pop(struct List* head) {
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

int list_popat(struct List* head, int index) {
	if (head->head == NULL) return 0;
	struct Task* curr = head->head;
	while (index-1) {
		if (curr->next == NULL) return 0;
		curr = curr->next;
		index--;
	}
	struct Task* tmp = curr->next->next;
	free(curr->next);
	curr->next = tmp;
	return 1;
}

void list_rpop(struct List* head) {
	if (head->head == NULL) return;
	struct Task* next = head->head->next;
	free(head->head);
	head->head = next;
}

struct Task* list_get(struct List* head, int index) {
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

int list_search(struct List* head, char* id) {
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

int list_size(struct List* head) {
	struct Task* curr =head->head;
	int size=0;
	if (curr == NULL) return size;
	while (curr->next != NULL) {curr=curr->next;size++;}
	size++;
	return size;
}
