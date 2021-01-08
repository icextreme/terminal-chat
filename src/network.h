#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include "list.h"

// Macros
#define ARG_COUNT 4
#define MIN_PORT 1024
#define MAX_PORT 65535
#define BUFFER_LENGTH 512

// Prototypes
void Network_connect(char *argv[]);
void Network_check_args(int argc, char *argv[]);
void Network_freeaddrinfo();
void Network_start_chat(List *recv_list, List *send_list);
void Network_join_threads();
void Network_exit_chat();
void Network_cancel_pthreads();

// Mutex and condition variable getters
pthread_mutex_t *Network_get_recv_mutex();
pthread_mutex_t *Network_get_send_mutex();
pthread_cond_t *Network_get_recv_cond();
pthread_cond_t *Network_get_send_cond();

#endif