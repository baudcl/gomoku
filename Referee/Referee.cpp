#include "Referee.hpp"
#include <sstream>
#include "ia.hpp"

//DEBUG
#include <iostream>

Referee::Referee(QObject *parent) : QObject(parent)
{
  //by default the 2 optionnal rules are enable
  this->doubleThreeFreeEnable = true;
  this->eatAtEndEnable = true;
  this->plateau = new board();
  //ia = new ia(&plateau, WHITE);
  _time = 10;
}

void Referee::iaTime10()
{
  _time = 10;
}

void Referee::iaTime20()
{
  _time = 20;
}

void Referee::iaTime50()
{
  _time = 50;
}

void    Referee::setDoubleThreeRule(bool b)
{
  this->doubleThreeFreeEnable = b;
}

void    Referee::setEatingAtEndForWin(bool b)
{
  this->eatAtEndEnable = b;
}

board   *Referee::getBoard()
{
  return (this->plateau);
}

void Referee::reset()
{
  plateau->reset();
  for (int i = 0; i < 361; ++i)
    {
      emit s_deletePawl(i);
    }
}

void Referee::getPosition(int pos)
{
  qDebug() << "pos :" << pos;
}

bool Referee::setStone(int pos, int color)
{
    std::ostringstream hmiText;

    //check if requested cqse is empty
    if (plateau->getNodeColor(pos) == BOTH_CAN_PLAY)
    {
        //because it is more simple, we add the node in memory
        plateau->addNode(pos, color);
        //check if it is respecting rules
        if (checkRules(pos, color))
        {
            //rules : ok, check if this node eat others nodes
            checkIfNewNodeEat(pos, color);
            //send the signal that notifiate to the hmi new pawl are set
            emit s_newPawl(pos, color);
            hmiText << "Ajout du pion " << NODE_NB(pos) << " pour le joueur " << GET_N_PLAYER(color) + 1;
            QString s = QString::fromStdString(hmiText.str());
            emit s_sendHmiText(s);
            //check if the new node make the player win
            emit s_newPawl(pos, color);
            //check if this node make win by alignement
            if (checkWinByAlignement(pos, color))
            {
                std::ostringstream victText;

                victText << "Fin de la partie, le joueur " << GET_N_PLAYER(color) + 1 << " a gagné par alignement de 5 !";
                s = QString::fromStdString(victText.str());
                emit s_sendHmiText(s);
                emit s_notifyVictory(color);
		return true;
            }
            //check if this node make the player win by the number of eaten nodes
            if (plateau->getEatenNode(GET_N_PLAYER(color)) >= 5)
            {
                std::ostringstream victMsg;

                victMsg << "Fin de la partie, le joueur " << GET_N_PLAYER(color) + 1 << " a gagné en mangeant 10 pions à l'adversaire !";
                s = QString::fromStdString(victMsg.str());
                emit s_sendHmiText(s);
                emit s_notifyVictory(color);
		return true;
            }
            //Check if in the list that stores node with a 5 alignements situation changed
            if (!winningAlignements.empty())
            {
                std::vector<int>::iterator  it, itEnd;
                for(it = winningAlignements.begin(), itEnd = winningAlignements.end();
                    it != itEnd;)
                {
                    if (pos != *it && checkWinByAlignement(*it, plateau->getNodeColor(*it)))
                    {
                        std::ostringstream victText;

                        victText << "Fin de la partie, le joueur " << GET_N_PLAYER(*it) + 1 << " a gagné par alignement de 5 !";
                        s = QString::fromStdString(victText.str());
                        emit s_sendHmiText(s);
                        emit s_notifyVictory(*it);
                        break;
                    }
                    else
                    {//check if there is always a five alignement
                        bool fiveAlign = false;
                        for (int i = 0; i < 4; ++i)
                        {
                            alignementInfo info = plateau->getAlignementInformations(*it, i);
                            if (info.nbAlign >= 5)
                            {
                                fiveAlign = true;
                                break;
                            }
                        }
                        if (!fiveAlign)
                        {//if there isn't we delete the node
                            std::vector<int>::iterator  tmp = it;
                            it++;
                            winningAlignements.erase(tmp);
                        }
                        else
                            ++it;
                    }
                }
            }
	    if (color == BLACK)
	      emit iaPlay();
            return (true);
        }
        else
        {
            plateau->deleteNode(pos);
            return (false);
        }
    }
    else
    {
        plateau->DEBUG_METHOD_print_node(pos, true);
        hmiText << "il existe déjà un noeud a la position " << pos;
        QString s = QString::fromStdString(hmiText.str());
        emit s_sendHmiText(s);
    }
    return (false);
}

