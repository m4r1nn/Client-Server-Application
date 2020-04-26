#include "./include/subscriber_helpers.h"

// for reading commands from stdin
void resolve_read_stdin(int sockfd, fd_set& read_fds) {

    // get command
    char buffer[BUFFLEN];
    memset(buffer, 0, BUFFLEN);
    char* read_check = fgets(buffer, BUFFLEN - 1, stdin);
    DIE(read_check == NULL, "read");

    // close client
    if (strncmp(buffer, "exit", 4) == 0) {
        close_client(sockfd, read_fds);
    }
    
    std::string message(buffer);
    if (message.size() > 0) {
        message.pop_back();
    }

    // check message type --- subscribe or unsubscribe
    DIE(strncmp(buffer, "subscribe", 9) != 0 && strncmp(buffer, "unsubscribe", 11) != 0, "invalid command");
    std::string message_type = message.substr(0, message.find(' '));
    message = message.substr(message.find(' ') + 1);
    std::string topic_name = message.substr(0, message.find(' '));

    int send_check = send(sockfd, buffer, strlen(buffer), 0);
    DIE(send_check < 0, "send");

    std::cout << message_type << "d " << topic_name << std::endl;
}

// for receiving news
void resolve_server_interaction(int sockfd) {

    // get news
    char buffer[BUFFLEN];
    memset(buffer, 0, BUFFLEN);
    int received = recv(sockfd, buffer, BUFFLEN, 0);
    DIE(received < 0, "recv");

    // print all news from susbcribed topics
    std::string message(buffer);
    if (received == 0) {
        close(sockfd);
        exit(0);
    }
    while (message.find('#') != std::string::npos) {
        std::cout << message.substr(0, message.find('#')) << std::endl;
        message = message.substr(message.find('#') + 1);
    }
    std::cout << message << std::endl;
}

// for closing the client
void close_client(int sockfd, fd_set& read_fds) {
    close(sockfd);
    close(STDIN_FILENO);
    FD_CLR(sockfd, &read_fds);
    FD_CLR(STDIN_FILENO, &read_fds);
    exit(0);
}
