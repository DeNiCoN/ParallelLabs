#include <iostream>
#include <cstdio>
#include <queue>
#include <string>
#include <condition_variable>
#include <mutex>
#include <future>
#include <thread>
#include <random>
#include <shared_mutex>

using namespace std;
using namespace chrono;

int random(int from, int to)
{
    static std::random_device r;
    static std::default_random_engine prng(r());

    std::uniform_int_distribution uniform(from, to);

    return uniform(prng);
}

class CPUQueue
{
public:
    explicit CPUQueue(std::size_t max_size)
        : m_max_size(max_size)
    {}

    void put(std::string str)
    {
        {
            std::unique_lock lock(m_mutex);
            m_max_size_cv.wait(lock, [&] { return m_queue.size() < m_max_size; });
            m_queue.push(std::move(str));
        }
        m_empty_cv.notify_one();
    }

    std::string get()
    {
        std::string result;
        {
            std::unique_lock lock(m_mutex);
            m_empty_cv.wait(lock, [&] { return m_queue.size() > 0; });
            result = std::move(m_queue.front());
            m_queue.pop();
        }
        m_max_size_cv.notify_one();
        return result;
    }

    std::size_t size() const
    {
        std::scoped_lock lock(m_mutex);
        return m_queue.size();
    }

    bool empty() const
    {
        std::scoped_lock lock(m_mutex);
        return m_queue.empty();
    }

private:
    std::queue<std::string> m_queue;
    std::size_t m_max_size;
    mutable std::mutex m_mutex;
    std::condition_variable m_max_size_cv;
    std::condition_variable m_empty_cv;
};

class CPU
{
public:
    explicit CPU(CPUQueue& queue)
        : m_queue(queue), m_thread([&] (std::stop_token stoken) {impl(std::move(stoken));})
    {
        std::printf("CPU has started execution\n");
    }

    ~CPU()
    {
        std::printf("CPU is allowed to halt\n");
        m_thread.request_stop();
    }

    std::string getCurrentTask() const
    {
        std::shared_lock lock(m_currentTaskMutex);
        return m_currentTask;
    }

    void setCurrentTask(std::string task)
    {
        std::unique_lock lock(m_currentTaskMutex);
        m_currentTask = std::move(task);
    }

private:
    std::jthread m_thread;
    std::shared_mutex m_currentTaskMutex;
    std::string m_currentTask;
    CPUQueue& m_queue;

    void process(const std::string& str)
    {
        int wait = random(100, 3000);
        std::this_thread::sleep_for(milliseconds(wait));
        std::printf("CPU finished processing %s in: %d\n", str.c_str(), wait);
    }

    void impl(std::stop_token stoken)
    {
        while (!(stoken.stop_requested() && m_queue.empty()))
        {
            m_currentTask = m_queue.get();
            process(task);
        }
    }
};

class CPUSecondProcess
{
public:
    CPUSecondProcess(CPUQueue& queue, std::size_t count_to_generate)
        : m_left(count_to_generate), m_queue(queue),
          m_thread(&CPUSecondProcess::impl, this)
    {}

private:
    std::string m_currentTask;
    std::size_t m_left;
    std::jthread m_thread;
    CPUQueue& m_queue;

    void impl()
    {
        while(m_left--)
        {

            int wait = random(100, 3000);
            std::this_thread::sleep_for(milliseconds(wait));
            std::printf("Second process generated with delay %d\n", wait);
            m_queue.put("second");
        }
    }
};

class CPUFirstProcess
{

};

int main(int argc, char *argv[]) {
    CPUQueue queue(10);
    CPU cpu(queue);
    //CPUFirstProcess first_process(cpu, queue, 100);
    CPUSecondProcess second_process(queue, 15);
    return 0;
}