bool Referee::checkRules(int pos, int color, bool dispHmi, board *otherBoard)
{
    std::ostringstream  hmiText;
    board             * oldBoard = NULL;
    bool                rulesOk = true;

    if (otherBoard != NULL)
    {
        oldBoard = this->plateau;
        this->plateau = otherBoard;
    }
    if (doubleThreeFreeEnable)
    {
        if (this->checkForDoubleThreeAlign(pos, color))
        {
            if (dispHmi)
            {
                hmiText << "Impossible de placer le pion ici, il ne respecte pas la règle des doubles 3 libre";
                QString s = QString::fromStdString(hmiText.str());
                emit s_sendHmiText(s);
            }
            rulesOk = false;
            goto end;
        }

    }
    end:
    if (otherBoard != NULL)
        this->plateau = oldBoard;
    return (rulesOk);
}

bool Referee::checkIfNewNodeEat(int pos, int color)
{
    int                 oppositeColor = GET_OPPOSITE_COLOR(color);
    bool                error;
    alignementInfo      alignInfo;
    int                 neighbIndex;
    bool                eat = false;
    std::ostringstream  hmiText;

    for (int i = 0; i < 8; ++i)
    {
        if (plateau->getNeighbourColor(pos, i) == oppositeColor)
        {
            neighbIndex = plateau->getNewNodeNumber(pos, i, error);
            alignInfo = plateau->getAlignementInformations(neighbIndex, i % 4);
            if (alignInfo.nbAlign == 2 && alignInfo.closedOnLeft == 1 && alignInfo.closedOnRight == 1)
            {
                plateau->deleteNode(neighbIndex);
                emit s_deletePawl(neighbIndex);

                hmiText << "Le joueur " << GET_N_PLAYER(color) + 1 << " mange les pions " << neighbIndex << " et ";
                neighbIndex = plateau->getNewNodeNumber(neighbIndex, i, error);
                hmiText << neighbIndex << "du joueurs adverse.";

                plateau->deleteNode(neighbIndex);
                emit s_deletePawl(neighbIndex);

                QString s = QString::fromStdString(hmiText.str());
                emit s_sendHmiText(s);

                plateau->eatenNodeUp(GET_N_PLAYER(color));
                eat = true;
            }
        }
    }
    return eat;
}

bool Referee::checkWinByAlignement(int pos, int color)
{
    alignementInfo      alignInfo;
    std::ostringstream  hmiText;

    for (int i = 0; i < 4; ++i)
    {
        alignInfo = plateau->getAlignementInformations(pos, i);
        if (alignInfo.nbAlign >= 5)
        {
            if (eatAtEndEnable && checkIfWinningAlignementCanBeEaten(pos, color, i))
            {
                hmiText << "Le joueur " << GET_N_PLAYER(color) + 1 << " a fait un alignement de 5, mais il contient des noeuds qui peuvent être mangé, la partie n'est pas terminé !";
                QString s = QString::fromStdString(hmiText.str());
                emit s_sendHmiText(s);
                bool    find = false;
                for (int i = 0; i < (int) winningAlignements.size(); ++i)
                    if (winningAlignements[i] == pos)
                        find = true;
                if (!find)
                    winningAlignements.push_back(pos);
                return (false);
            }
            else
                return (true);
        }
    }
    return (false);
}

bool    Referee::checkIfWinningAlignementCanBeEaten(int pos, int color, int alignementDirection)
{
    int             currentNodeIdx = pos;
    bool            error = false;
    alignementInfo  alignInfo;
    int             oppositeAlignementDirection = (alignementDirection + 4) % 8;

    while (plateau->getNeighbourColor(currentNodeIdx, alignementDirection) == color)
    {
        for (int i = 0; i < 3; ++i)
        {
            int currentAlignementDirection = alignementDirection + 1 + i;

            alignInfo = plateau->getAlignementInformations(currentNodeIdx, currentAlignementDirection % 4);
            if (alignInfo.nbAlign == 2 &&
                ((alignInfo.closedOnLeft &&  !alignInfo.closedOnRight) ||
                 (!alignInfo.closedOnLeft &&  alignInfo.closedOnRight)))
                 return (true);
        }
        currentNodeIdx = plateau->getNewNodeNumber(currentNodeIdx, alignementDirection, error);
        if (error)
            break;
    }

    currentNodeIdx = pos;
    while (plateau->getNeighbourColor(currentNodeIdx, oppositeAlignementDirection) == color)
    {
        for (int i = 0; i < 3; ++i)
        {
            int currentAlignementDirection = alignementDirection + 1 + i;

            alignInfo = plateau->getAlignementInformations(currentNodeIdx, currentAlignementDirection % 4);
            if (alignInfo.nbAlign == 2 &&
                ((alignInfo.closedOnLeft &&  !alignInfo.closedOnRight) ||
                 (!alignInfo.closedOnLeft &&     alignInfo.closedOnRight)))
                 return (true);
        }
        currentNodeIdx = plateau->getNewNodeNumber(currentNodeIdx, oppositeAlignementDirection, error);
        if (error)
            break;
    }
    return (false);
}

