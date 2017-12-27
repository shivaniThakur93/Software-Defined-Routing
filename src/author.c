/**
 * @author
 * @author  Shivani Thakur <sthakur3@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * AUTHOR [Control Code: 0x00]
 */

#include <string.h>

#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/connection_manager.h"
#include "../include/router_handler.h"
#include "../include/data_handler.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define AUTHOR_STATEMENT "I, sthakur3, have read and understood the course academic integrity policy."
#define EMPTY_PAYLOAD_LEN 0
#define INET4_ADDRSTRLEN 128
#define CNTRL_RESP_CONTROL_CODE_OFFSET 0x04
#define CNTRL_RESP_RESPONSE_CODE_OFFSET 0x05
#define CNTRL_RESP_PAYLOAD_LEN_OFFSET 0x06
#define ROUTING_TABLE_ROUTER_OFFSET 8
#define SENDFILE_OFFSET 8
#define FILE_DEF_SIZE 4
#define SEQ_NO_SIZE 2
#define TWELVE_SIZE 12

//INIT PAYLOAD
struct __attribute__((__packed__)) INIT_PAYLOAD_ROUTER_INFO
    {
    	uint16_t router_id;
        uint16_t router_port;
        uint16_t data_port;
        uint16_t cost;
        uint32_t router_ip_addr;
    };
struct __attribute__((__packed__)) INIT_PAYLOAD
    {
        uint16_t no_routers;
        uint16_t update_interval;
        struct INIT_PAYLOAD_ROUTER_INFO routerInfo[ROUTERS_MAX];
    };
//ROUTING TABLE
struct __attribute__((__packed__)) ROUTER_PAYLOAD_ROUTER_INFO
    {
    	uint16_t router_id;
        uint16_t padding;
        uint16_t next_hop;
        uint16_t cost;
    };
struct __attribute__((__packed__)) ROUTER_PAYLOAD
    {
        struct ROUTER_PAYLOAD_ROUTER_INFO routerInfo[ROUTERS_MAX];
    };
struct __attribute__((__packed__)) UPDATE_PAYLOAD
    {
		uint16_t router_id;
        uint16_t cost;
	};
struct __attribute__((__packed__)) SENDFILE_PAYLOAD
    {
    	uint32_t dest_ip_addr;
		uint8_t ttl;
        uint8_t trans_id;
        uint16_t seq_no;        
	};
struct __attribute__((__packed__)) SENDFILE_STATS_PAYLOAD
    {
    	uint8_t trans_id;
		uint8_t ttl;
        uint16_t padding;        
	};
struct __attribute__((__packed__)) FILE_TRANS_ID
    {
    	uint8_t trans_id;        
	};
struct __attribute__((__packed__)) FILE_STAT_PAYLOAD
    {
    	uint8_t trans_id;   
    	uint8_t ttl;
    	uint16_t padding; 
	};
struct __attribute__((__packed__)) FILE_SEQ_NO
    {
    	uint16_t seq_no;        
	};
struct __attribute__((__packed__)) SENDFILE_DATA_PACKET
    {
    	uint32_t dest_ip_addr;
		uint8_t trans_id;
        uint8_t ttl;
        uint16_t seq_no; 
        uint16_t fin_bit;
        uint16_t padding;      
	};
