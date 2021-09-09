/*
 * 	Name: Telephrase
 * 
 *	Description: 
 *	A game that tests the players ability to type a qoute the fastest
 * 	For each correctly typed qoute, you are given points. The first player to reach 100 points, wins. 
 */

#include "server.h"

#define MAX_PLAYERS 3
#define BUFFER_SIZE 2048
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x)[0])

player* players[MAX_PLAYERS];


static char* all_ready[MAX_PLAYERS]; // # of players that are ready to begin
static int player_count = 0;
static int uid = 10;
static char* current_phrase; 

pthread_mutex_t players_mutex = PTHREAD_MUTEX_INITIALIZER;

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
 * Sends a message to the connected players
 */
void message_players(char *s, int uid) {
	pthread_mutex_lock(&players_mutex);

	for(int i = 0; i < MAX_PLAYERS; ++i) {
		if(players[i]) {
			if(players[i]->user_id != uid) // don't send msg to sender
				write(players[i]->sock_fd, s, strlen(s));
		}
	}

	pthread_mutex_unlock(&players_mutex);
}

/*
 *	Add player to the server queue
 */
void add_player(player* p){
	pthread_mutex_lock(&players_mutex);

	for(int i = 0; i < MAX_PLAYERS; ++i){
		if(!players[i]){
			players[i] = p;
			break;
		}
	}

	pthread_mutex_unlock(&players_mutex);
}

/*
 * Removes a player from the server queue
 */
void remove_player(int uid){
	pthread_mutex_lock(&players_mutex);

	for(int i = 0; i < MAX_PLAYERS; ++i){
		if(players[i]) {
			if(players[i]->user_id == uid) {
				players[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&players_mutex);
}

/*
 * Handler for a player thread communication
 */
void* player_handler(void* arg) {
	int leave = 0; // leave server flag	
	char name[32];
	char buffer[BUFFER_SIZE];
	

	player_count++;
	player *p = (player*)arg;

	// validate player name
	if(recv(p->sock_fd, name, 32, 0) <= 0 || strlen(name) >= 31){
		printf("Please enter a valid username.\n");
		leave = 1;
	} 
	else {
		strcpy(p->user_name, name);
		sprintf(buffer, "%s has joined the game!\n", p->user_name);
		printf("%s", buffer);
		message_players(buffer, p->user_id);
	}

	bzero(buffer, BUFFER_SIZE);

	// handle communication
	while(1) {
		if (leave)
			break;
		
		int receive = recv(p->sock_fd, buffer, BUFFER_SIZE, 0);
		
		// handle player sending message
		if (receive > 0) {
			if (strlen(buffer) > 0) {
				message_players(buffer, p->user_id);
				trim_str(buffer, strlen(buffer));
				printf("%s: %s\n", buffer, p->user_name);
			}
		} 
		
		// handle player leaving game
		if (receive == 0 || strcmp(buffer, ":exit") == 0) {
			sprintf(buffer, "%s has left the game\n", p->user_name);
			printf("%s", buffer);
			message_players(buffer, p->user_id);
			leave = 1;
		} 
		
		bzero(buffer, BUFFER_SIZE);
	}

	// Remove player
	close(p->sock_fd);
	remove_player(p->user_id);
	free(p);
	
	player_count --;
	pthread_detach(pthread_self());
	return NULL;
}

/*
 * Check if game is ready to start
 */
int check_status() {
	if (player_count == MAX_PLAYERS && ARRAY_SIZE(all_ready) == player_count)
		return 1;
	return 0;
}


/*
 * Game handler
 */
void start_game() {
	// list of telephrase phrases
	const char* phrases[5] = {
		"Peter Piper picked a peck of pickled peppers",
		"How much wood would a woodchuck chuck if a woodchuck could chuck wood",
		"I saw a kitten eating chicken in the kitchen",
		"Lesser leather never weather",
		"Eleven benevolent elephants were yelling at David Letterman"
	};
	// 			TODO: 
	// iterate through questions, 
	// give users x amount of time to answer,
	// award points for correct answers, 
	// update scoreboard, 
	// check if game is finished(max_score reached)
}

void set_commands() {
	const char* commands[3] = {
		":htp",
		":exit",
		":scoreboard"
	};
}

/*
 * starter function
 */
int main() {
	
	int option = 1;
	int listener = 0, connection = 0; // socket file descriptors
	
	struct sockaddr_in server_addr;
	struct sockaddr_in player_addr;

	pthread_t tid;

	// socket info
	listener = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);
	signal(SIGPIPE, SIG_IGN);

	setsockopt(listener, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option));
	bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr));
	listen(listener, 10);

	printf("\nServer up and running\n");
	set_commands();

	while (1) {
		socklen_t player_len = sizeof(player_addr);
		connection = accept(listener, (struct sockaddr*)&player_addr, &player_len);

		if ((player_count + 1) == MAX_PLAYERS) {
			printf("The game server is currently full. Try again later\n");
			close(connection);
			break;
		}

		// player settings
		player* p = (player*)malloc(sizeof(player));
		p->address = player_addr;
		p->sock_fd = connection;
		p->user_id = uid++;

		// Add player to the server queue
		add_player(p);
		pthread_create(&tid, NULL, &player_handler, (void*)p);

		// Check if all players are ready & start the game
		if (check_status() == 1)
			start_game();

		sleep(2.5);
	}

	return 0;
}



