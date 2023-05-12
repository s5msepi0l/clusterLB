#include <iostream>
#include <vector>
#include <string>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "networking.hpp"
#include "ThreadPool.hpp"

struct HTTP_headers{
	std::string request_type;
	std::string destination;
	//html cookies å han typed av skit
};

class clusterLB{
private:
	int socket_fd;
	struct sockaddr_in server;

	const char *addr;
	int addrlen;

	ThreadPool thpool;
	std::vector<Node>nodes;
public:
	clusterLB(std::string host, unsigned int port, int thread_count) : thpool(thread_count){ 	//constructor main shitin ska fa jär
		this->socket_fd = socket(AF_INET, SOCK_STREAM, 0); // tcp stream
		this->addr = host.c_str();

		this->server.sin_family = AF_INET;
		this->server.sin_port = htons(port);
		this->server.sin_addr.s_addr = inet_addr(this->addr);
	}

	int reconnect() { // -1 = koppel in ethernet kapeln igen
		if (connect(this->socket_fd, (struct sockaddr*)&this->server, this->addrlen) < 0) {
			throw std::runtime_error("Unable to connect/reconnect\n");
			return -1;
		}
		return 0;
	}

	unsigned int execute(struct n_params param)
	{
		return this->thpool.execute(param);
	}

	~clusterLB() {
		//pool.~ThreadPool();
	}
};

int main(int argc, char **argv) {
	clusterLB server(argv[1], 4444, 8);

	
	return 0;
}