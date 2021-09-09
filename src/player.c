#include "server.h"

#define MESSAGE_LENGTH 2048

//
static int sock_fd = 0;
static char player_name[32];
volatile sig_atomic_t exit_flag = 0;

/*
 * Sends a message to the game server
 */
void message_server() {

}

/*
 * Recieves messages from the game server
 */
void recieve_message() {

}

/*
 * Handles users pressing ctrl+c
 */
void exit_handler(int sig) {
    exit_flag = 1;
}

int main(int argc, char const *argv[])
{
	/* code */
    printf("hello\n");
	return 0;
}