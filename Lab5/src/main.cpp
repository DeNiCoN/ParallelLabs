#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace nlohmann;

int main(int argc, char *argv[]) {
    std::string array;

    std::cout << "Enter an array as json([1, 2, ...]):" << std::endl;
    std::cin >> array;
    json array_json = json::parse(array);

    return 0;
}
