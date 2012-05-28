/*****************************************************************************
** COMP3130 GROUP PROJECTS IN COMPUTER SCIENCE, SEMESTER 1, 2012
** Copyright (c) 2012, Stephen Gould
** All rights reserved.
**
******************************************************************************
** FILENAME:    leader.h
** AUTHOR(S):   Stephen Gould <stephen.gould@anu.edu.au>
**
*****************************************************************************/

#ifndef _LEADER_H_
#define _LEADER_H_

#include <map>
#include <string>

using namespace std;

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

#endif
