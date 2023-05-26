#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <chrono>
#include <sys/select.h>
#include <sys/time.h>
#include <algorithm>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <vector>
#include <mutex>
#include <thread>

#include "cache.hpp"

#define uint_128 unsigned long long int
#define BUFFER_SIZE 8192

static int optval = 1;
std::mutex mutexpool;
/*
	file contents fetcher from another node via tcp due to spoofin vulnurabilities in udp based connections
	caches the read files for later use when the node requests it, the cache expiers after a certain
	amount of time in order in increase performance
*/

//returns file content's, need's to be deallocated manually also is a 
//memory hogging piece of shit
char *fetch(char *path)
{
	std::ifstream file(path, std::ios::in|std::ios::binary|std::ios::ate);
	std::string buf, tmp;
	char *membuf = nullptr;
	if (file.is_open())
	{
		file.seekg(0, std::ios::end);
		uint_128 size = (file.tellg() * sizeof(char));
		file.seekg(0, std::ios::beg);

		while(file)
		{
			std::getline(file, tmp);
			buf.append(tmp);
		}
		file.close();

		char *membuf = new char[size + 1];
		std::memcpy(membuf, buf.c_str(), size);
		membuf[size] = '\0';
		
		return membuf;
	}

	return NULL;
}

class n_src_cli
{
public:
	n_src_cli(std::string source, unsigned short port) : addr(source), port(port), cache(20)
	{
		sock_init(source, port);
		
	}

	//seamless request source documents from loadbalancer 
	char *get(std::string filename)
	{
		char *mem = this->cache.get((char*)filename.c_str());

		if (mem != nullptr)
			return mem;
		
		//this->cache_content = filename;
		
		std::cout << "connect: " << connect(this->socket_fd, (struct sockaddr*)&this->server, (socklen_t)sizeof(this->server)) << "\n";
			//throw std::runtime_error("unable to connect to desired source\n");
	
		if (send(this->socket_fd, (const void *)filename.c_str(), filename.size(), 0) < 0) {
			throw std::runtime_error("you're fucked\n");
		}

		char *tmp = new char[BUFFER_SIZE];
		int recv_amt = recv(this->socket_fd, tmp, BUFFER_SIZE-1, 0);
		if (recv_amt < 0) 
		{
			delete[] tmp;
			return NULL;
		}
		tmp[recv_amt] = '\0';
		
		this->cache.insert_h(tmp, (char*)filename.c_str());

		delete[] tmp;

		close(this->socket_fd);
		sock_init(this->addr, this->port);
		
		return this->cache.get((char*)filename.c_str());
	}

	void exit()
	{
		std::cout << "Destructor\n";

		this->cache.~Cache();
	}

private:
	std::string addr;
	unsigned short port;

	int socket_fd;
	struct sockaddr_in server;

	Cache cache;

	struct timeval tv = {
	 .tv_sec = 2 
	};

	void sock_init(std::string source, int port)
	{
		this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);

		this->server.sin_family = AF_INET;
		this->server.sin_port = htons(port);
		this->server.sin_addr.s_addr = inet_addr(source.c_str());

		if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &optval, sizeof(int)) < 0) 
			throw std::runtime_error("unable to reuse db port\n");
	}

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
		this->server.sin_addr.s_addr =INADDR_ANY;

		//reuse port
		//setsockopt(this->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &this->tv, sizeof(this->tv));
		setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
		
		std::cout << bind(this->socket_fd, (struct sockaddr*)&this->server, (socklen_t)sizeof(this->server)) << "\n";
		listen(this->socket_fd, 256);

		this->n_accept = std::thread(&n_src_host::n_master_accept, this);
	}

	//close concurrent threads
	void exit()
	{
		mutexpool.lock();
		this->thread_running = false;
		mutexpool.unlock();
	
		if (this->n_accept.joinable())
			this->n_accept.join();
	}

private:
	int socket_fd;
	struct sockaddr_in server;
	
	bool thread_running = true;
	std::thread n_accept;

	void *n_master_accept()
	{
		std::cout << "starting thread acpt\n";
		//mainly just for temporarily holding the file path
		char *tmp = new char[512];
		char *buf;
		int sock_buffer;
		while (this->thread_running)
		{
			int addrlen = sizeof(this->server);
			sock_buffer = accept(this->socket_fd, (struct sockaddr*)&this->server, (socklen_t*)&addrlen);
			if (recv(sock_buffer, (void *)tmp, 512 -1, 0) < 0) {
				delete[] tmp;
				throw std::runtime_error("unable to recv packet\n");
			}

			buf = fetch(tmp);
			send(sock_buffer, buf, strlen(buf), 0);
			delete[] buf;
		}

		return nullptr;
	}
};
