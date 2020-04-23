#include "./include/tcp_client_helpers.h"

int main(int argc, char* argv[]) {
    
    DIE(argc < 4, "wrong call client");

    // descriptors 'arrays'
    fd_set read_fds;
    fd_set temp_fds;

    // initialize descriptors 'arrays'
    FD_ZERO(&read_fds);
    FD_ZERO(&temp_fds);

    // get descriptor for server connection
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");

    int port = atoi(argv[3]);
    DIE(port <= 0, "invalid port");

    // set socket for server interaction
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    int addr_check = inet_aton(argv[2], &serv_addr.sin_addr);
    DIE(addr_check == 0, "invalid ip");

    // add main descriptors to 'array'
    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd, &read_fds);

    // connect to server
    int connect_check = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(struct sockaddr));
    DIE(connect_check < 0, "conect");

    int id = atoi(argv[1]);
    DIE(id < 0, "atoi");

    // send id to server
    char buffer[BUFFLEN];
    memset(buffer, 0, BUFFLEN);
    memcpy(buffer, argv[1], strlen(argv[1]));    
    int send_check = send(sockfd, buffer, strlen(buffer), 0);
    DIE(send_check < 0, "send"); 

    // run client
    while (true) {
        temp_fds = read_fds;

        // see what descriptors are ready for interactions
        int select_check = select(sockfd + 1, &temp_fds, NULL, NULL, NULL);
        DIE(select_check < 0, "select");

        // read from stdin
        if (FD_ISSET(STDIN_FILENO, &temp_fds)) {
            resolve_read_stdin(sockfd, read_fds);

        // send message to server
        } else if (FD_ISSET(sockfd, &temp_fds)) {
            resolve_server_interaction(sockfd);
        }
    }

    close_client(sockfd, read_fds);

    return 0;
}
