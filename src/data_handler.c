/**
 * @control_handler
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
 * Handler for the control plane.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <sys/ioctl.h>

#include "../include/global.h"
#include "../include/network_util.h"
#include "../include/control_header_lib.h"


#define IP4_ADDR_LEN 16
#define FILE_PACKET_HEADER_SIZE 12
#define DEFAULT_FILE_APPENDER file_
#define DEFAULT_FILE_APPENDER_SIZE 5
#define FILE_NAME_SIZE 6 //5+1=6 bytes

/* Linked List for active control connections */
struct DataConn
{
    int sockfd;
    LIST_ENTRY(DataConn) next;
}*connection, *conn_temp;
LIST_HEAD(DataConnsHead, DataConn) data_conn_list;

int file_payload_size=1024;
int file_offset=0;
int file_write_socket=0;
int file_write_flag=0;
int peer_socket_forward=0;
int peer_socket_flag=0;

FILE *file_write;
//start
struct __attribute__((__packed__)) FILE_TRANSFER_HEADER
    {
    	uint32_t dest_ip_addr;
    	uint8_t trans_id;
		uint8_t ttl;
        uint16_t seq_no; 
        uint16_t fin_bit;       
	    uint16_t padding; 
	};

int createPeerSocket(int destIP){
	printf("ENTRY CREATE PEER SOCKET \n");
	int remote_data_port,remote_IP,next_hop_id,peer_socket;
    struct sockaddr_in remote_addr;
    struct in_addr addr;
    char s_remote_IP[IP4_ADDR_LEN];
    socklen_t addrlen = sizeof(remote_addr);
    //query the data port of next hop router
    for(int i=0;i<NO_ROUTERS;i++){
    	if(dv_matrix[COST_MAT_INDEX][i].ip_addr==destIP){
    		next_hop_id=dv_matrix[COST_MAT_INDEX][i].next_hop;
    		break;
    	}
    }
    printf("******************NEXT HOP ID IS :: %d****************** \n",next_hop_id);
    //use this id to find ip addr and data port of next hop router
    for(int i=0;i<NO_ROUTERS;i++){
    	if(next_hop_id==costmatarr[i].router_id){
    		remote_data_port=costmatarr[i].router_dport;
    		remote_IP=costmatarr[i].ip_addr;
    	}
    }
    printf("******************NEXT HOP INT IP IS :: %d****************** \n",remote_IP);
    printf("******************NEXT HOP DATA PORT IS :: %d****************** \n",remote_data_port);
    /* Zeroing remote_addr struct */
    memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(remote_data_port);
    memcpy(&(remote_addr.sin_addr),&(remote_IP),sizeof(struct in_addr));
    
    //print string version of IP
    addr.s_addr=remote_IP;
    inet_ntop(AF_INET, &(addr.s_addr),s_remote_IP, IP4_ADDR_LEN);
    printf("******************NEXT HOP STRING IP IS::%s****************** \n",s_remote_IP);
    //end
    
    peer_socket=socket(AF_INET, SOCK_STREAM, 0);;
    if(peer_socket < 0)
        ERROR("socket() failed");

    /* Make socket re-usable */
    if(setsockopt(peer_socket, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) < 0)
        ERROR("setsockopt() failed");

 	/* Connect to the server */
    if (connect(peer_socket, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
        {
                fprintf(stderr, "Error on connect --> %s\n", strerror(errno));
                exit(EXIT_FAILURE);
        }
    printf("******************CREATED PEER SOCKET::%d****************** \n",peer_socket);
    return peer_socket;
}

int create_data_sock(){
	int sock;
    struct sockaddr_in data_addr;
    socklen_t addrlen = sizeof(data_addr);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
        ERROR("socket() failed");

    /* Make socket re-usable */
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) < 0)
        ERROR("setsockopt() failed");

    bzero(&data_addr, sizeof(data_addr));

    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    data_addr.sin_port = htons(DATA_PORT);

    if(bind(sock, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0)
        ERROR("bind() failed");

    if(listen(sock, 5) < 0)
        ERROR("listen() failed");

    LIST_INIT(&data_conn_list);//this initializes the list header
    
    return sock;
}

int new_data_conn(int sock_index)
{
    int fdaccept, caddr_len;
    struct sockaddr_in remote_controller_addr;

    caddr_len = sizeof(remote_controller_addr);
    fdaccept = accept(sock_index, (struct sockaddr *)&remote_controller_addr, &caddr_len);
    if(fdaccept < 0)
        ERROR("accept() failed");

    /* Insert into list of active control connections */
    connection = malloc(sizeof(struct DataConn));
    connection->sockfd = fdaccept;
    LIST_INSERT_HEAD(&data_conn_list, connection, next);

    return fdaccept;
}

bool isData(int sock_index)
{
    LIST_FOREACH(connection, &data_conn_list, next)
        if(connection->sockfd == sock_index) return TRUE;

    return FALSE;
}

//
void forwardFileToDest(int destIP,int dec_ttl,int trans_ID,int seq_no,int fin_bit,char file_payload[FILE_PAYLOAD_SIZE]){
	//decrement TTl
	char file_packet_forward[FILE_PACKET_SIZE];
	//forward packet
	if(!peer_socket_flag){
		//find dest IP of next min hop from DV
		peer_socket_forward=createPeerSocket(destIP);
		peer_socket_flag=1;
	}
	struct FILE_TRANSFER_HEADER *file_header = (struct FILE_TRANSFER_HEADER *) file_packet_forward;
	file_header->dest_ip_addr=destIP;
	file_header->trans_id=trans_ID;
	file_header->ttl=dec_ttl;
	file_header->seq_no=htons(seq_no);
	file_header->fin_bit=htons(fin_bit);
	file_header->padding=htons(0);//always
	memcpy(file_packet_forward+FILE_PACKET_HEADER_SIZE,file_payload,FILE_PAYLOAD_SIZE);
	if(sendALL(peer_socket_forward,file_packet_forward,FILE_PACKET_SIZE)<0){
		printf("ERROR ON SENDING 1036 BYTES OF FILE \n");     
	}
	if(fin_bit!=0){//indicates last data pkt
		printf("FILE TRANSFER COMPLETE::%d \n",fin_bit);
		close(peer_socket_forward);
		peer_socket_flag=0;
	}
}

void saveFileToDisk(char file_payload[FILE_PAYLOAD_SIZE],int trans_id,int fin_bit){
	//write only socket for file transfer id :: file-transfer id
	if(!file_write_flag){
		char file_name[20];
		snprintf(file_name, sizeof file_name, "file-%d", trans_id);
		file_write=fopen(file_name,"wb");
		file_write_flag=1;
	}
	//write 1024 bytes
	if(fwrite(file_payload,FILE_PAYLOAD_SIZE,1,file_write) < 0){
			printf("ERROR ON WRITING 1024 BYTES FROM FILE \n");            
        }
    if(fin_bit!=0){//close file after receiving fin bit
   		printf("******************FILE TRANSFER COMPLETE::%d****************** \n",fin_bit);
    	fclose(file_write);
    	file_write_flag=0;
    }
    
}
bool data_recv_hook(int sock_index)
{	
	//recv all data and then save/forward depending on IP
	//extract dest IP and TTL values
	char file_payload[FILE_PAYLOAD_SIZE];
	char file_packet[FILE_PACKET_SIZE];
	int destIP,ttl,trans_id,seq_no,fin_bit,peer_socket,dec_ttl,trans_flag,file_index,array_index;   
	trans_flag=0; 
	/* Get control header */
    if(recvALL(sock_index, file_packet, FILE_PACKET_SIZE) < 0){
        //remove_control_conn(sock_index);
        printf("ERROR ON RECEIVING DATA FROM DATA SOCKET \n");
        return FALSE;
    }
    struct FILE_TRANSFER_HEADER *file_header = (struct FILE_TRANSFER_HEADER *) file_packet;
    destIP=file_header->dest_ip_addr;
	trans_id=file_header->trans_id;
	ttl=file_header->ttl;
	seq_no=ntohs(file_header->seq_no);
	fin_bit=ntohs(file_header->fin_bit);
	memcpy(file_payload,file_packet+FILE_PACKET_HEADER_SIZE,FILE_PAYLOAD_SIZE);
	dec_ttl=ttl-1;
	if(dec_ttl>0){
		//load file stats
		for(int i=0;i<ROUTERS_MAX;i++){
			if(FILESTATS[i].trans_id==trans_id){
				file_index=i;
				trans_flag=1;
				break;
				}
			else if(FILESTATS[i].trans_id==0){
				file_index=i;
				break;
			}
			}
		if(trans_flag){
			//update seq no array
			array_index=FILESTATS[file_index].seqNoArraySize;
			FILESTATS[file_index].seqNoArray[array_index]=seq_no;
			FILESTATS[file_index].seqNoArraySize+=1;
		}
		else{
			FILESTATS[file_index].trans_id=trans_id;
			FILESTATS[file_index].ttl=dec_ttl;
			FILESTATS[file_index].seqNoArray[0]=seq_no;
			FILESTATS[file_index].seqNoArraySize+=1;
		}
		//load last data and penultimate data stats
		if(lastDataPkt.destIP==0 || lastDataPkt.destIP!=destIP){//first data pkt , load the pkt
			lastDataPkt.destIP=destIP;
			lastDataPkt.trans_id=trans_id;
			lastDataPkt.ttl=dec_ttl;
			lastDataPkt.seq_no=seq_no;
			lastDataPkt.fin_bit=fin_bit;
			memcpy(lastDataPkt.file_payload,file_payload,FILE_PAYLOAD_SIZE);
		}
		else {
			penDataPkt.destIP=lastDataPkt.destIP;//copy last data pkt to pen data pkt
			penDataPkt.trans_id=lastDataPkt.trans_id;
			penDataPkt.ttl=lastDataPkt.ttl;
			penDataPkt.seq_no=lastDataPkt.seq_no;
			penDataPkt.fin_bit=lastDataPkt.fin_bit;
			memcpy(penDataPkt.file_payload,lastDataPkt.file_payload,FILE_PAYLOAD_SIZE);
			lastDataPkt.destIP=destIP;
			lastDataPkt.trans_id=trans_id;
			lastDataPkt.ttl=dec_ttl;
			lastDataPkt.seq_no=seq_no;
			lastDataPkt.fin_bit=fin_bit;
			memcpy(lastDataPkt.file_payload,file_payload,FILE_PAYLOAD_SIZE);
		}
		if(destIP!=ROUTER_IP){//forward to next hop
	    	forwardFileToDest(destIP,dec_ttl,trans_id,seq_no,fin_bit,file_payload);
		}
		else{//recv and reconstruct file
			saveFileToDisk(file_payload,trans_id,fin_bit);//??
		}
	}
	return TRUE;
}
int sendFileToDest(int destIP,int ttl,int trans_id,int init_seq_no,char *file_name){
	int file_socket,file_rem_len,read_loop,peer_socket,seq_no,fin_bit;
	char file_payload[FILE_PAYLOAD_SIZE];
	char file_packet[FILE_PACKET_SIZE];
	FILE *file_read;
	//create peer socket
	file_read=fopen(file_name,"rb");
	if(file_socket==-1){
		printf("COULD NOT OPEN FILE ::%s \n",file_name);
	}
	//create 1024 byte sized packets and start sending over peer socket until all are sent
	fseek(file_read, 0L, SEEK_END);
	file_rem_len = ftell(file_read);
	rewind(file_read);	
	read_loop=(file_rem_len/file_payload_size);
	peer_socket=createPeerSocket(destIP);
	seq_no=init_seq_no;
	struct timeval check;
	gettimeofday(&check,NULL);
	printf("FILE SEND START TIME::%d \n",check.tv_sec);
	struct FILE_TRANSFER_HEADER *file_header = (struct FILE_TRANSFER_HEADER *) file_packet;
	fin_bit=0;
	for(int i=0;i<read_loop;i++){
		//read 1024 bytes
		if(fread(file_payload,FILE_PAYLOAD_SIZE,1,file_read) < 0){
			printf("ERROR ON READING 1024 BYTES FROM FILE \n");            
        }
		file_header->dest_ip_addr=destIP;
		file_header->trans_id=trans_id;
		file_header->ttl=ttl;
		file_header->seq_no=htons(seq_no);
		if(i==(read_loop-2)){//last data pkt
			penDataPkt.destIP=destIP;//copy last data pkt to pen data pkt
			penDataPkt.trans_id=trans_id;
			penDataPkt.ttl=ttl;
			penDataPkt.seq_no=seq_no;
			penDataPkt.fin_bit=fin_bit;
			memcpy(penDataPkt.file_payload,file_payload,FILE_PAYLOAD_SIZE);
		}
		else if(i==(read_loop-1)){//pen data packet
			fin_bit=LAST_FIN_BIT;
			lastDataPkt.destIP=destIP;
			lastDataPkt.trans_id=trans_id;
			lastDataPkt.ttl=ttl;
			lastDataPkt.seq_no=seq_no;
			lastDataPkt.fin_bit=fin_bit;
			memcpy(lastDataPkt.file_payload,file_payload,FILE_PAYLOAD_SIZE);
		}
		file_header->fin_bit=htons(fin_bit);
		file_header->padding=htons(0);//always
		memcpy(file_packet+FILE_PACKET_HEADER_SIZE,file_payload,FILE_PAYLOAD_SIZE);
		//initially set to 65535 for the last packet
		if(sendALL(peer_socket,file_packet,FILE_PACKET_SIZE)<0){
			printf("ERROR ON SENDING 1036 BYTES OF FILE \n");     
		}
		seq_no++;
	}
	gettimeofday(&check,NULL);
	printf("FILE SEND END TIME::%d \n",check.tv_sec);	
	close(peer_socket);
	fclose(file_read);
	printf("******************SENT COMPLETE FILE & CLOSED FILE AND PEER SOCKETS****************** \n");
	return read_loop;
}