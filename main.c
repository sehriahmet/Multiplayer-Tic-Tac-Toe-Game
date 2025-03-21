// Ahmet Sehriyaroglu 
// 2581015 

#include <unistd.h> // these 2 may not useful just look for further applications 
#include <stdio.h>
#include <stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<poll.h>

#include "game_structs.h"
#include "print_output.h"

#include <sys/socket.h>

// #define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)

// IMPORTANT this pipe protocol should be checked in lab computers!!
#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, 0, fd)

typedef struct {
    char player_char;
    int total_argument_count;
    char executable_path[256];
    char **arguments; 
    int player_pid;
    int player_pipe[2]; 
} PlayerStruct;

int grid_width, grid_height, streak_size, player_count;
PlayerStruct *player;
int filled_count = 0;

void read_input() {
    scanf("%d %d %d %d", &grid_width, &grid_height, &streak_size, &player_count);

    player = (PlayerStruct *)malloc(player_count * sizeof(PlayerStruct));

    for (int i = 0; i < player_count; i++) {
        scanf(" %c %d", &player[i].player_char, &player[i].total_argument_count);

        scanf("%s", player[i].executable_path);

        player[i].arguments = (char **)malloc(player[i].total_argument_count * sizeof(char *));

        for (int j = 0; j<player[i].total_argument_count; j++) {
            player[i].arguments[j] = (char *)malloc(256 * sizeof(char)); 
            
            scanf("%s", player[i].arguments[j]);
        }
    }
}

// TODO: for debugging purposes should be deleted!!
void print_for_debug () {
    printf("Grid: %d x %d, Streak Size: %d, player: %d\n", grid_width, grid_height, streak_size, player_count);
    for (int i = 0; i < player_count; i++) {
        printf("Player %d: %c, Executable: %s, Arguments: ", i + 1, player[i].player_char, player[i].executable_path);
        for (int j = 0; j < player[i].total_argument_count; j++) {
            printf("%s ", player[i].arguments[j]);
        }
        printf("\n");
    }
    
}

