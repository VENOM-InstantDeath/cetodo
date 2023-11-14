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
	struct List* next;
	int size;
};
struct Lists {
	struct List* head;
	int size;
};
struct List list_init();
void task_add(struct List* head, char* id, int status);
void list_add(struct Lists* head, char* id);

void task_ladd(struct List* head, char* id, int status);
void list_ladd(struct Lists* head, char* id);

void task_print(struct List* head);
void list_print(struct Lists* head);

void task_pop(struct List* head);
void list_pop(struct Lists* head);

int task_popat(struct List* head, int index);
int list_popat(struct Lists* head, int index);

void task_rpop(struct List* head);
void list_rpop(struct Lists* head);

struct Task* task_get(struct List* head, int index);
struct List* list_get(struct Lists* head, int index);

int task_search(struct List* head, char* id);
int list_search(struct Lists* head, char* id);

int task_size(struct List* head);
int list_size(struct Lists* head);
#endif
