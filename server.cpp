/*****************************************************************************
** COMP3130 GROUP PROJECTS IN COMPUTER SCIENCE, SEMESTER 1, 2012
** Copyright (c) 2012, Stephen Gould
** All rights reserved.
**
******************************************************************************
** FILENAME:    server.cpp
** AUTHOR(S):   Stephen Gould <stephen.gould@anu.edu.au>
**
*****************************************************************************/

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <ctime>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <map>

#include "board.h"
#include "comms.h"

using namespace std;


// Player --------------------------------------------------------------------

class Player {
public:
    int client_socket;     //!< socket for communicating with this player
    string name;           //!< player's name
    double time_remaining; //!< time remaining for this player

public:
    Player() : client_socket(-1), time_remaining(0.0) { /* do nothing */ }
    ~Player() {
        // close the client socket
        if (client_socket != -1) {
            close(client_socket);
        }
    }

    //! clear existing data
    void clear() { client_socket = -1; name.clear(); time_remaining = 0.0; }
    //! returns true if a player has successfully connected
    bool connected() const { return (client_socket >= 0); }
    //! makes a new connection to a client
    void connect(int sock, const string& n, double t) {
        client_socket = sock; name = n; time_remaining = t;
    }
};

// PlayerStatistics ----------------------------------------------------------

class PlayerStatistics {
public:
    int gamesPlayed, gamesWon, gamesLost, gamesTied;
    int pointsFor, pointsAgainst;

public:
    PlayerStatistics() : gamesPlayed(0), gamesWon(0), gamesLost(0), gamesTied(0),
        pointsFor(0), pointsAgainst(0) { /* do nothing */ }
    ~PlayerStatistics() { /* do nothing */ }
};

// LeaderBoard ---------------------------------------------------------------

class LeaderBoard {
protected:
    map<string, PlayerStatistics> stats;

public:
    LeaderBoard() { /* do nothing */ }
    virtual ~LeaderBoard() { /* do nothing */ }

    //! clears the statistics
    void clear() {
        stats.clear();
    }

    //! updates leader board with results of last game
    void update(const string& whiteName, const string& blackName,
        int whitePieces, int blackPieces) {

        // update each player
        updatePlayer(whiteName, whitePieces, blackPieces);
        updatePlayer(blackName, blackPieces, whitePieces);
    }

    //! displayes the leaderboard
    virtual void visualize() const {
        cout << "------------------------------------------------------------------------------\n";
        cout << "\tP\tW\tL\tT\tF\tA\tName\n";
        for (map<string, PlayerStatistics>::const_iterator it = stats.begin();
             it != stats.end(); ++it) {
            cout << "\t" << it->second.gamesPlayed
                 << "\t" << it->second.gamesWon
                 << "\t" << it->second.gamesLost
                 << "\t" << it->second.gamesTied
                 << "\t" << it->second.pointsFor
                 << "\t" << it->second.pointsAgainst
                 << "\t" << it->first << "\n";
        }
        cout << "------------------------------------------------------------------------------\n";
    }

protected:

    //! updates statistics for each player individually
    void updatePlayer(const string& name, int playerPieces, int opponentPieces) {
        map<string, PlayerStatistics>::iterator it = stats.find(name);
        if (it == stats.end()) {
            it = stats.insert(it, make_pair(name, PlayerStatistics()));
        }
        it->second.gamesPlayed += 1;
        if (playerPieces == opponentPieces) {
            it->second.gamesTied += 1;
        } else if (playerPieces > opponentPieces) {
            it->second.gamesWon += 1;
        } else {
            it->second.gamesLost += 1;
        }
        it->second.pointsFor += playerPieces;
        it->second.pointsAgainst += opponentPieces;
    }
};

// comms ---------------------------------------------------------------------

//! Initializes the server to listen for client connections on the given port.
//!
int initServerComms(int port)
{
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "SERVER: error initializing server socket (socket)" << endl;
        exit(-1);
    }

    int t = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&t, sizeof(int)) < 0) {
        cerr << "SERVER: error initializing server socket (setsockopt)" << endl;
        exit(-1);
    }

    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0) {
        cerr << "SERVER: error initializing server socket (bind)" << endl;
        exit(-1);
    }

    return sock;
}

//! Waits for two players, one WHITE and one BLACK, to join the game.
//!
void waitForPlayers(Player players[2], int server_socket, double time_limit)
{
    cerr << "SERVER: waiting for clients to connect (press Ctrl-C to quit)..." << endl;
    
    players[0].clear();
    players[1].clear();

    while ((players[0].client_socket == -1) || (players[1].client_socket == -1)) {
        struct sockaddr_in client;
        int size = sizeof(sockaddr_in);
        int socket = accept(server_socket, (struct sockaddr *)&client, (socklen_t*)&size);
        if (socket < 0) {
            cerr << "SERVER: error listening for client (accept)" << endl;
        }
        cerr << "SERVER: accepted a connection from " << inet_ntoa(client.sin_addr)
             << " on port " << client.sin_port << endl;
        
        ConnectMessage message;
        message.receive(socket);
        
        // check that a player hasn't already connected for this side
        if (!players[message.side].connected()) {
            players[message.side].connect(socket, message.name, time_limit);
            cerr << "SERVER: " << message.name << " connected as "
                 << ((message.side == 0) ? "WHITE" : "BLACK") << endl;
            
            // set timeout for receive message
            struct timeval timeout;
            timeout.tv_sec = (int)time_limit;
            timeout.tv_usec = 0;
            
            if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval)) < 0) {
                cerr << "SERVER: error initializing server socket (setsockopt)" << endl;
                exit(-1);
            }
        } else {
            // a player for this side has already connected so simply close the connection
            cerr << "SERVER: error " << message.name << " attempting to connect as "
                 << ((message.side == 0) ? "WHITE" : "BLACK") << " but " <<
                players[message.side].name << " is already connected" << endl;
            close(socket);
        }
    }
}

