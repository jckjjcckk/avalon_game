//
// Created by jck on 04/01/25.
//

#include "gameutil.h"
#include "socketutil.h"

extern struct AcceptedSocket global_clients[MAX_CLIENTS];

int vote_num_fails(const struct Game* game, const int voteIndex) {
    if(game->voteEvents[voteIndex][0] == -1) return -1; // vote haven't started
    int numFails = 0;
    for (int i = 0; i < 5; i++) {
        if(game->voteEvents[voteIndex][i] == 0) {
            numFails++;
        }
    }
    return numFails;
}

int count_vote_events(const struct Game* game) {
    int count = 0;
    for (int i = 0; i < 5; i++) {
        if (game->voteEvents[i][0] >= 0) {
            count++;
        }
    }
    return count;
}

bool vote_passed(struct Game* game, const int voteIndex) {
    int numFails = vote_num_fails(game, voteIndex);
    if (game->needsTwoFails[voteIndex]) {
        return numFails < 2;
    } else {
        return numFails < 1;
    }
}

int player_voted(const struct Game* game, const int voteIndex, const int player_id) {
    for(int i = 0; i < 5; i++) {
        if(game->voters[voteIndex][i] == player_id) {
            return game->voteEvents[voteIndex][i];
        }
    }
    return -1;
}

void register_vote(struct Game* game, const int voteIndex, const int player_id, const int vote_result) {
    for(int i = 0; i < 5; i++) {
        if(game->voters[voteIndex][i] != -1) continue;
        game->voters[voteIndex][i] = player_id;
        game->voteEvents[voteIndex][i] = vote_result;
        return;
    }
    send_msg_to_client(player_id, "ERROR_VOTE_FULL\n");
}

int find_id_by_role(const struct Game* game, const int role) {
    for (int i = 0; i < game->numPlayers; i++) {
        if (game->players[i]->role == role) {
            return game->players[i]->socket_fd;
        }
    }
    return -1;
}

char* find_name_by_id(const struct Game* game, const int player_id) {
    for (int i = 0; i < game->numPlayers; i++) {
        if (game->players[i]->socket_fd == player_id) {
            return game->players[i]->name;
        }
    }
    return NULL;
}

