#ifndef MANCALA_RESOURCE_H
#define MANCALA_RESOURCE_H

#include <iostream>
#include <fstream>

#include "base/array.h"
#include "base/vector.h"
#include "tools/string_utils.h"
#include "tools/Random.h"
#include "tools/random_utils.h"

#include "../push-3.1.0/src/CodeUtils.h"
#include "../push-3.1.0/src/RNG.h"
#include "../push-3.1.0/src/GeneticOps.h"
#include "../push-3.1.0/src/Instruction.h"
#include "../push-3.1.0/src/Literal.h"

#include "play_mancala.h"

class MancalaResource {
protected:
    emp::vector<std::pair<emp::array<int, 14>, int> > test_cases;
    emp::Random * random;
    int trials = 20;
    push::Env env;

public:
    MancalaResource(std::string filename, emp::Random * r) : random(r) {
        GetTestcases(filename);
    }

    MancalaResource(std::string filename, emp::Random * r, push::Env env)
                        : random(r), env(env) {
        GetTestcases(filename);
    }

    void GetTestcases(std::string filename) {
        std::ifstream infile(filename);
        std::string line;

        if (!infile.is_open()){
            std::cout << "ERROR: " << filename << " did not open correctly" << std::endl;
            return;
        }

        while ( getline (infile,line)) {
            emp::vector<std::string> split_line = emp::slice(line, ',');
            emp::array<int, 14> test_case;
            for (int i = 0; i < 14; i++) {
                test_case[i] = std::atoi(split_line[i].c_str());
            }
            test_cases.push_back(std::make_pair(test_case, std::atoi(split_line[14].c_str())));
        }
        infile.close();
    }

    // Push needs its env, so we'll need to write a separate version for AvidaGP
    double Fitness(push::Code * org) {
        emp::vector<size_t> cases = emp::Choose(*random, test_cases.size(), trials);
        double correct = 0;

        for (size_t test_case : cases) {
            int choice = PlayMancala(org, test_cases[test_case].first, 0, env.next());
            correct += (int)(choice == test_cases[test_case].second);
        }

        return correct;
    }

};

#endif