// main ----------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int server_socket = -1;
    uint16_t port = DEFAULT_PORT;
    double time_limit = 150.0; // 5 minute game limit
    const char *board_file = NULL;

    // process command line arguments
    if (argc > 5) {
        cerr << "USAGE: " << argv[0] << " [random seed] [port] [time limit] [board]" << endl;
        return 0;
    }

    if (argc > 1) srand48(atoi(argv[1])); // seed random number generator
    if (argc > 2) port = atoi(argv[2]); // set server port
    if (argc > 3) time_limit =  0.5 * atof(argv[3]); // set game time
    if (argc > 4) board_file = argv[4]; // initial board

    // initialize server communications
    server_socket = initServerComms(port);
    cout << "SERVER: listening on port " << port << endl;
    listen(server_socket, 5);
    signal(SIGPIPE, SIG_IGN);

    // keep playing games until server dies
    LeaderBoard leaderBoard;

    while (1) {

        // wait for clients to connect
        Player players[2];
        waitForPlayers(players, server_socket, time_limit);

        // initialize boad
        Board board;
        if (board_file != NULL) {
            board.read(board_file);
        }
        board.visualize();

        // start game
        BoardState player = WHITE;
        BoardState opponent = BLACK;
        BoardState forfeit = EMPTY;
        ServerMessage messageOut;
        ClientMessage messageIn;

        while (1) {
            const int side = (player == WHITE) ? 0 : 1;
            const set<pair<int, int> > moves = board.legalMoves(player);

            // prepare server message
            if (moves.empty()) {
                if (board.legalMoves(opponent).empty()) {
                    break;
                }
                messageOut.status = NO_MOVE;
            } else {
                messageOut.status = GIVE_MOVE;
            }

            messageOut.timeRemaining = players[side].time_remaining;
            messageOut.board = board;

            // send the message
            if (!messageOut.send(players[side].client_socket)) {
                cerr << "SERVER: player " << getString(player) << " send error" << endl;
                forfeit = player;
                break;
            }                

            // wait for response
            struct timeval startTime, endTime;
            gettimeofday(&startTime, NULL);
            if (!messageIn.receive(players[side].client_socket)) {
                cerr << "SERVER: player " << getString(player) << " receive error" << endl;
                forfeit = player;
                break;
            }
            gettimeofday(&endTime, NULL);

            const double duration = endTime.tv_sec - startTime.tv_sec +
                1.0e-6 * (endTime.tv_usec - startTime.tv_usec);
            players[side].time_remaining -= duration;

            // check if player responded within the time limit
            if (players[side].time_remaining < 0.0) {
                // player forfeits
                cerr << "SERVER: player " << getString(player) << " ran out of time" << endl;
                forfeit = player;
                break;
            }

            // make the move
            if (!moves.empty()) {

                // check player made a legal move
                if (moves.find(make_pair(messageIn.x, messageIn.y)) == moves.end()) {
                    // player forfeits
                    cerr << "SERVER: player " << getString(player) << " made an illegal move" << endl;
                    forfeit = player;
                    break;
                } else {
                    board.makeMove(messageIn.x, messageIn.y, player);
                }
            }

            // show the board state
            board.visualize();

            // switch players
            std::swap(player, opponent);
            cerr << "SERVER: time remaining " << players[0].time_remaining << " and "
                 << players[1].time_remaining << endl;
        }

        // find winner and update leader board
        int nWhite = board.cellCount(WHITE);
        int nBlack = board.cellCount(BLACK);
        if (forfeit == WHITE) {
            nWhite = 0;
            nBlack = BOARD_SIZE * BOARD_SIZE - 4;
        } else if (forfeit == BLACK) {
            nWhite = BOARD_SIZE * BOARD_SIZE - 4;
            nBlack = 0;
        }

        if (nWhite == nBlack) {
            messageOut.winner = EMPTY;
            cout << "TIE";
        } else if (nWhite < nBlack) {
            messageOut.winner = BLACK;
            cout << "BLACK wins";
        } else {
            messageOut.winner = WHITE;
            cout << "WHITE wins";
        }
        cout << " (" << nWhite << " / " << nBlack << ")\n";

        leaderBoard.update(players[0].name, players[1].name, nWhite, nBlack);
        leaderBoard.visualize();

        // send end of game message to clients
        messageOut.status = GAME_END;
        messageOut.board = board;

        messageOut.timeRemaining = players[0].time_remaining;
        if (!messageOut.send(players[0].client_socket)) {
            cerr << "SERVER: error sending end of game message to WHITE" << endl;
        }
        close(players[0].client_socket);

        messageOut.timeRemaining = players[1].time_remaining;
        if (!messageOut.send(players[1].client_socket)) {
            cerr << "SERVER: error sending end of game message to BLACK" << endl;
        }
        close(players[1].client_socket);
    }

    // close server socket
    close(server_socket);
    cout << "SERVER: closed communications" << endl;

    return 0;
}