struct Game* initializeGame(const int numPlayers) {
    struct Game* game = malloc(sizeof(struct Game));
    // init game->numPlayers
    game->numPlayers = numPlayers;
    // pre-init game->players
    game->players = malloc(sizeof(struct Player*) * numPlayers);
    // pre-init game->needsTwoFails
    for(int i = 0; i < 5; i++) game->needsTwoFails[i] = false;
    int votes[5];
    int roles[numPlayers];
    switch (numPlayers) {
    case 5:
        memcpy(votes, (int[]){2, 3, 2, 3, 3}, sizeof(votes));
        memcpy(roles, (int[]){ML, PX, GG, MGN, ASS}, numPlayers * sizeof(int));
        break;
    case 6:
        memcpy(votes, (int[]){2, 3, 4, 3, 4}, sizeof(votes));
        memcpy(roles, (int[]){ML, PX, GG, GG, MGN, ASS}, numPlayers * sizeof(int));
        break;
    case 7:
        memcpy(votes, (int[]){2, 3, 3, 4, 4}, sizeof(votes));
        memcpy(roles, (int[]){ML, PX, GG, GG, MGN, ASS, ABL}, numPlayers * sizeof(int));
        game->needsTwoFails[3] = true;
        break;
    case 8:
        memcpy(votes, (int[]){3, 4, 4, 5, 5}, sizeof(votes));
        memcpy(roles, (int[]){ML, PX, GG, GG, GG, MGN, ASS, ABL}, numPlayers * sizeof(int));
        game->needsTwoFails[3] = true;
        break;
    case 9:
        memcpy(votes, (int[]){3, 4, 4, 5, 5}, sizeof(votes));
        memcpy(roles, (int[]){ML, PX, GG, GG, GG, MGN, ASS, ABL}, numPlayers * sizeof(int));
        game->needsTwoFails[3] = true;
        break;
    case 10:
        memcpy(votes, (int[]){3, 4, 4, 5, 5}, sizeof(votes));
        memcpy(roles, (int[]){ML, PX, GG, GG, GG, GG, MGN, ASS, ABL, MD}, numPlayers * sizeof(int));
        game->needsTwoFails[3] = true;
        break;
    default:
        perror("ERROR PLAYER NUMBER (5-10)");
        free(game->players);
        free(game);
        return NULL;
    }
    // init game->voteEvents
    // init game->voters
    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 5; j++) {
            if(j < votes[i]) {
                game->voteEvents[i][j] = -1;
                game->voters[i][j] = -1;
            } else {
                game->voteEvents[i][j] = -2;
                game->voters[i][j] = -2;
            }
        }
    }
    // randomizing roles
    int rand_roles[numPlayers];
    for (int i = 0; i < numPlayers; i++) rand_roles[i] = roles[i];
    srand((unsigned int)time(NULL));
    // Fisher-Yates shuffle
    for (int i = (numPlayers - 1); i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = rand_roles[i]; rand_roles[i] = rand_roles[j]; rand_roles[j] = temp;
    }
    // init player->role
    for(int i = 0; i < numPlayers; i++)
    {
        game->players[i] = malloc(sizeof(struct Player));
        game->players[i]->role = rand_roles[i];
    }
    // init player->name and player->id
    int player_i = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (global_clients[i].socket_fd == 0) {continue;}
        game->players[player_i]->socket_fd = global_clients[i].socket_fd;
        game->players[player_i]->name = malloc(strlen(global_clients[i].name) + 1);
        strcpy(game->players[player_i]->name, global_clients[i].name);
        player_i++;
    }
    return game;
}

char* display_personalized_msg(struct Game* game, int role)
{
    char* buffer = malloc(sizeof(char) * 256);
    buffer[0] = '\0';
    switch (role) {
    case ML:
        strcat(buffer, "You are ***Merlin***\nThe bad guys are ");
        for(int i = 0; i < game->numPlayers; i++) {
            if(game->players[i]->role == MGN || game->players[i]->role == ASS) {
                strcat(buffer, game->players[i]->name);
                strcat(buffer, ", ");
            }
        } strcat(buffer, "\n");
        break;
    case PX:
        strcat(buffer, "You are ***Percival***\nThe Merlins are ");
        for(int i = 0; i < game->numPlayers; i++) {
            if(game->players[i]->role == MGN || game->players[i]->role == ML) {
                strcat(buffer, game->players[i]->name);
                strcat(buffer, ", ");
            }
        } strcat(buffer, "\n");
        break;
    case ASS:
        strcat(buffer, "You are the ***assassin***!\nYour evil partner is ");
        for(int i = 0; i < game->numPlayers; i++) {
            if(game->players[i]->role == MGN) {
                strcat(buffer, game->players[i]->name);
            }
        } strcat(buffer, "\n");
        break;
    case MGN:
        strcat(buffer, "You are ***Morgana***\nYour evil partner is ");
        for(int i = 0; i < game->numPlayers; i++) {
            if(game->players[i]->role == ASS) {
                strcat(buffer, game->players[i]->name);
            }
        } strcat(buffer, "\n");
        break;
    case GG:
        strcat(buffer, "You are just a good guy\n");
        break;
    case ABL:
        strcat(buffer, "You are Oberon\n");
        break;
    }
    return buffer;
}
// if player_id == -1 means public display
char* display_vote_status(struct Game* game, const int voteIndex, const int player_id) {
    char* buffer = malloc(sizeof(char) * 256);
    sprintf(buffer, "Vote Number %d", voteIndex + 1);
    if (game->voteEvents[voteIndex][0] == -1) {
        strcat(buffer, " hasn't started\n");
        return buffer;
    } else {
        if(vote_passed(game, voteIndex)) {
            strcat(buffer, " PASSED, ");
        } else {
            strcat(buffer, " FAILED, ");
        }
        strcat(buffer, "Participants: ");
        for(int i = 0; i < count_voters(game, voteIndex); i++) {
            strcat(buffer, find_name_by_id(game, game->voters[voteIndex][i]));
            strcat(buffer, ", ");
        }
        if(vote_num_fails(game, voteIndex) > 0) {
            strcat(buffer, "THERE ARE ");
            char numFails[12];
            sprintf(numFails, "%d", vote_num_fails(game, voteIndex));
            strcat(buffer, numFails);
            strcat(buffer, " FAIL VOTES");
        }


        if (player_voted(game, voteIndex, player_id) == 1) {
            strcat(buffer, ", you voted PASS\n");
        } else if (player_voted(game, voteIndex, player_id) == 0) {
            strcat(buffer, ", you voted FAIL\n");
        } else {
            strcat(buffer, "\n");
        }
    return buffer;
    }
}

