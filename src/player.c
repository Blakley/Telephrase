#include "server.h"

#define MESSAGE_LENGTH 2048

//
static int sock_fd = 0;
static char player_name[32];
volatile sig_atomic_t exit_flag = 0;


/*
 * Handles users pressing ctrl+c
 */
void exit_handler(int sig) {
    exit_flag = 1;
}

/*
 * flushes stdout
 */
void flush_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}

/*
 * Creats a standard string
 */
void trim_str(char* arr, int len) {
	for (int i = 0; i < len; i++) {
		if (arr[i] == '\n') {
			arr[i] = '\0';
			break;
		}
	}
}

/*
 * Sends a message to the game server
 */
void send_message() {
    char message[MESSAGE_LENGTH] = {};
    char buffer[MESSAGE_LENGTH + 32] = {};

    while(1) {
        flush_stdout();
        fgets(message, MESSAGE_LENGTH, stdin);
        trim_str(message, MESSAGE_LENGTH);

        if (strcmp(message, "exit") == 0) {
            break;
        } 
        else {
            sprintf(buffer, "%s: %s\n", player_name, message);
            send(sock_fd, buffer, strlen(buffer), 0);
        }

        bzero(message, MESSAGE_LENGTH);
        bzero(buffer, MESSAGE_LENGTH + 32);
    }

    exit_handler(2);
}

/*
 * Recieves messages from the game server
 */
void recieve_message() {
    char message[MESSAGE_LENGTH] = {};
    while (1) {
        int receive = recv(sock_fd, message, MESSAGE_LENGTH, 0);

        if (receive > 0) {
            printf("%s", message);
            flush_stdout();
        } 
        if (receive == 0)
            break; 
        memset(message, 0, sizeof(message));
    }
}


/*
 *	New player welcome message
 */
void welcome_msg() {
	char* line = "----------------------------------------------\n";
	printf("%s", line);
	printf("\tWelcome to Telephrase\n");
	printf("Don't know how to play? Simply type ':htp'\n");
	printf("Once there are 3 players, the game will start!\n");
	printf("%s", line);
	printf("\n");
}

int main(int argc, char const *argv[]) {
	signal(SIGINT, exit_handler);

    printf("Create a user name: ");
    fgets(player_name, 32, stdin);
    trim_str(player_name, strlen(player_name));

    // check name
    if (strlen(player_name) > 32 || strlen(player_name) < 1) {
        printf("Invalid name length.\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;

    // Socket info
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    send(sock_fd, player_name, 32, 0);

    welcome_msg();

    pthread_t send_msg_thread;
    if(pthread_create(&send_msg_thread, NULL, (void *) send_message, NULL) != 0){
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    pthread_t recv_msg_thread;
    if(pthread_create(&recv_msg_thread, NULL, (void *) recieve_message, NULL) != 0){
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    while (1){
        if (exit_flag){
            // printf("\nThanks for playing\n");
            break;
        }
    }

    close(sock_fd);
	return 0;
}