#ifndef LINKED_H
#define LINKED_H

struct Task {
	char* id;
	int status;
	struct Task* next;
};
struct List {
	char* id;
	struct Task* head;
	int size;
};
void list_init(struct List* head);
void list_add(struct List* head, char* id, int status);
void list_ladd(struct List* head, char* id, int status);
void list_print(struct List* head);
void list_pop(struct List* head);
int list_popat(struct List* head, int index);
void list_rpop(struct List* head);
struct Task* list_get(struct List* head, int index);
int list_search(struct List* head, char* id);
int list_size(struct List* head);
#endif
