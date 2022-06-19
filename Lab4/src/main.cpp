#include <ranges>
#include <algorithm>
#include <random>
#include <iostream>
#include <vector>
#include <future>
#include <fmt/ranges.h>

using namespace std;

template<typename T>
void fill_random(std::vector<T>& mat)
{
    std::random_device r;
    std::uniform_int_distribution<T> uniform(10, 99);
    std::default_random_engine prng(r());

    std::ranges::generate(mat, [&] {return uniform(prng);});
}

int main(int argc, char *argv[]) {
    auto fill_task1 = std::async([] () {
        std::vector<int> arr1(20);
        fill_random(arr1);
        fmt::print("array1: [{}]\n", fmt::join(arr1, ", "));
        return arr1;
    });
    auto max_filter_task = std::async([] (auto task) {
        auto array = task.get();
        auto max = *std::ranges::max_element(array);
        std::erase_if(array, [&max] (const auto& e) { return e <= max * 0.8; });
        fmt::print("array1 filtered: [{}]\n", fmt::join(array, ", "));
        return array;
    }, std::move(fill_task1));

    auto fill_task2 = std::async([] () {
        std::vector<int> arr2(20);
        fill_random(arr2);
        fmt::print("array2: [{}]\n", fmt::join(arr2, ", "));
        return arr2;
    });
    auto filter_task = std::async([] (auto task) {
        auto array = task.get();
        std::erase_if(array, [] (const auto& e) { return e % 3 == 0; });
        fmt::print("array2 filtered: [{}]\n", fmt::join(array, ", "));
        return array;
    }, std::move(fill_task2));

    auto combine_task2 = std::async([] (auto task1, auto task2) {
        auto array1 = task1.get();
        std::ranges::sort(array1);

        auto array2 = task2.get();
        std::ranges::sort(array2);

        std::erase_if(array1, [&array2] (const auto& e) {
            return std::ranges::binary_search(array2, e);
        });

        return array1;
    }, std::move(max_filter_task), std::move(filter_task));

    auto result = combine_task2.get();
    fmt::print("result: [{}]\n", fmt::join(result, ", "));
    return 0;
}
