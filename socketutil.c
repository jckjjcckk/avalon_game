//
// Created by jck on 04/01/25.
//

#include "socketutil.h"

char* hostname_to_ip(const char* hostname) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(hostname, NULL, &hints, &res);
    if (ret != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(ret));
        return NULL;
    }

    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;

    // Convert the IP address to a string
    char ipStr[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &(addr->sin_addr), ipStr, sizeof(ipStr))) {
        perror("inet_ntop");
        freeaddrinfo(res);
        return NULL;
    }

    // Allocate and copy the IP string
    char *result = malloc(strlen(ipStr) + 1);
    if (!result) {
        perror("malloc");
        freeaddrinfo(res);
        return NULL;
    }
    strcpy(result, ipStr);

    freeaddrinfo(res);
    return result;
}

int create_ipv4_socket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

struct sockaddr_in* create_ipv4_address(char* addr, const int port) {
    char* ip = hostname_to_ip(addr);
    struct sockaddr_in* address = malloc(sizeof(struct sockaddr_in));
    address->sin_port = htons(port);
    address->sin_family = AF_INET;

    if (strlen(addr) == 0) {
        address->sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    }
    printf("IP address of %s is %s\n", addr, ip);
    free(ip);
    return address;
}

struct AcceptedSocket* accept_connection(const int server_socket_fd) {
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_address, &client_address_size);

    if (client_socket_fd < 0) {
        perror("Accept failed");
        return NULL;
    }

    struct AcceptedSocket* accepted = malloc(sizeof(struct AcceptedSocket));
    accepted->socket_fd = client_socket_fd;
    accepted->address = client_address;

    return accepted;
}

void send_msg_to_client(const int client, const char* msg) {
    ssize_t bytes_sent = send(client, msg, strlen(msg), 0);
    if (bytes_sent < 0) {
        perror("Send failed");
    }
}

char* receive_msg_from_client(const int client) {
    char* buffer = malloc(sizeof(char) * BUFFER_SIZE);
    ssize_t bytes_received = recv(client, buffer, sizeof(char) * (BUFFER_SIZE - 1), 0);
    if (bytes_received < 0) {
        perror("Receive failed");
        return NULL;
    } else if (bytes_received == 0) {
        printf("Connection closed by client\n");
        free(buffer);
        return NULL;
    } else {
        buffer[bytes_received] = '\0'; // Null-terminate the received data
        return buffer;
    }
}
