/*****************************************************************************
** COMP3130 GROUP PROJECTS IN COMPUTER SCIENCE, SEMESTER 1, 2012
** Copyright (c) 2012, Stephen Gould
** All rights reserved.
**
******************************************************************************
** FILENAME:    board.cpp
** AUTHOR(S):   Stephen Gould <stephen.gould@anu.edu.au>
**
*****************************************************************************/

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>

#include "board.h"

using namespace std;

// Board ---------------------------------------------------------------------

Board::Board()
{
    // clear the board
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            _state[i][j] = EMPTY;
        }
    }

    // set starting state
    const int mid = BOARD_SIZE/2 - 1;
    _state[mid][mid] = WHITE;
    _state[mid + 1][mid + 1] = WHITE;
    _state[mid + 1][mid] = BLACK;
    _state[mid][mid + 1] = BLACK;

    // add blocked positions
    int blockedCount = 0;
    while (blockedCount != 4) {
        const int x = (int)(drand48() * BOARD_SIZE);
        const int y = (int)(drand48() * BOARD_SIZE);
        if (_state[y][x] == EMPTY) {
            _state[y][x] = BLOCKED;
            blockedCount += 1;
        }
    }
}

Board::Board(const Board& board)
{
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            _state[i][j] = board._state[i][j];
        }
    }
}

Board::~Board()
{
    // do nothing
}

void Board::read(const char *filename)
{
    assert(filename != NULL);
    ifstream ifs(filename);

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {

            // read a non-whitespace character from the filestream
            int ch = (int)' ';
            while (isspace(ch)) {
                ch = ifs.get();
                assert(!ifs.fail());
            }

            // interpret the character
            switch (ch) {
            case (int)'.': _state[i][j] = EMPTY; break;
            case (int)'w': _state[i][j] = WHITE; break;
            case (int)'b': _state[i][j] = BLACK; break;
            case (int)'*': _state[i][j] = BLOCKED; break;
            default: assert(false);
            }
        }
    }

    ifs.close();
}

void Board::write(const char *filename) const
{
    assert(filename != NULL);
    ofstream ofs(filename);

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            switch (_state[i][j]) {
            case EMPTY: ofs << "."; break;
            case WHITE: ofs << "w"; break;
            case BLACK: ofs << "b"; break;
            case BLOCKED: ofs << "*"; break;
            default: assert(false);
            }
        }
        ofs << "\n";
    }

    ofs.close();
}

void Board::deserialize(const unsigned char *buffer)
{
    assert(buffer != NULL);
    unsigned n = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            _state[i][j] = (BoardState)buffer[n++];
        }
    }
}

void Board::serialize(unsigned char *buffer) const
{
    assert(buffer != NULL);
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            *buffer++ = (unsigned char)_state[i][j];
        }
    }
}

bool Board::makeMove(int x, int y, const BoardState& player)
{
    // check that the move is valid
    if (!checkMove(x, y, player))
        return false;

    // make the move
    _state[y][x] = player;

    // flip pieces in each direction
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if ((dx == 0) && (dy == 0)) continue;
            if (checkInDirection(x, y, dx, dy, player)) {
                flipInDirection(x, y, dx, dy, player);
            }
        }
    }

    return true;
}

set<pair<int, int> > Board::legalMoves(const BoardState& player) const
{
    set<pair<int, int> > moves;
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (checkMove(x, y, player)) {
                moves.insert(make_pair(x, y));
            }
        }
    }

    return moves;
}


int Board::cellCount(const BoardState& state) const
{
    int n = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_state[i][j] == state) {
                n += 1;
            }
        }
    }

    return n;
}

void Board::visualize() const
{
    for (int i = 0; i < BOARD_SIZE; i++) {
        cout << " ";
        for (int j = 0; j < BOARD_SIZE; j++) {
            cout << " ";
            switch (_state[i][j]) {
            case EMPTY: cout << "."; break;
            case WHITE: cout << "w"; break;
            case BLACK: cout << "b"; break;
            case BLOCKED: cout << "*"; break;
            default: assert(false);
            }
        }
        cout << "\n";
    }

    cout << endl;
}

bool Board::checkMove(int x, int y, const BoardState& player) const
{
    assert((player == WHITE) || (player == BLACK));
    assert((x >= 0) && (x < BOARD_SIZE) && (y >= 0) && (y < BOARD_SIZE));

    // check if the cell is empty
    if (_state[y][x] != EMPTY) return false;

    // check in each direction
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if ((dx == 0) && (dy == 0)) continue;
            if (checkInDirection(x, y, dx, dy, player)) {
                return true;
            }
        }
    }

    return false;
}

bool Board::checkInDirection(int x, int y, int dx, int dy, const BoardState& player) const
{
    assert((dx != 0) || (dy != 0));

    // get opponent colour
    const BoardState opponent = (player == WHITE) ? BLACK : WHITE;

    // check for line in given direction
    y += dy; x += dx;
    if ((y < 0) || (y >= BOARD_SIZE) ||
        (x < 0) || (x >= BOARD_SIZE) ||
        (_state[y][x] != opponent)) {
        return false;
    }

    while ((y >= 0) && (y < BOARD_SIZE) && (x >= 0) && (x < BOARD_SIZE)) {
        if (_state[y][x] == player) return true;
        if (_state[y][x] != opponent) break;
        y += dy; x += dx;
    }

    return false;
}

void Board::flipInDirection(int x, int y, int dx, int dy, const BoardState& player)
{
    y += dy; x += dx;
    while ((y >= 0) && (y < BOARD_SIZE) && (x >= 0) && (x < BOARD_SIZE)) {
        if (_state[y][x] == player) break;
        _state[y][x] = player;
        y += dy; x += dx;
    }
}