char* display_game_status(struct Game* game, const int player_id) {
    char* buffer = malloc(sizeof(char) * 2048);
    buffer[0] = '\0';
    size_t index = 0;

    index += sprintf(buffer + index, "Game status\n");

    for (int i = 0; i < 5; i++) {
        char* current_line = display_vote_status(game, i, player_id);
        index += sprintf(buffer + index, "%s", current_line);
        free(current_line);
    }
    return buffer;
}

void freeGame(struct Game* game) {
    for(int i = 0; i < game->numPlayers; i++) {
        free(game->players[i]->name);
        free(game->players[i]);
    }
    free(game->players);
    free(game);
}

int count_voters(const struct Game* game, const int voteIndex) {
    int count = 0;
    for (int i = 0; i < 5; i++) {
        if (game->voters[voteIndex][i] != -2) {
            count++;
        }
    }
    return count;
}

int detect_winning(struct Game* game) {
    int good = 0;
    int bad = 0;
    for(int i = 0; i < 5; i++) {
        if(game->voteEvents[i][0] == -1) break;  // vote haven't started
        if(vote_passed(game, i)) {
            good++;
        } else {
            bad++;
        }
    }
    if(good >= 3) {
        return 1;
    } else if(bad >= 3) {
        return 0;
    } else {
        return -1;
    }
}


/**
 * collect_votes:
 *   1) Broadcast a request for votes (O or X) to all players.
 *   2) Wait until num_votes have been collected from any players.
 *   3) Store each vote in the first "round" of voteEvents (voteIndex = 0) as an example.
 *      Adjust if you need multiple rounds.
 * 
 * @param game       Pointer to the current Game
 * @param num_votes  Number of votes you want to collect (e.g., 3)
 */
