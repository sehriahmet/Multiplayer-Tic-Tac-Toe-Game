// Ahmet Sehriyaroglu 
// 2581015 

#include <unistd.h> // these 2 may not useful just look for further applications 
#include <stdio.h>
#include <stdlib.h>

#include "game_structs.h"

#include <sys/socket.h>
#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)

typedef struct 
{
    char *player_char;
    int total_argument_count;
    char executable_path[256];
    char **arguments; 
    int player_pid;
} Player;

int grid_width, grid_height, streak_size, player_count;
Player *players;

void read_file() {
    FILE *file;

    file = fopen("input.txt", "r");
    // file = fopen("input_linux.txt", "r");

    fscanf(file, "%d %d %d %d", &grid_width, &grid_height, &streak_size, &player_count);

    players = (Player *)malloc(player_count * sizeof(Player));

    for (int i = 0; i < player_count; i++) {
        fscanf(file, " %c %d", &players[i].player_char, &players[i].total_argument_count);

        fscanf(file, "%s", players[i].executable_path);

        players[i].arguments = (char **)malloc(players[i].total_argument_count * sizeof(char *));

        for (int j = 0; j<players[i].total_argument_count; j++) {
            players[i].arguments[j] = (char *)malloc(256 * sizeof(char)); 
            
            fscanf(file, "%s", players[i].arguments[j]);
        }
    }

    fclose(file);
}

// TODO: for debugging purposes should be deleted!!
void print_for_debug () {
    printf("Grid: %d x %d, Streak Size: %d, Players: %d\n", grid_width, grid_height, streak_size, player_count);
    for (int i = 0; i < player_count; i++) {
        printf("Player %d: %c, Executable: %s, Arguments: ", i + 1, players[i].player_char, players[i].executable_path);
        for (int j = 0; j < players[i].total_argument_count; j++) {
            printf("%s ", players[i].arguments[j]);
        }
        printf("\n");
    }
}

void clean_up() {
    for (int i = 0; i < player_count; i++) {
        for (int j = 0; j < players[i].total_argument_count; j++) {
            free(players[i].arguments[j]);
        }
        free(players[i].arguments);
    }
    free(players);
}

int main() {
    int i, j;

    read_file();

    // initialize game state 
    // printf("%s %s %s %s %s \n", players[0].executable_path, players[0].arguments[0], players[0].arguments[1], players[0].arguments[2], players[0].arguments[3], NULL); 
    // execl(players[0].executable_path, players[0].executable_path, players[0].arguments[0], players[0].arguments[1], players[0].arguments[2], players[0].arguments[3], NULL); 
    // perror("ilk kisim");

    char *args_with_null[players[0].total_argument_count + 2];

    args_with_null[0] = players[0].executable_path;
    for (i = 0; i < players[0].total_argument_count; i++) {
        args_with_null[i + 1] = players[0].arguments[i];
        // printf("%s ", players[0].arguments[i]);
    }
    args_with_null[players[0].total_argument_count + 1] = NULL;
    int status;
    pid_t pid = fork();
    if (pid==0) {
        execv(players[0].executable_path, args_with_null);
        perror("error: ");  
    } else {
        wait(&status);
        // sleep(5);
        printf("%d \n", pid);
    }

    // loop until game ends 

    // announce winner or draw 
    
    // print_for_debug();
    clean_up();
    // sleep(100000);

    return 0;
}
