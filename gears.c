#include <curses.h>
#include <json-c/json_object.h>
#include <json-c/json_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ncurses.h>
#include <sys/stat.h>
#include <json-c/json.h>
#include "gears.h"
#include "libncread/ncread.h"
#include "libncread/vector.h"
#include "linked/linked.h"

void create_dir_if_not_exist(const char* path) {
	struct vector str = string_split((char*)path, '/');
	if (!strlen(str.str[0])) {vector_popat(&str, 0);}
	struct string P; string_init(&P);
	if (path[0] == '/') {string_addch(&P, '/');}
	for (int i=0; i<str.size; i++) {
		string_add(&P, str.str[i]);
		string_addch(&P, '/');
		if (open(P.str, O_RDONLY) == -1) {
			mkdir(P.str, 0777);
		}
	}
	string_free(&P);
	vector_free(&str);
}

void dialog(WINDOW** wins, const char* s) {
	int y,x; getmaxyx(wins[0],y,x);
	int size=strlen(s)+2;
	if (size >= x-25) size=x-25;
	WINDOW *win = newwin(4, size, y/2-2, x/2-size/2);
	box(win, ACS_VLINE, ACS_HLINE);
	getmaxyx(win,y,x);
	wbkgd(win, COLOR_PAIR(2));
	mvwaddstr(win,1,1,s);
	wattron(win, COLOR_PAIR(1));
	mvwaddstr(win,2,x/2-2,"[OK]");
	wattroff(win, COLOR_PAIR(1));
	wrefresh(win);
	for (;;) {
	int ch=wgetch(win);
	if (ch == 27 || ch == 10) break;
	}
	delwin(win);
	touchwin(wins[1]);
	touchwin(wins[2]);
	wrefresh(wins[1]);
	wrefresh(wins[2]);
}


int search_binding(int ch, struct Binding bind) {
	for (int i=0; i<bind.nmemb; i++) {
		if (ch == bind.keys[i]) return i;
	}
	return -1;
}

void display_opts(WINDOW* win, struct Data* data, char** ls, int size, int start, int top, int* ptrs, int mode) {
	void** _data = (void**)data->data;
	struct Nopt* nopt = (struct Nopt*)_data[0];
	char *str = malloc(nopt->str_size); str[nopt->str_size-1]=0;
	switch (mode) {
		case 0:
			if (size < top) { top = size; }
			int p=0;
			for (int i=start; i<top; i++) {
				if (data->islist) {
					char *buff = calloc(4,1);
					snprintf(buff, 4, "%d", ((struct List*)_data[1])[i].size);
					mvwaddstr(win, p, 0, buff);
					free(buff);
				} else {
					if (list_get((struct List*)_data[1],i)->status) mvwaddstr(win, p, 0, "[x]");
					else mvwaddstr(win, p, 0, "[ ]");
				}
				mvwaddnstr(win, p, 4, ls[i], nopt->str_size-1);p++;
			}
			free(str);
			return;
		case 1:
			memset(str, ' ', nopt->str_size-1);strncpy(str, ls[ptrs[1]], nopt->str_size);
			mvwaddnstr(win, ptrs[0], 4, str, nopt->str_size-1);

			if (nopt->underline) wattron(win, A_UNDERLINE); else wattron(win, COLOR_PAIR(1));
			memset(str, ' ', nopt->str_size-1);strncpy(str, ls[ptrs[1]-1], nopt->str_size);
			mvwaddnstr(win, ptrs[0]-1, 4, str, nopt->str_size-1);
			if (nopt->underline) wattroff(win, A_UNDERLINE); else wattroff(win, COLOR_PAIR(1));
			free(str);
			return;
		case 2:
			memset(str, ' ', nopt->str_size-1);strncpy(str, ls[ptrs[1]], nopt->str_size);
			mvwaddnstr(win, ptrs[0], 4, str, nopt->str_size-1);

			if (nopt->underline) wattron(win, A_UNDERLINE); else wattron(win, COLOR_PAIR(1));
			memset(str, ' ', nopt->str_size-1);strncpy(str, ls[ptrs[1]+1], nopt->str_size);
			mvwaddnstr(win, ptrs[0]+1, 4, str, nopt->str_size-1);
			if (nopt->underline) wattroff(win, A_UNDERLINE); else wattroff(win, COLOR_PAIR(1));
			free(str);
			return;
		case 3:
			memset(str, ' ', nopt->str_size-1);strncpy(str, ls[ptrs[1]], nopt->str_size);
			if (nopt->underline) wattron(win, A_UNDERLINE); else wattron(win, COLOR_PAIR(1));
			mvwaddnstr(win, ptrs[0], 4, str, nopt->str_size-1);
			if (nopt->underline) wattroff(win, A_UNDERLINE); else wattroff(win, COLOR_PAIR(1));
			free(str);
			return;
	}
}

