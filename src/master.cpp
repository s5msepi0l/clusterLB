#include <iostream>
#include <random>
#include <sys/socket.h>

#include "networking.hpp"

#define uint_64 unsigned long

/*
	TODO:
		multithreading för client response
		token routing
*/

struct Node{
	int socket_fd;
	int active_connections;
};

struct Client {
	int socket_fd;
	uint_64 auth_token;
	std::string message;
};

int main() {
	//token generation: dist(rng) -> unsigned long
	std::mt19937_64 rng(std::random_device{}());
	std::uniform_int_distribution<uint_64> dist(0, std::numeric_limits<uint_64>::max());

	int active_nodes = 0;
	std::vector<Node> nodes;
	std::vector<Client> clients;
	
	char buf[BUFFER_SIZE];

	int addrlen = sizeof(sockaddr_in) * sizeof(char);
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server;

	server.sin_family = AF_INET;
	server.sin_port = htons(4444);
	bind(socket_fd, (struct sockaddr*)&server, addrlen);
	listen(socket_fd, 256); //2**8 får lysten, all ader får fuck off

	bool running = true;
	int socket_buffer;
	while (1) {
		socket_buffer = accept(socket_fd, (struct sockaddr*)&server, (socklen_t*)&addrlen);
		recv(socket_buffer, buf, BUFFER_SIZE, 0);
		if (strcmp(buf, global_response) == 0) {
			nodes[active_nodes++] = {socket_buffer, 0};
		}
	}

	return 0;
}