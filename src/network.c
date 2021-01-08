#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "network.h"
#include "list.h"
#include "ui.h"

// Static variables
static int src_port;
static int dest_port;
static int socket_fd;

static struct addrinfo src_hints;
static struct addrinfo *src_res;
static struct addrinfo dest_hints;
static struct addrinfo *dest_res;

static pthread_t send_pthread;
static pthread_t recv_pthread;

static pthread_mutex_t recv_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t recv_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t send_cond = PTHREAD_COND_INITIALIZER;

static char *newstr = NULL;

// Check arguments for errors
void Network_check_args(int argc, char *argv[]) {
    if (argc != ARG_COUNT) {
        printf("Usage: ./t-chat [my port number] [remote machine name] [remote port number]\n");
        exit(EXIT_FAILURE);
    } 

    char *psrc;
    char *pdest;
    src_port = strtol(argv[1], &psrc, 10);
    dest_port = strtol(argv[3], &pdest, 10);

    bool bad_src_port_format = *psrc != '\0' || *psrc == src_port;
    bool bad_dest_port_format = *pdest != '\0' || *pdest == dest_port;
    bool bad_src_port_num = src_port < MIN_PORT || src_port > MAX_PORT;
    bool bad_dest_port_num = dest_port < MIN_PORT || dest_port > MAX_PORT;

    if (bad_src_port_format && bad_dest_port_format) {
        printf("Invalid source and destination port number format.\n");
        exit(EXIT_FAILURE);
    }  

    if (bad_src_port_format) {
        printf("Invalid source port number format.\n");
        exit(EXIT_FAILURE);
    } 
     
    if (bad_dest_port_format) {
        printf("Invalid destination port number format.\n");
        exit(EXIT_FAILURE);
    }

    if (bad_src_port_num && bad_dest_port_num ) {
        printf("Invalid source port and destination port.\n");
        printf("Please enter a port number between %d and %d inclusive.\n", MIN_PORT, MAX_PORT);
        exit(EXIT_FAILURE);
    }

    if (bad_src_port_num) {
        printf("Invalid source port.\n");
        printf("Please enter a port number between %d and %d inclusive.\n", MIN_PORT, MAX_PORT);
        exit(EXIT_FAILURE);
    }

    if (bad_dest_port_num) {
        printf("Invalid destination port.\n");
        printf("Please enter a port number between %d and %d inclusive.\n", MIN_PORT, MAX_PORT);
        exit(EXIT_FAILURE);
    }
}

// Connect source 
static void src_connect(struct addrinfo *src_hints, struct addrinfo **src_res, char *argv[]) {
    memset(src_hints, 0, sizeof(struct addrinfo));
    src_hints->ai_family = AF_INET; 
    src_hints->ai_socktype = SOCK_DGRAM;
    src_hints->ai_flags = AI_PASSIVE; 

    int gai_result;

    if ((gai_result = getaddrinfo(NULL, argv[1], src_hints, src_res)) != 0) {
        printf("<SRC>   Failed to resolve host: %s\n", gai_strerror(gai_result));
        exit(EXIT_FAILURE);
    } else {
        printf("<SRC>   Successfully resolved host\n");
    }
}

// Connect destination
static void dest_connect(struct addrinfo *dest_hints, struct addrinfo **dest_res, char *argv[]) {
    memset(dest_hints, 0, sizeof(struct addrinfo));
    dest_hints->ai_family = AF_INET; // IPV4
    dest_hints->ai_socktype = SOCK_DGRAM; // UDP Datagram

    int gai_result;
    char ip_str[INET6_ADDRSTRLEN];

    if ((gai_result = getaddrinfo(argv[2], argv[3], dest_hints, dest_res)) != 0) {
        printf("<DEST>  Failed to resolve address for %s: %s\n", argv[2], gai_strerror(gai_result));
        exit(EXIT_FAILURE);
    } else {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)(*dest_res)->ai_addr;
        inet_ntop((*dest_res)->ai_family, &(addr_in->sin_addr), ip_str, sizeof ip_str);
        printf("<DEST>  Successfully resolved address for %s: %s\n", argv[2], ip_str);
    }
}

