#include "Board.hpp"
#include <iostream>
#include <math.h>
#include <string.h>
#include <QTime>

std::string DEBUG_METHOD_convBase(unsigned long long v, long base, int length, int start = 0);

//utile pour printer le state, découpe le binaire par paquet de 2 byte pour un meilleur visu
std::string DEBUG_METHOD_convBase(unsigned long long v, long base, int length, int start)
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

// Constructor
board::board()
{
  this->_board = new unsigned long long[361];
  memset(this->_board, 0, 361 * sizeof(*this->_board));
  resetEatenNode();
  //reset();
}

board::~board()
{
    if (this->_board != NULL)
        delete [] this->_board;
}

void board::resetEatenNode()
{
  eatenNode[0] = eatenNode[1] = 0;
}

board::board(board *board)
{
  this->_board = new unsigned long long[361];
  memcpy(this->_board, board->_board, 361 * sizeof(*this->_board));
  eatenNode[0] = board->eatenNode[0];
  eatenNode[1] = board->eatenNode[1];
}

board *board::clone()
{
    board *clone = new board(this);
    if (clone == NULL)
    {
        std::cout << "Error : not enough memory" << std::endl;
        exit(1);
    }
    return (clone);
}

void board::reset()
{
  delete [] this->_board;
  this->_board = new unsigned long long[361];
  memset(this->_board, 0, 361 * sizeof(*this->_board));
  resetEatenNode();
}

unsigned long long   *board::getBoard()
{
    return this->_board;
}

int     board::getNewNodeNumber(int nodeNumber, int direction, bool &error)
{
    int y = nodeNumber / 19;
    int x = nodeNumber - (y * 19);

    error = false;
    switch (direction)
    {
    case N:
        if (y == 0)
            error = true;
        return (nodeNumber - BOARD_HEIGHT);
    case NE:
        if (y == 0 || x == 18)
            error = true;
        return (nodeNumber - (BOARD_HEIGHT - 1));
    case E:
        if (x == 18)
            error = true;
        return (nodeNumber + 1);
    case SE:
        if (y == 18 || x == 18)
            error = true;
        return (nodeNumber + (BOARD_HEIGHT + 1));
    case S:
        if (y == 18)
            error = true;
        return (nodeNumber + BOARD_HEIGHT);
    case SW:
        if (y == 18 || x == 0)
            error = true;
        return (nodeNumber + (BOARD_HEIGHT - 1));
    case W:
        if (x == 0)
            error = true;
        return (nodeNumber - 1);
    case NW:
        if (y == 0 || x == 0)
            error = true;
        return (nodeNumber - (BOARD_HEIGHT + 1));
    default:
        return -1;
    }
}

void  board::setNodeColor(int nodeNb, int color)
{
    int colorDecal = color << (C * NODE_STATE_NB_BITS);
    this->_board[nodeNb] &= GET_ORIENTATION_MASK(C);
    this->_board[nodeNb] += colorDecal;
}

int    board::getNodeColor(int nodeNb)
{
    return (GET_ORIENTATION_STATE(C, this->_board[nodeNb]));
}

void    board::setNeighbourColor(int nodeNb, int direction, int color)
{
    color = color << (direction * NODE_STATE_NB_BITS);
    this->_board[nodeNb] &= GET_ORIENTATION_MASK(direction);
    this->_board[nodeNb] += color;
}

int     board::getNeighbourColor(int nodeNb, int direction)
{
    return (GET_ORIENTATION_STATE(direction, this->_board[nodeNb]));
}

void    board::setAlignementInformations(int nodeNb, int direction, int howMany, int closedLeft, int closedRight)
{
    if (howMany > 7)// maximum that can be stored in alignement
        howMany = 7;
    long long   information = howMany + (closedLeft << 3)  + (closedRight << 4);
    this->_board[nodeNb] &= GET_ALIGNEMENT_MASK(direction);
    information = (information << (ALIGNEMENT_STATE_FIRST_BIT + (direction * ALIGNEMENT_STATE_NB_BITS)));
    this->_board[nodeNb] += information;
}

