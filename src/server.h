#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#ifndef SERVER_H   
#define SERVER_H

/* */
void flush_stdout();

/* */
void trim_str(char* arr, int len);

/* */
void exit_handler(int sig);

/* */
static char* ip = "127.0.0.1"; 
static int port = 9999;

/* player structure */
typedef struct {
	struct sockaddr_in address;
	char user_name[32];
	int sock_fd;
	int user_id;
	int score;
} player;

// list of game commands
extern const char* commands[5];

#endif 