#include <asm-generic/socket.h>
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
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
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

void display_opts(WINDOW* win, struct Data* data, int size, int p, int start, int stop, int attrs) {
	void** _data = (void**)data->data;
	struct Nopt* nopt = (struct Nopt*)_data[0];
	char *str = malloc(nopt->str_size); str[nopt->str_size-1]=0;
	if (stop==-1) {
		if (data->islist) {
			char *buff = calloc(4,1);
			snprintf(buff, 4, "%d", list_get((struct Lists*)_data[1],start)->size);
			mvwaddstr(win, p, 0, buff);
			free(buff);
		} else {
			if (task_get((struct List*)_data[1],start)->status) mvwaddstr(win, p, 0, "[x]");
			else mvwaddstr(win, p, 0, "[ ]");
		}
		memset(str, ' ', nopt->str_size-1);strncpy(str, data->ls[start], nopt->str_size);
		wmove(win, p, 4); wclrtoeol(win);
		if (attrs!=-1)wattron(win, attrs);
		mvwaddnstr(win, p, 4, str, nopt->str_size-1);
		if (attrs!=-1)wattroff(win, attrs);
		free(str);
		return;
	}
	int e=p;
	if (stop>size) stop=size;
	for (int i=start; i<stop; i++) {
		if (data->islist) {
			char *buff = calloc(4,1);
			snprintf(buff, 4, "%d", list_get((struct Lists*)_data[1],i)->size);
			mvwaddstr(win, e, 0, buff);
			free(buff);
		} else {
			if (task_get((struct List*)_data[1],i)->status) mvwaddstr(win, e, 0, "[x]");
			else mvwaddstr(win, e, 0, "[ ]");
		}
		mvwaddnstr(win, e, 4, data->ls[i], nopt->str_size-1);
		e++;
	}
	free(str);
}

void mscroll(WINDOW* win, struct Data* data, int size, int ptrs[2], int mode) {
	switch (mode) {
		case 0:
			data->menu.dcb(win, data, size, ptrs[0], ptrs[1], -1, -1);
			wscrl(win, -1);
			data->menu.dcb(win, data, size, ptrs[0], ptrs[1]-1, -1, COLOR_PAIR(1));
			ptrs[1]--;
			return;
		case 1:
			data->menu.dcb(win, data, size, ptrs[0], ptrs[1], -1, -1);
			wscrl(win, 1);
			data->menu.dcb(win, data, size, ptrs[0], ptrs[1]+1, -1, COLOR_PAIR(1));
			ptrs[1]++;
			return;
	}
}

int menu(WINDOW* win, struct Callback cb, struct Data* data, struct Binding bind, int ptrs[2]) {
	int y = getmaxy(win);
	if (cb.nmemb) {
		data->menu.dcb(win, data, cb.nmemb, ptrs[0], ptrs[1], -1, COLOR_PAIR(1));
	} else mvwaddstr(win, 0, 4, "No hay tareas por aquí.");
	for (;;) {
		int ch = wgetch(win);
		switch (ch) {
			case 27:
				return 0;
			case 10:
				if (!cb.nmemb) break;
				return cb.func[ptrs[1]](win, data, cb.args[ptrs[1]]);
			case KEY_UP:
				if (!cb.nmemb||!ptrs[1]) break;
				if (!ptrs[0]) {
					mscroll(win, data, cb.nmemb, data->menu.ptrs, 0);
					break;
				}
				data->menu.dcb(win, data, cb.nmemb, ptrs[0], ptrs[1], -1, -1);
				data->menu.dcb(win, data, cb.nmemb, ptrs[0]-1, ptrs[1]-1, -1, COLOR_PAIR(1));
				ptrs[0]--;ptrs[1]--;
				break;
			case KEY_DOWN:
				if (!cb.nmemb||ptrs[1]==cb.nmemb-1) break;
				if (ptrs[0]==y-1) {
					mscroll(win, data, cb.nmemb, data->menu.ptrs, 1);
					break;
				}
				data->menu.dcb(win, data, cb.nmemb, ptrs[0], ptrs[1], -1, -1);
				data->menu.dcb(win, data, cb.nmemb, ptrs[0]+1, ptrs[1]+1, -1, COLOR_PAIR(1));
				ptrs[0]++;ptrs[1]++;
				break;
			default: {
				int index = search_binding(ch, bind);
				if (index != -1) {
					void* param = cb.nmemb ? cb.args[data->menu.ptrs[1]] : NULL;
					return bind.func[index](win, data, param);
				}
			}
		}
	}
	return 1;
}

