// Costin Didoaca
// 323CA

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include "helpers.h"
#include "server_help_func.h"

/*
    sends a TCP message to a client.
*/
void sendTcpMessageToClient(client *client, struct tcp_struct *send_to_tcp) {
    int ret = send(client->socket, send_to_tcp, sizeof(struct tcp_struct), 0);
    DIE(ret < 0, "send");
}

/*
    stores an unsent message in a client's unsent messages array.
*/
void storeUnsentMessage(client *client, struct tcp_struct *send_to_tcp) {
    client->unsent[client->dim_unsent++] = *send_to_tcp;
}

/*
    checks if a client's topic matches the target topic
*/
int isMatchingTopic(client *client, int topicIndex, const char *targetTopic) {
    return strcmp(client->topics[topicIndex].nume, targetTopic) == 0;
}

/*
    processes a single client's topics, sending messages 
        to online clients and storing messages for offline clients if the topic matches
*/
void processTopic(client *client, struct tcp_struct *send_to_tcp) {
    for (int i = 0; i < client->dim_topics; i++) {
        if (isMatchingTopic(client, i, send_to_tcp->topic)) {
            if (client->online) {
                sendTcpMessageToClient(client, send_to_tcp);
            } else if (client->topics[i].sf == 1) {
                storeUnsentMessage(client, send_to_tcp);
            }
            break;
        }
    }
}

/*
    loops through the clients and calls processTopic for each client.
*/
void processClients(client *clients, int max_sock, struct tcp_struct send_to_tcp) {
    for (int i = 5; i <= max_sock; i++) {
        processTopic(&clients[i], &send_to_tcp);
    }
}

/*
    function searches for a client in the clients array by their socket descriptor
        if the client is found, returns a pointer to the client structure, otherwise it returns NULL
*/
client* find_client_by_socket(client *clients, int max, int socket) {
    for(int i = 5; i <= max; i++) {
        if(clients[i].socket == socket) {
            return &clients[i];
        }
    }
    return NULL;
}

/*
    searches for a client in the clients array by their unique ID if the client is 
        found, returns a pointer to the client structure, otherwise it returns NULL
*/
client* find_client_by_id(client *clients, int max, const char *id) {
    for(int i = 5; i <= max; i++) {
        if(strcmp(clients[i].id, id) == 0) {
            return &clients[i];
        }
    }
    return NULL;
}

/*
    function sets socket options for the given TCP socket
        disables the Nagle algorithm to improve communication latency
*/
void set_socket_options(int tcp_sock) {
	
    setsockopt(tcp_sock, IPPROTO_TCP, TCP_NODELAY, (int*)1, 4);
}

/*
    creates a new UDP socket and returns the socket descriptor
*/
int create_udp_socket() {
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    DIE(sock < 0, "socket");
    return sock;
}

/*
    creates a new TCP socket and returns the socket descriptor
*/
int create_tcp_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sock < 0, "socket");
    return sock;
}

/*
    binds the given socket to the provided address and returns the result
*/
int bind_socket(int sock, struct sockaddr_in addr) {
    int bnd = bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    DIE(bnd < 0, "bind");
    return bnd;
}
