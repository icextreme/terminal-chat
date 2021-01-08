#ifndef _UI_H_
#define _UI_H_

#include "list.h"

// Prototypes
void Ui_start_chat(List *recv_list, List *send_list);
void Ui_join_threads();
void Ui_cancel_pthreads();
void Ui_exit_chat();

#endif