#include "base/array.h"
#include "base/assert.h"
#include "tools/math.h"

class Mancala {
private:
    //Should end cells be their own members? For now lets say no.
    emp::array<int, 14> board;
    bool over = false;

public:

    Mancala() {
        Reset();
    }

    void Reset() {
        for (int i = 0; i < (int)board.size(); i++) {
            board[i] = 4;
        }
        board[0] = 0;
        board[7] = 0;
        over = false;
    }

    int operator[](int i){
        return board[i];
    }

    emp::array<int, 14> GetBoard() {
        return board;
    }

    // Returns bool indicating whether player can go again
    bool ChooseCell(int cell) {
        emp_assert(cell != 0 && cell != 7); //You can't choose the end cells
        int count = board[cell];
        int curr_cell = cell;
        board[cell] = 0;

        while (count > 0) {
            curr_cell++;
            if (curr_cell > 13) {
                curr_cell = 0;
            }
            if ((curr_cell == 0 && cell < 7) || (curr_cell == 7 && cell > 7)) {
                curr_cell++;
            }

            board[curr_cell]++;
            count--;
        }

        // Go again if you ended in your store
        if (curr_cell == 0 || curr_cell == 7) {
            UpdateIsOver();
            return true;
        }

        // Capturing
        if (board[curr_cell] == 1 &&
                ((curr_cell < 7 && cell < 7) || (curr_cell > 7 && cell > 7))) {
            int opposite = abs(7 - emp::Mod(curr_cell, 7));
            if (curr_cell < 7) {
                opposite += 7;
                board[7] += board[opposite];
                board[opposite] = 0;
            } else {
                board[0] += board[opposite];
                board[opposite] = 0;
            }

        }

        UpdateIsOver();
        return false;
    }

    bool IsMoveValid(int move, int player) {

        if (move > 13 || move < 0 || !board[move]) {
            return false;
        }
        if (!player && move < 7 && move > 0) {
            return true;
        }
        if (player && move > 7) {
            return true;
        }
        return false;
    }

    bool IsOver() {
        return over;
    }

    void UpdateIsOver() {
        bool side_1_empty = true;
        bool side_2_empty = true;

        for (int i = 1; i < 7; i++) {
            if (board[i] > 0) {
                side_1_empty = false;
            }
        }

        for (int i = 8; i < 14; i++) {
            if (board[i] > 0) {
                side_2_empty = false;
            }
        }

        over = side_1_empty || side_2_empty;
    }

    void PrintBoard() {
        std::cout << "  ";
        for (int i = 6; i > 0; i--) {
            std::cout << board[i] << " ";
        }
        std::cout << std::endl;
        std::cout << board[7] << "              " << board[0] << std::endl;
        std::cout << "  ";
        for (int i = 8; i < 14; i++) {
            std::cout << board[i] << " ";
        }
        std::cout << std::endl;

    }

    int GetWinner() {
        int player1 = board[0];
        int player0 = board[7];

        for (int i = 1; i < 7; i++) {
            player0 += board[i];
        }

        for (int i = 8; i < 14; i++) {
            player1 += board[i];
        }

        return (int)(player1 > player0);
    }

    int ScoreDiff(int player) {
        int player1 = board[0];
        int player0 = board[7];

        for (int i = 1; i < 7; i++) {
            player0 += board[i];
        }

        for (int i = 8; i < 14; i++) {
            player1 += board[i];
        }

        if (player) {
            return player1 - player0;
        } else {
            return player0 - player1;
        }
    }
};
