/*****************************************************************************
** COMP3130 GROUP PROJECTS IN COMPUTER SCIENCE, SEMESTER 1, 2012
** Copyright (c) 2012, Stephen Gould
** All rights reserved.
**
******************************************************************************
** FILENAME:    comms.cpp
** AUTHOR(S):   Stephen Gould <stephen.gould@anu.edu.au>
**
*****************************************************************************/

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <cstring>

#include "comms.h"

using namespace std;

// CommsMessages -------------------------------------------------------------

CommsMessage::CommsMessage()
{
    // do nothing
}

CommsMessage::~CommsMessage()
{
    // do nothing
}

// ConnectMessage ------------------------------------------------------------

const size_t ConnectMessage::MAX_NAME = 1024;

ConnectMessage::ConnectMessage() :
    CommsMessage(), side(-1), name("unknown")
{
    // do nothing
}

ConnectMessage::ConnectMessage(const BoardState& player, const char *n) :
    CommsMessage(), side(-1), name(n)
{
    if (player == WHITE) side = 0;
    else if (player == BLACK) side = 1;
}

ConnectMessage::~ConnectMessage()
{
    // do nothing
}

bool ConnectMessage::send(int socket) const
{
    // encode the message (truncating name if necessary)
    const int nBytes = 2 * sizeof(uint32_t) + std::min(name.size(), MAX_NAME);
    unsigned char *buffer = new unsigned char[nBytes];

    uint32_t n = htonl((uint32_t)side);
    memcpy(&buffer[0], &n, sizeof(uint32_t));

    n = htonl((uint32_t)name.size());
    memcpy(&buffer[sizeof(uint32_t)], &n, sizeof(uint32_t));

    memcpy(&buffer[2 * sizeof(uint32_t)], name.c_str(), std::min(name.size(), MAX_NAME));

    // send the message
    if (write(socket, buffer, nBytes) < 0) {
        cerr << "COMMS: error sending connect message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ConnectMessage::receive(int socket)
{
    // allocate space for the message
    const int nBytes = 2 * sizeof(uint32_t) + MAX_NAME;
    unsigned char *buffer = new unsigned char[nBytes];

    int bytesRead = read(socket, buffer, nBytes);
    if (bytesRead < 0) {
        cerr << "COMMS: error receiving connect message (" << bytesRead << ")" << endl;
        delete[] buffer;
        return false;
    }

    // decode the message
    uint32_t n;
    memcpy(&n, &buffer[0], sizeof(uint32_t));
    side = (int)ntohl(n);
    if ((side != 0) && (side != 1)) {
        cerr << "COMMS: error receiving connect message (invalid side " << side << ")" << endl;
        delete[] buffer;
        return false;
    }

    memcpy(&n, &buffer[sizeof(uint32_t)], sizeof(uint32_t));
    n = ntohl(n);
    if (n > MAX_NAME) {
        cerr << "COMMS: error receiving connect message (invalid name size " << n << ")" << endl;
        delete[] buffer;
        return false;
    }

    name = string((const char *)&buffer[2 * sizeof(uint32_t)], (size_t)n);

    delete[] buffer;
    return true;
}

// ServerMessage -------------------------------------------------------------

ServerMessage::ServerMessage() :
    CommsMessage(), status(ABORT), timeRemaining(0.0), x(-1), y(-1), winner(EMPTY)
{
    // do nothing
}

ServerMessage::ServerMessage(GameStatus s, double t, int xx, int yy,
    const Board& b, BoardState w) : CommsMessage(), status(s), timeRemaining(t),
    x(xx), y(yy), winner(w)
{
    // do nothing
}

ServerMessage::~ServerMessage()
{
    // do nothing
}

bool ServerMessage::send(int socket) const
{
    // encode the message
    const int nBytes = 5 * sizeof(uint32_t) +
        BOARD_SIZE * BOARD_SIZE * sizeof(unsigned char);
    unsigned char *buffer = new unsigned char[nBytes];

    int pos = 0;
    uint32_t n = htonl((uint32_t)status);
    memcpy(&buffer[0], &n, sizeof(uint32_t));
    pos += sizeof(uint32_t);

    n = htonl((uint32_t)(1000 * timeRemaining));
    memcpy(&buffer[pos], &n, sizeof(uint32_t));
    pos += sizeof(uint32_t);

    n = htonl((uint32_t)x);
    memcpy(&buffer[pos], &n, sizeof(uint32_t));
    pos += sizeof(uint32_t);

    n = htonl((uint32_t)y);
    memcpy(&buffer[pos], &n, sizeof(uint32_t));
    pos += sizeof(uint32_t);

    n = htonl((uint32_t)winner);
    memcpy(&buffer[pos], &n, sizeof(uint32_t));
    pos += sizeof(uint32_t);

    board.serialize(&buffer[pos]);    

    // send the message
    if (write(socket, buffer, nBytes) < 0) {
        cerr << "COMMS: error sending server message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ServerMessage::receive(int socket)
{
    // allocate space for the message
    const int nBytes = 5 * sizeof(uint32_t) +
        BOARD_SIZE * BOARD_SIZE * sizeof(unsigned char);
    unsigned char *buffer = new unsigned char[nBytes];

    // receive the message
    int bytesRead = read(socket, buffer, nBytes);
    if (bytesRead != nBytes) {
        cerr << "COMMS: error receiving server message (" << bytesRead << ")" << endl;
        delete[] buffer;
        return false;
    }

    // decode message
    int pos = 0;
    uint32_t n;
    memcpy(&n, &buffer[0], sizeof(uint32_t));
    pos += sizeof(uint32_t);
    status = (GameStatus)ntohl(n);

    memcpy(&n, &buffer[pos], sizeof(uint32_t));
    pos += sizeof(uint32_t);
    timeRemaining = (double)ntohl(n) / 1000.0;

    memcpy(&n, &buffer[pos], sizeof(uint32_t));
    pos += sizeof(uint32_t);
    x = (int)ntohl(n);

    memcpy(&n, &buffer[pos], sizeof(uint32_t));
    pos += sizeof(uint32_t);
    y = (int)ntohl(n);

    memcpy(&n, &buffer[pos], sizeof(uint32_t));
    pos += sizeof(uint32_t);
    winner = (BoardState)ntohl(n);

    board.deserialize(&buffer[pos]);

    delete[] buffer;
    return true;
}

// ClientMessage -------------------------------------------------------------

ClientMessage::ClientMessage() :
    CommsMessage(), x(-1), y(-1)
{
    // do nothing
}

ClientMessage::ClientMessage(int xx, int yy) :
    CommsMessage(), x(xx), y(yy)
{
    // do nothing
}

ClientMessage::~ClientMessage()
{
    // do nothing
}

bool ClientMessage::send(int socket) const
{
    // encode the message
    const int nBytes = 2 * sizeof(uint32_t);
    unsigned char *buffer = new unsigned char[nBytes];

    uint32_t n = htonl((uint32_t)x);
    memcpy(&buffer[0], &n, sizeof(uint32_t));

    n = htonl((uint32_t)y);
    memcpy(&buffer[sizeof(uint32_t)], &n, sizeof(uint32_t));

    // send the message
    if (write(socket, buffer, nBytes) < 0) {
        cerr << "COMMS: error sending client message" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    return true;
}

bool ClientMessage::receive(int socket)
{
    // allocate space for the message
    const int nBytes = 2 * sizeof(uint32_t);
    unsigned char *buffer = new unsigned char[nBytes];

    int bytesRead = read(socket, buffer, nBytes);
    if (bytesRead != nBytes) {
        cerr << "COMMS: error receiving client message (" << bytesRead << ")" << endl;
        delete[] buffer;
        return false;
    }

    // decode the message
    uint32_t n;
    memcpy(&n, &buffer[0], sizeof(uint32_t));
    x = (int)ntohl(n);

    memcpy(&n, &buffer[sizeof(uint32_t)], sizeof(uint32_t));
    y = (int)ntohl(n);

    delete[] buffer;
    return true;
}