void collect_votes(struct Game* game, int num_votes, int voteIndex)
{
    // 1. Broadcast the prompt to all players
    for (int i = 0; i < game->numPlayers; i++) {
        char* status = display_game_status(game, game->players[i]->socket_fd);
        send_msg_to_client(game->players[i]->socket_fd, status);
        free(status);
        char* msg = malloc(sizeof(char) * 128);
        sprintf(msg, "==========Vote Number %d==========\n=========%d voters needed==========\n", voteIndex + 1, count_voters(game, voteIndex));
        send_msg_to_client(game->players[i]->socket_fd, msg);
        free(msg);
    }

    int votes_collected = 0;

    // 2. Keep collecting until we have num_votes
    while (votes_collected < num_votes)
    {
        // Build the fd_set for select()
        fd_set readfds;
        FD_ZERO(&readfds);

        int max_fd = -1;
        // Add each player's socket to the fd_set
        for (int i = 0; i < game->numPlayers; i++) {
            int sock = game->players[i]->socket_fd;
            if (sock > 0) {
                FD_SET(sock, &readfds);
                if (sock > max_fd) {
                    max_fd = sock;
                }
            }
        }

        // Block until one or more sockets have data
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            return;  // Depending on your logic, you might want to continue or break
        }

        // For each player, check if there's data to read
        for (int i = 0; i < game->numPlayers && votes_collected < num_votes; i++) {
            int sock = game->players[i]->socket_fd;
            if (sock > 0 && FD_ISSET(sock, &readfds)) {
                // 2a. Read message from client
                char* msg = receive_msg_from_client(sock);
                if (msg == NULL) {
                    // This usually indicates client disconnected or an error
                    // You can handle that here, e.g., mark player as disconnected
                    continue; 
                }

                // 2b. Parse the vote from the message
                //     We just check the first character (O or X).
                //     You may want more robust parsing.
                char c = msg[0];
                int vote_result = -1;  // 1 for pass (O), 0 for fail (X)
                if (c == 'O' || c == 'o') {
                    vote_result = 1;  // pass
                } else if (c == 'X' || c == 'x') {
                    vote_result = 0;  // fail
                }

                // If invalid, ask again. Otherwise, record the vote.
                if (vote_result == -1) {
                    send_msg_to_client(sock, 
                        "Invalid vote. Please type 'O' or 'X' and press Enter.\n");
                } else {
                    // 2c. Register the vote in your Game data structures.
                    //     This uses the existing register_vote() function
                    //     and stores the vote under voteIndex = 0.
                    register_vote(game, voteIndex, sock, vote_result);
                    votes_collected++;
                }
                free(msg); 
            }
        }
    }
}



void game_start(int numPlayers)
{
    struct Game* game = initializeGame(numPlayers);
    // display initial message
    for(int i = 0; i < numPlayers; i++) {
        send_msg_to_client(game->players[i]->socket_fd, "==========Game Start==========\n");
        char* msg = display_personalized_msg(game, game->players[i]->role);
        send_msg_to_client(game->players[i]->socket_fd, msg);
        free(msg);
    }
    int voteIndex = 0;
    while(detect_winning(game) == -1) {
        // wait for votes and process
        collect_votes(game, count_voters(game, voteIndex), voteIndex);
        // display vote status
        char* msg = malloc(sizeof(char) * 256);
        sprintf(msg, "Vote Number %d has finished\n", voteIndex + 1);
        if(vote_passed(game, voteIndex)) {
            strcat(msg, "PASSED");
        } else {
            strcat(msg, "FAILED");
        }
        if (vote_num_fails(game, voteIndex) > 0) {
            strcat(msg, " with ");
            char numFails[12];
            sprintf(numFails, "%d", vote_num_fails(game, voteIndex));
            strcat(msg, numFails);
            strcat(msg, " FAIL votes\n");
        } else {
            strcat(msg, "\n");
        }
        for(int i = 0; i < numPlayers; i++) {
            send_msg_to_client(game->players[i]->socket_fd, msg);
        }
        free(msg);
        voteIndex++;
    }
    // display game result
    if(detect_winning(game) == 1) {
        send_msg_to_client(find_id_by_role(game, ASS), "Please enter your assassination target\n");
        char* ass_target = receive_msg_from_client(find_id_by_role(game, ASS));
        char* real_ml = find_name_by_id(game, find_id_by_role(game, ML));
        for(int i = 0; i < numPlayers; i++) {
            if(strncmp(ass_target, real_ml, strlen(real_ml)) == 0) {
                send_msg_to_client(game->players[i]->socket_fd, "Bad guys win, Merlin is killed\n");
            } else {
                send_msg_to_client(game->players[i]->socket_fd, "Good guys win, assissation failed\n");
            }
        }
        free(ass_target);
    } else {
        for(int i = 0; i < numPlayers; i++) {
            send_msg_to_client(game->players[i]->socket_fd, "Bad guys win!\n");
        }
    }
    freeGame(game);
}
