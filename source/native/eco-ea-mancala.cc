// This is the main function for the NATIVE version of this project.

#include <iostream>
#include "../mancala.h"
#include "../../handcoded_AI/Mancala/mancala/AI-Trivial.hh"

int main()
{
    Mancala game;
    int player = 1;
    int choice;

    while (!game.IsOver()) {
        if (!player) {
            std::cout << "Player 0, please choose a cell from 1-6" << std::endl;
            std::cin >> choice;
            while (choice > 6 || choice == 0) {
                std::cout << "Invalid number. Please try again" << std::endl;
                std::cin >> choice;
            }
        } else {
            std::cout << "Player 1, please choose a cell from 8-13" << std::endl;
            // std::cin >> choice;
            choice = TrivialMove(game.GetBoard(), player);
            while (choice < 8) {
                std::cout << "Invalid number. Please try again" << std::endl;
                std::cin >> choice;
            }
        }

        if (!game.ChooseCell(choice)) {
            player = !player;
        }

        game.PrintBoard();

    }


}
