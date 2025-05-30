#include <stdio.h>
#include "print_output.h"

void print_output(cmp *client_msg, smp *server_msg, gu *grid_updates, int update_count) {
    // Print client message if provided
    if (client_msg != NULL) {
        printf("CENG334_GRADER_CLIENT %d ", client_msg->process_id);
        if (client_msg->client_message->type == START) {
            printf("START\n");
        } else if (client_msg->client_message->type == MARK) {
            printf("MARK %d %d\n", 
                client_msg->client_message->position.x,
                client_msg->client_message->position.y);
        }
    }

    // Print server message if provided
    if (server_msg != NULL) {
        printf("CENG334_GRADER_SERVER %d ", server_msg->process_id);
        if (server_msg->server_message->type == END) {
            printf("END\n");
        } else if (server_msg->server_message->type == RESULT) {
            printf("RESULT %d %d\n", 
                server_msg->server_message->success,
                server_msg->server_message->filled_count);
        }
    }

    // Print grid updates if provided
    if (grid_updates != NULL && update_count > 0) {
        for (int i = 0; i < update_count; i++) {
            printf("CENG334_GRADER_UPDATE %d %d %c\n",
                grid_updates[i].position.x,
                grid_updates[i].position.y,
                grid_updates[i].character);
        }
    }

    fflush(stdout);
} 