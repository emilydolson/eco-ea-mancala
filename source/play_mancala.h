#ifndef RUN_PROGRAM_H
#define RUN_PROGRAM_H

#include "../push-3.1.0/src/CodeUtils.h"
#include "../push-3.1.0/src/RNG.h"
#include "../push-3.1.0/src/GeneticOps.h"
#include "../push-3.1.0/src/Instruction.h"
#include "../push-3.1.0/src/Literal.h"

#include "../handcoded_AI/Mancala/mancala/AI-Trivial.hh"
#include "mancala.h"

int PlayMancala(push::Code * genome, emp::array<int, 14> & board, int player, push::Env & workEnv) {
    push::Env env = workEnv;
    env.clear_stacks();

    for (int count : board) {
        push::push<int>(env, count);
    }
    // std::cout << "Booard initialized" << std::endl;
    push::push_call(env, *genome);
    env.go(1000);
    // std::cout << "Code called" << std::endl;
    int choice = -1;
    if (push::has_elements<int>(env,1)) {
        choice = push::pop<int>(env);
    }
    // std::cout << "Validating move" << std::endl;
    // while (!IsMoveValid(choice, board, player) && push::has_elements<int>(env,1)) {
    //     choice = push::pop<int>(env);
    // }

    if (!IsMoveValid(choice, board, player)) {
        choice = -1;
    }

    return choice;
}

#endif