int menu(WINDOW* win, struct Callback cb, struct Data* data, struct Binding bind, int ptrs[2], void (*dcb)(WINDOW*,struct Data*,char**,int,int,int,int*,int)) {
	int y; int x; getmaxyx(win, y, x);
	char** ls = data->ls;
	/*int p = ptrs[0];
	int sp = ptrs[1];*/
	data->ptrs=ptrs;
	int top = y;
	int size = cb.nmemb;
	if (size) {
		dcb(win, data, ls, cb.nmemb, 0, y, ptrs,  0);
		dcb(win, data, ls, cb.nmemb, 0, top, ptrs, 3);
	} else mvwaddstr(win, 0, 4, "No hay tareas por aquí.");
	for (;;) {
		int ch = wgetch(win);
		if (ch == KEY_UP) {
			if (!ptrs[1]) continue;
			if (!ptrs[0]) {
				wscrl(win, -1);
				top--;ptrs[1]--;
				wmove(win,0,0);wclrtobot(win);
				dcb(win, data, ls, cb.nmemb, top-y, top, ptrs, 0);

				ptrs[0]++;ptrs[1]++;
				dcb(win, data, ls, cb.nmemb,top-y,top,ptrs,1);

			} else {
				dcb(win, data, ls, cb.nmemb,top-y,top,ptrs,1);
				ptrs[0]--;ptrs[1]--;
			}
		}
		else if (ch == KEY_DOWN) {
			if (ptrs[1] == size-1 || !size) continue;
			if (ptrs[1] == top-1) {
				wscrl(win, 1);
				top++;ptrs[1]++;
				wmove(win,0,0);wclrtobot(win);
				dcb(win, data, ls, cb.nmemb, top-y, top, ptrs,  0);
				
				ptrs[0]++;ptrs[1]--;
				dcb(win, data, ls, cb.nmemb,top-y,top,ptrs,2);
			} else {
				dcb(win, data, ls, cb.nmemb,top-y,top,ptrs,2);
				ptrs[0]++;ptrs[1]++;
			}
		}
		else if (ch == 27) {return 0;}
		else if (ch == 10) {
			if(!size)continue;
			if (cb.func[data->ptrs[1]] == NULL) return 1;
			int res = cb.func[data->ptrs[1]](win, data, cb.args[data->ptrs[1]]);
			return res;
		} else {
			int index = search_binding(ch, bind);
			if (index != -1) {
				void* param = size ? cb.args[data->ptrs[1]] : NULL;
				return bind.func[index](win, data, param);
			}
		}
	}
}

