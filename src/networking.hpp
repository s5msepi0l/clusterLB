#ifndef LEAN_AND_MEAN_NETWORKING
#define LEAN_AND_MEAN_NETWORKING

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>

struct Node{
	int socket_fd;
	int active_connections;
};

struct n_params{
	int socket_fd;
	std::string html_data;
};

//ska data ska parsas
void handle_request(struct n_params param) { //main entry point for threads

}

#endif