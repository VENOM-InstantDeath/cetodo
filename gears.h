#ifndef GEARS_H
#define GEARS_H
#include <ncurses.h>
#include "linked/linked.h"
struct Data {
	void* data;
	char** ls;
	int* ptrs;
	WINDOW** wins;
	int wins_size;
	int islist;
};
struct Callback {
	int (**func)(WINDOW* win, struct Data*, void*);
	void **args;
	int nmemb;
};
struct Binding {
	int *keys;
	int (**func)(WINDOW* win, struct Data*, void*);
	int nmemb;
};
struct Nopt {
	int str_size;
	int underline;
};
void create_dir_if_not_exist(const char* path);
void dialog(WINDOW** wins, const char* s);
void display_opts(WINDOW* win, struct Data* data, char** ls, int size, int start, int top, int* ptrs, int mode);
int menu(WINDOW* win, struct Callback cb, struct Data* data, struct Binding bind, int ptrs[2], void (*dcb)(WINDOW*,struct Data*,char**,int,int,int,int*,int));
int load_data(struct List** list);
int open_list(WINDOW* win, struct Data* data, void* _list);
int add_task(WINDOW* win, struct Data* data, void* _task);
int goback(WINDOW* win, struct Data* data, void* _task);
int quit(WINDOW* win, struct Data* data, void* _task);
int del_task(WINDOW* win, struct Data* data, void* _task);
int rename_task(WINDOW* win, struct Data* data, void* _task);
#endif
