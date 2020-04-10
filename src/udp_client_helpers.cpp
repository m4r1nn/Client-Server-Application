#include "./include/upd_client_helpers.h"

// for sending news
void resolve_read_stdin(int sockfd, fd_set& read_fds, struct sockaddr_in& serv_addr) {

    // read news from stdin
    char buffer[BUFFLEN];
    memset(buffer, 0, BUFFLEN);
    char* read_check = fgets(buffer, BUFFLEN - 1, stdin);
    DIE(read_check == NULL, "read");

    // close client
    if (strncmp(buffer, "exit", 4) == 0) {
        close_client(sockfd, read_fds);
    }

    // send news to server
    int send_check = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*) &serv_addr, sizeof(struct sockaddr));
    DIE(send_check < 0, "sendto");

    std::cout << "message sent" << std::endl;
}

// for closing the client
void close_client(int sockfd, fd_set& read_fds) {
    close(sockfd);
    close(STDIN_FILENO);
    FD_CLR(sockfd, &read_fds);
    FD_CLR(STDIN_FILENO, &read_fds);
}