void    board::setAlignementInformations(int nodeNb, int direction, const alignementInfo& info)
{
    this->setAlignementInformations(nodeNb, direction, info.nbAlign, info.closedOnLeft, info.closedOnRight);
}

alignementInfo    board::getAlignementInformations(int nodeNb, int direction)
{
    long long   informations;
    alignementInfo  info;

    informations = GET_ALIGNEMENT_STATE(direction, this->_board[nodeNb]);
    info.nbAlign = informations & 0x7;
    info.closedOnLeft = (informations >> 3) & 0x1;
    info.closedOnRight = (informations >> 4) & 0x1;
    return info;
}

void    board::addNode(int nodeNb, int color)
{
    this->setNodeColor(nodeNb, color);
    additionUpdateNeighbour(nodeNb, color);
}

void board::additionUpdateNeighbourState(int currentNodeNb, int color)
{
    bool            error;
    int             neighbIndex;
    bool            closedOnRight = false;
    bool            closedOnLeft = false;
    alignementInfo  currentAlignInfo;
    int             oppositeColor = GET_OPPOSITE_COLOR(color);
    int             currentNodeColor;

    //update current node state
    //loop on each direction
    for (int i = 0; i < 4; ++i)
    {
        int nbAlign = 1;
        //default values
        closedOnLeft = false;
        closedOnRight = false;

        //get the first neighbour node
        neighbIndex = this->getNewNodeNumber(currentNodeNb, i, error);
        if (!error)
        {
            currentNodeColor = this->getNodeColor(neighbIndex);
            //set neighbour color
            this->setNeighbourColor(currentNodeNb, i, currentNodeColor);
            this->setNeighbourColor(neighbIndex, i + 4, color);
            currentAlignInfo = this->getAlignementInformations(neighbIndex, i);
            //if color is the same we add it to nbAlign, otherwise we set the closed flag
            if (currentNodeColor == color)
            {
                nbAlign += currentAlignInfo.nbAlign;
                closedOnRight = (currentAlignInfo.closedOnRight == 1 ? true : false);
            }
            else if (currentNodeColor == oppositeColor)
                closedOnRight = true;
        }

        //do the same with opposite direction
        neighbIndex = this->getNewNodeNumber(currentNodeNb, i + 4, error);
        if (!error)
        {
            currentNodeColor = this->getNodeColor(neighbIndex);
            this->setNeighbourColor(currentNodeNb, i + 4, currentNodeColor);
            this->setNeighbourColor(neighbIndex, i, color);
            currentAlignInfo = this->getAlignementInformations(neighbIndex, i);
            if (currentNodeColor == color)
            {
                nbAlign += currentAlignInfo.nbAlign;
                closedOnLeft = (currentAlignInfo.closedOnLeft == 1 ? true : false);
            }
            else if (currentNodeColor == oppositeColor)
                closedOnLeft = true;
        }
        this->setAlignementInformations(currentNodeNb, i, nbAlign, (int) closedOnLeft, (int) closedOnRight);
    }
}

