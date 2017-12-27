#ifndef AUTHOR_H_
 #define AUTHOR_H_
void author_response(int sock_index);
void init_response(int sock_index,char *payload);
void routing_table_response(int sock_index);
void update_response(int sock_index);
void crash_response(int sock_index);
void sendfile_response(int sock_index,char *payload,int payload_len);
void sendfile_stats_response(int sock_index,char *payload);
void last_data_packet_response(int sock_index);
void penultimate_data_packet_response(int sock_index);
#endif