struct INIT_PAYLOAD *init_payload;
struct INIT_PAYLOAD_ROUTER_INFO *init_payload_router_info;
//struct INIT_PAYLOAD_ROUTER_INFO *init_payload_router_info;
void initializeDistanceVector(){
	int routerPort;
	int dataPort;
	int cost;
	int ip_value;
	char ip4[128];
	int k=0;
	for(int i=0;i<NO_ROUTERS;i++){
	   init_payload_router_info=&(init_payload->routerInfo[i]);
	   //costmatarr is mapping array
	   costmatarr[i].router_id=ntohs(init_payload_router_info->router_id);
	   costmatarr[i].router_rport=ntohs(init_payload_router_info->router_port);
	   costmatarr[i].router_dport=ntohs(init_payload_router_info->data_port);
	   costmatarr[i].cost=ntohs(init_payload_router_info->cost);
	   //memcpy(&(cntrl_resp_header->controller_ip_addr), &(addr.sin_addr), sizeof(struct in_addr));
	   costmatarr[i].ip_addr=init_payload_router_info->router_ip_addr;
	   if(costmatarr[i].cost==0){
	   		COST_MAT_INDEX=i;
	   		ROUTER_ID=costmatarr[i].router_id;
			sprintf(ROUTER_PORT,"%d",costmatarr[i].router_rport);
			DATA_PORT=costmatarr[i].router_dport;
			ROUTER_IP=costmatarr[i].ip_addr;
			ROUTER_IPORT=costmatarr[i].router_rport;
	   }
	   else if(costmatarr[i].cost!=INF){
	   		NEIGHBOURS[k].n_index=i;
	   		NEIGHBOURS[k].router_rport=costmatarr[i].router_rport;
	   		NEIGHBOURS[k].ip_addr=costmatarr[i].ip_addr;
	   		NEIGHBOURS[k].cost_est=costmatarr[i].cost;
	   		NEIGHBOURS[k].r_id=costmatarr[i].router_id;
	   		k++;
	   }
	}
	//printing cost matrix array
	for(int i=0;i<NO_ROUTERS;i++){
		printf("RID ::%d RPORT ::%d COST ::%d IP ::%d \n",costmatarr[i].router_id,costmatarr[i].router_rport, costmatarr[i].cost);
	}
	//printing neighbours array
	NO_NEIGHBOURS=k;
	printf("COST MATRIX INDEX:: %d \n",COST_MAT_INDEX);
	printf("NO OF NEIGHBOURS :: %d \n",NO_NEIGHBOURS);
	for(int i=0;i<NO_ROUTERS;i++){
		for(int j=0;j<NO_ROUTERS;j++){
			if(i==COST_MAT_INDEX){
				dv_matrix[i][j].ip_addr=costmatarr[j].ip_addr;
				dv_matrix[i][j].router_rport=costmatarr[j].router_rport;
				dv_matrix[i][j].cost=costmatarr[j].cost;
				dv_matrix[i][j].router_id=costmatarr[j].router_id;
				if(dv_matrix[i][j].cost==INF){
					dv_matrix[i][j].next_hop=INF;
				}
				else if(dv_matrix[i][j].cost<INF){
					dv_matrix[i][j].next_hop=costmatarr[j].router_id;
				}
			}
			else{
				dv_matrix[i][j].router_id=costmatarr[j].router_id;
				if(i==j){//set diagonal entries 0
				dv_matrix[i][j].cost=0;
				dv_matrix[i][j].next_hop=0;
				}
				else{
				dv_matrix[i][j].cost=INF;
				dv_matrix[i][j].next_hop=INF;
				}
				
			}	
		}
	}
	//dv matrix after init
	
	//after update
    printf("DV AFTER INIT \n");
   	 for(int i=0;i<NO_ROUTERS;i++){
   	 	for(int j=0;j<NO_ROUTERS;j++){
   	 		printf("%5d ",dv_matrix[i][j].cost);
   	 	}
   	 	printf("\n");
   	 }
   	 
	//create socket for router port
	router_socket=create_router_sock();
	printf("##### CREATED ROUTER SOCKET:: %d #######\n",router_socket);
	FD_SET(router_socket, &master_list);
	if(router_socket>head_fd) head_fd=router_socket;
	//create data socket
	data_socket=create_data_sock();
	FD_SET(data_socket, &master_list);
	if(data_socket>head_fd) head_fd=data_socket;
	//printing cost matrix
	printf("ROUTER ID::%d \n",ROUTER_ID);
	printf("ROUTER IP::%d \n",ROUTER_IP);
	printf("NEIGHBOURS INFO \n");
	for(int i=0;i<NO_NEIGHBOURS;i++){
		printf("@@@@@@@@@@@@@@@@@@@ \n");
		printf("NEIGHBOURS INDEX::%d \n",NEIGHBOURS[i].n_index);
    	printf("NEIGHBOURS IP::%d \n",NEIGHBOURS[i].ip_addr);
		printf("NEIGHBOURS RPORT::%d \n",NEIGHBOURS[i].router_rport);
		printf("NEIGHBOURS COST::%d \n",NEIGHBOURS[i].cost_est);
	}
	printf("NEIGHBOURS \n");
	//set timeout interval for periodic distance vector update
	printf("UPDATE INTERVAL:: %d \n",UPDATE_INTERVAL);
	TIMERARRAYLENGTH=1;
	memset(timerarr, 0, sizeof(timerarr));//clear timer array for new init
	timerarr[0].begin.tv_sec=0;
	timerarr[0].begin.tv_usec=0;
	timerarr[0].end.tv_sec=UPDATE_INTERVAL;
	timerarr[0].end.tv_usec=0;
	timerarr[0].router_ip=ROUTER_IP;
	timerarr[0].noupdatecount=0;
	tv.tv_sec=UPDATE_INTERVAL;
	tv.tv_usec=0;
	INITFLAG=1;
}
void author_response(int sock_index)
{
	uint16_t payload_len, response_len;
	char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;

	payload_len = sizeof(AUTHOR_STATEMENT)-1; // Discount the NULL chararcter
	cntrl_response_payload = (char *) malloc(payload_len);
	memcpy(cntrl_response_payload, AUTHOR_STATEMENT, payload_len);

	cntrl_response_header = create_response_header(sock_index, 0, 0, payload_len);
	printf("CONTROL RESPONSE HEADER::%s \n",cntrl_response_header);
	response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	cntrl_response = (char *) malloc(response_len);
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);
	/* Copy Payload */
	memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	free(cntrl_response_payload);

	sendALL(sock_index, cntrl_response, response_len);
	printf("SENT AUTHOR RESPONSE::%s \n",cntrl_response);
	free(cntrl_response);
}
void init_response(int sock_index,char *payload){
	printf("INIT ENTRY:: \n");
	uint16_t payload_len, response_len;
	char *cntrl_response_header;
	cntrl_response_header = create_response_header(sock_index, 1, 0, EMPTY_PAYLOAD_LEN);
	response_len=CNTRL_RESP_HEADER_SIZE;
	//send header only
	sendALL(sock_index, cntrl_response_header, response_len);
	free(cntrl_response_header);
	init_payload = (struct INIT_PAYLOAD *) payload;
	NO_ROUTERS=ntohs(init_payload->no_routers);
	UPDATE_INTERVAL=ntohs(init_payload->update_interval);
	printf("INIT No of routers :: %d \n",NO_ROUTERS);
	initializeDistanceVector();
	//setNeighbours();
}
void routing_table_response(int sock_index){
	uint16_t payload_len, response_len;
	char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;
	int offset=0;
	//no of routers * 8 bytes
	payload_len = NO_ROUTERS*ROUTING_TABLE_ROUTER_OFFSET;
	cntrl_response_payload = (char *) malloc(payload_len);
	for(int i=0;i<NO_ROUTERS;i++){
	    struct ROUTER_PAYLOAD_ROUTER_INFO *router_payload=(struct ROUTER_PAYLOAD_ROUTER_INFO *) (cntrl_response_payload+offset);
		router_payload->router_id=htons(dv_matrix[COST_MAT_INDEX][i].router_id);
		router_payload->padding=htons(0);
		router_payload->next_hop=htons(dv_matrix[COST_MAT_INDEX][i].next_hop);
		router_payload->cost=htons(dv_matrix[COST_MAT_INDEX][i].cost);
		offset+=ROUTING_TABLE_ROUTER_OFFSET;
	}
	cntrl_response_header = create_response_header(sock_index, 2, 0, payload_len);
	response_len=CNTRL_RESP_HEADER_SIZE+payload_len;
	cntrl_response = (char *) malloc(response_len);
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);
	/* Copy Payload */
	memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	free(cntrl_response_payload);
	offset=8;
	sendALL(sock_index, cntrl_response , response_len);
	// printf("SENT ROUTING TABLE RESPONSE DECODING \n");