// Bind to socket
static void socket_bind(int *socket_fd, struct addrinfo **dest_res, struct addrinfo **src_res) {
    int bind_result;

    // Shared socket
    if ((*socket_fd = socket((*dest_res)->ai_family, (*dest_res)->ai_socktype, (*dest_res)->ai_protocol)) < 0) {
        printf("<DEBUG> Failed to create socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        printf("<DEBUG> Successfully created socket\n");
    }
    
    // Bind
    if ((bind_result = bind(*socket_fd, (*src_res)->ai_addr, (*src_res)->ai_addrlen)) < 0) {
        printf("<SRC>   Failed to bind socket: %s\n", strerror(errno));
        close(*socket_fd);
        exit(EXIT_FAILURE);
    } else {
        printf("<SRC>   Successfully bound to socket\n");
    }
}

// Helper function to connect network
void Network_connect(char *argv[]) {
    src_connect(&src_hints, &src_res, argv);
    dest_connect(&dest_hints, &dest_res, argv);
    socket_bind(&socket_fd, &dest_res, &src_res);
}

// Helper function to free and close network
void Network_freeaddrinfo(){
    freeaddrinfo(src_res);
    freeaddrinfo(dest_res);
    close(socket_fd);
}

// Thread for sending data
static void *send_run(void *send_list) {
    char send_buffer[BUFFER_LENGTH] = "";
    ssize_t bytes = 0;

    while (true) {
        pthread_mutex_lock(&send_mutex);
        {
            while(List_count(send_list) == 0) {
                pthread_cond_wait(&send_cond, &send_mutex);
            }

            strcpy(send_buffer, List_last((List *)send_list));
            free(List_last((List *)send_list));
            List_trim((List *)send_list);
        }
        pthread_mutex_unlock(&send_mutex);

        bytes = sendto(socket_fd, send_buffer, BUFFER_LENGTH, 0, dest_res->ai_addr, sizeof(struct sockaddr_in));

        if (bytes < 0) {
            printf("send_to: %s\n", strerror(errno));
        }
        
        if (strcmp(send_buffer, "!\n") == 0) {
            pthread_cond_signal(&send_cond);
            break;
        }

        pthread_mutex_lock(&send_mutex);
        {
            pthread_cond_signal(&send_cond);
        }
        pthread_mutex_unlock(&send_mutex);
    }

    int recv_cancel_result = 0;
    int send_cancel_result = 0;

    recv_cancel_result = pthread_cancel(recv_pthread);
    send_cancel_result = pthread_cancel(send_pthread);

    if (recv_cancel_result < 0 || send_cancel_result < 0) {
        printf("Error cancelling keyboard or screen thread.\n");
    }

    return NULL;
}

// Thread for receiving data
static void *recv_run(void *recv_list) {
    char recv_buffer[BUFFER_LENGTH] = "";
    ssize_t bytes = 0;

    while (true) {
        newstr = malloc(BUFFER_LENGTH + 1);
        memset(newstr, '\0', BUFFER_LENGTH + 1);
    
        bytes = recvfrom(socket_fd, recv_buffer, sizeof(recv_buffer), 0,
                 NULL, NULL); 

        if (bytes < 0) {
            printf("recvfrom: %s\n", strerror(errno));
        }

        recv_buffer[bytes] = '\0';
        
        strncpy(newstr, recv_buffer, bytes);

        pthread_mutex_lock(&recv_mutex);
        {
            if(List_count(recv_list) == (LIST_MAX_NUM_NODES / 2)) {
                printf("List is full! -> %s\n", newstr);
            } else {
                List_prepend((List *)recv_list, newstr);
            }
        }
        pthread_mutex_unlock(&recv_mutex);

        newstr = NULL;

        if (strcmp(recv_buffer, "!\n") == 0) {
            pthread_cond_signal(&recv_cond);
            break;
        }

        pthread_mutex_lock(&recv_mutex);
        {
            if(List_count(recv_list) != (LIST_MAX_NUM_NODES / 2)) {
                pthread_cond_signal(&recv_cond);
                pthread_cond_wait(&recv_cond, &recv_mutex);
            } 
        }
        pthread_mutex_unlock(&recv_mutex);
    }

    int recv_cancel_result = 0;
    int send_cancel_result = 0;

    send_cancel_result = pthread_cancel(send_pthread);
    recv_cancel_result = pthread_cancel(recv_pthread);

    if (recv_cancel_result < 0 || send_cancel_result < 0) {
        printf("Error cancelling keyboard or screen thread.\n");
    }

    return NULL;
}

// Helper function to start network threads
void Network_start_chat(List *recv_list, List *send_list) {
    int recv_result = 0;
    int send_result = 0;

    if ((recv_result = pthread_create(&recv_pthread, NULL, recv_run, recv_list)) != 0
    || (send_result = pthread_create(&send_pthread, NULL, send_run, send_list)) != 0) {
        printf("Error creating recv or send thread. Exiting\n");
        exit(EXIT_FAILURE);
    }
}

// Helper function that cancels pthreads (called by UI)
void Network_cancel_pthreads() {
    pthread_mutex_lock(&recv_mutex);
    {
        pthread_cond_signal(&recv_cond);
    }
    pthread_mutex_unlock(&recv_mutex);

    pthread_mutex_lock(&send_mutex);
    {
        pthread_cond_signal(&send_cond);
    }
    pthread_mutex_unlock(&send_mutex);
  
    int recv_cancel_result = 0;
    int send_cancel_result = 0;

    recv_cancel_result = pthread_cancel(recv_pthread);
    send_cancel_result = pthread_cancel(send_pthread);

    if (recv_cancel_result < 0 || send_cancel_result < 0) {
        printf("Error cancelling keyboard or screen thread.\n");
    }
}

// Helper function to destory mutex and condition variables
void Network_exit_chat() {
    int recv_mutex_result = 0;
    int send_mutex_result = 0;
    int recv_cond_result = 0;
    int send_cond_result = 0;

    pthread_mutex_trylock(&recv_mutex);
    pthread_mutex_unlock(&recv_mutex);

    pthread_mutex_trylock(&send_mutex);
    pthread_mutex_unlock(&send_mutex);

    if ((recv_mutex_result = pthread_mutex_destroy(&recv_mutex)) != 0
    || (send_mutex_result = pthread_mutex_destroy(&send_mutex)) != 0) {
        printf("Error destroying recv or send mutex.\n");
    }

    if ((recv_cond_result = pthread_cond_destroy(&recv_cond)) != 0
    || (send_cond_result = pthread_cond_destroy(&send_cond)) != 0) {
        printf("Error destroying recv or send condition variable.\n");
    }

    if (newstr) {
        free(newstr);
        newstr = NULL;
    }
}

// Helper function to join threads
void Network_join_threads() {
    int recv_result = 0;
    int send_result = 0;

    if ((recv_result = pthread_join(recv_pthread, NULL)) != 0
    || (send_result = pthread_join(send_pthread, NULL)) != 0) {
        printf("Error joining recv or send thread. Exiting\n");
        exit(EXIT_FAILURE);
    }
}

// Getter for recv mutex
pthread_mutex_t *Network_get_recv_mutex() {
    return &recv_mutex;
}

// Getter for send mutex
pthread_mutex_t *Network_get_send_mutex() {
    return &send_mutex;
}

// Getter for recv condition variable
pthread_cond_t *Network_get_recv_cond() {
    return &recv_cond;
}

// Getter for send condition variable
pthread_cond_t *Network_get_send_cond() {
    return &send_cond;
}
