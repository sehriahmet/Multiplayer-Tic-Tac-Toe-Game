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

// this should be deleted no need!
void print_grid(char grid[grid_width][grid_height]) {
    printf("  ");
    for (int j = 0; j < grid_width; j++) {
        printf("%d ", j);
    }
    printf("\n");

    for (int i = 0; i < grid_width; i++) {
        printf("%d ", i);
        for (int j = 0; j < grid_height; j++) {
            printf("%c ", grid[j][i]);
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
    // check vertical first
    int i=0; 
    int j=0;
    int count = 0;
    while (grid[x-i][y] == character && x-i>-1) {
        count++;
        // printf("check_win for vertical -> x: %d y: %d\n char: %c\ncount:%d\n", y, x-i, grid[x-i][y], count);
        i++;
    }
    j=1;
    while (grid[x+j][y] == character && x+j< grid_width) {
        count++;
        // printf("check_win for vertical -> x: %d y: %d\n char: %c\ncount:%d\n", y, x+j, grid[x+j][y], count);
        j++;
    }
    if (count>=streak_size) {
        // this should be deleted! 
        // printf("Found in y vertical -> x: %d  y: %d \n char : %c count : %d\n", y, x, grid[x][y], count); 
        return 1;
    }

    // check horizontal 
    i=0;
    count=0;
    while (grid[x][y-i] == character && y-i>-1) {
        count++;
        // printf("check_win for horizontal-> x: %d y: %d\nchar: %c \ncount:%d\n", y-i, x, grid[x][y-i], count);
        i++;
    }
    j=1;
    while (grid[x][y+j] == character && y+j<grid_height) {
        count++;
        // printf("check_win for horizontal-> x: %d y: %d\n char: %c\ncount:%d\n", y+j, x, grid[x][y+j], count);
        j++;
    }
    if (count>=streak_size) {
        // this should be deleted! 
        // printf("found in x horizontal -> x: %d  y: %d \n char : %c count : %d\n", y, x, grid[x][y], count); 
        return 1;
    }

    // check diagonal for left 
    i=0;
    j=0;
    count=0;
    while (grid[x+i][y+j] == character && x+i<grid_width && y+j < grid_height) {
        count++;
        // printf("check_win for diagonal left -> x: %d y: %d\n char: %c\ncount:%d\n", y+j, x+i, grid[x+i][y+j], count);
        i++;
        j++;
    }
    i=1;
    j=1;
    while (grid[x-i][y-j] == character && x-i>-1 && y-j > -1) {
        count++;
        // printf("check_win for diagonal left -> x: %d y: %d\n char: %c\ncount:%d\n", y-j, x-i, grid[x-i][y-j], count);
        i++;
        j++;
    }
    if (count>=streak_size) {
        // this should be deleted! 
        // printf("Found in diagonal left -> x: %d  y: %d \n char : %c count : %d\n", y, x, grid[x][y], count); 
        return 1;
    }

    // check diagonal for right 
    i=0;
    j=0;
    count=0;
    while (grid[x+i][y-j] == character && x+i<grid_width && y-j > -1) {
        count++;
        // printf("check_win in diagonal right -> x: %d y: %d\n char: %c\ncount:%d\n", y-j, x+i, grid[x+i][y-j], count);
        i++;
        j++;
    }
    i=1;
    j=1;
    while (grid[x-i][y+j] == character && x-i>-1 && y+j<grid_height) {
        count++;
        // printf("check_win in diagonal right -> x: %d y: %d\n char: %c\ncount:%d\n", y+j, x-i, grid[x-i][y+j], count);
        i++;
        j++;
    }
    if (count>=streak_size) {
        // this should be deleted! 
        // printf("Found in diagonal right -> x: %d  y: %d \n char : %c count : %d\n", y, x, grid[x][y], count); 
        return 1;
    }

    return 0;


}

void take_player_input(int a, char grid[grid_width][grid_height], gu grid_updates[grid_height * grid_width],int *update_count) {
    sm server_msg;
    smp server_msg_print; 
    cm client_msg;
    cmp client_msg_print; 
    client_msg_print.process_id = player[a].player_pid;
    client_msg_print.client_message = &client_msg;
    server_msg_print.process_id = player[a].player_pid;
    server_msg_print.server_message = &server_msg;
    gd grid_datas[grid_height * grid_width];

    if (read(player[a].player_pipe[0], &client_msg, sizeof(cm))) { 
        print_output(&client_msg_print,NULL,NULL,0);
        if (client_msg.type == START) {
            server_msg.type = RESULT;
            server_msg.success = 0;
            server_msg.filled_count = filled_count;
        } else if (client_msg.type == MARK) {
            int x = client_msg.position.x;
            int y = client_msg.position.y;
            server_msg.type = RESULT;
            if (grid[x][y] == '.') {
                server_msg.success = 1;
                grid[x][y] = player[a].player_char;
                grid_updates[*update_count].position.x = x;
                grid_updates[*update_count].position.y = y;
                grid_updates[*update_count].character = player[a].player_char;

                grid_datas[*update_count].position.x = x;
                grid_datas[*update_count].position.y = y;
                grid_datas[*update_count].character = player[a].player_char;
                
                (*update_count)++;
                filled_count++;

                if (check_win(x, y, player[a].player_char, grid)) {
                    sm end_msg;
                    end_msg.type = END;
                    for (int i = 0; i < player_count; i++) {
                        print_output(NULL,&(smp){player[i].player_pid, &end_msg} , NULL, 0);
                        write(player[i].player_pipe[0], &end_msg, sizeof(end_msg));
                    }
                    // this print_grid should be deleted!.
                    print_grid(grid);

                    printf("Winner: Player%c\n", player[a].player_char);
                    exit(0);
                }

                if (filled_count == grid_width * grid_height) {
                    sm end_msg;
                    end_msg.type = END;
                    for (int i = 0; i < player_count; i++) {
                        print_output(NULL,&(smp){player[i].player_pid, &end_msg} , NULL, 0);
                        write(player[i].player_pipe[0], &end_msg, sizeof(end_msg));
                    }
                    // this print_grid should be deleted!.
                    print_grid(grid);

                    printf("Draw\n");
                    exit(0);
                }
            } else {
                server_msg.type = RESULT;
                server_msg.success = 0; 
            }
            server_msg.filled_count = filled_count;
        }

        write(player[a].player_pipe[0], &server_msg, sizeof(sm)); 
        write(player[a].player_pipe[0], &grid_datas, filled_count * sizeof(gd)); 
    
        print_output(NULL, &server_msg_print, grid_updates, *update_count);

        // this print_grid should be deleted!.
        print_grid(grid);

    }
}



int main() {
    int i, j,k;

    // initialize game state 
    read_input(); 
    
    char grid[grid_width][grid_height];
    
    for (i=0;i<grid_height;i++) {
        for (j=0;j<grid_width;j++) {
            grid[i][j] = '.';
        }
    }

    // set up communication channels 
    for (i=0;i<player_count; i++) {
        char *args_with_null[player[i].total_argument_count + 2];

        args_with_null[0] = player[i].executable_path;
        for (j = 0; j < player[i].total_argument_count; j++) {
            args_with_null[j + 1] = player[i].arguments[j];
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
    // gd grid_datas[grid_height * grid_width];
    int update_count = 0; 

    // loop until game ends 
    while (1) {
        int ready = poll(fds, player_count, 1000);
        if (ready < 0) {
            perror("poll");
            exit(1);
        }
        if (ready == 0) {
            printf("Poll timed out, no data received.\n");
        }
        for (int i = 0; i < player_count; i++) {
            if (fds[i].revents & POLLIN) {
                // printf("pipe data received for player %d. server pid: %d \n", i, getpid());
                take_player_input(i, grid, *grid_updates, &update_count);
            } 
        }
    }

    clean_up();
    
    return 0;
}
