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

#include "../include/global.h"
#include "../include/network_util.h"
#include "../include/control_header_lib.h"

#define SOURCE_ROUTER_FIELDS_SIZE 6
#define ROUTERS_INFO_FIELDS_OFFSET 12
#define CNTRL_PAYLOAD_LEN_OFFSET 0x06
#define INET4_ADDRSTRLEN 128
#define ROUTER_INFO 8

struct __attribute__((__packed__)) UPDATE_FIELDS
    {
        uint16_t no_fields;
        uint16_t src_rport;
        uint32_t src_rtr_ip_addr;
    };
struct __attribute__((__packed__)) ROUTING_UPDATE_ROUTERS
    {
    	uint32_t rtr_ip_addr;
        uint16_t rtr_rport;
        uint16_t padding;
        uint16_t rtr_id;
        uint16_t rtr_cost;
    };
int change;
int ip_matched;
void updateRoutingTable(){
    //bellman ford
    int cost;
   	int mincost;
   	int n_index;
   	int next_hop;
   	int n_flag;
    for(int i=0;i<NO_ROUTERS;i++){
    //if neighbour, mincost= direct cost , else INF
    		mincost=INF;
    		if(dv_matrix[COST_MAT_INDEX][i].cost==0){// do nothing
    			printf("SELF COST :: DO NOTHING \n");
    			}
    		else{
    		//recalculate my first row distance vectors : update:: you must recalculate your costs now	
    				for(int j=0;j<NO_NEIGHBOURS;j++){
    						n_index=NEIGHBOURS[j].n_index;
    						printf("***************DIRECT NEIGHBOUR COST EST:%d COST VIA NEIGHBOUR::%d*************** \n",NEIGHBOURS[j].cost_est,dv_matrix[n_index][i].cost);
    						cost=NEIGHBOURS[j].cost_est+dv_matrix[n_index][i].cost;
    						printf("***************NEIGHBOUR NO::%d INDEX:: %d CAL COST::%d MIN COST ::%d*************** \n",j,n_index,cost,mincost);
    						if(mincost>cost){
    							mincost=cost;
    							next_hop=NEIGHBOURS[j].r_id;
    							dv_matrix[COST_MAT_INDEX][i].cost=mincost;
    						    dv_matrix[COST_MAT_INDEX][i].next_hop=next_hop;
    						    printf("TO ROUTER ID ::%d NEW MIN COST VALUE:: %d NEXT HOP:: %d\n",dv_matrix[COST_MAT_INDEX][i].router_id,cost,next_hop);
    							}
    					}		
    			}
    	}
    //after update
    printf("******************DV AFTER UPDATE****************** \n");
   	 for(int i=0;i<NO_ROUTERS;i++){
   	 	for(int j=0;j<NO_ROUTERS;j++){
   	 		printf("%5d ",dv_matrix[i][j].cost);
   	 	}
   	 	printf("\n");
   	 }
   	 printf("******************HOP VALUES::****************** \n");
   	 for(int i=0;i<NO_ROUTERS;i++){
   	 	printf("%5d ",dv_matrix[COST_MAT_INDEX][i].next_hop);
   	 }
   	 printf("\n ");
    }
