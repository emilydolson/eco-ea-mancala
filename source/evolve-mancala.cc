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

#include "config/ArgManager.h"
#include "../handcoded_AI/Mancala/mancala/AI-Trivial.hh"
// #include "../handcoded_AI/mancala/mancala/contest/AI-Thelen.hh"
#include "play_mancala.h"
#include "MancalaResource.h"

EMP_BUILD_CONFIG( EcoEaConfig,
  GROUP(DEFAULT, "Default settings for NK model"),
  VALUE(TRIALS, uint32_t, 10, "Level of epistasis in the NK model"),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(POP_SIZE, uint32_t, 100, "Number of organisms in the popoulation."),
  VALUE(MAX_GENS, uint32_t, 2000, "How many generations should we process?"),
  VALUE(SELECTION_TYPE, std::string, "eco", "What type of selection? eco, tournament, or lexicase")
)


template <typename WORLD>
void init(WORLD & world, const push::Env& env, int POP_SIZE) {
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
    EcoEaConfig config;

    config.Read("EcoEAPush.cfg");

    auto args = emp::cl::ArgManager(argc, argv);
    if (args.ProcessConfigOptions(config, std::cout, "EcoEAPush.cfg", "EcoEAPush-macros.h") == false) {
      exit(0);
    }
    if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

    const uint32_t POP_SIZE = config.POP_SIZE();
    const uint32_t MAX_GENS = config.MAX_GENS();
    const uint32_t TRIALS = config.TRIALS();

    push::init_push();
    std::string config_file = "test.config";
    std::ifstream push_config( config_file.c_str() );
    emp::Random random(config.SEED());
    emp::evo::World<push::Code, emp::evo::DefaultStats> world(random);
    world.fitM.Resize(POP_SIZE);
    Mancala game;
    push::Env env;

    env.configure(push::parse(push_config));

    push::Env workEnv;

    // std::cout << "about to initialize" << std::endl;
    init(world, env.next(), POP_SIZE);
    // std::cout << "Initialize" << std::endl;

    workEnv = env.next();

    MancalaResource ExtraMoveResource("extra_move_testcases.csv", &random, workEnv);
    MancalaResource CaptureResource("capture_testcases.csv", &random, workEnv);
    MancalaResource ELDResource("ELD_mancala_records.csv", &random, workEnv);

    std::function<double(push::Code*)> fitness_func = [TRIALS, &game, &workEnv](push::Code * genome) {
        int sum = 0;
        game.Reset();
        // std::cout << "In fit fun" << std::endl;
        for (int t = 0; t < TRIALS; t++) {
            while (!game.IsOver()) {
                if (game.GetCurrPlayer()) {
                    game.ChooseCell(TrivialMove(game.GetBoard(), game.GetCurrPlayer()));
                    // game.ChooseCell(thelen::ThelenMove(game, game.GetCurrPlayer()));
                } else {
                    // std::cout << "Player 0 turn" << std::endl;
                    int choice = PlayMancala(genome, game.GetBoard(), game.GetCurrPlayer(), workEnv);
                    // std::cout << "Making move " << choice << std::endl;
                    if (choice == -1) {
                        // std::cout << "Forfiting " << std::endl;
                        game.Forfeit();
                    } else {
                        // std::cout << "Not Forfiting " << std::endl;
                        game.ChooseCell(choice);
                    }
                }
                // game.PrintBoard();
            }
            // std::cout << "Score: " << game.ScoreDiff(0) << std::endl;
            sum += game.ScoreDiff(0)+48;
        }
        return sum/TRIALS;
    };

    // For EcoSelect
    emp::vector<std::function<double(push::Code*)> > extra_funs(3);
    extra_funs[0] = [&ExtraMoveResource](push::Code* org){return ExtraMoveResource.Fitness(org);};
    extra_funs[1] = [&CaptureResource](push::Code* org){return CaptureResource.Fitness(org);};
    extra_funs[2] = [&ELDResource](push::Code* org){return ELDResource.Fitness(org);};

    // For tournament select
    emp::vector<std::function<double(push::Code *)> > no_extra_funs(0);

    // For Lexicase select
    emp::vector<std::function<double(push::Code *)> > all_fit_funs(4);
    all_fit_funs[0] = fitness_func;
    all_fit_funs[1] = [&ExtraMoveResource](push::Code* org){return ExtraMoveResource.Fitness(org);};
    all_fit_funs[2] = [&CaptureResource](push::Code* org){return CaptureResource.Fitness(org);};
    all_fit_funs[3] = [&ELDResource](push::Code* org){return ELDResource.Fitness(org);};


    push::CodeBase& swapcode = *push::parse("CODE.SWAP");

    std::function<bool(push::Code*, emp::Random&)> mut_func = [&workEnv, &env, &swapcode](push::Code* genome, emp::Random & r) {
        workEnv = env.next();
        workEnv.clear_stacks();
        push::push(workEnv, *genome);

        if (r.P(0.5)) { // should be a parameter
          if (r.P(0.5)) {
              push::subtree_mutation(workEnv);
          } else {
              push::deletion_mutation(workEnv);
          }
        } else {
          swapcode(workEnv);
        //   push::subtree_xover(workEnv);
      }

      push::Code old = *genome;
      *genome = push::pop<push::Code>(workEnv);
      return old == *genome;
    };


    world.SetDefaultFitnessFun(fitness_func);
    world.SetDefaultMutateFun(mut_func);
    world.OnOffspringReady([&world, &mut_func](push::Code * org){mut_func(org, *(world.random_ptr));});

    workEnv = env.next();

    std::cout << "Starting loop" << std::endl;
    for (int ud = 0; ud < MAX_GENS; ud++) {
        if (config.SELECTION_TYPE() == "eco") {
            world.EcoSelect(fitness_func, extra_funs, 10000.0, 5, POP_SIZE);
        } else if (config.SELECTION_TYPE() == "tournament") {
            world.EcoSelect(fitness_func,  no_extra_funs, 10000.0, 5, POP_SIZE);
        } else if (config.SELECTION_TYPE() == "lexicase") {
            world.LexicaseSelect(all_fit_funs, POP_SIZE);
        } else {
            std::cout << "Invalid selection type " << config.SELECTION_TYPE() << std::endl;
            exit(1);
        }
        // std::cout << "Average fitness: "  << std::endl;
        world.Update();
        // world.MutatePop();
    }

    double sum_fitness = 0;
    double max_fitness = 0;
    for (auto org : world) {
        double fitness = fitness_func(org);
        // std::cout << "Fitness: " << fitness << /*" Code: " << print_code(*org, env.next()) <<*/std::endl;
        sum_fitness += fitness;
        if (fitness > max_fitness) {
            max_fitness = fitness;
        }
        // static push::Env env(1000);
        // int sum = 0;
        // game.Reset();
        // // std::cout << "In fit fun" << std::endl;
        // while (!game.IsOver()) {
        //     if (game.GetCurrPlayer()) {
        //         game.ChooseCell(TrivialMove(game.GetBoard(), game.GetCurrPlayer()));
        //     } else {
        //         // std::cout << "Player 0 turn" << std::endl;
        //         env = workEnv;
        //         env.clear_stacks();
        //
        //
        //         for (int count : game.GetBoard()) {
        //             push::push<int>(env, count);
        //         }
        //         // std::cout << "Booard initialized" << std::endl;
        //         push::push_call(env, *org);
        //         env.go(1000);
        //         // std::cout << "Code called" << std::endl;
        //         int choice = -1;
        //         if (push::has_elements<int>(env,1)) {
        //             choice = push::pop<int>(env);
        //         }
        //         // std::cout << "Validating move" << std::endl;
        //         while (!game.IsMoveValid(choice)) {
        //             if (push::has_elements<int>(env,1)) {
        //                 choice = push::pop<int>(env);
        //             } else {
        //                 choice = TrivialMove(game.GetBoard(), game.GetCurrPlayer());
        //             }
        //         }
        //
        //         // std::cout << "Making move " << choice << std::endl;
        //
        //         game.ChooseCell(choice);
        //     }
        //     game.PrintBoard();
        // }
        // std::cout << "Score: " << game.ScoreDiff(0) << std::endl;

    }
    std::cout << "Average fitness: " << sum_fitness/POP_SIZE << " Max fitness: " << max_fitness << std::endl;



}
