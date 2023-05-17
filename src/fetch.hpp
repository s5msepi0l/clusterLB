#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <vector>
#include <mutex>
#include <thread>

#define uint_128 unsigned long long int
#define BUFFER_SIZE 8192
static int optval = 1;

/*
	file contents fetcher from another node via tcp due to spoofin vulnurabilities in udp based connections
	caches the read files for later use when the node requests it, the cache expiers after a certain
	amount of time in order in increase performance
*/

//returns file content's, need's to be deallocated
char *fetch(std::string path)
{
	std::ifstream file(path, std::ios::in|std::ios::binary|std::ios::ate);
	if (file.is_open())
	{
		file.seekg(0, std::ios::end);
		uint_128 size = (file.tellg() * sizeof(char));
		char *membuf = new char[]
	}

	return membuf;
}

class n_src_cli
{
public:
	n_src_cli(std::string source, unsigned short port, uint_128 s_expiration) : addr(inet_addr(source.c_str())), port(port), s_expiration(s_expiration)
	{
		this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);

		this->server.sin_family = AF_INET;
		this->server.sin_port = htons(port);
		this->server.sin_addr.s_addr = addr;

		if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int)) < 0) 
			throw std::runtime_error("unable to reuse db port\n");
		
		if (setsockopt(this->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &this->tv, sizeof(this->tv)) < 0)
			throw std::runtime_error("unable to setup socket timeout\n");
	
		if (connect(this->socket_fd, (struct sockaddr*)&this->server, sizeof(this->server)) < 0)
			throw std::runtime_error("unable to connect to desired source\n");
	}

	std::string get(std::string filename)
	{
		if (this->cache_content == filename && this->cache == NULL)
			return this->cache;

		this->cache_content = filename;
		if (send(this->socket_fd, (const void *)filename.c_str(), filename.size(), 0) < 0) {
			if (reconnect(10) < 0)
				return nullptr;
		}

		char *tmp = new char[BUFFER_SIZE];
		recv(socket_fd, tmp, BUFFER_SIZE-1, 0);

		strcpy(this->cache, tmp);
		this->cache_size = strlen(this->cache);
		delete[] tmp;

		return this->cache;
	}

	~n_src_cli()
	{
		delete[] this->cache;
		close(this->socket_fd);
	}

private:
	unsigned long addr;
	unsigned short port;
	unsigned long s_expiration;

	int socket_fd = -1;
	struct sockaddr_in server;

	struct timeval tv = {
	 .tv_sec = 120 
	};
	
	//chaching using c style string's for ease of use, conveniance and slight performance gains 
	char *cache;
	uint_128 cache_size;
	std::string cache_content;

	int reconnect(int max_attempts)
	{
		for (int i = 0; i<max_attempts; i++)
		{
			if (connect(this->socket_fd, (struct sockaddr*)&this->server, sizeof(server)) < 0)
				return 0;
		}
		return -1;
	}
};

/*
	server side hosting for nodes. While one thread purpetually accept's connections from new nodes.
	The other thread listen's on the vector list socket's for 1-2 seconds each then moves on to the next one and so fourth
*/
class n_src_host
{
public:
	n_src_host(int port)
	{
		this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);

		this->server.sin_family = AF_INET;
		this->server.sin_port = htons(port);

		setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
		setsockopt(this->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &this->tv, sizeof(this->tv));

		bind(this->socket_fd, (struct sockaddr*)&this->server, sizeof(this->server));
		listen(this->socket_fd, 1024);
	}

private:
	int socket_fd;
	struct sockaddr_in server;
	std::vector<int> nodes;

	struct timeval tv = {
		.tv_sec = 120 
	};
	
	bool thread_running = true;
	std::mutex mutexpool;
	std::thread n_accept;
	std::thread n_recv;

	void n_master_accept()
	{
		int sock_buffer;
		while (this->thread_running)
		{
			sock_buffer = accept(this->socket_fd, (struct sockaddr*)&this->server, (socklen_t*)sizeof(server));
			
			this->mutexpool.lock();
			this->nodes.push_back(sock_buffer);
			this->mutexpool.unlock();
		}
	}

	void n_master_recv()
	{

	}
};
