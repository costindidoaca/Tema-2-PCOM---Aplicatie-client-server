#ifndef _SERVER_HELP_FUNC_H
#define _SERVER_HELP_FUNC_H

void sendTcpMessageToClient(client *client, struct tcp_struct *send_to_tcp);
void storeUnsentMessage(client *client, struct tcp_struct *send_to_tcp);
int isMatchingTopic(client *client, int topicIndex, const char *targetTopic);
void processTopic(client *client, struct tcp_struct *send_to_tcp);
void processClients(client *clients, int max_sock, struct tcp_struct send_to_tcp);
client* find_client_by_socket(client *clients, int max, int socket);
client* find_client_by_id(client *clients, int max, const char *id);
void set_socket_options(int tcp_sock);
int create_udp_socket();
int create_tcp_socket();
int bind_socket(int sock, struct sockaddr_in addr);

#endif