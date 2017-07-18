#ifndef TEST_CASE_SET_H
#define TEST_CASE_SET_H

#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>

#include "base/array.h"
#include "base/vector.h"
#include "tools/string_utils.h"
#include "tools/Random.h"
#include "tools/random_utils.h"

template <int INPUT_LENGTH>
class TestcaseSet {
protected:
    using input_t = emp::array<int, INPUT_LENGTH>;
    using output_t = std::set<int>;
    using test_case_t = std::pair<input_t, output_t>;
    emp::vector<test_case_t> test_cases;
    emp::vector<std::function<output_t(input_t)> > groups;
    emp::vector<emp::vector<output_t> > correct_choices;
    emp::Random * random;
    int trials  = 20;

public:
    TestcaseSet(std::string filename, emp::Random * r) : random(r) {
        LoadTestcases(filename);
    }

    emp::vector<std::pair<input_t, output_t> >& GetTestcases() {
        return test_cases;
    }

    emp::vector<emp::vector<output_t> >& GetCorrectChoices() {
        return correct_choices;
    }

    int GetNFuncs() {
        return groups.size();
    }

    emp::vector<int> GetBestPossible(emp::vector<size_t> choices) {
        emp::vector<int> count;
        for (int i = 0; i < groups.size(); i++) {
            count.push_back(0);
            for (size_t choice : choices) {
                if (correct_choices[i][choice].size()){
                    count[i]++;
                }
            }
        }
        return count;
    }

    emp::vector<size_t> GetValidSubset() {
        return emp::Choose(*random, test_cases.size(), trials);

        // int count = 0;
        // while (count < 1000) {
        //     count++;
        //     emp::vector<size_t> choices = emp::Choose(*random, test_cases.size(), trials);
        //
        //     bool all_valid = true;
        //     for (int i = 0; i < groups.size(); i++) {
        //         for (int j = 1; j < groups.size(); j++) {
        //             if (i == j) {
        //                 continue;
        //             }
        //             bool valid = false;
        //             for (int choice : choices) {
        //                 emp::vector<int> result(100);
        //                 auto it = std::set_difference(correct_choices[i][choice].begin(),
        //                                       correct_choices[i][choice].end(),
        //                                       correct_choices[j][choice].begin(),
        //                                       correct_choices[j][choice].end(),
        //                                       result.begin());
        //                 result.resize(it - result.begin());
        //                 if (result.size() != 0) {
        //                     valid = true;
        //                     break;
        //                 }
        //             }
        //
        //             if (!valid) {
        //                 all_valid = false;
        //                 break;
        //             }
        //         }
        //         if (!all_valid) {
        //             break;
        //         }
        //     }
        //
        //     if (all_valid) {
        //         return choices;
        //     }
        // }
        // std::cout << "ERROR: The functions you supplied to TestcaseSet don't seem to be reliably differentiatable." << std::endl;
        // exit(1);
    }

    void AddGroup(std::function<output_t(input_t)> func) {
        groups.push_back(func);
        correct_choices.push_back(emp::vector<output_t>());
        for (auto test_case : test_cases) {
            correct_choices[correct_choices.size() - 1].push_back(func(test_case.first));
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
            output_t answer;
            if (split_line.size() == INPUT_LENGTH + 1) {
                answer.insert(std::atoi(split_line[INPUT_LENGTH].c_str()));
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
