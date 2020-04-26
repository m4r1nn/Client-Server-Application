#ifndef UDP_CLIENT_HELPERS_H_
#define UDP_CLIENT_HELPERS_H_

#include "./other_helpers.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void resolve_read_stdin(int sockfd, fd_set& read_fds, struct sockaddr_in& serv_addr);
void close_client(int sockfd, fd_set& read_fds);

#endif
