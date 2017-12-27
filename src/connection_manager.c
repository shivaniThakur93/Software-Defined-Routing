/**
 * @connection_manager
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
 * Connection Manager listens for incoming connections/messages from the
 * controller and other routers and calls the desginated handlers.
 */

#include <sys/select.h>

#include "../include/connection_manager.h"
#include "../include/global.h"
#include "../include/control_handler.h"
#include "../include/router_handler.h"
#include "../include/data_handler.h"

int isTimeOut;
void sortAndSetTimer(){
	struct timerArray temp;
	struct timeval check;
	long int secDiff;
	long int usecDiff;
	//sort timer array
	for(int i=0;i<TIMERARRAYLENGTH;i++){
		for(int j=0;j<(TIMERARRAYLENGTH-i-1);j++){
				if(timerarr[j+1].router_ip!=0 && timerarr[j].router_ip!=0){
					secDiff=timerarr[j+1].end.tv_sec-timerarr[j].end.tv_sec;
					    if(secDiff<0){
						    temp.end.tv_sec=timerarr[j].end.tv_sec;
							temp.end.tv_usec=timerarr[j].end.tv_usec;
							temp.begin.tv_sec=timerarr[j].begin.tv_sec;
							temp.begin.tv_usec=timerarr[j].begin.tv_usec;
							temp.router_ip=timerarr[j].router_ip;
							temp.noupdatecount=timerarr[j].noupdatecount;
							timerarr[j].end.tv_sec=timerarr[j+1].end.tv_sec;
							timerarr[j].end.tv_usec=timerarr[j+1].end.tv_usec;
							timerarr[j].begin.tv_sec=timerarr[j+1].begin.tv_sec;
							timerarr[j].begin.tv_usec=timerarr[j+1].begin.tv_usec;
							timerarr[j].router_ip=timerarr[j+1].router_ip;
							timerarr[j].noupdatecount=timerarr[j+1].noupdatecount;
							timerarr[j+1].end.tv_sec=temp.end.tv_sec;
							timerarr[j+1].end.tv_usec=temp.end.tv_usec;
							timerarr[j+1].begin.tv_sec=temp.begin.tv_sec;
							timerarr[j+1].begin.tv_usec=temp.begin.tv_usec;
							timerarr[j+1].router_ip=temp.router_ip;
							timerarr[j+1].noupdatecount=temp.noupdatecount;
					      }
					    else if(secDiff==0){
					        usecDiff=timerarr[j+1].end.tv_usec-timerarr[j].end.tv_usec;
							if(usecDiff<0){
							temp.end.tv_sec=timerarr[j].end.tv_sec;
							temp.end.tv_usec=timerarr[j].end.tv_usec;
							temp.begin.tv_sec=timerarr[j].begin.tv_sec;
							temp.begin.tv_usec=timerarr[j].begin.tv_usec;
							temp.router_ip=timerarr[j].router_ip;
							temp.noupdatecount=timerarr[j].noupdatecount;
							timerarr[j].end.tv_sec=timerarr[j+1].end.tv_sec;
							timerarr[j].end.tv_usec=timerarr[j+1].end.tv_usec;
							timerarr[j].begin.tv_sec=timerarr[j+1].begin.tv_sec;
							timerarr[j].begin.tv_usec=timerarr[j+1].begin.tv_usec;
							timerarr[j].router_ip=timerarr[j+1].router_ip;
							timerarr[j].noupdatecount=timerarr[j+1].noupdatecount;
							timerarr[j+1].end.tv_sec=temp.end.tv_sec;
							timerarr[j+1].end.tv_usec=temp.end.tv_usec;
							timerarr[j+1].begin.tv_sec=temp.begin.tv_sec;
							timerarr[j+1].begin.tv_usec=temp.begin.tv_usec;
							timerarr[j+1].router_ip=temp.router_ip;
							timerarr[j+1].noupdatecount=temp.noupdatecount;
							}
							}
					}
				}
			}
		//set timer
		if(timerarr[0].end.tv_usec>=currtime.tv_usec){
			usecDiff=timerarr[0].end.tv_usec-currtime.tv_usec;
			secDiff=timerarr[0].end.tv_sec-currtime.tv_sec;
		}
		else{
			usecDiff=MICRO-(currtime.tv_usec-timerarr[0].end.tv_usec);
			secDiff=timerarr[0].end.tv_sec-currtime.tv_sec-1;
		}	
		//set micro seconds
		printf("######## TIMER SET FOR %d SECONDS %d MICROSECONDS ########\n",secDiff,usecDiff);
		tv.tv_sec=secDiff;
		tv.tv_usec=usecDiff;
}
void noUpdateMethod(){
	int no_update_ip,nu_index,del_pos;
	no_update_ip=timerarr[0].router_ip;
	//update neighbour cost to INF
	for(int i=0;i<NO_ROUTERS;i++){
		if(no_update_ip==costmatarr[i].ip_addr){
			nu_index=i;
			break;
		}
	}
	dv_matrix[COST_MAT_INDEX][nu_index].cost=INF;
    dv_matrix[COST_MAT_INDEX][nu_index].next_hop=INF;
	printf("3 UPDATES MISSED FROM :: %d \n",no_update_ip);
	//delete from timer array
	for(int i=0;i<TIMERARRAYLENGTH-1;i++){
		if(timerarr[i].router_ip!=0 && timerarr[i+1].router_ip!=0){
			timerarr[i].router_ip=timerarr[i+1].router_ip;
			timerarr[i].begin.tv_sec=timerarr[i+1].end.tv_sec;
			timerarr[i].begin.tv_usec=timerarr[i+1].begin.tv_usec;
			timerarr[i].end.tv_sec=timerarr[i+1].end.tv_sec;
			timerarr[i].end.tv_usec=timerarr[i+1].end.tv_usec;
			timerarr[i].noupdatecount=timerarr[i+1].noupdatecount;
		}
	}
	TIMERARRAYLENGTH--;
	//delete from neighbour list and set cost in dv matrix to INF
	//find position where element needs to be deleted
	for(int i=0;i<NO_NEIGHBOURS;i++){
		if(NEIGHBOURS[i].ip_addr==no_update_ip){
			del_pos=i;
		}
	}
	//deleting
	printf("NEIGHBOUR TO BE DELETED AT POISITION::%d\n",del_pos);
	for(int j=del_pos;j<NO_NEIGHBOURS-1;j++){
		  NEIGHBOURS[j].ip_addr=NEIGHBOURS[j+1].ip_addr;
		  NEIGHBOURS[j].router_rport=NEIGHBOURS[j+1].router_rport;
		  NEIGHBOURS[j].n_index=NEIGHBOURS[j+1].n_index;
		  NEIGHBOURS[j].cost_est=NEIGHBOURS[j+1].cost_est;
		  NEIGHBOURS[j].r_id=NEIGHBOURS[j+1].r_id;
	}
	NO_NEIGHBOURS--;
	//update routing table as cost has changed
	updateRoutingTable();
}
void updateTimeArrBegin(){
	timerarr[0].begin.tv_sec=timerarr[0].end.tv_sec;
	timerarr[0].begin.tv_usec=timerarr[0].end.tv_usec;
	timerarr[0].end.tv_sec=timerarr[0].end.tv_sec+UPDATE_INTERVAL;
	timerarr[0].end.tv_usec=timerarr[0].end.tv_usec;
}
void main_loop()
{
    int selret, sock_index, fdaccept;
    struct timeval check;
    long int secDiff;
	long int usecDiff;
    while(TRUE){
    	printf("######## WAITING FOR ACTIVITY ######### \n");
        watch_list = master_list;
        //periodic updates send to peers:: send on created router sockets
		//socket,dest ip and port
		if(!INITFLAG){
		   selret = select(head_fd+1, &watch_list, NULL, NULL, NULL);//write list of sockets  
		}
        else{
        	printf("######AFTER INIT SELECT####### \n");
        	printf("TV VAL:: %d \n",tv.tv_sec);
        	printf("TV VAL MICROSECONDS::%d \n",tv.tv_usec);
            selret = select(head_fd+1, &watch_list, NULL, NULL, &tv);//write list of sockets  
        	}
        if(selret < 0)
            ERROR("select failed.");
		/* check if timeout occured :: send dv to routers */
        if(selret==0){//timeout
        	//current Time :: given by end time of the array value whose timeout has occured
        	currtime.tv_sec=timerarr[0].end.tv_sec;
        	currtime.tv_usec=timerarr[0].end.tv_usec;
        	printf("CURRENT TIME SECONDS::%d MICROSECONDS::%d \n",currtime.tv_sec,currtime.tv_usec);
        	//set timeout for next item :: ??
        	if(timerarr[0].router_ip==ROUTER_IP){//expiry value always at 0 index
        	    //router handler
        	    printf("$$$$$$$$$$$$ BROADCAST TIME SECONDS:: %d MICROSECONDS:: %d $$$$$$$$$$$$ \n",currtime.tv_sec,currtime.tv_usec);
        		broadCastRoutingTable(router_socket);
        		//update time at 0 index
        		//reset timer
        		updateTimeArrBegin();
        		//sort function call :: and set timer
        		sortAndSetTimer();
        	}
        	else{
        		printf("TIMEOUT OCCURED FOR :: %d \n",timerarr[0].router_ip);
        		timerarr[0].noupdatecount=timerarr[0].noupdatecount+1;
        		//if no update count==3 : then delete that neighbour from neighbours and timer array
        		if(timerarr[0].noupdatecount==3){
        			noUpdateMethod();
        		}
        		else{
        		    //update time at 0 index 
        			updateTimeArrBegin();
        		}
        		//sort function call :: and set timer
				sortAndSetTimer();
        	}
        }
        else{
        for(sock_index=0; sock_index<=head_fd; sock_index+=1){
            if(FD_ISSET(sock_index, &watch_list)){

                /* control_socket */
                if(sock_index == control_socket){
                    fdaccept = new_control_conn(sock_index);

                    /* Add to watched socket list */
                    FD_SET(fdaccept, &master_list);
                    if(fdaccept > head_fd) head_fd = fdaccept;
                }

                /* router_socket */
                else if(sock_index ==  router_socket){
                    //call handler that will call recvfrom() and reset timer for the router.....
                    printf("######### DATA RECV ON ROUTER SOCKET AT REM TIME %d :: REM MICRO SECONDS :: %d ######### \n",tv.tv_sec,tv.tv_usec);
                    if(timerarr[0].end.tv_usec>=tv.tv_usec){
							secDiff=timerarr[0].end.tv_sec-tv.tv_sec;
							usecDiff=timerarr[0].end.tv_usec-tv.tv_usec;
							}
	 				   else{
							usecDiff=MICRO-(tv.tv_usec-timerarr[0].end.tv_usec);
							secDiff=timerarr[0].end.tv_sec-tv.tv_sec-1;
							}
                        currtime.tv_sec=secDiff;
        				currtime.tv_usec=usecDiff;
        			printf("######### CURRENT TIME SECONDS::%d MICROSECONDS::%d #########\n",currtime.tv_sec,currtime.tv_usec);
                    if(router_recv_hook(sock_index)){
                    	sortAndSetTimer();
                    }  
                }

                /* data_socket */
                else if(sock_index == data_socket){
                	fdaccept = new_data_conn(sock_index);
                    FD_SET(fdaccept, &master_list);
                    if(fdaccept > head_fd) head_fd = fdaccept;
                }

                /* Existing connection */
                else{
                    if(isControl(sock_index)){
                        if(!control_recv_hook(sock_index)) FD_CLR(sock_index, &master_list);
                    }
                    else if(isData(sock_index)){
                    	if(!data_recv_hook(sock_index)) FD_CLR(sock_index, &master_list);
                    }
                    else ERROR("Unknown socket index");
                }
           	 }
        	}
    	}
    }
}

void init()
{
    control_socket = create_control_sock();
	printf("Created Control Socket::%d \n",control_socket);
    //router_socket and data_socket will be initialized after INIT from controller

    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the control socket */
    FD_SET(control_socket, &master_list);
    head_fd = control_socket;

    main_loop();
}