// 	for(int i=0;i<NO_ROUTERS;i++){
// 	struct ROUTER_PAYLOAD_ROUTER_INFO *router_payload=(struct ROUTER_PAYLOAD_ROUTER_INFO *) (cntrl_response+offset);
// 	printf("RID::%d \n",ntohs(router_payload->router_id));
// 	printf("RPADDING::%d \n",ntohs(router_payload->padding));
// 	printf("RNEXTHOP::%d \n",ntohs(router_payload->next_hop));
// 	printf("RCOST::%d \n",ntohs(router_payload->cost));
// 	offset+=ROUTING_TABLE_ROUTER_OFFSET;
// 	}
	free(cntrl_response);
}
void update_response(int sock_index,char *payload){
	char *cntrl_response_header;
	cntrl_response_header = create_response_header(sock_index, 3, 0, EMPTY_PAYLOAD_LEN);
	int response_len;
	response_len=CNTRL_RESP_HEADER_SIZE;
	//send header only
	sendALL(sock_index, cntrl_response_header, response_len);
	free(cntrl_response_header);
	struct UPDATE_PAYLOAD *update_payload;
	update_payload=(struct UPDATE_PAYLOAD *) payload;
	int update_id;
	int cost;
	int router_index;
	update_id=ntohs(update_payload->router_id);
	cost=ntohs(update_payload->cost);
	printf("******************ROUTER ID:: %d****************** \n",update_id);
	printf("******************COST :: %d****************** \n",cost);
	//traverse neighbours array to find neighbour location	
	for(int i=0;i<NO_NEIGHBOURS;i++){
		if(NEIGHBOURS[i].r_id==update_id){
			printf("NEIGHBOUR INDEX::%d \n",i);
			NEIGHBOURS[i].cost_est=cost;
			break;
		}
	}
	printf("******************DV BEFORE UPDATE RESPONSE****************** \n");	
	//recalculate distance vectors now
	//updateRoutingTable();
	updateRoutingTable();
	printf("UPDATED ROUTING TABLE AFTER UPDATE RESPONSE \n");
}
void crash_response(int sock_index){
	char *cntrl_response_header;
	int response_len;
	cntrl_response_header = create_response_header(sock_index, 4, 0, EMPTY_PAYLOAD_LEN);
	response_len=CNTRL_RESP_HEADER_SIZE;
	//send header only
	sendALL(sock_index, cntrl_response_header, response_len);
 	exit(0);
}
void sendfile_response(int sock_index,char *payload,int payload_len){
	char *cntrl_response_header,*file_name;
	uint16_t response_len;
	int destIP,ttl,trans_id,init_seq_no,file_name_size,seq_no,read_loop;
	//extract dest IP addr, TTL, transfer ID, init seq no, filename
	struct SENDFILE_PAYLOAD *sf_payload=(struct SENDFILE_PAYLOAD *) payload;
	destIP=sf_payload->dest_ip_addr;
	ttl=sf_payload->ttl;
	trans_id=sf_payload->trans_id;
	init_seq_no=ntohs(sf_payload->seq_no);
	file_name_size=payload_len-SENDFILE_OFFSET;
	//size = payload length-default 8 bytes
	file_name=(char *) malloc(sizeof(char)*(file_name_size+1));
	//printing received info
	strncpy(file_name,payload+SENDFILE_OFFSET,file_name_size);
	file_name[file_name_size]='\0';
	cntrl_response_header = create_response_header(sock_index, 5, 0, EMPTY_PAYLOAD_LEN);
	response_len=CNTRL_RESP_HEADER_SIZE;
	printf("DEST IP,TTL,TRANS ID, INIT SEQ NO,FILE NAME:: FILE NAME SIZE:: %d %d %d %d %s %d \n",destIP,ttl,trans_id,init_seq_no,file_name,file_name_size);
	//read file from disk and packetize:: and start sending packets :: data handler.c ::
	read_loop=sendFileToDest(destIP,ttl,trans_id,init_seq_no,file_name);
	//send header only
	if(sendALL(sock_index, cntrl_response_header, response_len)<0){
		printf("COULD NOT SEND SENDFILE RESPONSE \n");
	}
	//load file stats
	for(int i=0;i<ROUTERS_MAX;i++){
		if(FILESTATS[i].trans_id==0){
			FILESTATS[i].trans_id=trans_id;
			FILESTATS[i].ttl=ttl;
			seq_no=init_seq_no;
			for(int j=0;j<read_loop;j++){
				FILESTATS[i].seqNoArray[j]=seq_no;
				FILESTATS[i].seqNoArraySize++;
				seq_no++;
			}
			break;
		}
	}
	if(file_name_size!=0){
		printf("FREED FILE NAME \n");
		free(file_name);
	}
	free(cntrl_response_header);
}
void sendfile_stats_response(int sock_index,char *payload){
	char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;
	int trans_id,index,payload_len,response_len,seq_no_count,ttl,offset;
	struct FILE_TRANS_ID *file_trans_id = (struct FILE_TRANS_ID *) payload;
	trans_id=file_trans_id->trans_id;
	printf("FILE TRANSFER ID:: %d \n",trans_id);
	//traverse array and find where the trans_id is there
	for(int i=0;i<ROUTERS_MAX;i++){
		if(FILESTATS[i].trans_id==trans_id){
			index=i;
			break;
		}
	}
	printf("FILE STATS FOUND AT INDEX::%d \n",index);
	seq_no_count=FILESTATS[index].seqNoArraySize;
	payload_len=FILE_DEF_SIZE+(seq_no_count*SEQ_NO_SIZE);
	ttl=FILESTATS[index].ttl;
	printf("NO OF SEQ NUMBERS ::%d TTL VALUE::%d \n",seq_no_count,ttl);
	cntrl_response_payload=(char *) malloc(payload_len);
	struct FILE_STAT_PAYLOAD *file_stat=(struct FILE_STAT_PAYLOAD *) cntrl_response_payload;
	file_stat->trans_id=trans_id;
	file_stat->ttl=ttl;
	file_stat->padding=htons(0);
	offset=FILE_DEF_SIZE;
	for(int i=0;i<seq_no_count;i++){
		struct FILE_SEQ_NO *file_seq_no=(struct FILE_SEQ_NO *) (cntrl_response_payload+offset);
		file_seq_no->seq_no=htons(FILESTATS[index].seqNoArray[i]);
		if(i==0){
			offset=FILE_DEF_SIZE+SEQ_NO_SIZE;
		}
		else{
			offset+=SEQ_NO_SIZE;
		}
	}
	cntrl_response_header = create_response_header(sock_index, 6, 0, payload_len);
	response_len=payload_len+CNTRL_RESP_HEADER_SIZE;
	cntrl_response = (char *) malloc(response_len);
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);
	/* Copy Payload */
	memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	free(cntrl_response_payload);
	sendALL(sock_index, cntrl_response , response_len);
	printf("SENT SENDFILE STATS RESPONSE \n");
	free(cntrl_response);
}
void last_data_packet_response(int sock_index){
	char *cntrl_response_header, *cntrl_response_payload, *cntrl_response,*file_name;
	int trans_id,index,payload_len,response_len,seq_no_count,ttl,offset;
	payload_len=FILE_PACKET_SIZE;
	cntrl_response_payload=(char *) malloc(payload_len);
	struct SENDFILE_DATA_PACKET *data_pkt=(struct SENDFILE_DATA_PACKET *) cntrl_response_payload;
	data_pkt->dest_ip_addr=lastDataPkt.destIP;
	data_pkt->trans_id=lastDataPkt.trans_id;
	data_pkt->ttl=lastDataPkt.ttl;
	data_pkt->seq_no=htons(lastDataPkt.seq_no);
	data_pkt->fin_bit=htons(lastDataPkt.fin_bit);
	data_pkt->padding=htons(0);
	memcpy(cntrl_response_payload+TWELVE_SIZE,lastDataPkt.file_payload,FILE_PAYLOAD_SIZE);
	cntrl_response_header = create_response_header(sock_index, 7, 0, payload_len);
	response_len=payload_len+CNTRL_RESP_HEADER_SIZE;
	cntrl_response = (char *) malloc(response_len);
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);
	/* Copy Payload */
	memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	free(cntrl_response_payload);
	sendALL(sock_index, cntrl_response , response_len);
	printf("SENT LAST DATA PACKET RESPONSE DECODING \n");
	free(cntrl_response);
}
void penultimate_data_packet_response(int sock_index){
	char *cntrl_response_header, *cntrl_response_payload, *cntrl_response,*file_name;
	int trans_id,index,payload_len,response_len,seq_no_count,ttl,offset;
	payload_len=FILE_PACKET_SIZE;
	cntrl_response_payload=(char *) malloc(payload_len);
	struct SENDFILE_DATA_PACKET *data_pkt=(struct SENDFILE_DATA_PACKET *) cntrl_response_payload;
	data_pkt->dest_ip_addr=penDataPkt.destIP;
	data_pkt->trans_id=penDataPkt.trans_id;
	data_pkt->ttl=penDataPkt.ttl;
	data_pkt->seq_no=htons(penDataPkt.seq_no);
	data_pkt->fin_bit=htons(penDataPkt.fin_bit);
	data_pkt->padding=htons(0);
	memcpy(cntrl_response_payload+TWELVE_SIZE,penDataPkt.file_payload,FILE_PAYLOAD_SIZE);
	cntrl_response_header = create_response_header(sock_index, 8, 0, payload_len);
	response_len=payload_len+CNTRL_RESP_HEADER_SIZE;
	cntrl_response = (char *) malloc(response_len);
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);
	/* Copy Payload */
	memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	free(cntrl_response_payload);
	sendALL(sock_index, cntrl_response , response_len);
	printf("SENT PEN LAST DATA PACKET RESPONSE DECODING \n");
	free(cntrl_response);
}