/**
 * @network_util
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
 * Network I/O utility functions. send/recvALL are simple wrappers for
 * the underlying send() and recv() system calls to ensure nbytes are always
 * sent/received.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
ssize_t recvALL(int sock_index, char *buffer, ssize_t nbytes)
{
    ssize_t bytes = 0;
    bytes = recv(sock_index, buffer, nbytes, 0);

    if(bytes == 0) return -1;
    while(bytes != nbytes)
        bytes += recv(sock_index, buffer+bytes, nbytes-bytes, 0);

    return bytes;
}

ssize_t sendALL(int sock_index, char *buffer, ssize_t nbytes)
{
    ssize_t bytes = 0;
    bytes = send(sock_index, buffer, nbytes, 0);

    if(bytes == 0) return -1;
    while(bytes != nbytes)
        bytes += send(sock_index, buffer+bytes, nbytes-bytes, 0);

    return bytes;
}

ssize_t recvALLUDP(int sock_index, char *buffer, ssize_t nbytes)
{
	struct sockaddr_storage their_addr;
	socklen_t addr_len; 
	addr_len=sizeof their_addr;
    ssize_t bytes = 0;
    struct timeval check;
    gettimeofday(&check,NULL);
    bytes = recvfrom(sock_index, buffer, nbytes, 0,(struct sockaddr *)&their_addr, &addr_len);
    if(bytes == 0) return -1;
    while(bytes != nbytes)
        bytes += recvfrom(sock_index, buffer+bytes, nbytes-bytes, 0,(struct sockaddr *)&their_addr, &addr_len);
    return bytes;
}
//send all UDP : 
// ssize_t sendALLUDP(int sock_index, char *buffer, ssize_t nbytes,struct sockaddr_in neighbour, socklen_t tolen)
// {
//     ssize_t bytes = 0;
//     bytes = sendto(sock_index, buffer, nbytes, 0,(struct sockaddr*)&neighbour,&tolen);
//     if(bytes == 0) return -1;
//     while(bytes != nbytes)
//         bytes += sendto(sock_index, buffer+bytes, nbytes-bytes, 0,(struct sockaddr*)&neighbour,&tolen);
// 
//     return bytes;
// }