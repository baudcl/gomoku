#include "ia.hpp"

#include "Board.hpp"
#include "Referee.hpp"

#include <iostream>
#include <map>

std::string DEBUG_METHOD_convBase2(unsigned long long v, long base, int length, int start = 0);

//utile pour printer le state, découpe le binaire par paquet de 2 byte pour un meilleur visu
std::string DEBUG_METHOD_convBase2(unsigned long long v, long base, int length, int start)
{
    std::string digits = "0123456789abcdef";
    std::string result;
    int i = 1;
    length += start;

    if((base < 2 || (base > 16)))
    {
        result = "Error: base out of range.";
    }
    else
    {
        do
        {
            if (i > start)
                result = digits[v % base] + result;
            v /= base;
            if (i > start && i % 2 == 0)
                result = " " + result;
            i++;
            if (i > length)
                break;
        }
        while(v);
        while(i <= length)
        {
            if (i > start)
            {
                result = "0" + result;
                if (i % 2 == 0)
                    result = " " + result;
            }
            i++;
        }
    }
    return result;
}

ia::ia(board *board, int iaColor, Referee *ref, int time)
{
    _iaColor = iaColor;
    _currentBoard = board;
    _ref = ref;
    _time = time;
}

int    ia::whereDoYouWanToPlay()
{
    QTime timer;
    board *workingBoard;

    timer.restart();
    workingBoard = _currentBoard->clone();
    std::cout << "AB : Get all possible" << std::endl;
    getAllPossibleNode(workingBoard);
    std::cout << "trouver " << _possibleNode[0].size() << " noeuds" << std::endl;
    _startTime.start();
    int idx = alphaBetaNegaMaxWithMap(INT_MIN, INT_MAX, workingBoard);
    std::cout << "L'ia veut jouer en " << idx << std::endl;
    std::cout << "Temps total : " << timer.elapsed() << std::endl;
    delete workingBoard;

    return (idx);
}

void ia::getAllPossibleNode(board *workingBoard)
{
    bool error;
    int  neighbIndex;
    board	*currentBoard = NULL;

    _possibleNode.clear();
    _possibleNode[0].clear();
    //loop on each nodes
    for (int index = 0; index < 361; ++index)
    {
        currentBoard = workingBoard->clone();
        _nodeInPossibleList[index] = 0;
        //check if it is an empty node
        if (currentBoard->getNodeColor(index) == BOTH_CAN_PLAY)
        {
            //loop on each directions
            for (int direction = 0; direction < 8; ++direction)
            {
                neighbIndex = currentBoard->getNewNodeNumber(index, direction, error);
                if (!error)
                {
                    //check if it isn't an emtpy nodes
                    if (currentBoard->getNodeColor(neighbIndex) != BOTH_CAN_PLAY)
                    {
                        //check if ia can add a nodes here (rules checking)
                        currentBoard->addNode(index, _iaColor);
                        if (_ref->checkRules(index, _iaColor, false, currentBoard))
                        {
                            _possibleNode[0].push_back(index);
                            _nodeInPossibleList[index] = 1;
                        }
                        break;
                    }
                }
            }
        }
        delete currentBoard;
    }
}

void    ia::alphaBetaNegaMaxDeleteMapContent(std::multimap<int, possibilities_mmap> possibilitiesMap)
{
    std::multimap<int, possibilities_mmap>::iterator it, itEnd;

    for (it = possibilitiesMap.begin(), itEnd = possibilitiesMap.end();
        it != itEnd; ++it)
    {
        delete it->second._board;
    }
}

void    ia::alphaBetaNegaMaxSetPossibilitesMap(std::multimap<int, possibilities_mmap> &possibilitiesMap,
                                                board *workingBoard, int depth)
{
    int  currentColor = _iaColor;
    int  mult = 1;

    //pair depth = ia play; impair depth : player play
    if ((depth & 0x1) == 1)
    {
        currentColor = GET_OPPOSITE_COLOR(_iaColor);
        mult = -1;
    }
    for (int j = 0; j <= depth; ++j)
    {
        for (unsigned int i = 0; i < _possibleNode[j].size(); ++i)
        {
            int currentIdx = _possibleNode[j][i];

            if (workingBoard->getNodeColor(currentIdx) == BOTH_CAN_PLAY && _ref->checkRules(currentIdx, currentColor, false, workingBoard))
            {
                board   *currentBoard = workingBoard->clone();
                currentBoard->addNode(currentIdx, currentColor);

                possibilitiesMap.insert(std::pair<int, possibilities_mmap>((mult * (workingBoard->getNodeHeuristic(currentIdx, currentColor) / (depth + 1))), possibilities_mmap(currentIdx, currentBoard)));
            }
        }
    }
}

void    ia::alphaBetaNegaMaxUpdatePossibleNode(int depth)
{
    for (unsigned int l = 0; l < _possibleNode[depth + 1].size(); ++l)
        _nodeInPossibleList[_possibleNode[depth + 1][l]] = 0;
    _possibleNode[depth + 1].clear();
    _possibleNode.erase(depth + 1);
}

int     ia::alphaBetaNegaMaxWithMap(int alpha, int beta, board *workingBoard, int depth, int currentScore)
{
    int                                                         value;
    int                                                         bestValue = INT_MIN;
    int                                                         bestValueIdx = -1;
    std::multimap<int, possibilities_mmap>                      possibilitiesMap;
    std::multimap<int, possibilities_mmap>::reverse_iterator    mapIt, mapItEnd;

    //get the possibilities map
    alphaBetaNegaMaxSetPossibilitesMap(possibilitiesMap, workingBoard, depth);

    //loop on each possibilites node
    for (mapIt = possibilitiesMap.rbegin(), mapItEnd = possibilitiesMap.rend();
        mapIt != mapItEnd; ++mapIt)
    {
        //if MAX_DEPTH we stop the recursion
        if (depth == MAX_DEPTH)
            value = mapIt->first + currentScore;
        else
        {//otherwise continue
            //loop on each direction arount the new node for add new possibilite node
            for (unsigned int k = 0; k < 8; ++k)
            {
                bool error;
                int neighbIndex = mapIt->second._board->getNewNodeNumber(mapIt->second._playedNode, k, error);
                if (!error && mapIt->second._board->getNodeColor(neighbIndex) == BOTH_CAN_PLAY && _nodeInPossibleList[neighbIndex] == 0)
                {
                    _possibleNode[depth + 1].push_back(neighbIndex);
                    _nodeInPossibleList[neighbIndex] = 1;
                }
            }
            //get heuristic value with recursion (negamax)
            value = - alphaBetaNegaMaxWithMap(-beta, -alpha, mapIt->second._board, depth + 1, currentScore + mapIt->first);
        }
        //alpha/beta implementation, supress node that haven't to be explored
        if (value > bestValue)
        {
            bestValue = value;
            bestValueIdx = mapIt->second._playedNode;
            if (value > alpha)
            {
                alpha = value;
                if (alpha >= beta)
                    break;
            }
        }

        if (depth != MAX_DEPTH)
            alphaBetaNegaMaxUpdatePossibleNode(depth);

        if (_startTime.elapsed() == _time)
            break;
    }

    alphaBetaNegaMaxDeleteMapContent(possibilitiesMap);
    if (depth != MAX_DEPTH)
        alphaBetaNegaMaxUpdatePossibleNode(depth);
    if (depth == 0)
        return (bestValueIdx);
    return (bestValue);
}
