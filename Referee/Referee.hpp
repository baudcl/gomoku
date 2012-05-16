#ifndef __H_REFEREE__

#define __H_REFEREE__

#include <QDebug>
#include "Board.hpp"
#include <QObject>
#include <vector>

#define ALIGNED_NODE_WIN 5
#define THREE_ALIGN_NB 3

#define BOARD_WIDTH 19
#define BOARD_HEIGHT 19

#define NODE_NB(NB) NB << "(" << NB / BOARD_WIDTH << ";" << NB - ((NB / BOARD_WIDTH) * BOARD_HEIGHT) << ")"

class Referee: public QObject
{
    Q_OBJECT

public:
  Referee(QObject *parent = 0);
  /** hmi request a new stone, check if it is possible put it here */
  bool setStone(int, int);

  board *getBoard();


    /** Check if a new node respect enabled rules */
    bool checkRules(int pos, int color, bool dispHmi = true, board * otherBoard = NULL);


public slots:
    void	getPosition(int);

  /** enable/disable double three rule */
  void setDoubleThreeRule(bool b);
  /** enable/disable eating to finish game */
  void setEatingAtEndForWin(bool b);
  /** Reset board **/
  void reset();

signals:
  /** signal corresponding to the addition of a node */
  void  s_newPawl(int, int);
  /** signal corresponding to the deletion of a node */
  void  s_deletePawl(int);
  /** send text message to the hmi */
  void  s_sendHmiText(const QString &msg);
  /** notify to the hmi that a player(color) has win */
  void  s_notifyVictory(int color);

private:
    /** enable/disable double three free rule */
    bool doubleThreeFreeEnable;
    /** enable/disable eating at end of the game to win the game */
    bool eatAtEndEnable;

    /** Board where all the the board informations are stored */
    board *plateau;
  int _time;

    /** for the rule that check if winning alignement can be eaten */
    std::vector<int>    winningAlignements;

    /** Check if a node eath other nodes, if it is the case other nodes are deleted */
    bool checkIfNewNodeEat(int pos, int color);

    /** Check if this node make win */
    bool	checkWinByAlignement(int pos, int color);
    bool    checkIfWinningAlignementCanBeEaten(int pos, int color, int alignementDirection);


    /** Double three free alignement methods **/

    /** Check if there is a double three free alignement */
    bool    checkForDoubleThreeAlign(int nodeIndex, int currentColor);

    /** With a double three alignement, check if there is one other (first alignement without hole)*/
    bool    searchForNewAlignement(int firstNode, int parcourDirection, int currentColor);
    /** With a double three alignement, check if there is one other (first alignement with hole)*/
    bool    searchForNewAlignementWithHole(int firstNodeIndex, int parcourDirection, int currentColor);
    /** Check if there is a three free alignement */
    bool    checkForThreeFreeAlign(int nodeIdx, int direction, int color, int *lastOne, bool &emptyOne);
    /** Go to the extremity of an alignement and check if it is a three free */
    bool    goToOneExtremity(int *originIdx, int direction, int color, bool &empty);
};

#endif
