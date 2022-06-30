#include <iostream>
#include "matrix.hpp"
#include <random>
#include <ranges>
#include <algorithm>
#include <cassert>
#include <future>
#include <deque>
#include <chrono>
#include <nlohmann/json.hpp>
#include <boost/program_options.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <optional>
#include <fmt/ranges.h>

using namespace std;

template<typename T, typename Container>
void fill_random(Matrix<T, Container>& mat)
{
    std::random_device r;
    std::uniform_int_distribution<T> uniform(10, 99);
    std::default_random_engine prng(r());

    std::ranges::generate(mat, [&] {return uniform(prng);});
}

template<typename T, typename Container>
Matrix<T, Container> multi_threaded_add(
    Matrix<T, Container> A,
    Matrix<T, Container> B,
    std::size_t num_threads
    )
{
    auto C = A;

    std::vector<std::future<void>> futures;
    for (std::size_t i = 0; i < num_threads; i++)
    {
        futures.push_back(
            std::async(std::launch::async, [&C, &A, &B, i, num_threads] {
                std::size_t t1 =
                    (i / static_cast<double>(num_threads)) * C.n();
                std::size_t t2 =
                    ((i + 1) / static_cast<double>(num_threads)) * C.n();

                for (std::size_t r = t1; r < t2; r++)
                {
                    for (std::size_t c = 0; c < C.n(); c++) {
                        C[r][c] = A[r][c] + B[r][c];
                    }
                }

            }));
    }

    for (auto& future : futures)
        future.get();

    return C;
}

template<typename T, typename Container>
Matrix<T, Container> single_threaded_add(
    Matrix<T, Container> A,
    Matrix<T, Container> B,
    std::size_t num_threads
    )
{
    auto C = A;

    for (int r = 0; r < C.m(); r++)
    {
        for (std::size_t c = 0; c < C.n(); c++) {
            C[r][c] = A[r][c] + B[r][c];
        }
    }

    return C;
}

int main(int argc, char *argv[]) {

    Matrix<int> A(5000, 5000);
    auto B = A;

    fill_random(A);
    fill_random(B);


    //fmt::print("A: [{}]\n", fmt::join(A.data(), ", "));
    //fmt::print("B: [{}]\n", fmt::join(B.data(), ", "));
    //fmt::print("C: [{}]\n", fmt::join(C.data(), ", "));


    auto start = std::chrono::steady_clock::now();
    auto C = single_threaded_add(A, B, std::thread::hardware_concurrency());
    auto end = std::chrono::steady_clock::now();

    auto elapsed =
        chrono::duration_cast<chrono::microseconds>(end - start).count();
    fmt::print("Single Elapsed: {}\n", elapsed);
    return 0;
}
