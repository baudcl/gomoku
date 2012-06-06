#ifndef __GOMOKU_IA_H__
#define __GOMOKU_IA_H__

#include <vector>
#include <map>
#include <QTime>

//MUST BE PAIR
#define MAX_DEPTH 4

class Referee;
class board;

typedef struct s_possibilities_mmap
{
    int     _playedNode;
    board   *_board;

    s_possibilities_mmap(int pn, board *b) : _playedNode(pn), _board(b) {}
}               possibilities_mmap;

class   ia
{
public:
  ia(board *board, int color, Referee *r, int time);

    int    whereDoYouWanToPlay();

private:
    board                              *_currentBoard;
    Referee                            *_ref;
    std::map<int, std::vector<int> >    _possibleNode;
    QTime                               _startTime;
    int                                 _nodeInPossibleList[361];
    int                                 _iaColor;
  int _time;

    void    getAllPossibleNode(board *workingBoard);

    void    alphaBetaNegaMaxDeleteMapContent(std::multimap<int, possibilities_mmap> possibilies);
    void    alphaBetaNegaMaxSetPossibilitesMap(std::multimap<int, possibilities_mmap> &possibilitiesMap,
                                                    board *workingBoard, int depth);
    void    alphaBetaNegaMaxUpdatePossibleNode(int depth);
    int     alphaBetaNegaMaxWithMap(int alpha, int beta, board *workingBoard, int depth = 0, int currentScore = 0);

};

#endif
