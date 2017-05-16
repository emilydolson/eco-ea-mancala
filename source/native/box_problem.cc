#include "evo/World.h"
#include "tools/BitSet.h"
#include "base/array.h"

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
  VALUE(ORG_SIZE, uint32_t, 2, "Number of dimensions in org"),
  VALUE(POP_SIZE, uint32_t, 1000, "Number of organisms in the popoulation."),
  VALUE(TOURNAMENT_SIZE, uint32_t, 5, "Number of organisms in the popoulation."),
  VALUE(MAX_GENS, uint32_t, 2000, "How many generations should we process?"),
  VALUE(SELECTION_TYPE, std::string, "eco", "What type of selection? eco, tournament, or lexicase"),
  VALUE(SUBTASKS, std::string, "all", "Which subtasks are rewarded? all, bad, or good")
)

using Org = emp::array<double, 2>;

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
    emp::evo::World<Org, emp::evo::DefaultStats> world(random);
    // world.fitM.Resize(POP_SIZE);

    // Build a random initial population
    for (uint32_t i = 0; i < POP_SIZE; i++) {
      Org next_org;
      for (uint32_t j = 0; j < 2; j++) next_org[j] = random.GetDouble(0, 1);
      world.Insert(next_org);
    }

    std::function<double(Org*)> goal_function = [ORG_SIZE](Org * org){
        double fitness = (int)(sqrt(emp::Pow(1 - (*org)[0], 2.0) + emp::Pow(1 - (*org)[1], 2.0)) < .01);
        // std::cout << emp::to_string(*org) << " " << emp::Pow2(1 - (*org)[0]) + emp::Pow2(1 - (*org)[1]) << " " << fitness << std::endl;
        return fitness;
    };

    std::function<double(Org*)> good_hint = [](Org * org){
        return 1 - std::abs((*org)[0] - (*org)[1]);
    };

    std::function<double(Org*)> bad_hint = [ORG_SIZE](Org * org){
        return 1 - (*org)[0];
    };

    emp::vector<std::function<double(Org*)> > all_funs_lexicase(3);
    all_funs_lexicase[0] = goal_function;
    all_funs_lexicase[1] = good_hint;
    all_funs_lexicase[2] = bad_hint;

    emp::vector<std::function<double(Org*)> > good_funs_lexicase(2);
    good_funs_lexicase[0] = goal_function;
    good_funs_lexicase[1] = good_hint;

    emp::vector<std::function<double(Org*)> > bad_funs_lexicase(2);
    bad_funs_lexicase[0] = goal_function;
    bad_funs_lexicase[1] = bad_hint;

    emp::vector<std::function<double(Org*)> > all_funs(2);
    all_funs[0] = good_hint;
    all_funs[1] = bad_hint;

    emp::vector<std::function<double(Org*)> > good_funs(1);
    good_funs[0] = good_hint;

    emp::vector<std::function<double(Org*)> > bad_funs(1);
    bad_funs[0] = bad_hint;

    // For tournament select
    emp::vector<std::function<double(Org *)> > no_extra_funs(0);

    std::function<bool(Org *, emp::Random&)> mut_func = [ORG_SIZE, MUT_RATE](Org * org, emp::Random& random) {

        for (uint32_t i = 0; i < ORG_SIZE; i++) {
            (*org)[i] += random.GetRandNormal(0, MUT_RATE);
            if ((*org)[i] < 0) {
                (*org)[i] = 0;
            } else if ((*org)[i] > 1) {
                (*org)[i] = 1;
            }
        }
        return true;
    };

    world.SetDefaultMutateFun(mut_func);
    world.SetDefaultFitnessFun(goal_function);

    for (uint32_t ud = 0; ud < MAX_GENS; ud++) {
        if (config.SELECTION_TYPE() == "eco" && config.SUBTASKS() == "all") {
            world.EcoSelectGradation(goal_function, all_funs, 1000.0, TOURNAMENT_SIZE, POP_SIZE);
        } else if (config.SELECTION_TYPE() == "eco" && config.SUBTASKS() == "good") {
            world.EcoSelectGradation(goal_function, good_funs, 1000.0, TOURNAMENT_SIZE, POP_SIZE);
        } else if (config.SELECTION_TYPE() == "eco" && config.SUBTASKS() == "bad") {
            world.EcoSelectGradation(goal_function, bad_funs, 1000.0, TOURNAMENT_SIZE, POP_SIZE);
        } else if (config.SELECTION_TYPE() == "tournament") {
            world.EcoSelectGradation(goal_function,  no_extra_funs, 1000.0, TOURNAMENT_SIZE, POP_SIZE);
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
        std::cout << emp::to_string(world[0]) << std::endl;
        world.Update();
        world.MutatePop();
    }

}
