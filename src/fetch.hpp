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

#define uint_128 unsigned long long int
#define BUFFER_SIZE 8192
static int optval = 1;

/*
	file contents fetcher from another node via tcp due to spoofin vulnurabilities in udp based connections
	caches the read files for later use when the node requests it, the cache expiers after a certain
	amount of time in order in increase performance
*/

//returns file content's, need's to be deallocated manually.
//Memory hogging piece of shit
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
	n_src_cli(std::string source, unsigned short port) : port(port)
	{
		this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);

		this->server.sin_family = AF_INET;
		this->server.sin_port = htons(port);
		this->server.sin_addr.s_addr = inet_addr(source.c_str());

		if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int)) < 0) 
			throw std::runtime_error("unable to reuse db port\n");
	
		if (connect(this->socket_fd, (struct sockaddr*)&this->server, sizeof(this->server)) < 0)
			throw std::runtime_error("unable to connect to desired source\n");
	
		this->cache = NULL;
	}

	//seamless request source documents from loadbalancer 
	char *get(std::string filename)
	{
		if (this->cache_content == filename)
			return this->cache;

		this->cache_content = filename;
		if (send(this->socket_fd, (const void *)filename.c_str(), filename.size(), 0) < 0) {
			if (reconnect(10) < 0)
				return NULL;
		}else
			return NULL;

		char *tmp = new char[BUFFER_SIZE];
		int recv_amt = recv(socket_fd, tmp, BUFFER_SIZE-1, 0);
		if (recv_amt < 0) 
		{
			delete[] tmp;
			return NULL;
		}
		tmp[recv_amt] = '\0';
		this->cache_size = strlen(tmp);
		this->cache = new char[strlen(tmp) + 1];
		std::memcpy(&this->cache, tmp, this->cache_size);
		delete[] tmp;

		return this->cache;
	}

	~n_src_cli()
	{
		std::cout << "cli destructor called\n";
		delete[] this->cache; 
		close(this->socket_fd);
	}

private:
	unsigned long addr;
	unsigned short port;

	int socket_fd = -1;
	struct sockaddr_in server;

	struct timeval tv = {
	 .tv_sec = 2 
	};
	
	//chaching using c style string's for ease of use, conveniance and slight performance gains 
	char *cache;
	uint_128 cache_size;
	std::string cache_content;

	bool thread_running = true;
	std::thread monitor;
	void monitor_cache()
	{
		while (this->thread_running)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
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
		std::cout << "Constructor activated\n";
		this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);

		this->server.sin_family = AF_INET;
		this->server.sin_port = htons(port);

		//reuse port
		//setsockopt(this->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &this->tv, sizeof(this->tv));
		setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
		
		bind(this->socket_fd, (struct sockaddr*)&this->server, sizeof(this->server));
		listen(this->socket_fd, 1024);

		this->n_accept = std::thread(&n_src_host::n_master_accept, this);
		this->n_recv = std::thread(&n_src_host::n_master_recv, this);

	}

	void moo()
	{
		std::cout << "Moo\n";
	}
	//close concurrent threads
	~n_src_host()
	{
		std::cout << "destructor activated\n";
		this->mutexpool.lock();
		this->thread_running = false;
		this->mutexpool.unlock();
	
		if (this->n_accept.joinable())
			this->n_accept.join();
		
		if (this->n_recv.joinable())
			this->n_recv.join();
	}

private:
	int socket_fd;
	struct sockaddr_in server;
	std::vector<int> nodes;
	
	bool thread_running = true;
	std::mutex mutexpool;
	std::thread n_accept;
	std::thread n_recv;

	void n_master_accept()
	{
		std::cout << "starting thread acpt\n";
		int sock_buffer;
		while (this->thread_running)
		{
			sock_buffer = accept(this->socket_fd, (struct sockaddr*)&this->server, (socklen_t*)sizeof(this->server));
			std::cout << "client connected!\n";
			this->nodes.push_back(sock_buffer);
		}
	}

	//listen for packets from nodes
	void n_master_recv()
	{
		std::cout << "starting thread recv\n";
		
		// find the maximum socket descriptor value
		int maxSocket = 0;
		if (!this->nodes.empty())
			maxSocket = *std::max_element(this->nodes.begin(), this->nodes.end());
	
		// create and initialize the file descriptor set
		fd_set readSet;
		FD_ZERO(&readSet);

		// add all sockets to the file descriptor set
		for (int i = 0; i<this->nodes.size(); i++)
		{
			FD_SET(this->nodes[i], &readSet);
		}
		
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;


		this->mutexpool.lock();
		while (this->thread_running)
		{
			this->mutexpool.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(200)); //sleep så he it slösar cpu cycles
			// make a copy of the file descriptor set, as `select` modifies it
			fd_set tempSet = readSet;

			// call select to check for readable sockets
			this->mutexpool.lock();

			// piece of shit, overcomplicated blocks for absolutely no goddamn reason,
			// tempted to just use threading paired with recvtimeout
			int activity = select(maxSocket + 1, &tempSet, nullptr, nullptr, &timeout);
				std::cout << "Hello world\n";
			if (activity == -1)
			{
				throw std::runtime_error("something broke");
				break;
			}
			this->mutexpool.unlock();

			//no activity within the specified timeout period
			if (activity == 0)
				continue;

			//clunky but gotta get this shit done like yesterday
			for (int i = 0; i<this->nodes.size(); i++)
			{
				if (FD_ISSET(this->nodes[i], &tempSet))
				{
					char buffer[512];
					std::memset(buffer, 0, sizeof(buffer));

					int bytesRead = recv(this->nodes[i], buffer, sizeof(buffer) - 1, 0);
					if (bytesRead == -1)
					{
						throw std::runtime_error("recv error");
					}
					else //something was recv'd
					{
						char *buf = fetch(buffer);
						std::cout << "responding: " << buffer << " on socket: " << this->nodes[i] << "\n";
						send(this->nodes[i], buf, strlen(buf)+1, 0);

						delete[] buf;
					}
				}
			}
			this->mutexpool.lock();
		}
	}
};
