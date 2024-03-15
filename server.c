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
#include "server_help_func.c"

/*
creates two sockets, one UDP and one TCP, and binds them to the 
    same address and port. It sets options for the TCP socket and 
    starts listening for connections on it
*/
void create_server_socket(int* udp_sock, int* tcp_sock, int port) {
    *udp_sock = create_udp_socket();
    *tcp_sock = create_tcp_socket();
    DIE(*udp_sock < 0 || *tcp_sock < 0, "socket");

    struct sockaddr_in serv_addr, udp_addr;

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
	bind_socket(*tcp_sock, serv_addr);
	

    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(port);
    udp_addr.sin_addr.s_addr = INADDR_ANY;
	bind_socket(*udp_sock, udp_addr);
    
	// No Neagle
    set_socket_options(*tcp_sock);

    listen(*tcp_sock, INT_MAX);
    DIE(listen(*tcp_sock, INT_MAX)< 0, "listen");
}

/*
    handles input from the keyboard. If the "exit" command is 
        received, the server will close
*/
int handle_stdin_input(char* buffer) {
    
    fgets(buffer, 100, stdin);

    if(strncmp(buffer, "exit", 4) == 0) {
        return 1;
    }
    DIE(strncmp(buffer, "exit", 4) != 0, "exit");

    return 0;
}

/*
    handles messages received on the UDP socket. Is getting relevant 
        information from the message and processes it then sends messages to subscribed clients
*/
void handle_udp_messages(char* buffer, int udp_sock, int i, int max_sock, struct sockaddr_in udp_addr, client* clients, socklen_t udp_len) {
    
    recvfrom(udp_sock, buffer, 1551, 0, (struct sockaddr *)&udp_addr, &udp_len);
    
    struct tcp_struct send_to_tcp;
    struct udp_struct *send_to_udp;
    in_port_t udp_port = udp_addr.sin_port;
    char* udp_ip = inet_ntoa(udp_addr.sin_addr);

    memset(&send_to_tcp, 0, sizeof(struct tcp_struct));
    send_to_tcp.port = htons(udp_port);
    strcpy(send_to_tcp.ip, udp_ip);
	
    send_to_udp = (struct udp_struct *)buffer;

    strcpy(send_to_tcp.topic, send_to_udp->topic);


	switch(send_to_udp->type) {
		case 0:
			{
				uint32_t num = ntohl(*(uint32_t *)(send_to_udp->content + 1));
				if(send_to_udp->content[0] == 1) {
					num *= -1;
				}
                sprintf(send_to_tcp.content, "%d", num);
				strcpy(send_to_tcp.type, INT);
				break;
			}
		case 1:
			{
				float real = abs(ntohs(*(uint16_t *)(send_to_udp->content)));
				strcpy(send_to_tcp.type, SHORT_REAL);
				sprintf(send_to_tcp.content, "%.2f", real / 100);
				break;
			}
		case 2:
			{
				
                int exp, divisor = 1;
                double realValue = ntohl(*(uint32_t *)(send_to_udp->content + 1));
                int decimalLast = send_to_udp->content[5];

                for (exp = 0; exp < decimalLast; exp++) {
                    divisor *= 10;
                }
                
                realValue /= divisor;
                strcpy(send_to_tcp.type, FLOAT);

                if (send_to_udp->content[0] == 1) {
                    realValue *= -1;
                    
                }
                sprintf(send_to_tcp.content, "%lf", realValue);
				break;
			}
		default:
			{
				strcpy(send_to_tcp.type, STRING);
				strcpy(send_to_tcp.content, send_to_udp->content);
				break;
			}
	}

    processClients(clients, max_sock, send_to_tcp);
}

/*
    handles TCP connections received from clients. Accepts new connections, checks if the 
        client is already in the list, and if so, processes the unread messages for client
*/
void handle_tcp_connections(char *buffer,int tcp_sock, int i, int* max_sock, struct sockaddr_in new_tcp, client* clients, fd_set* file_descr, socklen_t udp_len) {
    
    int socket = accept(tcp_sock, (struct sockaddr *) &new_tcp, &udp_len);
    DIE(socket < 0, "accept");
	
   
    int ret = recv(socket, buffer, 10, 0);
    DIE(ret < 0, "recv");

    client* client_found = find_client_by_id(clients, *max_sock, buffer);

    if(client_found == NULL) {
        // new client
        FD_SET(socket, file_descr);
        if(socket > *max_sock)
            *max_sock = socket;

        strcpy(clients[*max_sock].id, buffer);
        clients[*max_sock].socket = socket;
        clients[*max_sock].online = 1;
        printf("New client %s connected from %s:%d\n", clients[*max_sock].id,
            inet_ntoa(new_tcp.sin_addr), ntohs(new_tcp.sin_port));
    } else if(client_found->online == 0) {
        // old client
        FD_SET(socket, file_descr);
        client_found->socket = socket;
        client_found->online = 1;

        printf("New client %s connected from %s:%d.\n", client_found->id,
            inet_ntoa(new_tcp.sin_addr), ntohs(new_tcp.sin_port));

        for(int i = 0; i < client_found->dim_unsent; i++){
            int ret = send(client_found->socket, &client_found->unsent[i],
                        sizeof(struct tcp_struct), 0);
            DIE(ret < 0, "send");
        }
        client_found->dim_unsent = 0;
    } else {
        
        close(socket);
        printf("Client %s already connected.\n", client_found->id);
    }
}

