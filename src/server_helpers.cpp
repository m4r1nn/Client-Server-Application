#include "./include/server_helpers.h"

// for reading commands from server admin
void resolve_read_stdin(fd_set& read_fds, int fdmax) {
    
    // get command
    char buffer[BUFFLEN];
    memset(buffer, 0, BUFFLEN);
    char* read_check = fgets(buffer, BUFFLEN - 1, stdin);
    DIE(read_check == NULL, "read");

    // close server
    if (strncmp(buffer, "exit", 4) == 0) {
        close_server(read_fds, fdmax);
    } else {
        std::cout << "wrong command" << std::endl;
    }
}

// for receiving more clients
void resolve_tcp_connection(int sockfd, fd_set &read_fds, int& fdmax,
    std::unordered_map<int, struct tcp_client>& tcp_clients, std::unordered_map<int, int>& fd_to_id) {
    
    struct sockaddr_in tcp_cli_addr;
    socklen_t len = sizeof(tcp_cli_addr);

    // accept new tcp client
    int tcp_client_fd = accept(sockfd, (struct sockaddr*) &tcp_cli_addr, &len);
    DIE(tcp_client_fd < 0, "TCP accept");

    // add it to 'array'
    FD_SET(tcp_client_fd, &read_fds);
    if (tcp_client_fd > fdmax) {
        fdmax = tcp_client_fd;
    }

    // get client id
    char buffer[BUFFLEN];
    memset(buffer, 0, BUFFLEN);
    int received = recv(tcp_client_fd, buffer, BUFFLEN, 0);
    DIE(received < 0, "no id received");

    int id = atoi(buffer);
    DIE(id < 0, "atoi");

    // check if client was conneted before
    if (tcp_clients.find(id) == tcp_clients.end()) {

        // if not, add new client to map
        struct tcp_client tcp_client;
        tcp_client.socketfd = tcp_client_fd;
        tcp_client.id = id;
        tcp_client.active = true;
        tcp_clients[id] = tcp_client;
        fd_to_id[tcp_client_fd] = id;

    } else {
        // if yes, change status in active and update his fd
        struct tcp_client* tcp_client = &tcp_clients[id];
        DIE(tcp_client->active == true, "id already exists");
        fd_to_id.erase(fd_to_id.find(tcp_client->socketfd));
        fd_to_id[tcp_client_fd] = tcp_client->id;
        tcp_client->socketfd = tcp_client_fd;
        tcp_client->active = true;
        std::string to_forward;

        // sent him all saved messages
        for (auto it = tcp_client->topics.begin(); it != tcp_client->topics.end(); it++) {
            for (auto it2 = (*it).second.stored_messages.begin(); it2 != (*it).second.stored_messages.end(); it2++) {
                to_forward += (*it2 + '#');
            }
            (*it).second.stored_messages.clear();
        }

        memset(buffer, 0, BUFFLEN);
        strcpy(buffer, to_forward.c_str());
        int send_check = send(tcp_client_fd, buffer, strlen(buffer), 0);
        DIE(send_check < 0, "send");
    }

    std::cout << "New client (" << id << ") connected from " << inet_ntoa(tcp_cli_addr.sin_addr)
        << ":" << ntohs(tcp_cli_addr.sin_port) << std::endl;

}

