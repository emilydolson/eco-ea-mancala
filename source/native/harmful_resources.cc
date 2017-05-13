#include "evo/World.h"
#include "tools/BitSet.h"

#include <functional>
#include <fstream>
#include <vector>
#include <valarray>
#include <set>

#include "config/ArgManager.h"
// #include "../handcoded_AI/Mancala/mancala/AI-Trivial.hh"
// #include "../handcoded_AI/mancala/mancala/contest/AI-Thelen.hh"
// #include "play_mancala.h"
// #include "MancalaResource.h"

EMP_BUILD_CONFIG( EcoEaConfig,
  GROUP(DEFAULT, "Default settings for NK model"),
  VALUE(TRIALS, uint32_t, 10, "Level of epistasis in the NK model"),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(MUT_RATE, double, .001, "Per site"),
  VALUE(ORG_SIZE, uint32_t, 32, "Number of bits in org"),
  VALUE(POP_SIZE, uint32_t, 1000, "Number of organisms in the popoulation."),
  VALUE(TOURNAMENT_SIZE, uint32_t, 5, "Number of organisms in the popoulation."),
  VALUE(MAX_GENS, uint32_t, 2000, "How many generations should we process?"),
  VALUE(SELECTION_TYPE, std::string, "eco", "What type of selection? eco, tournament, or lexicase"),
  VALUE(SUBTASKS, std::string, "all", "Which subtasks are rewarded? all, bad, or good")
)

using BitOrg = emp::BitVector;

int main(int argc, char* argv[] ) {
    EcoEaConfig config;

    config.Read("harmful_resources.cfg");

    auto args = emp::cl::ArgManager(argc, argv);
    if (args.ProcessConfigOptions(config, std::cout, "harmful_resources.cfg", "harmful_resources-macros.h") == false) {
      exit(0);
    }
    if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

    const uint32_t ORG_SIZE = config.ORG_SIZE();
    const double MUT_RATE = config.MUT_RATE();
    const uint32_t POP_SIZE = config.POP_SIZE();
    const uint32_t MAX_GENS = config.MAX_GENS();
    const uint32_t TOURNAMENT_SIZE = config.TOURNAMENT_SIZE();

    emp::Random random(config.SEED());
    emp::evo::World<BitOrg, emp::evo::DefaultStats> world(random);
    // world.fitM.Resize(POP_SIZE);

    // Build a random initial population
    for (uint32_t i = 0; i < POP_SIZE; i++) {
      BitOrg next_org(ORG_SIZE);
      for (uint32_t j = 0; j < ORG_SIZE; j++) next_org[j] = random.P(0.5);
      world.Insert(next_org);
    }

    std::function<double(BitOrg*)> goal_function = [ORG_SIZE](BitOrg * org){
        double fitness = 0;
        bool hit_end = false;

        for (uint32_t i = 0; i < ORG_SIZE; i++) {
            if ((*org)[i]) {
                fitness++;
            } else {
                if ((i >= ORG_SIZE - 3) ||
                                !((*org)[i+1] && !(*org)[i+2] && (*org)[i+3])) {
                    fitness = 0;
                }
                hit_end = true;
                break;
            }
        }

        if (!hit_end) {
            fitness = 0;
        }
        // std::cout << *org << " Fitness: " << fitness << std::endl;

        return fitness;
    };

    std::function<double(BitOrg*)> good_hint = [](BitOrg * org){
        return org->CountOnes();
    };

    std::function<double(BitOrg*)> bad_hint = [ORG_SIZE](BitOrg * org){
        return ORG_SIZE - org->CountOnes();
    };

    emp::vector<std::function<double(BitOrg*)> > all_funs_lexicase(3);
    all_funs_lexicase[0] = goal_function;
    all_funs_lexicase[1] = good_hint;
    all_funs_lexicase[2] = bad_hint;

    emp::vector<std::function<double(BitOrg*)> > good_funs_lexicase(2);
    good_funs_lexicase[0] = goal_function;
    good_funs_lexicase[1] = good_hint;

    emp::vector<std::function<double(BitOrg*)> > bad_funs_lexicase(2);
    bad_funs_lexicase[0] = goal_function;
    bad_funs_lexicase[1] = bad_hint;

    emp::vector<std::function<double(BitOrg*)> > all_funs(2);
    all_funs[0] = good_hint;
    all_funs[1] = bad_hint;

    emp::vector<std::function<double(BitOrg*)> > good_funs(1);
    good_funs[0] = good_hint;

    emp::vector<std::function<double(BitOrg*)> > bad_funs(1);
    bad_funs[0] = bad_hint;

    // For tournament select
    emp::vector<std::function<double(BitOrg *)> > no_extra_funs(0);

    std::function<bool(BitOrg *, emp::Random&)> mut_func = [ORG_SIZE, MUT_RATE](BitOrg * org, emp::Random& random) {
        bool mutated = false;

        for (uint32_t i = 0; i < ORG_SIZE; i++) {
            if (random.P(MUT_RATE)){
                (*org)[i] = (int)!(*org)[i];
                mutated = true;
            }
        }
        return mutated;
    };

    world.SetDefaultMutateFun(mut_func);
    world.SetDefaultFitnessFun(goal_function);

    for (uint32_t ud = 0; ud < MAX_GENS; ud++) {
        if (config.SELECTION_TYPE() == "eco" && config.SUBTASKS() == "all") {
            world.EcoSelectGradation(goal_function, all_funs, 1000.0, TOURNAMENT_SIZE, POP_SIZE);
        } else if (config.SELECTION_TYPE() == "eco" && config.SUBTASKS() == "good") {
            world.EcoSelect(goal_function, good_funs, 1000.0, TOURNAMENT_SIZE, POP_SIZE);
        } else if (config.SELECTION_TYPE() == "eco" && config.SUBTASKS() == "bad") {
            world.EcoSelect(goal_function, bad_funs, 1000.0, TOURNAMENT_SIZE, POP_SIZE);
        } else if (config.SELECTION_TYPE() == "tournament") {
            world.EcoSelect(goal_function,  no_extra_funs, 1000.0, TOURNAMENT_SIZE, POP_SIZE);
        } else if (config.SELECTION_TYPE() == "lexicase" && config.SUBTASKS() == "all") {
            world.LexicaseSelect(all_funs_lexicase, POP_SIZE);
        } else if (config.SELECTION_TYPE() == "lexicase" && config.SUBTASKS() == "good") {
            world.LexicaseSelect(good_funs_lexicase, POP_SIZE);
        } else if (config.SELECTION_TYPE() == "lexicase" && config.SUBTASKS() == "bad") {
            world.LexicaseSelect(bad_funs_lexicase, POP_SIZE);
        } else {
            std::cout << "Invalid selection type " << config.SELECTION_TYPE() << std::endl;
            exit(1);
        }

        world.Update();
        world.MutatePop();
    }

}
