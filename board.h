/*****************************************************************************
** COMP3130 GROUP PROJECTS IN COMPUTER SCIENCE, SEMESTER 1, 2012
** Copyright (c) 2012, Stephen Gould
** All rights reserved.
**
******************************************************************************
** FILENAME:    board.h
** AUTHOR(S):   Stephen Gould <stephen.gould@anu.edu.au>
**
*****************************************************************************/

#ifndef _BOARD_H_
#define _BOARD_H_

#include <cstdlib>
#include <cassert>
#include <string>
#include <set>

// BoardSize -----------------------------------------------------------------

const int BOARD_SIZE = 10;

// BoardState ----------------------------------------------------------------

typedef enum { EMPTY = 0, WHITE, BLACK, BLOCKED } BoardState;

//! returns the opponent player
inline BoardState getOpponent(const BoardState& player) {
    return (player == WHITE) ? BLACK : WHITE;
}

//! returns string version of board state
inline std::string getString(const BoardState& player) {
    switch (player) {
    case EMPTY: return std::string("EMPTY"); break;
    case WHITE: return std::string("WHITE"); break;
    case BLACK: return std::string("BLACK"); break;
    case BLOCKED: return std::string("BLOCKED"); break;
    }

    return std::string();
}

// Board ---------------------------------------------------------------------

class Board {
 protected:
    //!< holds the state of each board position
    BoardState _state[BOARD_SIZE][BOARD_SIZE];

 public:
    //! create a new board
    Board();
    //! copy constructor
    Board(const Board& board);
    //! destructor
    virtual ~Board();    

    //! read board state from a file
    void read(const char *filename);
    //! write board state to a file
    void write(const char *filename) const;

    //! deserialize board state from memory (buffer must be BOARD_SIZE * BOARD_SIZE)
    void deserialize(const unsigned char *buffer);
    //! serialize board state to memory (buffer must be BOARD_SIZE * BOARD_SIZE)
    void serialize(unsigned char *buffer) const;

    //! return the state at a given position
    const BoardState& getState(int x, int y) const {
        assert((x >= 0) && (x < BOARD_SIZE) && (y >= 0) && (y < BOARD_SIZE));
        return _state[y][x];
    }

    //! make a move (either white or black) returning true
    //! if valid or false if illegal.
    bool makeMove(int x, int y, const BoardState& player);
    
    //! return legal moves for a given player (white or black)
    std::set<std::pair<int, int> > legalMoves(const BoardState& player) const;

    //! return the number of cell positions help by a given player
    int cellCount(const BoardState& state) const;

    //! visualization
    virtual void visualize() const;

 protected:
    //! checks whether a move is legal or not
    bool checkMove(int x, int y, const BoardState& player) const;

    //! check move in a given direction
    bool checkInDirection(int x, int y, int dx, int dy, const BoardState& player) const;

    //! flip pieces along a given direction (assumes move is valid in the direction)
    void flipInDirection(int x, int y, int dx, int dy, const BoardState& player);
};

#endif
