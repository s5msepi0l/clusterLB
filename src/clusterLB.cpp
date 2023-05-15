#include <iostream>
#include <vector>
#include <string>
#include <cstring>

#include "networking.hpp"
#include "ThreadPool.hpp"

struct HTTP_headers{
	std::string request_type;
	std::string destination;
	//html cookies å han typed av skit
};

int main(int argc, char **argv) {
	if (argc < 4) {
		throw std::runtime_error("No args included(address port thread_cout)\n");
		return -1;
	}

	//cmpiler probably was probably gonna do the same shit
	//this whole section is basically useless but impoves readability slightly 
	static std::string addr(argv[1]);
	unsigned short port = atoi(argv[2]);
	unsigned short mem = atoi(argv[3]);
	unsigned short thread_count = atoi(argv[4]);

	//github codespaces doesn't like this class for some reason
	ThreadPool thpool(thread_count);

	struct sockaddr_in server;
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	int addrlen = (sizeof(server) * sizeof(char));

	server.sin_family = AF_INET;
	server.sin_port = htons(4);
	server.sin_addr.s_addr = inet_addr(addr.c_str());

	if (connect(socket_fd, (struct sockaddr*)&server, addrlen) < 0) {
		throw std::runtime_error("[*]Unable to connect to load balancer");
		return -1;
	}

	bool running = true;
	char *buf = new char[BUFFER_SIZE];
	while (running) {
		accept(socket_fd, (struct sockaddr*)&server, (socklen_t*)&addrlen);
	}
	delete[] buf;

	return 0;
}