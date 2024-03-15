// Costin Didoaca
// 323CA

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/time.h>
#include "helpers.h"
#include "server_help_func.h"

/*
    ensures that the output is immediately printed in 
        terminal without waiting for the buffer to be filled
*/
void set_non_buffered_stdout() {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
}

/*
    creates a new TCP socket, checks for errors, and returns 
        the socket file descriptor
*/
int create_tcp_socket() {
    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    DIE(tcp_sock < 0, "socket");
    return tcp_sock;
}

/*
    init the server data IP address and port in the sockaddr_in structure
*/
void setup_server_data(struct sockaddr_in *server_data, const char *ip, const char *port) {
    server_data->sin_family = AF_INET;
    server_data->sin_port = htons(atoi(port));
    inet_aton(ip, &server_data->sin_addr);
}

/*
    connects the TCP socket to the server with the provided server data and 
        checks for errors in the connection process
*/
void connect_to_server(int tcp_sock, struct sockaddr_in *server_data) {
    int ret = connect(tcp_sock, (struct sockaddr *)server_data, sizeof(*server_data));
    DIE(ret < 0, "connect");
}

/*
    sends the client ID to the server using the TCP socket
*/
void send_client_id(int tcp_sock, const char *client_id) {
    int ret = send(tcp_sock, client_id, 10, 0);
    DIE(ret < 0, "send");
}

/*
    sends a packet with the type 'e'/(exit) to the server 
        using the TCP socket, saying that the client wants to disconnect
*/
void handle_exit(int tcp_sock, Packet *pack) {
    pack->type = 'e';
    int sending_exit = send(tcp_sock, pack, PACKLEN, 0);
    DIE(sending_exit < 0, "send");
}

/*
    handles the subscription process by sending a packet with the type 's'/(subscribe), 
        the desired topic and the data type to the server using the TCP socket
*/
void handle_subscribe(char *token, int tcp_sock, Packet *pack) {
    pack->type = 's';
    token = strtok(NULL, " ");
    strcpy(pack->topic, token);
    token = strtok(NULL, " ");
    pack->data_type= token[0] - '0';

    int sending_subscribe = send(tcp_sock, pack, PACKLEN, 0);
    DIE(sending_subscribe < 0, "send");
    printf("Subscribed to topic.\n");
}

/*
    handles the unsubscription process by sending a packet with the type 'u'/(unsubscribe) 
        and the topic to be unsubscribed from to the server using the TCP socket
*/
void handle_unsubscribe(char *token, int tcp_sock, Packet *pack) {
    pack->type = 'u';
    token = strtok(NULL, " ");
    strcpy(pack->topic, token);
    token = strtok(NULL, " ");
    pack->data_type = token[0];

    int sending_unsubscribe = send(tcp_sock, pack, PACKLEN, 0);
    DIE(sending_unsubscribe < 0, "send");
    printf("Unsubscribed to topic.\n");
}

/*
    receives messages from the server, process them, and prints the received information 
        to output. Returns -1 if the server closes the connection, and 0 otherwise
*/
int handle_tcp_socket(int tcp_sock) {

    char buffer[sizeof(struct tcp_struct)];
    memset(buffer, 0, sizeof(struct tcp_struct));

    uint32_t res = recv(tcp_sock, buffer, sizeof(struct tcp_struct), 0);
    
    if (res == 0) {
        return -1;
    } else {
		DIE(res < 0, "receive");
	}
	
    struct tcp_struct *pack_send = (struct tcp_struct *)buffer;
	char* ip = pack_send->ip;
	uint16_t port = pack_send->port;
	char* topic = pack_send->topic;
	char* type = pack_send->type;
	char* content = pack_send->content;

    printf("%s:%u - %s - %s - %s\n", ip, port, topic, type, content);

    return 0;
}

int main(int argc, char** argv) {

	set_non_buffered_stdout();
    DIE(argc < 4, "arguments");

    // creates a TCP socket
    int tcp_sock = create_tcp_socket();

    struct sockaddr_in server_data;
    setup_server_data(&server_data, argv[2], argv[3]);

    // init the file descriptor
    fd_set file_descr;
    FD_ZERO(&file_descr);
    FD_SET(tcp_sock, &file_descr);
    FD_SET(STDIN_FILENO, &file_descr);

    connect_to_server(tcp_sock, &server_data);
    send_client_id(tcp_sock, argv[1]);

	struct Packet pack;
    setsockopt(tcp_sock, IPPROTO_TCP, TCP_NODELAY, (char *)1, sizeof(int));
    
	while(1) {
		
		fd_set aux_set = file_descr;

    	select(tcp_sock + 1, &aux_set, NULL, NULL, NULL);
    	DIE(select(tcp_sock + 1, &aux_set, NULL, NULL, NULL) < 0, "select");

		int check_fd = FD_ISSET(STDIN_FILENO, &aux_set);
		if (check_fd) {
			char buffer[BUFFER_SIZE];
			memset(buffer, 0, BUFFER_SIZE);
			fgets(buffer, BUFFER_SIZE, stdin);
			memset(&pack, 0, PACKLEN);

			char *token = strtok(buffer, " ");
			switch (token[0]) {
				case EXIT: // exit
					handle_exit(tcp_sock, &pack);
					break;

				case SUBSCRIBE: // subscribe
					handle_subscribe(token, tcp_sock, &pack);
					break;

				case UNSUBSCRIBE: // unsubscribe
					handle_unsubscribe(token, tcp_sock, &pack);
					break;

				default:
					printf("Invalid cmd.\n");
					break;
			}
    	}
		// server message
		if (FD_ISSET(tcp_sock, &aux_set)) {
        	int status = handle_tcp_socket(tcp_sock);
			if (status == -1) {
				break;
			}
    	}
	}

	close(tcp_sock);
	return 0;
}

