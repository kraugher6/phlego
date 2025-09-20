#include "cpu_runner.h"
#include "logger.h"

/**
 * @brief Construct a new CPURunner object.
 *
 * @param cpu Reference to the CPU object.
 */
CPURunner::CPURunner(CPU &cpu) : cpu(cpu), is_running(false) {}

/**
 * @brief Start the CPU runner and its threads.
 */
void CPURunner::run() {
    is_running = true;

    try {
        fetch_thread = std::thread(&CPURunner::fetch_thread_function, this);
        decode_thread = std::thread(&CPURunner::decode_thread_function, this);
        execute_thread = std::thread(&CPURunner::execute_thread_function, this);
        mem_thread = std::thread(&CPURunner::mem_thread_function, this);
        write_back_thread = std::thread(&CPURunner::write_back_thread_function, this);
    } catch (const std::exception &e) {
        LOG_ERROR("Failed to create threads: " + std::string(e.what()));
        stop();
        return;
    }

    LOG_INFO("Started CPU threads");

    // Avvia solo la fetch all'inizio
    fetch_cv.notify_one();

    while (is_running) {
        if (cpu.is_halted()) {
            is_running = false;
            stop();
            LOG_INFO("Encountered halt flag. Terminating execution.");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Evita busy loop
    }

    LOG_INFO("CPU state after execution:");
    cpu.print_registers();
}

/**
 * @brief Stop the CPU runner and its threads.
 */
void CPURunner::stop() {
    is_running = false;

    fetch_cv.notify_all();
    decode_cv.notify_all();
    execute_cv.notify_all();
    mem_cv.notify_all();
    write_back_cv.notify_all();

    if (fetch_thread.joinable()) fetch_thread.join();
    if (decode_thread.joinable()) decode_thread.join();
    if (execute_thread.joinable()) execute_thread.join();
    if (mem_thread.joinable()) mem_thread.join();
    if (write_back_thread.joinable()) write_back_thread.join();

    LOG_INFO("Stopped CPU threads");
}

void CPURunner::fetch_thread_function() {
    std::unique_lock<std::mutex> lock(fetch_mutex);
    while (is_running) {
        fetch_cv.wait(lock, [this] { return !is_running || cpu.can_fetch(); });
        if (!is_running) break;
        LOG_DEBUG("Fetch thread running");
        cpu.fetch();
        decode_cv.notify_one(); // Sblocca decode
    }
    LOG_INFO("Fetch thread exiting");
}

void CPURunner::decode_thread_function() {
    std::unique_lock<std::mutex> lock(decode_mutex);
    while (is_running) {
        decode_cv.wait(lock, [this] { return !is_running || cpu.can_decode(); });
        if (!is_running) break;
        LOG_DEBUG("Decode thread running");
        cpu.decode();
        execute_cv.notify_one(); // Sblocca execute
    }
    LOG_INFO("Decode thread exiting");
}

void CPURunner::execute_thread_function() {
    std::unique_lock<std::mutex> lock(execute_mutex);
    while (is_running) {
        execute_cv.wait(lock, [this] { return !is_running || cpu.can_execute(); });
        if (!is_running) break;
        LOG_DEBUG("Execute thread running");
        cpu.execute();
        mem_cv.notify_one(); // Sblocca mem
    }
    LOG_INFO("Execute thread exiting");
}

void CPURunner::mem_thread_function() {
    std::unique_lock<std::mutex> lock(mem_mutex);
    while (is_running) {
        mem_cv.wait(lock, [this] { return !is_running || cpu.can_mem(); });
        if (!is_running) break;
        LOG_DEBUG("Mem thread running");
        cpu.mem();
        write_back_cv.notify_one(); // Sblocca write_back
    }
    LOG_INFO("Mem thread exiting");
}

void CPURunner::write_back_thread_function() {
    std::unique_lock<std::mutex> lock(write_back_mutex);
    while (is_running) {
        write_back_cv.wait(lock, [this] { return !is_running || cpu.can_write_back(); });
        if (!is_running) break;
        LOG_DEBUG("Write back thread running");
        cpu.write_back();
        fetch_cv.notify_one(); // Sblocca fetch per il prossimo ciclo
    }
    LOG_INFO("Write back thread exiting");
}