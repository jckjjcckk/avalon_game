//
// Created by jck on 04/01/25.
//

#pragma once
#include <stdbool.h>
#include <time.h>
#include <sys/select.h>   // for select()
#include <unistd.h>       // for close()
#include <stdio.h>        // for printf()
#include <errno.h>        // for perror()

#define ML 1
#define PX 2
#define GG 3
#define MGN 4
#define ASS 5
#define MD 6
#define ABL 7

struct Game {
    int numPlayers;
    struct Player** players;
    int voteEvents[5][5];  // -2: doesn't need this voter, -1: haven't registered, 0: fail, 1: pass
    bool needsTwoFails[5];
    int voters[5][5];      // -2: doesn't need this voter, -1: haven't registered, n > 0: socket_fd
};

/**
struct VoteEvent {
    int numVoters;
    int voters[5];
    bool voted;
    int results[5];
    bool needsTwoFails;
};
 */


struct Player {
    int socket_fd;
    int role;
    char* name;
};


int vote_num_fails(const struct Game* game, int voteIndex);

int count_vote_events(const struct Game* game);

bool vote_passed(struct Game* game, int voteIndex);

// return -1 if player did not vote in this round, 1 for vote pass, 0 for vote against}
int player_voted(const struct Game* game, int voteIndex, int player_id);

void register_vote(struct Game* game, int voteIndex, int player_id, int vote_result);

int find_id_by_role(const struct Game* game, int role);

char* find_name_by_id(const struct Game* game, int player_id);

struct Game* initializeGame(int numPlayers);

char* display_personalized_msg(struct Game* game, int role);

char* display_vote_status(struct Game* game, int voteIndex, int player_id);

char* display_game_status(struct Game* game, int player_id);

void freeGame(struct Game* game);

int count_voters(const struct Game* game, int voteIndex);

int detect_winning(struct Game* game);

void collect_votes(struct Game* game, int numVoters, int voteIndex);

void game_start(int numPlayers);
