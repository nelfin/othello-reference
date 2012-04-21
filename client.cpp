/*****************************************************************************
** COMP3130 GROUP PROJECTS IN COMPUTER SCIENCE, SEMESTER 1, 2012
** Copyright (c) 2012, Stephen Gould
** All rights reserved.
**
******************************************************************************
** FILENAME:    client.cpp
** AUTHOR(S):   Stephen Gould <stephen.gould@anu.edu.au>
**
*****************************************************************************/

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <string>

#include "board.h"
#include "comms.h"

using namespace std;

int main(int argc, char *argv[])
{
    BoardState player;
    string host = DEFAULT_HOST;
    uint16_t port = DEFAULT_PORT;

    // process command line arguments
    if ((argc < 2) || (argc > 3)) {
        cerr << "USAGE: " << argv[0] << " <white|black> [[host:]port]" << endl;
        return 0;
    }

    if (!strcmp(argv[1], "white")) {
        player = WHITE;
        cerr << "CLIENT: player is WHITE" << endl;
    } else if (!strcmp(argv[1], "black")) {
        player = BLACK;
        cerr << "CLIENT: player is BLACK" << endl;
    } else {
        cerr << "CLIENT: unknown player \"" << argv[1] << "\"" << endl;
    }

    if (argc == 3) {
        string hostString(argv[2]);
        size_t indexOfColon = hostString.find(':', 0);
        if(indexOfColon == string::npos) {
            port = atoi(argv[2]);
        } else {
            host = hostString.substr(0, indexOfColon);
            port = atoi(hostString.substr(indexOfColon + 1, hostString.size()).c_str());
        }
    }

    // initialize client communications
    cerr << "CLIENT: connecting to server " << host << ":" << port << endl;

    int client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        cerr << "CLIENT: error initializing client socket (socket)" << endl;
        exit(-1);
    }

    struct sockaddr_in server_name;
    struct hostent *hostinfo;
    server_name.sin_family = AF_INET;
    server_name.sin_port = htons(port);

    hostinfo = gethostbyname(host.c_str());
    if (hostinfo == NULL) {
        cerr << "CLIENT: unknown host " << host.c_str() << endl;
        exit(-1);
    }
    server_name.sin_addr = *(struct in_addr *)hostinfo->h_addr;

    if (connect(client_socket, (struct sockaddr *)&server_name, sizeof(server_name)) < 0) {
        cerr << "CLIENT: error connecting to server" << endl;
        exit(-1);
    }

    // send connect message to server
    ConnectMessage message(player, player == WHITE ?
        "Random White Player" : "Random Black Player");
    if (!message.send(client_socket)) {
        cerr << "CLIENT: could not connect to server" << endl;
        exit(-1);
    }

    // play game
    ServerMessage messageIn;
    ClientMessage messageOut;
    while (1) {
        if (!messageIn.receive(client_socket)) {
            cerr << "CLIENT: receive error" << endl;
            exit(-1);
        }
        messageIn.board.visualize();

        if (messageIn.status == ABORT) {
            cerr << "CLIENT: server abort" << endl;
            break;
        } else if (messageIn.status == GAME_END) {
            cerr << "CLIENT: end of game (winner was " 
                 << getString(messageIn.winner) << ")" << endl;
            break;
        } else if (messageIn.status == NO_MOVE) {
            messageOut.x = messageOut.y = -1;
            cerr << "CLIENT: has no move" << endl;
        } else if (messageIn.status == GIVE_MOVE) {
            // make a random move
            const set<pair<int, int> > moves = messageIn.board.legalMoves(player);
            assert(!moves.empty());
            int randomIndex = (int)(drand48() * moves.size());
            set<pair<int, int> >::const_iterator it = moves.begin();
            while (randomIndex-- > 0) {
                ++it;
            }
            messageOut.x = it->first;
            messageOut.y = it->second;
            cerr << "CLIENT: making move (" << messageOut.x << ", " << messageOut.y << ")" << endl;
        } else {
            cerr << "CLIENT: invalid message from server" << endl;
            exit(-1);
        }

        // send move to server
        if (!messageOut.send(client_socket)) {
            cerr << "CLIENT: send error" << endl;
            exit(-1);
        }
    }

    return 0;
}
