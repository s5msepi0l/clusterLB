#include <iostream>
#include <vector>
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
private:
	ThreadPool thpool;
	std::vector<Node>nodes;
public:
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

int main() {
	struct n_params par;
	par.html_data = "Hallo";
	par.socket_fd = 5;

	clusterLB server(8);
	std::cout << server.execute(par);

	
	return 0;
}