#ifndef AI_TRIVIAL_HH
#define AI_TRIVIAL_HH

#include "base/array.h"
#include "tools/Random.h"
#include "base/array.h"

int TrivialMove(emp::array<int, 14> board, int player)
{
  static emp::Random random;
  int test_move = -1;

  do {
    test_move = random.GetInt(1,7);  // Grab a random number from 0 to 5.
    if (player) {
        test_move += 7;
    }

  } while (!board[test_move]);

  return test_move;
}


#endif
