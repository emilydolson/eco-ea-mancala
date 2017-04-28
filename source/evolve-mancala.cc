#include "evo/World.h"
#include "mancala.h"

#include <functional>
#include <fstream>
#include <vector>
#include <valarray>
#include <set>

#include "../push-3.1.0/src/CodeUtils.h"
#include "../push-3.1.0/src/RNG.h"
#include "../push-3.1.0/src/GeneticOps.h"
#include "../push-3.1.0/src/Instruction.h"
#include "../push-3.1.0/src/Literal.h"

#include "../handcoded_AI/Mancala/mancala/AI-Trivial.hh"

int POP_SIZE = 100;
int TRIALS = 5;

template <typename WORLD>
void init(WORLD & world, const push::Env& env) {
    static push::Env workEnv(1000);
    static push::Code rnd = push::parse("CODE.RAND");

    for (int i = 0; i < POP_SIZE; ++i) {
        workEnv = env;
        (*rnd)(workEnv);
        if (not push::has_elements<push::Code>(workEnv,1)) { --i; continue; }
    	push::Code code = push::pop<push::Code>(workEnv); // get random program
        // std::cout << "About to insert " << i << std::endl;
        world.Insert(code);
    }

}

std::string print_code(push::Code code, push::Env& env) {

    if (code->size() == 1) {
	std::string result = code->to_string();

	// check if it's a name
	push::Literal<push::name_t>* name = dynamic_cast< push::Literal<push::name_t>* >(code.get());
	if (name) {
	    result += std::string(":") + get_code(name->get())->to_string();
	}

	return result;
    }

    std::string result = "( ";
    const push::CodeArray& pieces = code->get_stack();
    for (int i = pieces.size()-1; i >= 0; --i) {
	result += print_code(pieces[i], env);
	result += " ";
    }
    return result + " )";
}


int main(int argc, char* argv[] ) {
    push::init_push();
    std::string config_file = argc<2?"test.config":argv[1];
    std::ifstream config( config_file.c_str() );
    emp::Random random(0);
    emp::evo::World<push::Code, emp::evo::FitTrack> world(random);
    world.fitM.Resize(POP_SIZE);
    Mancala game;
    push::Env env;

    env.configure(push::parse(config));

    push::Env workEnv;
    int player = 0;

    std::cout << "about to initialize" << std::endl;
    init(world, env.next());
    std::cout << "Initialize" << std::endl;

    workEnv = env.next();

    std::function<double(push::Code*)> fitness_func = [&player, &game, &workEnv](push::Code * genome) {
        static push::Env env(1000);
        int sum = 0;
        game.Reset();
        // std::cout << "In fit fun" << std::endl;
        for (int t = 0; t < TRIALS; t++) {
            while (!game.IsOver()) {
                if (player) {
                    if (!game.ChooseCell(TrivialMove(game.GetBoard(), player))) {
                        player = (int)!player;
                    }
                } else {
                    // std::cout << "Player 0 turn" << std::endl;
                    env = workEnv;
                    env.clear_stacks();


                    for (int count : game.GetBoard()) {
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
                    while (!game.IsMoveValid(choice, player)) {
                        if (push::has_elements<int>(env,1)) {
                            choice = push::pop<int>(env);
                        } else {
                            choice = TrivialMove(game.GetBoard(), player);
                        }
                    }

                    // std::cout << "Making move " << choice << std::endl;

                    if (!game.ChooseCell(choice)){
                        player = (int)!player;
                    }
                }
                // game.PrintBoard();
            }
            // std::cout << "Score: " << game.ScoreDiff(0) << std::endl;
            sum += game.ScoreDiff(0)+48;
        }
        return sum/TRIALS;
    };

    world.SetDefaultFitnessFun(fitness_func);
    // world.SetDefaultMutateFun();

    workEnv = env.next();
    push::CodeBase& swapcode = *push::parse("CODE.SWAP");
    std::cout << "Starting loop" << std::endl;
    for (int ud = 0; ud < 500; ud++) {
        world.popM.CalcFitnessAll(fitness_func);
        // std::cout << "Rpro loop. ud: " << ud << std::endl;
        for (int n = 0; n < POP_SIZE; n++) {
          const double fit_pos = world.random_ptr->GetDouble(world.fitM.GetTotalFitness());
        //   std::cout << "fit pos: " << fit_pos << " n: " << n << std::endl;
          size_t id = world.fitM.At(fit_pos);
        //   std::cout << "id " << id << std::endl;
          workEnv = env.next();
          workEnv.clear_stacks();
          push::push(workEnv, world[id]);

          if (rng.flip(0.5)) { // should be a parameter
  		    if (rng.flip(0.5)) {
  		        push::subtree_mutation(workEnv);
            } else {
  		        push::deletion_mutation(workEnv);
            }
  	      } else {
  		    swapcode(workEnv);
  		  //   push::subtree_xover(workEnv);
  	    }

  	    push::Code new_code = push::pop<push::Code>(workEnv);


        //   if (not push::has_elements<push::Code>(workEnv,1)) { --n; continue; }
     //  	  push::Code new_code = push::pop<push::Code>(workEnv);
        //   double parent_fitness = fitness_func(&world[id]);
        //   std::cout << "Parent " << n << " reproduced. Fitness: " << parent_fitness << std::endl;
        //   std::cout << print_code(world[id], env.next()) << std::endl;
          world.InsertBirth( new_code, id, 1, fitness_func );
        }
        std::cout << "Average fitness: " << world.fitM.GetTotalFitness()/POP_SIZE << std::endl;
        world.Update();
        // world.MutatePop();
    }

    for (auto org : world) {
        double fitness = fitness_func(org);
        std::cout << "Fitness: " << fitness << " Code: " << print_code(*org, env.next()) <<std::endl;

        static push::Env env(1000);
        int sum = 0;
        game.Reset();
        // std::cout << "In fit fun" << std::endl;
        while (!game.IsOver()) {
            if (player) {
                if (!game.ChooseCell(TrivialMove(game.GetBoard(), player))) {
                    player = (int)!player;
                }
            } else {
                // std::cout << "Player 0 turn" << std::endl;
                env = workEnv;
                env.clear_stacks();


                for (int count : game.GetBoard()) {
                    push::push<int>(env, count);
                }
                // std::cout << "Booard initialized" << std::endl;
                push::push_call(env, *org);
                env.go(1000);
                // std::cout << "Code called" << std::endl;
                int choice = -1;
                if (push::has_elements<int>(env,1)) {
                    choice = push::pop<int>(env);
                }
                // std::cout << "Validating move" << std::endl;
                while (!game.IsMoveValid(choice, player)) {
                    if (push::has_elements<int>(env,1)) {
                        choice = push::pop<int>(env);
                    } else {
                        choice = TrivialMove(game.GetBoard(), player);
                    }
                }

                // std::cout << "Making move " << choice << std::endl;

                if (!game.ChooseCell(choice)){
                    player = (int)!player;
                }
            }
            game.PrintBoard();
        }
        std::cout << "Score: " << game.ScoreDiff(0) << std::endl;

    }



}
