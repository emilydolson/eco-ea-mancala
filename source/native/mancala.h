#include <base/array.h>
#include <base/assert.h>

class Mancala {
private:
    //Should end cells be their own members? For now lets say no.
    emp::array<int, 14> board;

public:

    Mancala() {
        Reset();
    }

    void Reset() {
        for (int i = 0; i < board.size(); i++) {
            board[i] = 4;
        }
        board[0] = 0;
        board[7] = 0;
    }

    void ChooseCell(int cell) {
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
    }

    bool IsOver() {
        bool over = true;
        for (int i = 0; i < board.size(); i++) {
            if (board[i] > 0 && i != 0 && i != 7) {
                over = false;
            }
        }

        return over;
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
};
