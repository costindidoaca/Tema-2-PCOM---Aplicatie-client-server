#ifndef _HELPERS_H
#define _HELPERS_H 1

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)
	
// macros for easier read	
#define PACKLEN sizeof(struct Packet)	
#define IP_SIZE 16	
#define ID_SIZE 10
#define TOPIC_SIZE_TCP 51
#define TOPIC_SIZE_UDP 50
#define INT "INT"
#define SHORT_REAL "SHORT_REAL"
#define FLOAT "FLOAT"
#define STRING "STRING"
#define SUBSCRIBE 's'
#define UNSUBSCRIBE 'u'
#define EXIT 'e'
#define BUFFER_SIZE 100


// enum for case handler
typedef enum {
    CASE_TCP_CONNECTIONS,
    CASE_UDP_MESSAGES,
    CASE_STDIN_INPUT,
    CASE_RECEIVED_MESSAGE,
    CASE_NONE
} CaseHandler;

/*
    case handler for handling connections and messages to
        determine which case should be executed based on the file descriptor 
*/
CaseHandler determine_case(int i, int tcp_sock, int udp_sock) {
    if (i == tcp_sock) {
        return CASE_TCP_CONNECTIONS;
    } else if (i == udp_sock) {
        return CASE_UDP_MESSAGES;
    } else if (i == STDIN_FILENO) {
        return CASE_STDIN_INPUT;
    } else {
        return CASE_RECEIVED_MESSAGE;
    }
}

// struct for Packet
typedef struct Packet {
	char type;  // exit = e, subscribe = s, unsubscribe = u
	char topic[51];
	char content[1501];
	char data_type;
	char ip[IP_SIZE];
	uint16_t port;
	int sf;
} Packet;

typedef struct tcp_struct {
	char ip[IP_SIZE];
	uint16_t port;
	char type[11];
	char topic[TOPIC_SIZE_TCP];
	char content[1501];
} msg_tcp;

typedef struct udp_struct {
	char topic[TOPIC_SIZE_UDP];
	uint8_t type;
	char content[1501];
} msg_udp;

typedef struct topic{
	char nume[51];
	int sf;
} topic;

typedef struct client{
	uint32_t online; // 1 on / 0 off
	char id[ID_SIZE];
	long socket;
	int dim_topics;
	int dim_unsent;
	msg_tcp unsent[BUFFER_SIZE];
	struct topic topics[BUFFER_SIZE];
	
} client;

// util for finding max between 2 ints
int max(int a, int b) {
    return a > b ? a : b;
}


#endif












