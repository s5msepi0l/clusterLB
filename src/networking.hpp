#ifndef LEAN_AND_MEAN_NETWORKING
#define LEAN_AND_MEAN_NETWORKING

#include <iostream>
#include <cstring>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096
#define global_response "wasssssaaaaaaaa"

struct n_params{
	int socket_fd;
	std::string html_data;
};

void handle_request(struct n_params param) { //main entry point for threads
	std::cout << "handle_request\n";
}

unsigned long n_status_check(int socket_fd) {
	char buf[BUFFER_SIZE];
	
	send(socket_fd, global_response, strlen(global_response), 0);	
	if (recv(socket_fd, buf, BUFFER_SIZE, 0) != 0 && strcmp(global_response, buf) == 0);
		return atoi(buf);

	return -1;
}

/*void n_reconnect(int socket_fd) {

}*/

#endif