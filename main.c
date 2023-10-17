#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include <locale.h>
#include "gears.h"
#include "logger/logger.h"

int main() {
	setlocale(LC_ALL, "");
	WINDOW* stdscr = initscr();
	start_color(); use_default_colors();
	curs_set(0); noecho();
    	init_pair(1,0,15);  // Fondo blanco con letras negras
    	init_pair(3,15,20); // Fondo azul con letras blancas
	int std_y, std_x; getmaxyx(stdscr,std_y,std_x);
	WINDOW* upbar = newwin(1, std_x, 0, 0);
	WINDOW* main = newwin(std_y-3, std_x, 2, 0);
	WINDOW* lowbar = newwin(1, std_x, std_y-1, 0);
	WINDOW* wins[3] = {main, upbar, lowbar};

	keypad(main, 1);
	wbkgd(upbar,COLOR_PAIR(1));

	mvwaddstr(upbar, 0, 2, "ETODO");
	mvwaddstr(upbar, 0, std_x/2-8, "Listas de tareas");
	wattron(lowbar, COLOR_PAIR(1));
		mvwaddstr(lowbar, 0, 0, " ? ");
	wattroff(lowbar, COLOR_PAIR(1));
	mvwaddstr(lowbar, 0, 4, "Ayuda");
	
	wrefresh(upbar); wrefresh(main);
	wrefresh(lowbar);

	struct List* list=NULL;
	int list_size = load_data(&list);
	struct Callback cb;
	cb.func = malloc(sizeof(int(*)(WINDOW*, struct Data*, void*))*list_size);
	cb.args = malloc(sizeof(void*)*list_size);
	char **ls = list_size ? malloc(sizeof(char*)*list_size) : NULL;
	for (int i=0; i<list_size; i++) {
		cb.func[i] = open_list;
		cb.args[i] = (void*)&(list[i]);
		ls[i] = list[i].id;
	}
	cb.nmemb = list_size;
	struct Nopt nopt; nopt.underline=0; nopt.str_size=30;
	void* _data[3] = {&nopt, list, &list_size};
	struct Data data; data.wins=wins; data.wins_size=3; data.data=_data; data.ls=ls;
	data.islist = 1;
	struct Binding bind = {NULL, NULL, 0};
	int ptrs[2] = {0,0};
	for (;;) {
		if (menu(main, cb, &data, bind, ptrs, display_opts)) {
			wmove(main, 0, 0); wclrtobot(main);
		} else break;
	}
	free(cb.func);
	free(cb.args);
	free(ls);
	endwin();
}
