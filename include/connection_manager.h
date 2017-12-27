#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_

fd_set master_list, watch_list;
int control_socket, router_socket,data_socket,head_fd;
void sortAndSetTimer();
void noUpdateMethod();
void updateTimeArrBegin();
void init();
#endif