void updateDistanceVectorMatrix(int senderIP,char *router_payload){
	//bellman ford algo to update distance vectors
	//update distance vector matrix
	//recompute routing table
   int senderIndex;
   int prevcost;
   int newcost;
   int next_hop;
   int n_index;
   struct ROUTING_UPDATE_ROUTERS *router_updates;
   int update_rt;
   int offset=ROUTER_INFO;
   //
   int IP;
   int PORT;
   int PADDING;
   int ID;
   int COST;
   //
   //iterate init array to find index in dv
   for(int i=0;i<NO_NEIGHBOURS;i++){
   		if(senderIP==NEIGHBOURS[i].ip_addr){
   			senderIndex=NEIGHBOURS[i].n_index;
   			break;
   		}	
   }
   	//iterate and check whether cost has changed
    for(int i=0;i<NO_ROUTERS;i++){
    struct ROUTING_UPDATE_ROUTERS *router_fields=(struct ROUTING_UPDATE_ROUTERS *) (router_payload+offset);
   	IP=router_fields->rtr_ip_addr;
   	PORT=ntohs(router_fields->rtr_rport);
   	PADDING=ntohs(router_fields->padding);
   	ID=ntohs(router_fields->rtr_id);
   	newcost=ntohs(router_fields->rtr_cost);
   	prevcost=dv_matrix[senderIndex][i].cost;
   	if(prevcost!=newcost){
   			dv_matrix[senderIndex][i].cost=newcost;
   			update_rt=1;
   		}
   	offset+=ROUTERS_INFO_FIELDS_OFFSET;
   	}	   
   	printf("******************DV BEFORE UPDATE FROM NEIGHBOUR****************** \n");
   	 for(int i=0;i<NO_ROUTERS;i++){
   	 	for(int j=0;j<NO_ROUTERS;j++){
   	 		printf("%5d ",dv_matrix[i][j].cost);
   	 	}
   	 	printf("\n");
   	 }	
   	if(update_rt==1){
    	updateRoutingTable();
    	}
    else{
    	printf("******************DISTANCE VECTOR OF SENDING NEIGHBOUR %d DID NOT CHANGE****************** \n",senderIP);
    	}
}

int create_router_sock()
{
	printf("ENTRY CREATE ROUTER SOCK::\n");
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    printf("PLACE 1 \n");
    printf("RPORT CRS::%s \n",ROUTER_PORT);
    if ((rv = getaddrinfo(NULL, ROUTER_PORT, &hints, &servinfo)) != 0) {
        printf("getaddrinfo::create_router_sock::failure \n");
        return 1;
     }
     // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            printf("listener: socket \n");
				continue; 
		}
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            printf("listener: bind \n");
			continue; 
		}
		break; 
	}
	printf("PLACE 2 \n");
	printf("SOCK FD:: %d \n",sockfd);
	if (p == NULL) {
        printf("listener: failed to bind socket \n");
        return 2;
    }
    freeaddrinfo(servinfo);	 
    return sockfd;
}


bool router_recv_hook(int sock_index)
{	
	struct timeval check;
	long int usecDiff,secDiff;
    char *router_payload;
    struct sockaddr_storage their_addr;
	socklen_t addr_len; 
	addr_len=sizeof their_addr;
    ssize_t bytes = 0;
    int offset=0;
	int router_packet_len,no_fields,senderIP,senderPort; // except no of update fields
    /* Get control header */
    router_packet_len=ROUTER_INFO+NO_ROUTERS*ROUTERS_INFO_FIELDS_OFFSET;
    router_payload = (char *) malloc(sizeof(char)*router_packet_len);
    bzero(router_payload, router_packet_len);
    gettimeofday(&check,NULL);
    if(recvALLUDP(sock_index, router_payload, router_packet_len) < 0){
        //remove_control_conn(sock_index);
        return FALSE;
    }
    gettimeofday(&check,NULL);
    /* Get number of fields from routing update packet and calculate length of the packet */
        struct UPDATE_FIELDS *update_fields = (struct UPDATE_FIELDS *)(router_payload+offset);
  		no_fields=ntohs(update_fields->no_fields);
  		senderPort=ntohs(update_fields->src_rport);
  		senderIP=update_fields->src_rtr_ip_addr;
    /* Get routers information */
      if(router_packet_len != 0){
	   updateDistanceVectorMatrix(senderIP,router_payload);
	   gettimeofday(&check,NULL);
	   }
	   
	  //reset timer for the given router
	  for(int i=0;i<NO_NEIGHBOURS+1;i++){
		 if(timerarr[i].router_ip==senderIP){
			 // start time = end time of first entry - remaining time
		     timerarr[i].begin.tv_sec=currtime.tv_sec;
		     timerarr[i].begin.tv_usec=currtime.tv_usec;
			 timerarr[i].end.tv_sec=currtime.tv_sec+UPDATE_INTERVAL;
			 timerarr[i].end.tv_usec=currtime.tv_usec;
			 timerarr[i].noupdatecount=0;
			 break;
		 }
		 else if(timerarr[i].router_ip==0){
		 	 //insert entry here
		 	 timerarr[i].router_ip=senderIP;
		 	 timerarr[i].begin.tv_sec=currtime.tv_sec;
		     timerarr[i].begin.tv_usec=currtime.tv_usec;
			 timerarr[i].end.tv_sec=currtime.tv_sec+UPDATE_INTERVAL;
			 timerarr[i].end.tv_usec=currtime.tv_usec;
		 	 timerarr[i].noupdatecount=0;
		 	 TIMERARRAYLENGTH++;
		 	 break;
		 }
	  }
	  gettimeofday(&check,NULL);
    if(router_packet_len != 0) free(router_payload);
    return TRUE;
}

