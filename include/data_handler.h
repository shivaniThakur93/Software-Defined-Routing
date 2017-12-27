#ifndef DATA_HANDLER_H_
#define DATA_HANDLER_H_

int create_data_sock();
int new_data_conn(int sock_index);
bool isData(int sock_index);
int sendFileToDest(int destIP,int ttl,int trans_id,int init_seq_no,char *file_name);
void forwardFileToDest(int destIP,int ttl,int trans_id,int seq_no,int fin_bit,char file_payload[FILE_PAYLOAD_SIZE]);
int createPeerSocket(int destIP);
void saveFileToDisk(char file_payload[FILE_PAYLOAD_SIZE],int trans_id,int fin_bit);
bool data_recv_hook(int sock_index);
#endif