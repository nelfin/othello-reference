/*****************************************************************************
** COMP3130 GROUP PROJECTS IN COMPUTER SCIENCE, SEMESTER 1, 2012
** Copyright (c) 2012, Stephen Gould
** All rights reserved.
**
******************************************************************************
** FILENAME:    comms.h
** AUTHOR(S):   Stephen Gould <stephen.gould@anu.edu.au>
**
*****************************************************************************/

#ifndef _COMMS_H_
#define _COMMS_H_

#include <cstdlib>
#include <cassert>
#include <string>
#include <sys/types.h>

#define _BSD_SOCKLEN_T_
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "board.h"

// Constants -----------------------------------------------------------------

#define DEFAULT_PORT 3130
#define DEFAULT_HOST "localhost"

// GameStatus ----------------------------------------------------------------

typedef enum { GIVE_MOVE = 0, //!< player needs to return a move
               NO_MOVE, //!< player has no move and should return (-1,-1)
               GAME_END, //!< the game has ended normally or due to forfeit
               ABORT //!< the game has ended abnormally (communications error)
} GameStatus;

// CommsMessages -------------------------------------------------------------

class CommsMessage {
 public:
    CommsMessage();
    virtual ~CommsMessage();

    //! send the message (return false on error)
    virtual bool send(int socket) const = 0;
    //! receive the message (return false on error)
    virtual bool receive(int socket) = 0;
};

// ConnectMessage ------------------------------------------------------------

class ConnectMessage : public CommsMessage {
 public:
    static const size_t MAX_NAME; //!< maximum length for a name

    int side;              //!< 0: WHITE, 1: BLACK
    std::string name;      //!< team name

 public:
    ConnectMessage();
    ConnectMessage(const BoardState& player, const char *n);
    ~ConnectMessage();

    bool send(int socket) const;
    bool receive(int socket);
};

// ServerMessage -------------------------------------------------------------

class ServerMessage : public CommsMessage {
 public:
    GameStatus status;     //!< game status
    double timeRemaining;  //!< time remaining for this player (in seconds)
    int x, y;              //!< opponents last move; (-1, -1) for no move
    Board board;           //!< board position
    BoardState winner;     //!< winner (only valid when status == GAME_END)

 public:
    ServerMessage();
    ServerMessage(GameStatus s, double t, int xx, int yy,
        const Board& b, BoardState w = EMPTY);
    ~ServerMessage();

    bool send(int socket) const;
    bool receive(int socket);
};

// ClientMessage -------------------------------------------------------------

class ClientMessage : public CommsMessage {
 public:
    int x, y;             //!< player's next move; (-1, -1) for no move

 public:
    ClientMessage();
    ClientMessage(int xx, int yy);
    ~ClientMessage();

    bool send(int socket) const;
    bool receive(int socket);
};

#endif
