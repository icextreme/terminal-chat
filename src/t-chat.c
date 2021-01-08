#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "network.h"
#include "ui.h"

// Static Lists
static List *recv_list;
static List *send_list;

// Prototypes
static void create_lists();
static void free_lists();

int main (int argc, char* argv[]) {
    // Network startup
    Network_check_args(argc, argv);
    Network_connect(argv);

    create_lists();

    printf("\nT-chat session started.\n\n");
    
    // Start threads
    Network_start_chat(recv_list, send_list);
    Ui_start_chat(recv_list, send_list);

    // Join threads
    Network_join_threads();
    Ui_join_threads();

    // Cleanup, free, and destroy remnants
    Network_exit_chat();
    Ui_exit_chat();
    
    // Free network information
    Network_freeaddrinfo();

    // Free lists
    free_lists();

    printf("\nT-chat session closed.\n");

    return 0;
}

static void create_lists() {
    recv_list = List_create();
    send_list = List_create();
}

static void free_lists() {
    List_free(recv_list, free);
    List_free(send_list, free);
}
