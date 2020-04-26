#ifndef SERVER_HELPER_H_
#define SERVER_HELPER_H_

#include "./other_helpers.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>

#define MAX_CLIENTS 100

// for storing topics that clients are subscribed to
struct topic {
    std::string name;
    int subscribe_type;
    std::vector<std::string> stored_messages;
};

// for storing connected clients
struct tcp_client {
    int socketfd;
    std::unordered_map<std::string, struct topic> topics;
    std::string id;
    bool active;
};

bool resolve_read_stdin(fd_set& read_fds, int fdmax);
void resolve_tcp_connection(int sockfd, fd_set &read_fds, int& fdmax,
    std::unordered_map<std::string, struct tcp_client>& tcp_clients, std::unordered_map<int, std::string>& fd_to_id);
void resolve_udp_interaction(int sockfd, int con_tcp_sockfd, struct sockaddr_in& serv_addr,
    std::unordered_map<std::string, struct tcp_client>& tcp_clients);
void resolve_tcp_interaction(int sockfd, fd_set& read_fds,
    std::unordered_map<std::string, struct tcp_client>& tcp_clients, std::unordered_map<int, std::string>& fd_to_id);
void close_server(fd_set &read_fds, int fdmax);

#endif
