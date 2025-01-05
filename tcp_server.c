#include "socketutil.h"
#include "gameutil.h"
#include <pthread.h>


struct AcceptedSocket global_clients[MAX_CLIENTS];

/**
 * Count how many clients are currently connected.
 */
int num_connected() {
    int count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (global_clients[i].socket_fd != 0) {
            count++;
        }
    }
    return count;
}

/**
 * Example thread function to handle basic I/O from a single client.
 * (Optional: If your game logic uses collect_votes() with select(),
 * you might skip per-client threads entirely.)
 */
void* handle_client(void* arg) {
    struct AcceptedSocket* client = (struct AcceptedSocket*) arg;

    while (1) {
        sleep(1);
    }

    // Clean up and mark slot as free
    printf("Player %s disconnected\n", client->name);
    free(client->name);
    client->socket_fd = 0;
    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    // 1. Check arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);

    // 2. Create and bind a socket
    int server_socket_fd = create_ipv4_socket();
    if (server_socket_fd < 0) {
        perror("Create socket failed");
        return 1;
    }

    struct sockaddr_in* server_address = create_ipv4_address("", port);
    if (bind(server_socket_fd, (struct sockaddr*)server_address, sizeof(*server_address)) < 0) {
        perror("Bind failed");
        free(server_address);
        close(server_socket_fd);
        return 1;
    }
    free(server_address);

    if (listen(server_socket_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_socket_fd);
        return 1;
    }
    printf("Server listening on port %d\n", port);

    // 3. Accept connections until we have MAX_CLIENTS connected
    while (num_connected() < MAX_CLIENTS) {
        struct AcceptedSocket* client = accept_connection(server_socket_fd);
        if (!client) {
            perror("Accept failed");
            continue;
        }

        // Find a free slot
        int free_index = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (global_clients[i].socket_fd == 0) {
                free_index = i;
                break;
            }
        }

        // If no slot is free (should rarely happen if we stop at MAX_CLIENTS)
        if (free_index < 0) {
            send_msg_to_client(client->socket_fd, "Server is full. Try again later.\n");
            close(client->socket_fd);
            free(client);
            continue;
        }

        // Store the client in the global array
        global_clients[free_index] = *client;
        free(client);

        // Prompt for username
        send_msg_to_client(global_clients[free_index].socket_fd, "Enter your username: ");
        char* name = receive_msg_from_client(global_clients[free_index].socket_fd);
        if (!name || strlen(name) == 0) {
            send_msg_to_client(global_clients[free_index].socket_fd, 
                               "Invalid username. Disconnecting.\n");
            close(global_clients[free_index].socket_fd);
            global_clients[free_index].socket_fd = 0;
            free(name);
            continue;
        }
        // Trim trailing newline
        char* newline = strchr(name, '\n');
        if (newline) *newline = '\0';

        global_clients[free_index].name = name;

        printf("Player %s joined (socket_fd: %d)\n",
               global_clients[free_index].name,
               global_clients[free_index].socket_fd);

        // (Optional) Create a thread to continuously read from this client
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, &global_clients[free_index]) != 0) {
            perror("pthread_create");
            close(global_clients[free_index].socket_fd);
            global_clients[free_index].socket_fd = 0;
            continue;
        }
        pthread_detach(tid);

        printf("Current connections: %d / %d\n", num_connected(), MAX_CLIENTS);
    }

    // 4. Once we have MAX_CLIENTS, start the game
    printf("Reached %d players! Starting the game...\n", MAX_CLIENTS);
    game_start(MAX_CLIENTS);

    // 5. Close the listening socket (no more connections)
    close(server_socket_fd);

    return 0;
}
