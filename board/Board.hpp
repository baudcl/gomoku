#ifndef __H_BOARD__

#define __H_BOARD__

#include <limits.h>
#include <QDebug>

#define BOTH_CAN_PLAY 0x00         // 00
#define WHITE 0x01                  // 01
#define BLACK 0x02                  // 10

#define BLACK_CAN_PLAY 0x01         // 001
#define WHITE_CAN_PLAY 0x02         // 010
#define BLACK_CAN_BE_EATEN 0x05     // 101
#define WHITE_CAN_BE_EATEN 0x07     // 111
#define NO_ONE_CAN_BE_EATEN 0x06    // 110

#define GET_OPPOSITE_COLOR(COLOR) (0x3 ^ COLOR)

#define ALIGNEMENT_FREE 0x0
#define ALIGNEMENT_CLOSED 0x1

#define N  0x00 // North
#define NE 0x01
#define E  0x02 // East
#define SE 0x03
#define S  0x04 // South
#define SW 0x05
#define W  0x06  // West
#define NW 0x07
#define C  0x08  // center position

#define NB_ORIENTATION 0x08 // 2 * 10
#define NB_NODE_ALIGNEMENT 0x4 // 4 (N NE E SE)

#define NODE_STATE_SIZE_BIN 0x3 // 11 = 0x3
#define NODE_STATE_NB_BITS 0x2 //2 bits per direction

#define ALIGNEMENT_STATE_FIRST_BIT ((NB_ORIENTATION + 1) * NODE_STATE_NB_BITS) //first bit of the alignement bits
#define ALIGNEMENT_STATE_NB_BITS 0x5 // 5 bits per alignement informations (3 for alignement + 2 for closed on left/right)
#define ALIGNEMENT_STATE_SIZE_BIN 0x1F // 11111

//#define MAX_ORIENTATION_NB 0x7ffffff  // 111 111 111 111 111 111 111 111 111
//#define MAX_ALIGNEMENT_NB 0xfffff

#define MASK_TOTAL ((long long) ULLONG_MAX)

#define GET_ORIENTATION_STATE(POS, STATE) ((((long long) STATE) >> (NODE_STATE_NB_BITS * POS)) & NODE_STATE_SIZE_BIN)
#define GET_ORIENTATION_MASK(POS) (MASK_TOTAL ^ (NODE_STATE_SIZE_BIN << (NODE_STATE_NB_BITS * POS)))

#define GET_ALIGNEMENT_STATE(DIR, STATE) ((((long long) STATE) >> ((DIR * ALIGNEMENT_STATE_NB_BITS) + ALIGNEMENT_STATE_FIRST_BIT) & ALIGNEMENT_STATE_SIZE_BIN))
#define GET_ALIGNEMENT_MASK(DIR) (MASK_TOTAL ^ (((long long) ALIGNEMENT_STATE_SIZE_BIN) << ((ALIGNEMENT_STATE_NB_BITS * DIR) + ALIGNEMENT_STATE_FIRST_BIT)))

#define GET_N_PLAYER(COLOR) (0x1 & COLOR)

#define BOARD_WIDTH 19
#define BOARD_HEIGHT 19

typedef struct  s_alignement_informations
{
    int     nbAlign;
    int     closedOnLeft;
    int     closedOnRight;
}               alignementInfo;

class board
{
public:
    // Methods
    board();
    ~board();
    board(board *board);
    unsigned long long *getBoard();
    board   *clone();

    int     getNodeColor(int nodeNb);
    void    setNodeColor(int nodeNb, int color);

    int     getNeighbourColor(int nodeNb, int direction);
    void    setNeighbourColor(int nodeNb, int direction, int color);

    alignementInfo      getAlignementInformations(int nodeNb, int direction);
    void                setAlignementInformations(int nodeNb, int direction, int howMany, int closedLeft, int closedRight);
    void                setAlignementInformations(int nodeNb, int direction, const alignementInfo& info);

    void    addNode(int nodeNb, int color);

    int     getNewNodeNumber(int nodeNumber, int direction, bool &error);

    /** delete a node */
    bool    deleteNode(int pos);

    void   reset();


  int getEatenNode(int);
  void eatenNodeUp(int);
  void resetEatenNode();

private:
  unsigned long long *_board;

  /** Number of times a player eat other player's node */
  int eatenNode[2];


  void    additionUpdateNeighbour(int currentNodeNb, int color);
  void    additionUpdateNeighbourAlignement(int currentNodeNb, int color);
  void    additionUpdateNeighbourState(int currentNodeNb, int color);

  void    deletionUpdateNeighbour(int currentNodeNb);
};

#endif
