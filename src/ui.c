#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "list.h"
#include "network.h"
#include "ui.h"

// Static variables
static pthread_t keyboard_pthread;
static pthread_t screen_pthread;
static char *newstr = NULL;

// Keyboard thread
static void *keyboard_run(void *send_list) {
    char keyboard_buffer[BUFFER_LENGTH] = "";

    while (true) {
        newstr = malloc(BUFFER_LENGTH + 1);
        memset(newstr, '\0', BUFFER_LENGTH + 1);

        // Reach end of file
        if (fgets(keyboard_buffer, BUFFER_LENGTH, stdin) == NULL) {
            if (ferror(stdin)) {
                printf("Error reading from stdin\n. Exiting.");
                exit(EXIT_FAILURE);
            }
           
            Network_cancel_pthreads();
            break;
        }

        strcpy(newstr, keyboard_buffer);
        
        pthread_mutex_lock(Network_get_send_mutex()); 
        {
            if(List_count(send_list) == (LIST_MAX_NUM_NODES / 2)) {
                printf("List is full! -> %s\n", newstr);
            } else {
                if (List_prepend((List *)send_list, newstr) < 0) {
                    printf("List_prepend: error\n");
                }
            }
        }
        pthread_mutex_unlock(Network_get_send_mutex());

        newstr = NULL;

        if (strcmp(keyboard_buffer, "!\n") == 0) {
            pthread_cond_signal(Network_get_send_cond());
            break;
        }

        pthread_mutex_lock(Network_get_send_mutex());
        {
            if(List_count(send_list) != (LIST_MAX_NUM_NODES) / 2) {
                pthread_cond_signal(Network_get_send_cond());
                pthread_cond_wait(Network_get_send_cond(), Network_get_send_mutex());
            }
        }
        pthread_mutex_unlock(Network_get_send_mutex());

    }

    int keyboard_cancel_result = 0;
    int screen_cancel_result = 0;

    screen_cancel_result = pthread_cancel(screen_pthread);
    keyboard_cancel_result = pthread_cancel(keyboard_pthread);

    if (keyboard_cancel_result < 0 || screen_cancel_result < 0) {
        printf("Error cancelling keyboard or screen thread.\n");
    }
    
    return NULL;
}

// Screen thread
static void *screen_run(void *recv_list) {
    char screen_buffer[BUFFER_LENGTH] = "";

    while (true) {
        pthread_mutex_lock(Network_get_recv_mutex()); 
        {
        
            while (List_count(recv_list) == 0) {
                pthread_cond_wait(Network_get_recv_cond(), Network_get_recv_mutex());
            }

            strcpy(screen_buffer, List_last((List *)recv_list));
            free(List_last((List *)recv_list));
            List_trim((List *)recv_list);
        }
        pthread_mutex_unlock(Network_get_recv_mutex());

        if (fputs(screen_buffer, stdout) == EOF) {
            printf("Error writing to stdout\n. Exiting.");
            exit(EXIT_FAILURE);
            break;
        }

        fflush(stdout);

        if (strcmp(screen_buffer, "!\n") == 0) {
            pthread_cond_signal(Network_get_recv_cond());
            break;
        }

        pthread_mutex_lock(Network_get_recv_mutex()); 
        {
            pthread_cond_signal(Network_get_recv_cond());
            pthread_cond_wait(Network_get_recv_cond(), Network_get_recv_mutex());
        }
        pthread_mutex_unlock(Network_get_recv_mutex());
    }

    int keyboard_cancel_result = 0;
    int screen_cancel_result = 0;

    keyboard_cancel_result = pthread_cancel(keyboard_pthread);
    screen_cancel_result = pthread_cancel(screen_pthread);

    if (keyboard_cancel_result < 0 || screen_cancel_result < 0) {
        printf("Error cancelling keyboard or screen thread.\n");
    }

    return NULL;
}

// Helper function to start network threads
void Ui_start_chat(List *recv_list, List *send_list) {
    int keyboard_result = 0;
    int screen_result = 0;

    if ((keyboard_result =  pthread_create(&keyboard_pthread, NULL, keyboard_run, send_list)) != 0
    || (screen_result = pthread_create(&screen_pthread, NULL, screen_run, recv_list)) != 0) {
        printf("Error creating keyboard or screen thread. Exiting\n");
        exit(EXIT_FAILURE);
    }
}

// Helper function that cancels ui threads 
void Ui_cancel_pthreads() {
    int keyboard_cancel_result = 0;
    int screen_cancel_result = 0;

    keyboard_cancel_result = pthread_cancel(keyboard_pthread);
    screen_cancel_result = pthread_cancel(screen_pthread);

    if (keyboard_cancel_result < 0 || screen_cancel_result < 0) {
        printf("Error cancelling keyboard or screen thread.\n");
    }
}

// Helper function for freeing
void Ui_exit_chat() {
    if (newstr) {
        free(newstr);
        newstr = NULL;
    }
}

// Helper function for joining threads
void Ui_join_threads() {
    int keyboard_result = 0;
    int screen_result = 0;

    if ((keyboard_result = pthread_join(keyboard_pthread, NULL)) != 0
    || (screen_result = pthread_join(screen_pthread, NULL)) != 0) {
        printf("Error joining keyboard or screen thread. Exiting\n");
        exit(EXIT_FAILURE);
    }
}
