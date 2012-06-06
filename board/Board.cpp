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
                        //if empty node, reset heuristic
                        if (currentNodeColor == BOTH_CAN_PLAY)
                        {
                            this->setNodeHeuristic(neighbIndex, color, 0);
                            this->setNodeHeuristic(neighbIndex, oppositeColor, 0);
                        }
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
                        //if empty node, reset heuristic
                        if (currentNodeColor == BOTH_CAN_PLAY)
                        {
                            this->setNodeHeuristic(neighbIndex, color, 0);
                            this->setNodeHeuristic(neighbIndex, oppositeColor, 0);
                        }
                        break;
                    }
                    neighbIndex = getNewNodeNumber(neighbIndex, i, error);
                }
            }
            else
            {//NO NODE ON THIS CASE
                //reset heuristic value of the siblings nodes
                this->setNodeHeuristic(neighbIndex, color, 0);
                this->setNodeHeuristic(neighbIndex, oppositeColor, 0);
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
                        //if empty node node, reset heuristic score
                        if (currentNodeColor == BOTH_CAN_PLAY)
                        {
                            this->setNodeHeuristic(neighbIndex, originColor, 0);
                            this->setNodeHeuristic(neighbIndex, oppositeColor, 0);
                        }
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
                        //if empty node node, reset heuristic score
                        if (currentNodeColor == BOTH_CAN_PLAY)
                        {
                            this->setNodeHeuristic(neighbIndex, originColor, 0);
                            this->setNodeHeuristic(neighbIndex, oppositeColor, 0);
                        }
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
            else
            {//NO NODE ON THIS CASE
                //reset heuristic value of the siblings nodes
                this->setNodeHeuristic(neighbIndex, originColor, 0);
                this->setNodeHeuristic(neighbIndex, oppositeColor, 0);
            }
        }
    }
    //delete node informations
    this->_board[pos] = 0;
    return (true);
}

void    board::DEBUG_METHOD_print_node(int nodeNb, bool travesal)
{
    alignementInfo info;
    travesal = true;

    std::cout << "Information sur le noeud " << nodeNb << " : il est de couleur " << this->getNodeColor(nodeNb) << std::endl;
    std::cout << "Heuristic B " << (int) this->getNodeHeuristic(nodeNb, BLACK, false) << " W " << (int) this->getNodeHeuristic(nodeNb, WHITE, false) << std::endl;
    for (int i = 0; i < 4; ++i)
    {
        info = this->getAlignementInformations(nodeNb, i);
        std::cout << "Alignement en " << i << " nb : " << info.nbAlign << " closed : R " << info.closedOnRight << " L " << info.closedOnLeft << std::endl;
    }

    //We want to have a human readable output :
    // NW N NE
    // W C E
    // SW S SE
    int orderIdx[] = {7, 0, 1,
                      6,/*C*/2,
                      5, 4, 3};

    for (unsigned int k = 0; k < 8; ++k)
    {
        if (k == 3 || k == 5)
            std::cout << std::endl;
            //std::cout << (k - 1) % 8 << std::endl;
        switch (this->getNeighbourColor(nodeNb, orderIdx[k]))
        {
            case BLACK:
                std::cout << "b ";
                break;
            case WHITE:
                std::cout << "w ";
                break;
            case BOTH_CAN_PLAY:
                std::cout << "0 ";
                break;
            default:
                std::cout << "? ";
        }
        if (k == 3)
        {
            switch (this->getNodeColor(nodeNb))
            {
                case BLACK:
                    std::cout << "b ";
                    break;
                case WHITE:
                    std::cout << "w ";
                    break;
                case BOTH_CAN_PLAY:
                    std::cout << "0 ";
                    break;
                default:
                    std::cout << "? ";
            }
        }
    }
    std::cout << std::endl;
}

int	board::getEatenNode(int color)
{
  return eatenNode[color];
}

void	board::eatenNodeUp(int color)
{
  eatenNode[color]++;
}


unsigned short    board::calculateNodeHeuristic(int pos, int color)
{
  alignementInfo info;
  int dir = 0;
  int finalHeur = 0;
  int heur;
  bool error;
  int dirCpt[4];

  while (dir < 4)
    {
      heur = 0;
      info = getAlignementInformations(pos, dir);
      dir++;
      if (info.nbAlign >= 5)
	  return 200;
      if (info.nbAlign > 1)
  	heur = pow(info.nbAlign, 2);
      if (!info.closedOnLeft && !info.closedOnRight)
  	heur += heur / 2;
      if (info.closedOnLeft && info.closedOnRight)
  	heur -= 20;
      if (info.closedOnLeft || info.closedOnRight)
	heur -= 10;
      if (!info.nbAlign)
       	heur -= 5;
      if (info.nbAlign - 1)
       	heur += 5;
      finalHeur += heur;
    }
  dir = dirCpt[0] = dirCpt[1] = dirCpt[2] = dirCpt[3] = 0;
  while (dir < 8)
    {
      heur = 0;
      int idx = getNewNodeNumber(pos, dir, error);
      if (!error && getNodeColor(idx) == (GET_OPPOSITE_COLOR(color)))
	{
	  info = getAlignementInformations(idx, dir % 4);
	  dirCpt[dir % 4] += info.nbAlign;
	  switch (info.nbAlign)
	    {
	    case 2:
	      if (info.closedOnRight && info.closedOnLeft)
		{
		  heur = pow((eatenNode[GET_N_PLAYER(color)] * 2) + 2, 5);
		  heur *= 2;
		}
	      break;
	    case 3:
	      if (info.closedOnRight || info.closedOnLeft)
		heur = 30;
	      else
		heur = 50;
	      break;
	    case 4:
	      heur = 100;
	      break;
	    }
	  finalHeur += heur;
	}
      dir++;
    }
  dir = 0;
  while (dir < 4)
    {
      heur = 0;
      heur = pow(dirCpt[dir], 5);
      heur += heur / 2;
      finalHeur += heur;
      dir++;
    }
  if (finalHeur <= 0)
    return 1;
  return finalHeur + 1;
}

void board::setNodeHeuristic(int pos, int color, unsigned short heuristic)
{
    int colorIdx = GET_N_PLAYER(color);
    if (heuristic > 8191)
        heuristic = 8191;
    long long lheur = ((long long) heuristic) << (HEURISTIC_FIRST_BIT + (colorIdx * HEURISTIC_NB_BITS));

    this->_board[pos] &= GET_HEURISTIC_MASK(colorIdx);
    this->_board[pos] += lheur;
}

unsigned short board::getNodeHeuristic(int pos, int color, bool calculate)
{
    int colorIdx = GET_N_PLAYER(color);
    unsigned short heuristic = GET_HEURISTIC_STATE(colorIdx, this->_board[pos]);
    if (heuristic == 0 && calculate)
    {
        heuristic = this->calculateNodeHeuristic(pos, color);
        this->setNodeHeuristic(pos, color, heuristic);
    }
    return heuristic;
}

