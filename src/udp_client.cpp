#include "./include/udp_client_helpers.h"

int main(int argc, char* argv[]) {
    
    DIE(argc < 3, "wring call client");

    // descriptors 'arrays'
    fd_set read_fds;
    fd_set temp_fds;

    // initialize descriptors 'arrays'
    FD_ZERO(&read_fds);
    FD_ZERO(&temp_fds);

    // get descriptor for server interaction
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(sockfd < 0, "socket");

    int port = atoi(argv[2]);
    DIE(port <= 0, "invalid port");

    // set socket for server interaction
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    int addr_check = inet_aton(argv[1], &serv_addr.sin_addr);
    DIE(addr_check == 0, "invalid ip");

    // add main descriptors to 'array'
    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd, &read_fds);

    // run client
    while (true) {
        temp_fds = read_fds;
        
        // see what descriptors are ready for interactions
        int select_check = select(sockfd + 1, &temp_fds, NULL, NULL, NULL);
        DIE(select_check < 0, "select");

        // read from stdin
        if (FD_ISSET(STDIN_FILENO, &temp_fds)) {
            resolve_read_stdin(sockfd, read_fds, serv_addr);
        }
    }

    close_client(sockfd, read_fds);

    return 0;
}
