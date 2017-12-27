#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/time.h>

typedef enum {FALSE, TRUE} bool;

#define ERROR(err_msg) {perror(err_msg); exit(EXIT_FAILURE);}

/* https://scaryreasoner.wordpress.com/2009/02/28/checking-sizeof-at-compile-time/ */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)])) // Interesting stuff to read if you are interested to know how this works
#define ROUTERS_MAX 5
#define INF 65535
#define DATASIZE 16
#define MICRO 1000000
#define FILE_PACKET_SIZE 1036
#define FILE_PAYLOAD_SIZE 1024
#define LAST_FIN_BIT 32768

uint16_t CONTROL_PORT;
uint16_t NO_ROUTERS;
uint16_t UPDATE_INTERVAL;
uint16_t ROUTER_ID;
char ROUTER_PORT[DATASIZE];
uint16_t ROUTER_IPORT;
uint16_t DATA_PORT;
uint32_t ROUTER_IP;
int NO_NEIGHBOURS;
int COST_MAT_INDEX;
int INITFLAG;
struct timeval currtime;
int TIMERARRAYLENGTH;
int FILESTATINDEX;
int SEQNOINDEX;
//data packet struct
struct filePacketCont{
	int destIP;
	int trans_id;
	int ttl;
	int seq_no;
	int fin_bit;
	char file_payload[1024];
};
struct filePacketCont lastDataPkt;
struct filePacketCont penDataPkt;
//routing table : router port for non-neighbour?
struct dv_table{
	uint32_t ip_addr;
	uint16_t router_rport;
	uint16_t next_hop;
	uint16_t cost;
	uint16_t router_id;
}dv_matrix[ROUTERS_MAX][ROUTERS_MAX];
//array for cost matrix mapping
struct costMatInitArray{
	uint16_t router_id;
    uint16_t router_rport;
    uint16_t router_dport;
	uint16_t cost;
	uint32_t ip_addr;
}costmatarr[ROUTERS_MAX];
struct timerArray{
	struct timeval begin,end;
	int router_ip;//compare to ROUTER_IP : my own: broadcast
	int noupdatecount;
}timerarr[ROUTERS_MAX];
//timeout struct
struct timeval tv;
struct neighboursInfo{
	int n_index;
	uint16_t router_rport;
	uint32_t ip_addr;
	int cost_est;
	int r_id;
}NEIGHBOURS[ROUTERS_MAX];
struct fileStats{//info for file statistics
	int trans_id;
	int ttl;
	int seqNoArray[10240];
	int seqNoArraySize;
}FILESTATS[ROUTERS_MAX];
#endif