#include "TestcaseSet.h"
#include <set>

int main() {
    emp::Random r;
    TestcaseSet<14> t = TestcaseSet<14>("../capture_testcases.csv", &r);
    std::function<std::set<int>(emp::array<int, 14>)> f1 =
        [](emp::array<int,14> test_case){
        std::set<int> s = std::set<int>();
        for (int i = 0; i < 14; i++) {
            if (test_case[i] == 0) {
                s.insert(i);
            }
        }
        return s;
    };

    std::function<std::set<int>(emp::array<int, 14>)> f2 =
        [](emp::array<int,14> test_case){
        std::set<int> s = std::set<int>();
        s.insert(1);
        return s;
    };

    std::function<std::set<int>(emp::array<int, 14>)> f3 =
        [](emp::array<int,14> test_case){
        std::set<int> s = std::set<int>();
        s.insert(1);
        return s;
    };


    t.AddGroup(f1);
    t.AddGroup(f2);
    t.AddGroup(f3);

    emp::vector<size_t> choices = t.GetValidSubset();
    for (int choice : choices) {
        std::cout << emp::to_string(t.GetTestcases()[choice].first) << std::endl;
    }
}