void print_grid(char grid[grid_width][grid_height]) {
    printf("  ");
    for (int j = 0; j < grid_width; j++) {
        printf("%d ", j);
    }
    printf("\n");

    for (int i = 0; i < grid_height; i++) {
        printf("%d ", i);
        for (int j = 0; j < grid_width; j++) {
            printf("%c ", grid[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void clean_up() {
    for (int i = 0; i < player_count; i++) {
        for (int j = 0; j <= player[i].total_argument_count; j++) {
            free(player[i].arguments[j]);
        }
        free(player[i].arguments);
    }
    free(player);
}

int check_win(int x, int y, char character, char grid[grid_width][grid_height]) {
    int count = 1;
    for (int i = x + 1; i < grid_width && grid[i][y] == character; i++) count++;
    for (int i = x - 1; i >= 0 && grid[i][y] == character; i--) count++;
    if (count >= streak_size) return 1;

    count = 1;
    for (int j = y + 1; j < grid_height && grid[x][j] == character; j++) count++;
    for (int j = y - 1; j >= 0 && grid[x][j] == character; j--) count++;
    if (count >= streak_size) return 1;

    count = 1;
    for (int i = x + 1, j = y + 1; i < grid_width && j < grid_height && grid[i][j] == character; i++, j++) count++;
    for (int i = x - 1, j = y - 1; i >= 0 && j >= 0 && grid[i][j] == character; i--, j--) count++;
    if (count >= streak_size) return 1;

    count = 1;
    for (int i = x + 1, j = y - 1; i < grid_width && j >= 0 && grid[i][j] == character; i++, j--) count++;
    for (int i = x - 1, j = y + 1; i >= 0 && j < grid_height && grid[i][j] == character; i--, j++) count++;
    if (count >= streak_size) return 1;

    return 0;
}

void take_player_input(int a, char grid[grid_width][grid_height], gu grid_updates[grid_height * grid_width],int *update_count) {
    sm server_msg;
    smp server_msg_print; 
    cm client_msg;
    cmp client_msg_print; 
    // gu grid_updates[grid_height * grid_width];
    client_msg_print.process_id = getpid();
    client_msg_print.client_message = &client_msg;
    server_msg_print.process_id = getpid();
    server_msg_print.server_message = &server_msg;

    if (read(player[a].player_pipe[0], &client_msg, sizeof(cm) )) { 
        print_output(&client_msg_print,NULL,NULL,0);
        if (client_msg.type == START) {
            server_msg.type = RESULT;
            server_msg.success = 0;
            server_msg.filled_count = filled_count;
            write(player[a].player_pipe[0], &server_msg, sizeof(server_msg));
        } else if (client_msg.type == MARK) {
            printf("in client msg is MARK condition bp\n");
            int x = client_msg.position.x;
            int y = client_msg.position.y;
            server_msg.type = RESULT;
            printf("after mark message if it is okey player: %c, grid: \n", player[a].player_char, grid[x][y]);
            
            if (grid[x][y] == '.') {
                printf("in grid is empty part bp update count: %d\n\n", (*update_count)+1);
                server_msg.success = 1;
                grid[x][y] = player[a].player_char;
                grid_updates[*update_count].position.x = x;
                grid_updates[*update_count].position.y = y;
                grid_updates[*update_count].character = player[a].player_char;
                (*update_count)++;
                filled_count++;

                if (check_win(x, y, player[a].player_char, grid)) {
                    sm end_msg;
                    end_msg.type = END;
                    for (int i = 0; i < player_count; i++) {
                        write(player[i].player_pipe[0], &end_msg, sizeof(end_msg));
                    }
                    printf("Winner: Player%c\n", player[a].player_char);
                    exit(0); 
                }

                if (filled_count == grid_width * grid_height) {
                    sm end_msg;
                    end_msg.type = END;
                    for (int i = 0; i < player_count; i++) {
                        write(player[i].player_pipe[0], &end_msg, sizeof(end_msg));
                    }
                    printf("Draw\n");
                    exit(0); 
                }

            } else {
                server_msg.success = 0; 
            }
            
            server_msg.filled_count = filled_count;
            
        }
        
        
        write(player[a].player_pipe[0], &server_msg, sizeof(sm)); 
        
        print_output(NULL, &server_msg_print, grid_updates, *update_count);
        print_grid(grid);
        // print_output(&client_msg_print,NULL,NULL,0);
        // printf("\na: %d \n", getpid());
        // printf("client_msg is written for %d\n", a);
    }
}



int main() {
    int i, j,k;

    // initialize game state 
    read_input(); 
    // print_for_debug();

    
    char grid[grid_width][grid_height];
    
    for (i=0;i<grid_height;i++) {
        for (j=0;j<grid_width;j++) {
            grid[i][j] = '.';
        }
    }

    // printf("grid[0][0]: %c\n", grid[0][0]);

    for (i=0;i<player_count; i++) {
        char *args_with_null[player[i].total_argument_count + 2];

        // printf("\n playercount: %d first bp \n", i);

        args_with_null[0] = player[i].executable_path;
        for (j = 0; j < player[i].total_argument_count; j++) {
            args_with_null[j + 1] = player[i].arguments[j];
            // printf("player arguments:  %s \n", player[i].arguments[j]);
        }
        args_with_null[player[i].total_argument_count + 1] = NULL;

        int status; 

        if(PIPE(player[i].player_pipe) == -1) {
            perror("error in pipe");
            exit(1);
        }
        
        player[i].player_pid = fork();
        if (player[i].player_pid == 0) { 
            close(player[i].player_pipe[0]);
            dup2(player[i].player_pipe[1], 0);
            dup2(player[i].player_pipe[1], 1);
            close(player[i].player_pipe[1]);
            execv(player[i].executable_path, args_with_null);
            perror("error: execv in fork");  
            // printf(player[i].executable_path);
            exit(1); // exit in here can be used for reaping zombie processes
            
        } else {
            close(player[i].player_pipe[1]); 
            dup2(player[i].player_pipe[0], 2); // in here pipe can be taken to the stdout maybe
            dup2(player[i].player_pipe[0], 0); 
            // close(player[i].player_pipe[0]); // cannot take to the stage poll() when it's closed 
        }
    }

    struct pollfd fds[player_count];
    for (k=0;k<player_count; k++) {
        fds[k].fd = player[k].player_pipe[0];
        fds[k].events = POLLIN;
    }

    gu grid_updates[grid_width][grid_height]; 
    int update_count = 0; 

    // loop until game ends 
    while (1) {
        int ready = poll(fds, player_count, 1);
        if (ready < 0) {
            perror("poll");
            exit(1);
        }

        for (int i = 0; i < player_count; i++) {
            if (fds[i].revents & POLLIN) {
                printf("pipe data received for player %d. server pid: %d \n",i, getpid());
                take_player_input(i, grid, grid_updates, &update_count);
            } 
        }
        // printf("in main loop");
    }

    // announce winner or draw 
    
    // print_for_debug();
    clean_up();
    
    return 0;
}
