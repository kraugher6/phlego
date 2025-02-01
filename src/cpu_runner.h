#ifndef CPU_RUNNER_H
#define CPU_RUNNER_H

#include "cpu.h"
#include <thread>
#include <mutex>
#include <condition_variable>

class CPURunner
{
public:
    CPURunner(CPU &cpu);
    void run();
    void stop();

private:
    CPU &cpu;
    bool is_running;
    std::thread fetch_thread;
    std::thread decode_thread;
    std::thread execute_thread;
    std::thread mem_thread;
    std::thread write_back_thread;

    void fetch_thread_function();
    void decode_thread_function();
    void execute_thread_function();
    void mem_thread_function();
    void write_back_thread_function();

    std::mutex fetch_mutex;
    std::mutex decode_mutex;
    std::mutex execute_mutex;
    std::mutex mem_mutex;
    std::mutex write_back_mutex;

    std::condition_variable fetch_cv;
    std::condition_variable decode_cv;
    std::condition_variable execute_cv;
    std::condition_variable mem_cv;
    std::condition_variable write_back_cv;
};

#endif // CPU_RUNNER_H