int load_data(struct List** list) {
	char* HOME = getenv("HOME");
	int datadir_size = strlen(HOME)+20;
	char* datadir = calloc(datadir_size+1,1);
	char* datafile = calloc(datadir_size+9+1, 1);
	strcpy(datadir, HOME); strcat(datadir, "/.local/share/etodo/");
	strcpy(datafile, datadir); strcat(datafile, "data.json");
	create_dir_if_not_exist(datadir);
	struct stat st;
	int size = 0;
	if (stat(datafile, &st) == -1) {
		struct json_object* jobj = json_object_new_object();
		json_object_object_add(jobj, "Default", json_object_new_object());
		const char *json = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
		FILE* F = fopen(datafile, "w");
		fputs(json, F);
		fclose(F);
		(*list)[0].id = "Default"; // FIXME ERROR Not initialized
		(*list)[0].head = NULL;
		free((void*)json);
		(*list)[0].size = 0;
		size = 1;
	} else {
		FILE* F = fopen(datafile, "r");
		char* buff = calloc(st.st_size+1,1);
		fread(buff, 1, st.st_size, F);
		fclose(F);
		struct json_object *jobj = json_tokener_parse(buff);
		int i=0;
		json_object_object_foreach(jobj, key, value) {
			*list = realloc(*list, sizeof(struct List)*(i+1));
			(*list)[i].id=key;
			(*list)[i].head=NULL;
			(*list)[i].size=0;
			int j=0;
			struct json_object* kobj = value;
			json_object_object_foreach(kobj, k, v) {
				list_add(&(*list)[i], k, json_object_get_int(v));
				(*list)[i].size++;
				j++;
			}
			i++;
		}
		size = i;
		free(buff);
	}
	free(datadir); free(datafile);
	return size;
}

void write_data(struct List* list, int size) {
	char* HOME = getenv("HOME");
	int datadir_size = strlen(HOME)+20;
	char* datadir = calloc(datadir_size+1,1);
	char* datafile = calloc(datadir_size+9+1, 1);
	strcpy(datadir, HOME); strcat(datadir, "/.local/share/etodo/");
	strcpy(datafile, datadir); strcat(datafile, "data.json");
	FILE *F = fopen(datafile, "w");
	struct json_object* jobj = json_object_new_object();
	for (int i=0; i<size; i++) {
		struct json_object* kobj = json_object_new_object();
		for (int j=0; j<list[i].size; j++) {
			json_object_object_add(kobj, list_get(&list[i], j)->id, json_object_new_int(list_get(&list[i], j)->status));
		}
		json_object_object_add(jobj, list[i].id, kobj);
	}
	const char *json = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
	fputs(json, F);
	fclose(F);
}

int task_check(WINDOW* win, struct Data* data, void* _task) {
	struct List* list_of_lists = (struct List*)((void**)data->data)[2];
	int* size = (int*)((void**)data->data)[3];
	struct Task* task = (struct Task*)_task;
	int p = data->ptrs[1];
	switch (task->status) {
		case 1:
			mvwaddch(win, p, 1, ' ');
			task->status = 0;
			break;
		case 0:
			mvwaddch(win, p, 1, 'x');
			task->status = 1;
			break;
	}
	write_data(list_of_lists, *size);
	return 1;
}

int open_list(WINDOW* win, struct Data* data, void* _list) {
	wmove(win,0,0); wclrtobot(win); wrefresh(win);
	struct List* list = (struct List*)_list;
	struct Nopt nopt; nopt.underline=0; nopt.str_size=50;
	void** ogdata = (void**)data->data;
	struct List* list_of_lists = (struct List*)ogdata[1];
	int size = *((int*)ogdata[2]);
	struct Callback cb;
	struct Data _data;
	void* data_arr[5] = {&nopt, list, list_of_lists, &size, &cb};
	_data.islist=0; _data.wins=data->wins; _data.wins_size=data->wins_size;
	_data.data=data_arr;
	cb.func = malloc(sizeof(int(*)(WINDOW*, struct Data*, void*))*list->size);
	cb.args = malloc(sizeof(void*)*list->size);
	char **ls = malloc(sizeof(char*)*list->size);
	for (int i=0; i<list->size; i++) {
		cb.func[i] = task_check;
		cb.args[i] = (void*)list_get(list, i);
		ls[i] = list_get(list, i)->id;
	}
	cb.nmemb=list->size;
	_data.ls = ls;
	int keys[7] = {'a', 's', 'q', 'D', 'r', 'o', 'l'};
	int (*func[7])(WINDOW*, struct Data*,void*) = {add_task, goback, quit, del_task, rename_task};
	struct Binding bind = {keys, func, 5};
	int ptrs[2] = {0,0};
	for (;;) {
		if (menu(win, cb, &_data, bind, ptrs, display_opts)) {
			wmove(win, 0, 0); wclrtobot(win);
		} else break;
	}
	return 1;
}

