//Заповнити квадратну матрицю випадковими числами.
//На побічній діагоналі розмістити мінімальний елемент рядка.

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
void print(const Matrix<T, Container>& mat)
{
    std::cout << "[\n";
    for (int i = 0; i < mat.m(); i++)
    {
        std::cout << "[";
        for (int j = 0; j < mat.n(); j++)
        {
            std::cout << mat[i][j] << ", ";
        }
        std::cout << "],\n";
    }
    std::cout << "]\n";
}

template<typename T, typename Container>
Matrix<T, Container> single_threaded(Matrix<T, Container> mat)
{
    for (int i = 0; i < mat.m(); i++)
    {
        mat[i][mat.n() - i - 1] = std::ranges::min(mat[i]);
    }

    return mat;
}

template<typename T, typename Container>
Matrix<T, Container> multi_threaded(Matrix<T, Container> mat, std::size_t num_threads)
{
    std::vector<std::future<void>> futures;
    for (std::size_t i = 0; i < num_threads; i++)
    {
        futures.push_back(
            std::async(std::launch::async, [&mat, i, num_threads] {
                std::size_t t1 =
                    (i / static_cast<double>(num_threads)) * mat.n();
                std::size_t t2 =
                    ((i + 1) / static_cast<double>(num_threads)) * mat.n();

                for (std::size_t r = t1; r < t2; r++)
                {
                    mat[r][mat.n() - r - 1] = std::ranges::min(mat[r]);
                }

            }));
    }

    for (auto& future : futures)
        future.get();

    return mat;
}

template<typename T, typename Container>
Matrix<T, Container> multi_threaded_pool(Matrix<T, Container> mat, std::size_t num_threads)
{
    boost::asio::thread_pool pool(num_threads);

    for (int i = 0; i < mat.m(); i++)
    {
        boost::asio::post(pool, [&mat, i] {
            mat[i][mat.n() - i - 1] = std::ranges::min(mat[i]);
        });
    }

    pool.join();

    return mat;
}

using namespace nlohmann;

json run(std::size_t n, std::optional<std::size_t> threads,
         unsigned iterations, bool pool)
{
    json result;
    result["params"]["n"] = n;
    result["pool"] = pool;
    if (threads)
        result["params"]["threads"] = threads.value();

    json& runs = result["runs"];
    for (int i = 0; i < iterations; i++)
    {
        Matrix<int> mat(n, n);
        fill_random(mat);
        json& run = runs[i];

        Matrix<int> mat_result(n, n);
        auto start = std::chrono::steady_clock::now();
        if (threads)
        {
            if (pool)
                mat_result = multi_threaded_pool(std::move(mat), threads.value());
            else
                mat_result = multi_threaded(std::move(mat), threads.value());
        }
        else
        {
            mat_result = single_threaded(std::move(mat));
        }
        auto end = std::chrono::steady_clock::now();

        run["elapsed"] =
            chrono::duration_cast<chrono::microseconds>(end - start).count();
    }

    result["average"] = std::reduce(runs.begin(), runs.end(), 0,
                                    [iterations](std::size_t sum, const json& j) {
        return (sum + j["elapsed"].get<std::size_t>());
    }) / iterations;

    return result;
}

void test(std::size_t n)
{
    Matrix<int, std::deque<int>> mat(n, n);

    fill_random(mat);
    Matrix<int, std::deque<int>> single(single_threaded(mat));

    Matrix<int, std::deque<int>> multi(
        multi_threaded(mat, std::thread::hardware_concurrency()));

    print(mat);
    std::cout << "\n";
    print(single);
    std::cout << "\n";
    print(multi);
    assert(single == multi);
}

int main(int argc, char *argv[])
{
    std::size_t n;
    std::optional<std::size_t> threads;
    unsigned iterations = 1;

    namespace ps = boost::program_options;

    ps::options_description desc("Options");
    desc.add_options()
        ("help", "produce a help message")
        ("size", ps::value<std::size_t>(&n)->required(), "matrix size")
        ("threads", ps::value<std::size_t>(), "Parallelize using N threads")
        ("test", "Test methods equality")
        ("pool", "Use thread pool")
        ("iterations", ps::value(&iterations), "Run iteration N times and average results");

    ps::variables_map vm;
    try
    {
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv)
                                      .options(desc).run(), vm);
        boost::program_options::notify(vm);

    }
    catch (const boost::program_options::error& e)
    {
        std::cout << "ERROR: " << e.what() << "\n";
        std::cout << desc << "\n";
        return 1;
    }

    if (vm.count("help"))
    {
        std::cout << desc << "\n";
        return 1;
    }
    if (vm.count("threads"))
        threads = vm.at("threads").as<std::size_t>();

    if (vm.count("test"))
    {
        test(n);
        std::cout << "successful\n";
        return 0;
    }

    std::cout << run(n, threads, iterations, vm.count("pool"));

    return 0;
}
