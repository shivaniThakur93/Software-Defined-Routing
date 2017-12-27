#ifndef ROUTER_HANDLER_H_
#define ROUTER_HANDLER_H_

int create_router_sock();
bool router_recv_hook(int sock_index);
void updateDistanceVectorMatrix();
void updateRoutingTable();
void broadCastRoutingTable(int router_socket);
#endif