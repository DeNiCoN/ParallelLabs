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

struct Task {
    size_t parent_id;
    size_t id;
};

std::atomic_size_t first_killed = 0;
std::atomic_size_t second_killed = 0;
std::atomic_size_t generated = 0;
std::atomic_size_t max_queue = 0;

double PERIOD = 0.25;

template<typename T>
void update_maximum(std::atomic<T>& maximum_value, T const& value) noexcept
{
    T prev_value = maximum_value;
    while(prev_value < value &&
            !maximum_value.compare_exchange_weak(prev_value, value))
        {}
}


class CPUQueue
{
public:
    explicit CPUQueue(std::size_t max_size)
        : m_max_size(max_size)
    {}

    void put(Task task)
    {
        {
            std::unique_lock lock(m_mutex);
            m_max_size_cv.wait(lock, [&] { return m_queue.size() < m_max_size; });
            m_queue.push(std::move(task));
            update_maximum(max_queue, m_queue.size());
        }
        m_empty_cv.notify_one();
    }

    Task pop()
    {
        Task result;
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
        std::shared_lock lock(m_mutex);
        return m_queue.size();
    }

    bool empty() const
    {
        std::shared_lock lock(m_mutex);
        return m_queue.empty();
    }

private:
    std::queue<Task> m_queue;
    std::size_t m_max_size;
    mutable std::shared_mutex m_mutex;
    std::condition_variable_any m_max_size_cv;
    std::condition_variable_any m_empty_cv;
};

class CPU
{
public:
    explicit CPU(CPUQueue& queue)
        : m_queue(queue),
          m_thread([&] (std::stop_token stoken) {impl(std::move(stoken));})
    {
        std::printf("CPU has started execution\n");
    }

    ~CPU()
    {
        std::printf("CPU is allowed to halt\n");
        m_thread.request_stop();
    }

    shared_lock<shared_mutex> lockForReading() const
    {
        return shared_lock(m_currentTaskMutex);
    }

    unique_lock<shared_mutex> lockForChanging()
    {
        return unique_lock(m_currentTaskMutex);
    }

    Task getCurrentTask() const
    {
        return m_currentTask;
    }

    void setCurrentTask(Task task)
    {
        m_currentTask = std::move(task);
    }

    void interrupt() {
        m_interrupt = true;
    }

private:
    std::jthread m_thread;
    mutable std::shared_mutex m_currentTaskMutex;
    Task m_currentTask;
    std::condition_variable_any m_interuptCv;
    bool m_interrupt = false;

    CPUQueue& m_queue;

    bool process(const Task& task)
    {
        const int POLL_RATE = 10;
        int wait = random(100, 1000) * PERIOD;
        std::printf("CPU: process %zu_%zu\n", task.parent_id, task.id);
        for (int i = 0; i < wait / POLL_RATE; i++)
        {
            std::unique_lock lock(m_currentTaskMutex);
            if (m_interuptCv.wait_for(lock, milliseconds(POLL_RATE),
                                      [&] { return m_interrupt; })) {
                std::printf("CPU: interrupt %zu_%zu\n", task.parent_id, task.id);
                return false;
            }

        }
        std::printf("CPU: finished %zu_%zu %d\n", task.parent_id, task.id, wait);
        return true;
    }

    void impl(std::stop_token stoken)
    {
        while (!(stoken.stop_requested() && m_queue.empty()))
        {
            Task current_task;
            {
                auto lock = lockForChanging();
                current_task = m_queue.pop();
                setCurrentTask(current_task);
                m_interrupt = false;
            }
            while (!process(current_task)) {
                auto lock = lockForChanging();
                current_task = getCurrentTask();
                m_interrupt = false;
            }
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
    std::size_t m_id = 0;
    std::jthread m_thread;
    CPUQueue& m_queue;

    void impl()
    {
        while(m_left--)
        {
            int wait = random(100, 1000) * PERIOD;
            std::this_thread::sleep_for(milliseconds(wait));
            std::printf("2: generated %zu\n", m_id);
            m_queue.put({2, m_id});
            m_id++;
            generated++;
        }
    }
};

class CPUFirstProcess
{
public:
    CPUFirstProcess(CPU& cpu, CPUQueue& queue, std::size_t count_to_generate)
        : m_cpu(cpu), m_left(count_to_generate), m_queue(queue),
          m_thread(&CPUFirstProcess::impl, this)
    {}

private:
    std::string m_currentTask;
    std::size_t m_left;
    std::size_t m_id = 0;
    std::jthread m_thread;
    CPUQueue& m_queue;
    CPU& m_cpu;

    void impl()
    {
        while(m_left--)
        {
            int wait = random(100, 1000) * PERIOD;
            std::this_thread::sleep_for(milliseconds(wait));
            std::printf("1: generated %zu\n", m_id);

            Task current;
            {
                auto lock = m_cpu.lockForChanging();
                current = m_cpu.getCurrentTask();
                m_cpu.setCurrentTask({1, m_id});
                m_id++;
                generated++;
                m_cpu.interrupt();
            }

            if (current.parent_id == 2)
            {
                m_queue.put(current);
                second_killed++;
            }
            else
            {
                first_killed++;
            }
        }
    }
};

int main(int argc, char *argv[]) {
    {
        CPUQueue queue(100);
        CPU cpu(queue);
        CPUFirstProcess first_process(cpu, queue, 100);
        CPUSecondProcess second_process(queue, 100);
    }

    std::printf("Max_queue: %zu\n", max_queue.load());
    std::printf("Generated: %zu\n", generated.load());
    std::printf("First killed: %f\n",
                static_cast<double>(first_killed.load()) / generated.load());
    std::printf("Second inerrupted: %zu\n", second_killed.load());
    return 0;
}