bool    Referee::goToOneExtremity(int *originIdx, int direction, int color, bool &empty)
{
    int     oppositeColor = GET_OPPOSITE_COLOR(color);
    int     i = 0;
    int     tmpIdx = *originIdx;
    int     tmpColor = color;
    bool    error;

    empty = false;
    //go to the extremity
    while (i <= 4)
    {
        tmpColor = plateau->getNodeColor(tmpIdx);

        if (tmpColor == oppositeColor)
        {
            return (false);
        }
        else if (tmpColor == BOTH_CAN_PLAY)
        {
            if (!empty)
            {
                if (i == 3)
                    return (true);
                empty = true;
            }
            else
            {
                if (i == 4)
                    return (true);
                return (false);
            }
        }
        else
            *originIdx = tmpIdx;

        tmpIdx = plateau->getNewNodeNumber(tmpIdx, direction, error);
        if (error)
            return (false);
        ++i;
    }
    return (false);
}

bool    Referee::checkForThreeFreeAlign(int nodeIdx, int direction, int color, int *lastOne, bool &emptyOne)
{
    int     tmpIndex = nodeIdx;
    int     tmpColor = color;
    int     oppositeColor = GET_OPPOSITE_COLOR(color);
    int     oppositeDirection = (direction + 4) % 8;
    bool    empty = false;
    bool    idxError, idxError2;

    if (plateau->getNodeColor(nodeIdx) == BOTH_CAN_PLAY)
        return (false);
    //go to the extremity
    goToOneExtremity(&tmpIndex, direction, color, empty);

    //the next node in "direction" direction has an opposite color so it isn't free, we return and the node just before the empty one
    if (plateau->getNeighbourColor(tmpIndex, direction) == oppositeColor && empty)
    {
        //(XBXX we are on the first X and we want to go on the second X)
        while (42)
        {
            //break is used to go out the while
            tmpColor = plateau->getNeighbourColor(tmpIndex, oppositeDirection);
            if (tmpColor == BOTH_CAN_PLAY)
            {
                //we are on the blanc (not alloc) so we work with index
                //we want to jump the not alloc node
                tmpIndex = plateau->getNewNodeNumber(plateau->getNewNodeNumber(tmpIndex, oppositeDirection, idxError), oppositeDirection, idxError2);
                if (idxError || idxError2)
                    return (false);
                break;
            }
            tmpIndex = plateau->getNewNodeNumber(tmpIndex, oppositeDirection, idxError);
            if (idxError)
                return false;
        }
    }
    //OK here we are on the first extremity
    //check if the next node hasn't the opposite color
    if (plateau->getNeighbourColor(tmpIndex, direction) == oppositeColor)
        return (false);//the alignement isn't free...

    //now we want to go on the other extremity and verify if it is a free alignement
    empty = false;
    if (goToOneExtremity(&tmpIndex, oppositeDirection, color, empty) == false)
        return (false);
    emptyOne = empty;
    //OK here we are on the second extremity
    //check if the next node hasn't the opposite color
    if (plateau->getNeighbourColor(tmpIndex, oppositeDirection) == oppositeColor)
        return (false);//the alignement isn't free...
    //set the lastOne node
    if (lastOne != NULL)
        *lastOne = tmpIndex;
    return true;
}