void board::additionUpdateNeighbourAlignement(int currentNodeNb, int color)
{
    bool            error;
    int             neighbIndex;
    alignementInfo  originAlignInfo, currentAlignInfo;
    int             oppositeColor = GET_OPPOSITE_COLOR(color);
    int             currentNodeColor;

    //loop on each neighbour and set their state
    for (int i = 0; i < 8; ++i)
    {
        neighbIndex = getNewNodeNumber(currentNodeNb, i, error);
        if (!error)
        {
            int     neighbColor = this->getNodeColor(neighbIndex);
            if (neighbColor == oppositeColor)
            { //OPPOSITE COLOR
                currentAlignInfo = this->getAlignementInformations(neighbIndex, i % 4);
                while (!error)
                {
                    currentNodeColor = this->getNodeColor(neighbIndex);
                    if (currentNodeColor == oppositeColor)
                    {
                        if (i < 4)
                            this->setAlignementInformations(neighbIndex, i % 4, currentAlignInfo.nbAlign, 1,currentAlignInfo.closedOnRight);
                        else
                            this->setAlignementInformations(neighbIndex, i % 4, currentAlignInfo.nbAlign, currentAlignInfo.closedOnLeft, 1);
                    }
                    else
                    {
                        break;
                    }
                    neighbIndex = getNewNodeNumber(neighbIndex, i, error);
                }
            }
            else if (neighbColor == color)
            { //SAME COLOR
                originAlignInfo = this->getAlignementInformations(currentNodeNb, i % 4);
                while (!error)
                {
                    currentNodeColor = this->getNodeColor(neighbIndex);
                    if (currentNodeColor == color)
                        this->setAlignementInformations(neighbIndex, i % 4, originAlignInfo);
                    else
                    {
                        break;
                    }
                    neighbIndex = getNewNodeNumber(neighbIndex, i, error);
                }
            }
        }
    }
}

void board::additionUpdateNeighbour(int currentNodeNb, int color)
{
    this->additionUpdateNeighbourState(currentNodeNb, color);
    this->additionUpdateNeighbourAlignement(currentNodeNb, color);
}

bool    board::deleteNode(int pos)
{
    int             nbAlign;
    int             neighbIndex;
    bool            error;
    int             originColor = this->getNodeColor(pos);
    int             currentNodeColor;
    alignementInfo  alignInfo;
    int             oppositeColor = GET_OPPOSITE_COLOR(originColor);

    for (int i = 0; i < 8; ++i)
    {
        //update alignement infos
        nbAlign = 0;
        neighbIndex = this->getNewNodeNumber(pos, i, error);
        if (!error)
        {
            //update neighbour info
            this->setNeighbourColor(neighbIndex, (i + 4) % 8, BOTH_CAN_PLAY);
            currentNodeColor = this->getNodeColor(neighbIndex);
            if (currentNodeColor == originColor)
            { //Same color
                //calculate the alignement
                while (!error)
                {
                    if (this->getNodeColor(neighbIndex) != originColor)
                        break;
                    ++nbAlign;
                    neighbIndex = this->getNewNodeNumber(neighbIndex, i, error);
                }
                neighbIndex = this->getNewNodeNumber(pos, i, error);
                //update the new alignement
                while (!error)
                {
                    if ((currentNodeColor = this->getNodeColor(neighbIndex)) != originColor)
                    {
                        break;
                    }
                    alignInfo = this->getAlignementInformations(neighbIndex, i % 4);
                    if (i < 4)
                        this->setAlignementInformations(neighbIndex, i % 4, nbAlign, 0, alignInfo.closedOnRight);
                    else
                        this->setAlignementInformations(neighbIndex, i % 4, nbAlign, alignInfo.closedOnLeft, 0);
                    neighbIndex = this->getNewNodeNumber(neighbIndex, i, error);
                }

            }
            else if (currentNodeColor == oppositeColor)
            { //opposite color
                //update closed value
                while (!error)
                {
                    if ((currentNodeColor = this->getNodeColor(neighbIndex)) != oppositeColor)
                    {
                        break;
                    }
                    alignInfo = this->getAlignementInformations(neighbIndex, i % 4);
                    if (i < 4)
                        this->setAlignementInformations(neighbIndex, i % 4, alignInfo.nbAlign, 0, alignInfo.closedOnRight);
                    else
                        this->setAlignementInformations(neighbIndex, i % 4, alignInfo.nbAlign, alignInfo.closedOnLeft, 0);
                    neighbIndex = this->getNewNodeNumber(neighbIndex, i, error);
                }
            }
        }
    }
    //delete node informations
    this->_board[pos] = 0;
    return (true);
}

int	board::getEatenNode(int color)
{
  return eatenNode[color];
}

void	board::eatenNodeUp(int color)
{
  eatenNode[color]++;
}
