#include <iostream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
    json j = {
        {"happy", true},
        {"pi", 3.141},
    };

    std::cout << j.dump(4) << std::endl;

    return 0;
}