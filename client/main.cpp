#include <iostream>
#include <curses.h>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/select.h>
#include <pthread.h>
#include <getopt.h>
#include <chrono>
#include "header/zone.h"
#include "header/logger.h"
#include "header/net_service.h"
using namespace std;

typedef std::chrono::high_resolution_clock chrono_clock;
typedef std::chrono::microseconds res;

static bool log_enable = true;
static bool flag = true;
pthread_t net_th;
net_service* service;

int ROW = 30;
int COL = 50;
int port = 0;
char* host = nullptr;

void init_ui()
{
    initscr();

    if(has_colors()){ //must support colors
        start_color();
    } else {
        endwin();
        printf("your system doesn't suport colors!\nAbort.\n\n");
        exit(1);
    }
    curs_set(0);//hide cursor
    cbreak();
    nonl();
    noecho();
    intrflush(stdscr,FALSE);
    keypad(stdscr,TRUE);
    refresh();
}

void end_ui() {
    getch();
    endwin();
}

static void *net_thread(void*) {
    chrono_clock::time_point last(res::zero());//start time
    int key;
    while(flag){
        //int ret = select(max_client_fd+1,&client_fdset, nullptr, nullptr,
        service->read();

        chrono_clock::time_point now = chrono_clock::now();
        long nsec = std::chrono::duration_cast<res>(now - last).count();
        last = chrono_clock::now();

        char fps_str[20];
        sprintf(fps_str,"%ld fps",1000*1000/nsec);
        mvaddnstr(2,COL*2+5,fps_str,strlen(fps_str));
        refresh();

        if(service->faild_times>=3) flag = false;
    }
    return nullptr;
}

void print_help() {
    cout << "usage: snake_client -H <host> -p <port> -c <colns> -r <rowns>" << endl;
}

int main(int argc, char *argv[]) {
    int opt;
    const char *str = "hH:p:c:r:";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt) {
            case 'p': {
                port = atoi(optarg);
                break;
            }
            case 'H': {
                host = optarg;
                break;
            }
            case 'c': {
                COL = atoi(optarg);
                break;
            }
            case 'r': {
                ROW = atoi(optarg);
                break;
            }
            case 'h': {
                print_help();
                exit(0);
                break;
            }
            default:
                break;
        }
    }

    if(port <= 0 || ROW <= 0 || COL <= 0) {
        cout << "illegal args." << endl;
        exit(1);
    }


    init_ui();

    zone _zone(ROW,COL);

    service = new net_service(host, port, &_zone);
    int ret=service->connect();

    int thread_ret = pthread_create(&net_th, nullptr,net_thread, nullptr);
    if(thread_ret) {
        end_ui();
        printf("thread error\n");
        exit(1);
    }


    int input;
    while(flag && input!='q') {
        input = getch();
        if(input==KEY_UP) {
            service->send("UP");
        } else if(input==KEY_DOWN) {
            service->send("DOWN");
        } else if(input==KEY_LEFT) {
            service->send("LEFT");
        } else if(input==KEY_RIGHT) {
            service->send("RIGHT");
        } else
            service->send("UNKNOWN");
    }

    //Todo:处理内存泄露

    end_ui();
    return 0;
}















