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

struct HTTP_headers html_parser(std::string input);

class clusterLB{
public:
	int socket_fd;
	struct sockaddr_in server;
	ThreadPool thpool;
	
	clusterLB(int thread_count) : thpool(thread_count){ 
		;;
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
	/*
	if (argc < 3) {
		throw std::runtime_error("No args included(address port thread_cout)\n");
		return -1;
	} */

	clusterLB cluster(8);

	cluster.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	cluster.server.sin_family = AF_INET;
	cluster.server.sin_port = htons(4);
	cluster.server.sin_addr.s_addr = inet_addr(127.0.0.1);
	return 0;
}