// for receivind and posting news from udp clients
void resolve_udp_interaction(int sockfd, int con_tcp_sockfd, struct sockaddr_in& serv_addr,
    std::unordered_map<int, struct tcp_client>& tcp_clients) {
    
    socklen_t len;

    // get news
    char buffer[BUFFLEN];
    memset(buffer, 0, BUFFLEN);
    int received = recvfrom(sockfd, buffer, BUFFLEN, 0, (struct sockaddr*) &serv_addr, &len);
    DIE(received < 0, "recvfrom");

    // get client's port and ip
    std::string to_forward(inet_ntoa(serv_addr.sin_addr));
    to_forward += (":" + std::to_string(ntohs(serv_addr.sin_port)) + " - ");

    std::string result(buffer);
    if (result.size() > 0) {
        result.pop_back();
    }

    // get topic name
    std::string topic_name = result.substr(0, result.find(' '));
    to_forward += (topic_name + " - ");
    result = result.substr(result.find(' ') + 1);

    char data_temp[SMALL_BUFF];
    strcpy(data_temp, result.substr(0, result.find(' ')).c_str());

    // check data type
    int data_type = atoi(data_temp);
    switch (data_type) {
        case 0:
            to_forward += "INT - ";
            break;
        case 1:
            to_forward += "SHORT_REAL - ";
            break;
        case 2:
            to_forward += "FLOAT - ";
            break;
        case 3:
            to_forward += "STRING - ";
            break;
        default:
            DIE(true, "invalid data type");
    }
    result = result.substr(result.find(' ') + 1);
    to_forward += result;

    memset(buffer, 0, BUFFLEN);
    strcpy(buffer, to_forward.c_str());

    // iterate through all tcp clients
    for (auto it = tcp_clients.begin(); it != tcp_clients.end(); it++) {
        struct tcp_client* tcp_client = &(*it).second;
        if (tcp_client->socketfd == sockfd || tcp_client->socketfd == con_tcp_sockfd) {
            continue;
        }

        if (tcp_client->topics.find(topic_name) != tcp_client->topics.end()) {

            // if client is active, sent him the message
            if (tcp_client->active == true) {
                int send_check = send(tcp_client->socketfd, buffer, strlen(buffer), 0);
                DIE(send_check < 0, "send");

            // if not check his subscribe type and save message if needed
            } else if (tcp_client->topics[topic_name].subscribe_type == 1) {
                tcp_client->topics[topic_name].stored_messages.push_back(to_forward);
            }
        }
    }
}

// for checking subscribe and unsubscribe messages from tcp clients
void resolve_tcp_interaction(int sockfd, fd_set& read_fds,
    std::unordered_map<int, struct tcp_client>& tcp_clients, std::unordered_map<int, int>& fd_to_id) {

    // get request
    char buffer[BUFFLEN];
    memset(buffer, 0, BUFFLEN);
    int received = recv(sockfd, buffer, BUFFLEN, 0);
    DIE(received < 0, "recv");

    // remove client
    if (received == 0) {
        close(sockfd);
        FD_CLR(sockfd, &read_fds);

        struct tcp_client* tcp_client = &tcp_clients[fd_to_id[sockfd]];
        tcp_client->active = false;

        std::cout << "Client (" << tcp_client->id << ") disconnected" << std::endl;

    } else {
        std::string result(buffer);
        if (result.size() > 0) {
            result.pop_back();
        }
        std::string command = result.substr(0, result.find(' '));
        DIE(command != "subscribe" && command != "unsubscribe", "wrong command");
        result = result.substr(result.find(' ') + 1);

        // subscribe client
        if (command == "subscribe") {
            std::string topic_name = result.substr(0, result.find(' '));
            result = result.substr(result.find(' ') + 1);

            // get subscribe type --- temporary or permanent
            char temp_type[SMALL_BUFF];
            strcpy(temp_type, result.c_str());
            int type = atoi(temp_type);
            DIE(type != 1 && type != 0, "invalid type");

            // build topic field
            struct topic topic;
            topic.name = topic_name;
            topic.subscribe_type = type;

            // add topic to client's subscribed topics
            struct tcp_client* tcp_client = &tcp_clients[fd_to_id[sockfd]];
            tcp_client->topics[topic_name] = topic;
        
        // unsubscribe client
        } else if (command == "unsubscribe") {
            std::string topic_name = result;

            // remove topic from client's subscribed topics
            struct tcp_client* tcp_client = &tcp_clients[fd_to_id[sockfd]];
            DIE(tcp_client->topics.find(topic_name) == tcp_client->topics.end(), "not sbscribed");
            tcp_client->topics.erase(tcp_client->topics.find(topic_name));
        }
    }
}

// for closing the server
void close_server(fd_set &read_fds, int fdmax) {

    // remove all clients
	for (int i = 0; i <= fdmax; i++) {
		close(i);
		FD_CLR(i, &read_fds);
	}
    exit(0);
}
