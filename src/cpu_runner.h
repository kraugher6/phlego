#ifndef CPU_RUNNER_H
#define CPU_RUNNER_H

#include <condition_variable>
#include <mutex>
#include <thread>

#include "cpu.h"

/**
 * @brief Manages the execution of the CPU pipeline stages.
 *
 * Responsible for managing the threads that execute the CPU's pipeline
 * stages: fetch, decode, execute, memory, and write-back. Ensures proper
 * synchronization between these stages using condition variables and mutexes.
 */
class CPURunner {
   public:
    /**
     * @brief Construct a new CPURunner object.
     *
     * Initializes the CPURunner with a reference to the CPU object.
     *
     * @param cpu Reference to the CPU object.
     */
    CPURunner(CPU &cpu);

    /**
     * @brief Start the CPU runner and its threads.
     *
     * Initializes and starts the threads responsible for the CPU's pipeline
     * stages: fetch, decode, execute, memory, and write-back. Monitors the
     * CPU's state and stops execution if a halt flag is encountered.
     */
    void run();

    /**
     * @brief Stop the CPU runner and its threads.
     *
     * Stops all threads by setting the running flag to false and notifying all
     * condition variables. Ensures all threads are joined before returning.
     */
    void stop();

   private:
    CPU &cpu;         ///< Reference to the CPU object.
    bool is_running;  ///< Flag indicating whether the runner is active.

    // Threads for each pipeline stage.
    std::thread fetch_thread;
    std::thread decode_thread;
    std::thread execute_thread;
    std::thread mem_thread;
    std::thread write_back_thread;

    // Thread functions for each pipeline stage.
    /**
     * @brief Function executed by the fetch thread.
     *
     * Waits for the CPU to be ready to fetch instructions and performs the
     * fetch operation. Notifies the decode thread after completing the fetch.
     */
    void fetch_thread_function();

    /**
     * @brief Function executed by the decode thread.
     *
     * Waits for the CPU to be ready to decode instructions and performs the
     * decode operation. Notifies the execute thread and fetch thread after
     * completing the decode.
     */
    void decode_thread_function();

    /**
     * @brief Function executed by the execute thread.
     *
     * Waits for the CPU to be ready to execute instructions and performs the
     * execute operation. Notifies the memory thread after completing the
     * execution.
     */
    void execute_thread_function();

    /**
     * @brief Function executed by the memory thread.
     *
     * Waits for the CPU to be ready for memory operations and performs the
     * memory operation. Notifies the write-back thread after completing the
     * memory operation.
     */
    void mem_thread_function();

    /**
     * @brief Function executed by the write-back thread.
     *
     * Waits for the CPU to be ready for write-back operations and performs the
     * write-back operation. Exits when the running flag is set to false.
     */
    void write_back_thread_function();

    // Mutexes for synchronizing each pipeline stage.
    std::mutex fetch_mutex;
    std::mutex decode_mutex;
    std::mutex execute_mutex;
    std::mutex mem_mutex;
    std::mutex write_back_mutex;

    // Condition variables for signaling between pipeline stages.
    std::condition_variable fetch_cv;
    std::condition_variable decode_cv;
    std::condition_variable execute_cv;
    std::condition_variable mem_cv;
    std::condition_variable write_back_cv;
};

#endif  // CPU_RUNNER_H