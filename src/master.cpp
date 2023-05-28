#include <iostream>
#include <random>
#include <vector>
#include <cstring>
#include <string>
#include <map>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "networking.hpp"

#define uint_64 unsigned long

static int optval = 1;

struct Node{
	int socket_fd;
	int activeResources;
};

struct Client {
	uint_64 auth_token;
	std::string message;
};


void traffic_routing(std::vector <struct Node> &socket)
{
	char buffer[BUFFER_SIZE];

    while (true) {
        fd_set readSet;
        int maxDescriptor = -1;
        FD_ZERO(&readSet);

        // Add client sockets to the set
        for (int i = 0; i<socket.size(); i++) {
            FD_SET(socket[i].socket_fd, &readSet);
            maxDescriptor = std::max(maxDescriptor, socket[i].socket_fd);
        }

        // Use select to monitor the sockets for activity
        int activity = select(maxDescriptor + 1, &readSet, nullptr, nullptr, nullptr);
        if (activity == -1) {
            std::cerr << "Error in select" << std::endl;
            break;
        }

        // Check for activity on client sockets
        for (auto it = sockets.begin(); it != sockets.end(); ++it) {
            int clientSocket = *it;
            if (FD_ISSET(clientSocket, &readSet)) {
                // Receive data from the client socket
                int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead == -1) {
                    std::cerr << "Error in recv" << std::endl;
                    // Handle error or close the client socket
                    // ...
                } else if (bytesRead == 0) {
                    std::cout << "Client disconnected" << std::endl;
                    // Close the client socket and remove it from the vector    char buffer[BUFFER_SIZE];

    while (true) {
        fd_set readSet;
        int maxDescriptor = -1;
        FD_ZERO(&readSet);

        // Add client sockets to the set
        for (const auto& clientSocket : clientSockets) {
            FD_SET(clientSocket, &readSet);
            maxDescriptor = std::max(maxDescriptor, clientSocket);
        }

        // Use select to monitor the sockets for activity
        int activity = select(maxDescriptor + 1, &readSet, nullptr, nullptr, nullptr);
        if (activity == -1) {
            std::cerr << "Error in select" << std::endl;
            break;
        }

        // Check for activity on client sockets
        for (auto it = clientSockets.begin(); it != clientSockets.end(); ++it) {
            int clientSocket = *it;
            if (FD_ISSET(clientSocket, &readSet)) {
                // Receive data from the client socket
                int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytesRead == -1) {
                    std::cerr << "Error in recv" << std::endl;
                    // Handle error or close the client socket
                } else if (bytesRead == 0) {
                    std::cout << "Client disconnected" << std::endl;
                    // Close the client socket and remove it from the vector
                    close(clientSocket);
                    clientSockets.erase(it);
                    --it;
                } else {
                    // Process the received data
                }
            }
        }
    }
                    close(clientSocket);
                    clientSockets.erase(it);
                    --it;
                } else {
                    // Process the received data
                    // ...
                }
            }
        }
    }
}

//this is fucking stupid, but it still works somehow...
int node_selection(std::vector<Node> *nodes) {
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
	return n_buffer->socket_fd;
}

int main() {
	//token generation: dist(rng) -> unsigned long
	std::mt19937_64 rng(std::random_device{}());
	std::uniform_int_distribution<uint_64> dist(0, std::numeric_limits<uint_64>::max());

	/*
		key = token, value = file descriptor
		when a node responds to the load balancer, it routes the packet to the client via 
		a token generation system
	*/
	std::map<uint_64, int> clients;

	int active_nodes = 0;
	std::vector<Node> nodes;	
	

	int addrlen = sizeof(sockaddr_in) * sizeof(char);
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	//this shit is supposed to make it so you can reuse the port, but wont work for some goddamn reason
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int)) < 0 ) {
		throw std::runtime_error("Couldn't reuse port, for whatever reason");
	}

	//recv timeout setting's shouldn't interfere with the above statement
	struct timeval tv;
	tv.tv_sec = recv_timeout;
	tv.tv_usec = 0;
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(12354);

	//256 people can queue everyone else fucks off
	if (bind(socket_fd, (struct sockaddr*)&server, addrlen) < 0 || listen(socket_fd, 256) < 0) {
		throw std::runtime_error("reconnect your ethernet cable dipshit");
	}

	char *buf = new char[BUFFER_SIZE];
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
			std::cout << "Node connected\n";
		} else {
			clients[dist(rng)] = socket_buffer;
			for (int i = 0; i<nodes.size(); i++)
			{
				nodes[i].activeResources = n_status_check(nodes[i].socket_fd);
			}
			int tmp = node_selection(&nodes);
			send(tmp, buf, strlen(buf)+1, 0);

		}
	}
	delete[] (buf);
	return 0;
}
