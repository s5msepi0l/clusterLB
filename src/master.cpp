#include <iostream>
#include <random>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <cstring>
#include <string>
#include "networking.hpp"

#define uint_64 unsigned long

//....
#define status 0x100
#define handle 0x200

static int optval = 1;
/*
	TODO:
		multithreading för client response
		token routing
*/
struct Node{
	int socket_fd;
	int used_resources;
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
	
	char *buf = new char[BUFFER_SIZE];

	int addrlen = sizeof(sockaddr_in) * sizeof(char);
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	//this shit is supposed to make it so you can reuse the port, but wont work for some goddamn reason
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int)) < 0 ) {
		throw std::runtime_error("Couldn't reuse port, for whatever reason");
	}
	struct sockaddr_in server;

	server.sin_family = AF_INET;
	server.sin_port = htons(12354);

	//256 people can queue everyone else fucks off
	if (bind(socket_fd, (struct sockaddr*)&server, addrlen) < 0 || listen(socket_fd, 256) < 0) {
		throw std::runtime_error("reconnect your ethernet cable dipshit");
	}

	bool running = true;
	int socket_buffer;

	while (running) {
		socket_buffer = accept(socket_fd, (struct sockaddr*)&server, (socklen_t*)&addrlen);
		recv(socket_buffer, buf, BUFFER_SIZE, 0);

		//evil ancient c function, look but dont touch
		//Node connection instance
		if (strcmp((const char *)buf, global_response.c_str()) == 0) {
			nodes.push_back({socket_buffer, 0});
			active_nodes++;
			std::cout << "[*]Node connection received\n";
		}
	}
	delete[] (buf);
	return 0;
}

//this is fucking stupid, but it still works somehow...
Node *node_selection(std::vector<Node> *nodes) {
	int buffer, max = 0;
	Node *n_buffer;
	for (int i = 0; i<nodes->size(); i++) {
		send((*nodes)[i].socket_fd, (const void *)status, sizeof(int), 0);
		recv((*nodes)[i].socket_fd, &buffer, sizeof(int), 0);

		if (buffer > max) {
			max = buffer;
			n_buffer = &(*nodes)[i];
		}
	}	
	return n_buffer;
}
