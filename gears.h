#ifndef GEARS_H
#define GEARS_H
#include <ncurses.h>
#include "linked/linked.h"
struct Data;
struct MenuData {
	void (*dcb)(WINDOW*,struct Data*,int,int,int,int,int);
	int* ptrs;
	int mtop;
};
struct Data {
	void* data;
	char** ls;
	WINDOW** wins;
	int wins_size;
	int islist;
	struct MenuData menu;
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
void display_opts(WINDOW* win, struct Data* data, int size, int p, int start, int stop, int attrs);
int menu(WINDOW* win, struct Callback cb, struct Data* data, struct Binding bind, int ptrs[2]);
int load_data(struct List** list);
int open_list(WINDOW* win, struct Data* data, void* _list);
int add_task(WINDOW* win, struct Data* data, void* _task);
int goback(WINDOW* win, struct Data* data, void* _task);
int quit(WINDOW* win, struct Data* data, void* _task);
int del_task(WINDOW* win, struct Data* data, void* _task);
int rename_task(WINDOW* win, struct Data* data, void* _task);
int move_up(WINDOW* win, struct Data* data, void* _task);
int move_down(WINDOW* win, struct Data* data, void* _task);
#endif