int findTopicIndex(client *client, char *topicName, int numTopics) {
    for (int i = 0; i < numTopics; i++) {
        if (strcmp(client->topics[i].nume, topicName) == 0) {
            return i;
        }
    }
    return -1;
}

/*
    adds the new topic to the client's topics list.
*/
void add_new_topic(client *client, Packet *input) {
    strcpy((*client).topics[client->dim_topics].nume, input->topic);
    (*client).topics[(*client).dim_topics].sf = (*input).data_type;
    (*client).dim_topics++;
}

/*
    shifts the topics to the left, effectively removing the topic 
        at the specified index, and then decrements the len of topics 
*/
void remove_topic_by_index(client *client, int topicIdx) {
    for (int l = topicIdx; l < client->dim_topics - 1; l++) {
        client->topics[l] = client->topics[l + 1];
    }
    client->dim_topics--;
}

/*
    handles messages received from TCP clients. If the message indicates 
        a subscription or unsubscription to a topic, the client's topic list is updated
*/
void handle_received_message(int i, client* clients, int max_sock, fd_set *file_descr) {
    char buffer[PACKLEN];
    memset(buffer, 0, PACKLEN);
    int read_bytes = recv(i, buffer, PACKLEN, 0);
    DIE(read_bytes < 0, "recv");

    client* client_by_sock = find_client_by_socket(clients, max_sock, i);

    if (client_by_sock != NULL) {
        if (read_bytes) {
            struct Packet *input = (struct Packet *) buffer;
            switch (input->type) {
                case 's':
                    
                    int topicIndex = findTopicIndex(client_by_sock, input->topic, client_by_sock->dim_topics);
                    
                    if (topicIndex < 0) 
                        add_new_topic(client_by_sock, input);
                    break;
                case 'u':
                    int topicIdx = findTopicIndex(client_by_sock, input->topic, client_by_sock->dim_topics);;
                    
                    if (topicIdx >= 0) 
                        remove_topic_by_index(client_by_sock, topicIdx);
                    break;
                case 'e':
                    printf("Client %s disconnected.\n", client_by_sock->id);
                    client_by_sock->online = 0;
                    client_by_sock->socket = -1;
                    FD_CLR(i, file_descr);
                    close(i);
                    break;
            }
        } else 
            {
            printf("Client %s disconnected.\n", client_by_sock->id);
            client_by_sock->online = 0;
            client_by_sock->socket = -1;
            FD_CLR(i, file_descr);
            close(i);
        }
    }
}

/*
    setup the file descriptors structure
*/
void init_fd_set(fd_set *file_descr, int tcp_sock, int udp_sock) {
    FD_ZERO(file_descr);
    FD_SET(tcp_sock, file_descr);
    FD_SET(udp_sock, file_descr);
    FD_SET(STDIN_FILENO, file_descr);
}

int main(int argc, char** argv) {

	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	DIE(argc < 2, "arguments");

	int tcp_sock; 
	int udp_sock;
	int server_on = 0;
	struct client *clients = calloc(1000, sizeof(struct client));
	

	struct sockaddr_in udp_addr, new_tcp;

	create_server_socket(&udp_sock, &tcp_sock, atoi(argv[1]));

	// init fd
	fd_set file_descr;
    init_fd_set(&file_descr, tcp_sock, udp_sock);
    
	socklen_t udp_len = sizeof(struct sockaddr);
	int max_sock = max(tcp_sock, udp_sock);
    
    do {
        char buffer[PACKLEN];
        fd_set aux_fd = file_descr;
        select(max_sock + 1, &aux_fd, NULL, NULL, NULL);
        DIE(select(max_sock + 1, &aux_fd, NULL, NULL, NULL) < 0, "select");

        // handle the cases received
        for (int i = 0; i <= max_sock; i++) {
            int check_fd = FD_ISSET(i, &aux_fd);
            if (check_fd) {
                memset(buffer, 0, PACKLEN);
                CaseHandler case_type = determine_case(i, tcp_sock, udp_sock);

                switch (case_type) {
                    case CASE_TCP_CONNECTIONS:
                        handle_tcp_connections(buffer, tcp_sock, i, &max_sock, new_tcp, clients, &file_descr, udp_len);
                        break;
                    case CASE_UDP_MESSAGES:
                        handle_udp_messages(buffer, udp_sock, i, max_sock, udp_addr, clients, udp_len);
                        break;
                    case CASE_STDIN_INPUT:
                        server_on = handle_stdin_input(buffer);
                        break;
                    case CASE_RECEIVED_MESSAGE:
                        handle_received_message(i, clients, max_sock, &file_descr);
                        break;
                    case CASE_NONE:
                    default:
                        break;
                }
            }
        }
    } while(!server_on);

	for(int i = 3; i <= max_sock; i++) {
		if(FD_ISSET(i, &file_descr))
			close(i);
	}

	return 0;
}