void broadCastRoutingTable(int router_socket){
  //send RT to neighbours
  struct UPDATE_FIELDS *update_fields;
  struct sockaddr_in neighbour;
  struct in_addr addr;
  char ip4[128];
  char *rt_pkt;
  int rt_pkt_len;
  int offset=ROUTER_INFO;
  int broadCastSocket;
  ssize_t bytes = 0;
  int tolen=sizeof(neighbour);
  int no_fields,senderIP,senderPort; // except no of update fields
  rt_pkt_len=ROUTER_INFO+ROUTERS_INFO_FIELDS_OFFSET*NO_ROUTERS;
  rt_pkt=(char *) malloc(rt_pkt_len);//
  update_fields=(struct UPDATE_FIELDS *) rt_pkt;
  update_fields->no_fields=htons(NO_ROUTERS);
  update_fields->src_rport=htons(ROUTER_IPORT);
  update_fields->src_rtr_ip_addr=ROUTER_IP;
  printf("****************** BROADCAST ROUTING TABLE FROM :: %d MY ID \n",ROUTER_IP);
  for(int i=0;i<NO_ROUTERS;i++){
		struct ROUTING_UPDATE_ROUTERS *router_fields=(struct ROUTING_UPDATE_ROUTERS *)(rt_pkt+offset);
		printf("IP ADDR::%d \n",dv_matrix[COST_MAT_INDEX][i].ip_addr);
		printf("RPORT ::%d \n",dv_matrix[COST_MAT_INDEX][i].router_rport);
		printf("PADDING::%d \n",htons(0));
		printf("RID ::%d \n",dv_matrix[COST_MAT_INDEX][i].router_id);
		printf("RCOST ::%d \n",dv_matrix[COST_MAT_INDEX][i].cost);
		router_fields->rtr_ip_addr=dv_matrix[COST_MAT_INDEX][i].ip_addr;
		router_fields->rtr_rport=htons(dv_matrix[COST_MAT_INDEX][i].router_rport);
		router_fields->padding=htons(0);
		router_fields->rtr_id=htons(dv_matrix[COST_MAT_INDEX][i].router_id);
		router_fields->rtr_cost=htons(dv_matrix[COST_MAT_INDEX][i].cost);
		offset+=ROUTERS_INFO_FIELDS_OFFSET;
	}
  for(int i=0;i<NO_NEIGHBOURS;i++){
    memset((char *) &neighbour, 0, sizeof(neighbour));
    neighbour.sin_family = AF_INET;
    neighbour.sin_port = htons(NEIGHBOURS[i].router_rport);
    memcpy(&(neighbour.sin_addr),&(NEIGHBOURS[i].ip_addr),sizeof(struct in_addr));
  	//sendALLUDP(router_socket,rt_pkt,rt_pkt_len,neighbour,tolen);
    bytes = sendto(router_socket, rt_pkt, rt_pkt_len, 0,(struct sockaddr*)&neighbour,tolen);
    if(bytes == 0) printf("ERROR IN BROADCASTING \n");
    while(bytes != rt_pkt_len)
        bytes += sendto(router_socket, rt_pkt+bytes, rt_pkt_len-bytes, 0,(struct sockaddr*) &neighbour,tolen);
    printf("@@@@@@@@@@@ RT BROADCASTED TO PORT:: %d IP::%d \n",NEIGHBOURS[i].router_rport,NEIGHBOURS[i].ip_addr);
  	}
  if(rt_pkt_len!=0) free(rt_pkt);
}