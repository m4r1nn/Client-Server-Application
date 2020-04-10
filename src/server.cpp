#include "./include/server_helpers.h"

int main(int argc, char* argv[]) {

    DIE(argc < 2, "wrong call server");

    // descriptors 'arrays'
    fd_set read_fds;
    fd_set temp_fds;
    
    // for searching clients by id
    std::unordered_map<int, struct tcp_client> tcp_clients;

    // for searching clients' ids by socket they are connected
    std::unordered_map<int, int> fd_to_id;

    // initialize descriptors 'arrays'
    FD_ZERO(&read_fds);
    FD_ZERO(&temp_fds);

    // get descriptors for main interactions (tcp listen and udp)
    int con_tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(con_tcp_sockfd < 0, "TCP socket");
    int con_udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(con_udp_sockfd < 0, "UDP socket");

    int port = atoi(argv[1]);
    DIE(port <= 0, "invalid port");

    // set socket for the 2 main interactions
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // bind server and tcp station
    int tcp_bind_check = bind(con_tcp_sockfd, (struct sockaddr*) &serv_addr, sizeof(struct sockaddr));
    DIE(tcp_bind_check < 0, "TCP bind");
    // bind server and udp station
    int udp_bind_check = bind(con_udp_sockfd, (struct sockaddr*) &serv_addr, sizeof(struct sockaddr));
    DIE(udp_bind_check < 0, "UDP bind");

    // start listening from tcp station
    int tcp_listen_check = listen(con_tcp_sockfd, MAX_CLIENTS);
    DIE(tcp_listen_check < 0, "TCP listen");

    // add main descriptors to 'array'
    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(con_tcp_sockfd, &read_fds);
    FD_SET(con_udp_sockfd, &read_fds);
    int fdmax = std::max(STDIN_FILENO, std::max(con_tcp_sockfd, con_udp_sockfd));

    std::cout << "server started working" << std::endl;

    // run server
    while (true) {
        temp_fds = read_fds;

        // see what descriptors are ready for interactions
        int select_check = select(fdmax + 1, &temp_fds, NULL, NULL, NULL);
        DIE(select_check < 0, "select");

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &temp_fds)) {

                // read from stdin
                if (i == STDIN_FILENO) {
                    resolve_read_stdin(read_fds, fdmax);

                // receive more tcp clients
                } else if (i == con_tcp_sockfd) {
                    resolve_tcp_connection(i, read_fds, fdmax, tcp_clients, fd_to_id);

                // receive message from udp station
                } else if (i == con_udp_sockfd) {
                    resolve_udp_interaction(i, con_tcp_sockfd, serv_addr, tcp_clients);

                // receive message from tcp client
                } else {
                    resolve_tcp_interaction(i, read_fds, tcp_clients, fd_to_id);
                }
            }
        }
    }

    // close all descriptors
    close_server(read_fds, fdmax);

    return 0;
}
