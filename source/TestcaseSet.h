#ifndef TEST_CASE_SET_H
#define TEST_CASE_SET_H

#include <iostream>
#include <fstream>

#include "base/array.h"
#include "base/vector.h"
#include "tools/string_utils.h"

template <int INPUT_LENGTH>
class TestcaseSet {
protected:
    using input_t = emp::array<int, INPUT_LENGTH>;
    using output_t = int;
    emp::vector<std::pair<input_t, output_t> > test_cases;
    emp::vector<std::function<output_t(input_t)> > groups;
    emp::vector<emp::vector<int> > correct_choices;

public:
    TestcaseSet(std::string filename) {
        LoadTestcases(filename);
    }

    emp::vector<std::pair<input_t, output_t> >& GetTestcases() {
        return test_cases;
    }

    emp::vector<emp::vector<int> >& GetCorrectChoices() {
        return correct_choices;
    }

    void AddGroup(std::function<output_t(input_t)> func) {
        groups.push_back(func);
        correct_choices.push_back(emp::vector<int>());
        for (auto test_case : test_cases) {
            correct_choices[correct_choices.size() - 1].push_back(func(test_case[0]));
        }
    }

    // Currently assumes
    void LoadTestcases(std::string filename) {
        std::ifstream infile(filename);
        std::string line;

        if (!infile.is_open()){
            std::cout << "ERROR: " << filename << " did not open correctly" << std::endl;
            return;
        }

        while ( getline (infile,line)) {
            emp::vector<std::string> split_line = emp::slice(line, ',');
            input_t test_case;
            for (int i = 0; i < INPUT_LENGTH; i++) {
                test_case[i] = std::atoi(split_line[i].c_str());
            }
            int answer = 0;
            if (split_line.size() == INPUT_LENGTH + 1) {
                answer = std::atoi(split_line[INPUT_LENGTH].c_str());
            }
            test_cases.push_back(std::make_pair(test_case, answer));
            for (int i = 0; i < groups.size(); i++) {
                correct_choices[i].push_back(groups[i](test_case));
            }
        }
        infile.close();
    }



};

#endif
