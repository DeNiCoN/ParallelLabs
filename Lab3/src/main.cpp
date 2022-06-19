#include <iostream>
#include <vector>
#include <random>
#include <ranges>
#include <algorithm>
#include <bits/stdc++.h>
#include <boost/program_options.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <execution>

using namespace std;

template<typename T>
void fill_random(std::vector<T>& mat)
{
    std::random_device r;
    std::uniform_int_distribution<T> uniform(10, 9999999);
    std::default_random_engine prng(r());

    std::ranges::generate(mat, [&] {return uniform(prng);});
}

template<typename T>
int update_maximum(std::atomic<T>& maximum_value, T const& value) noexcept
{
    T prev_value = maximum_value;
    while(prev_value < value &&
            !maximum_value.compare_exchange_weak(prev_value, value))
        {}
    return prev_value < value ? prev_value : 0;
}

void print3largestSeq(const std::vector<int>& arr)
{
    int first, second, third;
    third = first = second = INT_MIN;

    for(int i = 0; i < arr.size(); i++)
    {

        if (arr[i] > first)
        {
            third = second;
            second = first;
            first = arr[i];
        }

        else if (arr[i] > second)
        {
            third = second;
            second = arr[i];
        }

        else if (arr[i] > third)
            third = arr[i];
    }

    cout << "Sequential: "
        << first << " " << second << " "
        << third << endl;
}

void print3largestAtomic(const std::vector<int>& arr)
{
    std::atomic_int first, second, third;
    third = first = second = INT_MIN;

    boost::asio::thread_pool pool(std::thread::hardware_concurrency());

    for (int i = 0; i < arr.size(); i++)
    {
        boost::asio::post(pool, [&, i] {
            if (int f = update_maximum(first, arr[i]))
            {
                update_maximum(third, second.load());
                update_maximum(second, f);
            }
            else if (int s = update_maximum(second, arr[i]))
            {
                update_maximum(third, s);
            }
            else
            {
                update_maximum(third, arr[i]);
            }
        });
    }

    pool.join();

    cout << "Non-blocking boost pool: "
        << first << " " << second << " "
        << third << endl;
}

void print3largestAtomicStd(const std::vector<int>& arr)
{
    std::atomic_int first, second, third;
    third = first = second = INT_MIN;

    std::for_each(std::execution::par, arr.begin(), arr.end(), [&] (auto e) {
        if (int f = update_maximum(first, e))
        {
            update_maximum(third, second.load());
            update_maximum(second, f);
        }
        else if (int s = update_maximum(second, e))
        {
            update_maximum(third, s);
        }
        else
        {
            update_maximum(third, e);
        }
    });

    cout << "Non-blocking std: "
        << first << " " << second << " "
        << third << endl;
}


int main(int argc, char *argv[]) {
    std::cout << "Hello World" << std::endl;
    std::vector<int> arr(100000*20);
    fill_random(arr);

    print3largestSeq(arr);
    print3largestAtomic(arr);
    print3largestAtomicStd(arr);
    return 0;
}