bool    Referee::searchForNewAlignementWithHole(int firstNodeIndex, int parcourDirection, int currentColor)
{
    int     tmpDirection;
    int     tmpIdx;
    int     currentNodeIdx;
    bool    idxError;
    bool    emptyOne;

    //we loop on each other direction
    for (unsigned int j = 0; j < 3; ++j)
    {
        currentNodeIdx = firstNodeIndex;
        //new direction
        tmpDirection = j + parcourDirection + 1;
        for (unsigned k = 0; k < 4; ++k)
        {
            //get the first sibling node in tmpDirection from the currentIndex (with tmp = null so we use index)
            tmpIdx = plateau->getNewNodeNumber(currentNodeIdx, tmpDirection, idxError);
            if (!idxError)
                if (checkForThreeFreeAlign(tmpIdx, tmpDirection, currentColor, NULL, emptyOne))
                    return (true);
            //get the first sibling node in tmpDirection opposite direction from the currentIndex (with tmp = null so we use index)
            tmpIdx = plateau->getNewNodeNumber(currentNodeIdx, (tmpDirection + 4) % 8, idxError);
            if (!idxError)
                if (checkForThreeFreeAlign(tmpIdx, tmpDirection, currentColor, NULL, emptyOne))
                    return (true);

            currentNodeIdx = plateau->getNewNodeNumber(currentNodeIdx, parcourDirection, idxError);
            if (idxError) //normally never go here
                break;
            //get the next node
        }
    }
    // no one alignement founded
    return (false);
}

bool    Referee::searchForNewAlignement(int firstNode, int parcourDirection, int currentColor)
{
    int     tmpDirection;
    int     currentNodeIdx;
    int     currentNodeColor;
    bool    idxError;
    bool    emptyOne;
    int     tmpIdx;

    //we loop on the three others direction
    for (unsigned int j = 0; j < 3; ++j)
    {
        //new direction
        tmpDirection = j + parcourDirection + 1;
        //get his index, but using mathematic and not getNodeNumber on tmp because tmp can be null...
        currentNodeIdx = plateau->getNewNodeNumber(firstNode, (parcourDirection + 4) % 8, idxError);
        if (idxError)
        {
            //get the next index
            currentNodeIdx = plateau->getNewNodeNumber(currentNodeIdx, parcourDirection, idxError);
            //getNewNodeNumber return us an error because currentNodeIdx isn't valid otherwise we will not be here...
            idxError = false;
            //check if it is valid, we cannot use idxError because we start with a not valid node index
            if (currentNodeIdx < 0 || currentNodeIdx >= 361)
                continue ;//isn't valid, go out
        }

        for (unsigned int k = 0; k < 5; ++k)
        {
            currentNodeColor = plateau->getNodeColor(currentNodeIdx);
            if (currentNodeColor == BOTH_CAN_PLAY)
            {
                //get the first sibling node in tmpDirection from the currentIndex (with tmp = null so we use index)
                tmpIdx = plateau->getNewNodeNumber(currentNodeIdx, tmpDirection, idxError);
                if (!idxError)
                    if (checkForThreeFreeAlign(tmpIdx, tmpDirection, currentColor, NULL, emptyOne))//check if there is an alignement
                        return (true);
                //get the first sibling node in tmpDirection opposite direction from the currentIndex (with tmp = null so we use index)
                tmpIdx = plateau->getNewNodeNumber(currentNodeIdx, (tmpDirection + 4) % 8, idxError);
                if (!idxError)
                    if (checkForThreeFreeAlign(tmpIdx, tmpDirection, currentColor, NULL, emptyOne))//check if there is an alignement
                        return (true);
            }
            else
            {
                if (checkForThreeFreeAlign(currentNodeIdx, tmpDirection, currentColor, NULL, emptyOne))
                    return (true);
            }
            //get the next node index
            currentNodeIdx = plateau->getNewNodeNumber(currentNodeIdx, parcourDirection, idxError);
            if (idxError) //normally never go here
                break;
        }
    }
    //no one alignement founded.
    return (false);
}

bool    Referee::checkForDoubleThreeAlign(int nodeIndex, int currentColor)
{
    int     lastNodeIndex;
    bool    emptyOne;

    //search if there is an alignement on each direction (search on both side E = W)
    for (unsigned int i = 0; i < 4; ++i)
    {
        if (checkForThreeFreeAlign(nodeIndex, i, currentColor, &lastNodeIndex, emptyOne))
        {
            //We found an alignement in "i" direction
            if (emptyOne)
            {
                //if that alignement contains a hole (XXBX|XBXX)
                //search if there is an other alignement traversing the current alignement
                if (searchForNewAlignementWithHole(lastNodeIndex, i, currentColor))
                    return (true);
            }
            else
            {
                //that aligment doesn't contain hole (BXXXB)
                //search if there is an other alignement traversing the current alignement
                if (searchForNewAlignement(lastNodeIndex, i, currentColor))
                    return (true);
            }
        }
    }
    return (false);
}

void Referee::iaCanPlay(int player)
{
  ia *iaaa = new ia(plateau, player, this, _time);
  setStone(iaaa->whereDoYouWanToPlay(), player);
}