int add_task(WINDOW* win, struct Data* data, void* _task) {
	struct List* list = (struct List*)((void**)data->data)[1];
	struct List* list_of_lists = (struct List*)((void**)data->data)[2];
	int* size = (int*)((void**)data->data)[3];
	struct Callback* cb = (struct Callback*)((void**)data->data)[4];
	int y, x; getmaxyx(stdscr, y, x);
	WINDOW* _win = newwin(3, 30, y/2-1, x/2-15);
	wbkgd(_win, COLOR_PAIR(3));
	keypad(_win, 1);
	box(_win, ACS_VLINE, ACS_HLINE);
	mvwaddstr(_win, 0, 1, "Añadir tarea");
	mvwaddstr(_win, 1, 2, "Tarea: ");
	wrefresh(_win);
	char* ptr=NULL;
	int r = ampsread(_win, &ptr, 1, 9, 19, 50, 0, 1);
	if (!r) {
		list_add(list, ptr, 0);

		data->ls = realloc(data->ls, sizeof(char*)*(list->size+1));
		data->ls[list->size] = ptr;
		cb->func = realloc(cb->func, sizeof(int(*)(WINDOW*, struct Data*, void*))*(list->size+1));
		cb->func[list->size] = task_check;
		cb->args = realloc(cb->args, sizeof(void*)*(list->size+1));
		cb->args[list->size] = list_get(list, list->size);
		cb->nmemb++;

		list->size++;
		write_data(list_of_lists, *size);
	}
	return 1;
}

int goback(WINDOW* win, struct Data* data, void* _task) {return 0;}
int quit(WINDOW* win, struct Data* data, void* _task) {
	endwin();
	exit(0);
}

void _popat(void** var, int index, int size) {
	var[index] = NULL;
	for (int i=index; i<size-1; i++) {
		void* temp = var[i];
		var[i] = var[i+1];
		var[i+1] = temp;
	}
}

int del_task(WINDOW* win, struct Data* data, void* _task) {
	if (!_task) return 1;
	struct List* list = (struct List*)((void**)data->data)[1];
	struct List* list_of_lists = (struct List*)((void**)data->data)[2];
	int* size = (int*)((void**)data->data)[3];
	struct Callback* cb = (struct Callback*)((void**)data->data)[4];
	int r = list_popat(list, data->ptrs[1]);
	if (!r) return 1;
	_popat((void**)data->ls, data->ptrs[1], list->size);
	data->ls = realloc(data->ls, sizeof(char*)*(list->size-1));
	_popat((void**)cb->func, data->ptrs[1], list->size);
	cb->func = realloc(cb->func, sizeof(int(*)(WINDOW*, struct Data*, void*))*(list->size+1));
	_popat((void**)cb->args, data->ptrs[1], list->size);
	cb->args = realloc(cb->args, sizeof(void*)*(list->size-1));
	cb->nmemb--;
	list->size--;
	if (data->ptrs[1] && data->ptrs[1] == list->size) {
		data->ptrs[0]--;data->ptrs[1]--;
	}
	write_data(list_of_lists, *size);
	return 1;
}

int rename_task(WINDOW* win, struct Data* data, void* _task) {
	if (!_task) return 1;
	struct Task* task = (struct Task*)_task;
	struct List* list = (struct List*)((void**)data->data)[1];
	struct List* list_of_lists = (struct List*)((void**)data->data)[2];
	int* size = (int*)((void**)data->data)[3];
	struct Callback* cb = (struct Callback*)((void**)data->data)[4];
	int y, x; getmaxyx(stdscr, y, x);
	WINDOW* _win = newwin(3, 30, y/2-1, x/2-15);
	wbkgd(_win, COLOR_PAIR(3));
	keypad(_win, 1);
	box(_win, ACS_VLINE, ACS_HLINE);
	mvwaddstr(_win, 0, 1, "Renombrar tarea");
	mvwaddstr(_win, 1, 2, "Tarea: ");
	wrefresh(_win);
	int r = ampsread(_win, &task->id, 1, 9, 19, 50, 0, 1);
	if (!r) {
		data->ls[data->ptrs[1]] = task->id;
		write_data(list_of_lists, *size);
	}
	return 1;
}