int load_data(struct Lists* lists) {
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
		/* (*list)[0].id = "Default"; // FIXME ERROR Not initialized
		(*list)[0].head = NULL;
		free((void*)json);
		(*list)[0].size = 0; */
		size = 1;
	} else {
		FILE* F = fopen(datafile, "r");
		char* buff = calloc(st.st_size+1,1);
		fread(buff, 1, st.st_size, F);
		fclose(F);
		struct json_object *jobj = json_tokener_parse(buff);
		int i=0;
		json_object_object_foreach(jobj, key, value) {
			list_add(lists, key);
			int j=0;
			struct json_object* kobj = value;
			json_object_object_foreach(kobj, k, v) {
				task_add(list_get(lists, i), k, json_object_get_int(v));
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

void write_data(struct Lists* lists, int size) {
	char* HOME = getenv("HOME");
	int datadir_size = strlen(HOME)+20;
	char* datadir = calloc(datadir_size+1,1);
	char* datafile = calloc(datadir_size+9+1, 1);
	strcpy(datadir, HOME); strcat(datadir, "/.local/share/etodo/");
	strcpy(datafile, datadir); strcat(datafile, "data.json");
	struct json_object* jobj = json_object_new_object();
	for (int i=0; i<size; i++) {
		struct json_object* kobj = json_object_new_object();
		for (int j=0; j<list_get(lists, i)->size; j++) {
			json_object_object_add(kobj, task_get(list_get(lists,i), j)->id, json_object_new_int(task_get(list_get(lists,i), j)->status));
		}
		json_object_object_add(jobj, list_get(lists,i)->id, kobj);
	}
	const char *json = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
	FILE *F = fopen(datafile, "w");
	fputs(json, F);
	fclose(F);
}

int task_check(WINDOW* win, struct Data* data, void* _task) {
	struct Lists* list_of_lists = (struct Lists*)((void**)data->data)[2];
	int* size = (int*)((void**)data->data)[3];
	struct Task* task = (struct Task*)_task;
	int p = data->menu.ptrs[0];
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
	write_data(list_of_lists, list_of_lists->size);
	return 1;
}

int open_list(WINDOW* win, struct Data* data, void* _list) {
	WINDOW* main = dupwin(win);
	wmove(main,0,0); wclrtobot(main);
	struct List* list = (struct List*)_list;
	struct Nopt nopt; nopt.underline=0; nopt.str_size=50;
	void** ogdata = (void**)data->data;
	struct Lists* list_of_lists = (struct Lists*)ogdata[1];
	int size = *((int*)ogdata[2]);
	struct Callback cb;
	struct Data _data;
	void* data_arr[5] = {&nopt, list, list_of_lists, &size, &cb};
	int ptrs[2] = {0,0};
	_data.islist=0; _data.wins=data->wins; _data.wins_size=data->wins_size;
	_data.data=data_arr; _data.menu.dcb=display_opts; _data.menu.ptrs=ptrs;
	_data.wins[0]=main;
	cb.func = malloc(sizeof(int(*)(WINDOW*, struct Data*, void*))*list->size);
	cb.args = malloc(sizeof(void*)*list->size);
	char **ls = malloc(sizeof(char*)*list->size);
	for (int i=0; i<list->size; i++) {
		cb.func[i] = task_check;
		cb.args[i] = (void*)task_get(list, i);
		ls[i] = task_get(list, i)->id;
	}
	cb.nmemb=list->size;
	_data.ls = ls;
	int keys[7] = {'a', 's', 'q', 'D', 'r', 'o', 'l'};
	int (*func[7])(WINDOW*, struct Data*,void*) = {add_task, goback, quit, del_task, rename_task, move_up, move_down};
	struct Binding bind = {keys, func, 7};
	int y = getmaxy(main);
	_data.menu.dcb(main, &_data, cb.nmemb, 0, 0, y, -1);
	for (;;) {
		if (!menu(main, cb, &_data, bind, _data.menu.ptrs)) {break;}
	}
	delwin(main);
	touchwin(win);
	wrefresh(win);
	return 1;
}

int add_task(WINDOW* win, struct Data* data, void* _task) {
	struct List* list = (struct List*)((void**)data->data)[1];
	struct Lists* list_of_lists = (struct Lists*)((void**)data->data)[2];
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
		task_add(list, ptr, 0);

		data->ls = realloc(data->ls, sizeof(char*)*(list->size));
		data->ls[list->size-1] = ptr;

		cb->func = realloc(cb->func, sizeof(int(*)(WINDOW*, struct Data*, void*))*(list->size));
		cb->func[list->size-1] = task_check;
		cb->args = realloc(cb->args, sizeof(void*)*(list->size));
		cb->args[list->size-1] = task_get(list, list->size-1);
		cb->nmemb++;

		write_data(list_of_lists, list_of_lists->size);

		if (cb->nmemb<y-1) {data->menu.dcb(win, data, cb->nmemb, cb->nmemb-1, cb->nmemb-1, -1, -1);}
	}
	delwin(_win);
	touchwin(win);
	wrefresh(win);
	return 1;
}
int add_list(WINDOW* win, struct Data* data, void* _list) {
	//struct List* list = (struct List*)((void**)data->data)[1];
	struct Lists* list_of_lists = (struct Lists*)((void**)data->data)[1];
	int* size = (int*)((void**)data->data)[2];
	struct Callback* cb = (struct Callback*)((void**)data->data)[3];
	int y, x; getmaxyx(stdscr, y, x);
	WINDOW* _win = newwin(3, 30, y/2-1, x/2-15);
	wbkgd(_win, COLOR_PAIR(3));
	keypad(_win, 1);
	box(_win, ACS_VLINE, ACS_HLINE);
	mvwaddstr(_win, 0, 1, "Añadir lista");
	mvwaddstr(_win, 1, 2, "Lista: ");
	wrefresh(_win);
	char* ptr=NULL;
	int r = ampsread(_win, &ptr, 1, 9, 19, 50, 0, 1);
	if (!r) {
		list_add(list_of_lists, ptr);

		data->ls = realloc(data->ls, sizeof(char*)*(list_of_lists->size));
		data->ls[list_of_lists->size-1] = ptr;

		cb->func = realloc(cb->func, sizeof(int(*)(WINDOW*, struct Data*, void*))*(list_of_lists->size));
		cb->func[list_of_lists->size-1] = open_list;
		cb->args = realloc(cb->args, sizeof(void*)*(list_of_lists->size));
		cb->args[list_of_lists->size-1] = list_get(list_of_lists, list_of_lists->size-1);
		cb->nmemb++;
		(*size)++;

		write_data(list_of_lists, list_of_lists->size);

		if (cb->nmemb<y-1) {data->menu.dcb(win, data, cb->nmemb, cb->nmemb-1, cb->nmemb-1, -1, -1);}
	}
	delwin(_win);
	touchwin(win);
	wrefresh(win);
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
	struct Lists* list_of_lists = (struct Lists*)((void**)data->data)[2];
	int* size = (int*)((void**)data->data)[3];
	struct Callback* cb = (struct Callback*)((void**)data->data)[4];
	int y = getmaxy(win);

	int r = task_popat(list, data->menu.ptrs[1]);
	if (!r) return 1;

	_popat((void**)data->ls, data->menu.ptrs[1], list->size+1); data->ls = realloc(data->ls, sizeof(char*)*(list->size));
	_popat((void**)cb->func, data->menu.ptrs[1], list->size+1); cb->func = realloc(cb->func, sizeof(int(*)(WINDOW*, struct Data*, void*))*(list->size));
	_popat((void**)cb->args, data->menu.ptrs[1], list->size+1); cb->args = realloc(cb->args, sizeof(void*)*(list->size));
	cb->nmemb--;
	//list->size--;
	int top = (data->menu.ptrs[1]-data->menu.ptrs[0])+(y-1);
	if (list->size >= y) {
		if (top == list->size) {
			/*print from up top until sp (including sp)*/
			data->menu.ptrs[1]--;
			int lowtop = data->menu.ptrs[1] - data->menu.ptrs[0];
			data->menu.dcb(win, data, cb->nmemb, 0, lowtop, data->menu.ptrs[1]+1, COLOR_PAIR(1));
		} else {
			/*print from sp until bottom*/
			data->menu.dcb(win, data, cb->nmemb, data->menu.ptrs[0], data->menu.ptrs[1], top+1, COLOR_PAIR(1));
		}
	} else {
		wmove(win, data->menu.ptrs[0], 0); wclrtobot(win);
		data->menu.dcb(win, data, cb->nmemb, 0, 0, y, -1);
		if (data->menu.ptrs[1] == list->size && data->menu.ptrs[1]) {
			data->menu.ptrs[0]--; data->menu.ptrs[1]--;
		}
	}
	write_data(list_of_lists, list_of_lists->size);
	return 1;
}
int del_list(WINDOW* win, struct Data* data, void* _list) {
	if (!_list) return 1;
	struct Lists* list_of_lists = (struct Lists*)((void**)data->data)[1];
	int* size = (int*)((void**)data->data)[2];
	struct Callback* cb = (struct Callback*)((void**)data->data)[3];
	int y = getmaxy(win);

	int r = list_popat(list_of_lists, data->menu.ptrs[1]);
	if (!r) return 1;

	_popat((void**)data->ls, data->menu.ptrs[1], list_of_lists->size+1); data->ls = realloc(data->ls, sizeof(char*)*(list_of_lists->size));
	_popat((void**)cb->func, data->menu.ptrs[1], list_of_lists->size+1); cb->func = realloc(cb->func, sizeof(int(*)(WINDOW*, struct Data*, void*))*(list_of_lists->size));
	_popat((void**)cb->args, data->menu.ptrs[1], list_of_lists->size+1); cb->args = realloc(cb->args, sizeof(void*)*(list_of_lists->size));
	cb->nmemb--;
	//list->size--;
	int top = (data->menu.ptrs[1]-data->menu.ptrs[0])+(y-1);
	if (list_of_lists->size >= y) {
		if (top == list_of_lists->size) {
			/*print from up top until sp (including sp)*/
			data->menu.ptrs[1]--;
			int lowtop = data->menu.ptrs[1] - data->menu.ptrs[0];
			data->menu.dcb(win, data, cb->nmemb, 0, lowtop, data->menu.ptrs[1]+1, COLOR_PAIR(1));
		} else {
			/*print from sp until bottom*/
			data->menu.dcb(win, data, cb->nmemb, data->menu.ptrs[0], data->menu.ptrs[1], top+1, COLOR_PAIR(1));
		}
	} else {
		wmove(win, data->menu.ptrs[0], 0); wclrtobot(win);
		data->menu.dcb(win, data, cb->nmemb, 0, 0, y, -1);
		if (data->menu.ptrs[1] == list_of_lists->size && data->menu.ptrs[1]) {
			data->menu.ptrs[0]--; data->menu.ptrs[1]--;
		}
	}
	write_data(list_of_lists, list_of_lists->size);
	return 1;
}

int rename_task(WINDOW* win, struct Data* data, void* _task) {
	if (!_task) return 1;
	struct Task* task = (struct Task*)_task;
	struct List* list = (struct List*)((void**)data->data)[1];
	struct Lists* list_of_lists = (struct Lists*)((void**)data->data)[2];
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
		data->ls[data->menu.ptrs[1]] = task->id;
		write_data(list_of_lists, list_of_lists->size);
	}
	delwin(_win);
	touchwin(win);
	wrefresh(win);
	return 1;
}
int rename_list(WINDOW* win, struct Data* data, void* _list) {
	if (!_list) return 1;
	struct List* list = (struct List*)_list;
	struct Lists* list_of_lists = (struct Lists*)((void**)data->data)[1];
	int* size = (int*)((void**)data->data)[2];
	struct Callback* cb = (struct Callback*)((void**)data->data)[3];
	int y, x; getmaxyx(stdscr, y, x);
	WINDOW* _win = newwin(3, 30, y/2-1, x/2-15);
	wbkgd(_win, COLOR_PAIR(3));
	keypad(_win, 1);
	box(_win, ACS_VLINE, ACS_HLINE);
	mvwaddstr(_win, 0, 1, "Renombrar tarea");
	mvwaddstr(_win, 1, 2, "Tarea: ");
	wrefresh(_win);
	int r = ampsread(_win, &list->id, 1, 9, 19, 50, 0, 1);
	if (!r) {
		data->ls[data->menu.ptrs[1]] = list->id;
		write_data(list_of_lists, list_of_lists->size);
	}
	delwin(_win);
	touchwin(win);
	wrefresh(win);
	return 1;
}

int move_up(WINDOW* win, struct Data* data, void* _task) {
	if (!_task) return 1;
	struct Task* task = (struct Task*)_task;
	struct List* list = (struct List*)((void**)data->data)[1];
	struct Lists* list_of_lists = (struct Lists*)((void**)data->data)[2];
	int* size = (int*)((void**)data->data)[3];
	struct Callback* cb = (struct Callback*)((void**)data->data)[4];
	int y = getmaxy(win);

	if (!data->menu.ptrs[1]) return 1;
	char* temp = data->ls[data->menu.ptrs[1]];
	data->ls[data->menu.ptrs[1]] = data->ls[data->menu.ptrs[1]-1];
	data->ls[data->menu.ptrs[1]-1] = temp;
	int (*ftemp)(WINDOW*, struct Data*, void*) = cb->func[data->menu.ptrs[1]];
	cb->func[data->menu.ptrs[1]] = cb->func[data->menu.ptrs[1]-1];
	cb->func[data->menu.ptrs[1]-1] = ftemp;
	/*void* atemp = cb->args[data->menu.ptrs[1]];
	cb->args[data->menu.ptrs[1]] = cb->args[data->menu.ptrs[1]-1];
	cb->args[data->menu.ptrs[1]-1] = atemp;*/

	struct Task* Ttemp = task_get(list, data->menu.ptrs[1]);
	char* tid = strdup(Ttemp->id); int tstatus = Ttemp->status;
	struct Task* Ttemp1 = task_get(list, data->menu.ptrs[1]-1);
	task_set(list, data->menu.ptrs[1], Ttemp1->id, Ttemp1->status);
	task_set(list, data->menu.ptrs[1]-1, tid, tstatus);

	if (!data->menu.ptrs[0]) {
		mscroll(win, data, cb->nmemb,data->menu.ptrs, 0);
	} else {data->menu.ptrs[0]--;data->menu.ptrs[1]--;}

	data->menu.dcb(win, data, cb->nmemb, data->menu.ptrs[0]+1, data->menu.ptrs[1]+1, -1, -1);
	data->menu.dcb(win, data, cb->nmemb, data->menu.ptrs[0], data->menu.ptrs[1], -1, COLOR_PAIR(1));

	write_data(list_of_lists, list_of_lists->size);
	return 1;
}
int move_lUp(WINDOW* win, struct Data* data, void* _list) {
	if (!_list) return 1;
	struct List* list = (struct List*)_list;
	struct Lists* list_of_lists = (struct Lists*)((void**)data->data)[1];
	int* size = (int*)((void**)data->data)[2];
	struct Callback* cb = (struct Callback*)((void**)data->data)[3];
	int y = getmaxy(win);

	if (!data->menu.ptrs[1]) return 1;
	char* temp = data->ls[data->menu.ptrs[1]];
	data->ls[data->menu.ptrs[1]] = data->ls[data->menu.ptrs[1]-1];
	data->ls[data->menu.ptrs[1]-1] = temp;
	int (*ftemp)(WINDOW*, struct Data*, void*) = cb->func[data->menu.ptrs[1]];
	cb->func[data->menu.ptrs[1]] = cb->func[data->menu.ptrs[1]-1];
	cb->func[data->menu.ptrs[1]-1] = ftemp;
	/*void* atemp = cb->args[data->menu.ptrs[1]];
	cb->args[data->menu.ptrs[1]] = cb->args[data->menu.ptrs[1]-1];
	cb->args[data->menu.ptrs[1]-1] = atemp;*/

	struct List* Ltemp = list_get(list_of_lists, data->menu.ptrs[1]);
	char* lid = strdup(Ltemp->id);
	int lsize = Ltemp->size;
	struct Task* lhead = Ltemp->head;
	struct List* Ltemp1 = list_get(list_of_lists, data->menu.ptrs[1]-1);
	list_set(list_of_lists, data->menu.ptrs[1], Ltemp1->id, Ltemp1->size, Ltemp1->head);
	list_set(list_of_lists, data->menu.ptrs[1]-1, lid, lsize, lhead);

	if (!data->menu.ptrs[0]) {
		mscroll(win, data, cb->nmemb,data->menu.ptrs, 0);
	} else {data->menu.ptrs[0]--;data->menu.ptrs[1]--;}

	data->menu.dcb(win, data, cb->nmemb, data->menu.ptrs[0]+1, data->menu.ptrs[1]+1, -1, -1);
	data->menu.dcb(win, data, cb->nmemb, data->menu.ptrs[0], data->menu.ptrs[1], -1, COLOR_PAIR(1));

	write_data(list_of_lists, list_of_lists->size);
	return 1;
}

int move_down(WINDOW* win, struct Data* data, void* _task) {
	if (!_task) return 1;
	struct Task* task = (struct Task*)_task;
	struct List* list = (struct List*)((void**)data->data)[1];
	struct Lists* list_of_lists = (struct Lists*)((void**)data->data)[2];
	int* size = (int*)((void**)data->data)[3];
	struct Callback* cb = (struct Callback*)((void**)data->data)[4];
	int y = getmaxy(win);

	if (data->menu.ptrs[1] == cb->nmemb-1) return 1;

	char* temp = data->ls[data->menu.ptrs[1]];
	data->ls[data->menu.ptrs[1]] = data->ls[data->menu.ptrs[1]+1];
	data->ls[data->menu.ptrs[1]+1] = temp;
	int (*ftemp)(WINDOW*, struct Data*, void*) = cb->func[data->menu.ptrs[1]];
	cb->func[data->menu.ptrs[1]] = cb->func[data->menu.ptrs[1]+1];
	cb->func[data->menu.ptrs[1]+1] = ftemp;
	/*void* atemp = cb->args[data->menu.ptrs[1]];
	cb->args[data->menu.ptrs[1]] = cb->args[data->menu.ptrs[1]+1];
	cb->args[data->menu.ptrs[1]+1] = atemp;*/
	
	struct Task* Ttemp = task_get(list, data->menu.ptrs[1]);
	char* tid = strdup(Ttemp->id); int tstatus = Ttemp->status;
	struct Task* Ttemp1 = task_get(list, data->menu.ptrs[1]+1);
	task_set(list, data->menu.ptrs[1], Ttemp1->id, Ttemp1->status);
	task_set(list, data->menu.ptrs[1]+1, tid, tstatus);

	if (data->menu.ptrs[0] == y-1) {
		mscroll(win, data, cb->nmemb,data->menu.ptrs, 1);
	} else {data->menu.ptrs[0]++;data->menu.ptrs[1]++;}

	data->menu.dcb(win, data, cb->nmemb, data->menu.ptrs[0]-1, data->menu.ptrs[1]-1, -1, -1);
	data->menu.dcb(win, data, cb->nmemb, data->menu.ptrs[0], data->menu.ptrs[1], -1, COLOR_PAIR(1));

	write_data(list_of_lists, list_of_lists->size);
	return 1;
}
int move_lDown(WINDOW* win, struct Data* data, void* _list) {
	if (!_list) return 1;
	struct List* list = (struct List*)_list;
	struct Lists* list_of_lists = (struct Lists*)((void**)data->data)[1];
	int* size = (int*)((void**)data->data)[2];
	struct Callback* cb = (struct Callback*)((void**)data->data)[3];
	int y = getmaxy(win);

	if (data->menu.ptrs[1] == cb->nmemb-1) return 1;
	char* temp = data->ls[data->menu.ptrs[1]];
	data->ls[data->menu.ptrs[1]] = data->ls[data->menu.ptrs[1]+1];
	data->ls[data->menu.ptrs[1]+1] = temp;
	int (*ftemp)(WINDOW*, struct Data*, void*) = cb->func[data->menu.ptrs[1]];
	cb->func[data->menu.ptrs[1]] = cb->func[data->menu.ptrs[1]+1];
	cb->func[data->menu.ptrs[1]+1] = ftemp;
	/*void* atemp = cb->args[data->menu.ptrs[1]];
	cb->args[data->menu.ptrs[1]] = cb->args[data->menu.ptrs[1]+1];
	cb->args[data->menu.ptrs[1]+1] = atemp;*/
	
	struct List* Ltemp = list_get(list_of_lists, data->menu.ptrs[1]);
	char* lid = strdup(Ltemp->id);
	int lsize = Ltemp->size;
	struct Task* lhead = Ltemp->head;
	struct List* Ltemp1 = list_get(list_of_lists, data->menu.ptrs[1]+1);
	list_set(list_of_lists, data->menu.ptrs[1], Ltemp1->id, Ltemp1->size, Ltemp1->head);
	list_set(list_of_lists, data->menu.ptrs[1]+1, lid, lsize, lhead);

	if (data->menu.ptrs[0] == y-1) {
		mscroll(win, data, cb->nmemb,data->menu.ptrs, 1);
	} else {data->menu.ptrs[0]++;data->menu.ptrs[1]++;}

	data->menu.dcb(win, data, cb->nmemb, data->menu.ptrs[0]-1, data->menu.ptrs[1]-1, -1, -1);
	data->menu.dcb(win, data, cb->nmemb, data->menu.ptrs[0], data->menu.ptrs[1], -1, COLOR_PAIR(1));

	write_data(list_of_lists, list_of_lists->size);
	return 1;
}

void* _serv_sync() {return NULL;}

void _server(void* args) {
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(4445);
	socklen_t size = sizeof(addr);
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR|SO_REUSEPORT, &opt, sizeof(opt));
	if (bind(sock, (struct sockaddr*)&addr, size) == -1) {
		// An error has ocurred and syncronization with this device is unavailable.
	}
	listen(sock, 1);
	for (;;) {
		int conn = accept(sock, (struct sockaddr*)&addr, &size);
		pthread_t t;
		pthread_create(&t, NULL, _serv_sync, NULL);
		pthread_join(t, NULL);
	}
}

pid_t server_start() {
	pid_t pid = fork();
	if (!pid) {
		_server(NULL);
		exit(1);
	}
	return pid;
}