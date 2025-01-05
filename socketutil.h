//
// Created by jck on 04/01/25.
//

#pragma once


#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>

#define MAX_CLIENTS 6
#define BUFFER_SIZE 256

struct AcceptedSocket {
    int socket_fd;
    struct sockaddr_in address;
    char* name;
    int role;
};


char* hostname_to_ip(const char* hostname);

int create_ipv4_socket();

struct sockaddr_in* create_ipv4_address(char* addr, int port);

struct AcceptedSocket* accept_connection(int server_socket_fd);

void send_msg_to_client(int client, const char* msg);

char* receive_msg_from_client(int client);
