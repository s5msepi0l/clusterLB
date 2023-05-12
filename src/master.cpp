#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "networking.hpp"

//load balancer (ja han eina gräjin dehär projecti int är)
/* TODO:
    -Add så he acceptar client connection
    -Add så he håller active connections å mängden threads available
*/
std::vector<Node> nodes;

int main(int argc, char **argv) {
    bool running = true;

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(4444);

    //infinite loop tär 90% av allt händer
    

    return 0;
}