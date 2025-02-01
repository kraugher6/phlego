#include "cpu_runner.h"

#include "logger.h"

CPURunner::CPURunner(CPU &cpu) : cpu(cpu), running(false) {}

void CPURunner::run() {
    running = true;
    fetch_thread = std::thread(&CPURunner::fetch_thread_function, this);
    decode_thread = std::thread(&CPURunner::decode_thread_function, this);
    execute_thread = std::thread(&CPURunner::execute_thread_function, this);
    mem_thread = std::thread(&CPURunner::mem_thread_function, this);
    write_back_thread =
        std::thread(&CPURunner::write_back_thread_function, this);

    LOG_INFO("Started CPU threads");

    while (running) {
        if (cpu.is_halted()) {
            running = false;
            stop();
            LOG_INFO(
                "Encountered halt flag. Terminating "
                "execution.");
        }
    }

    LOG_INFO("CPU state after execution:");
    cpu.print_registers();
}

void CPURunner::stop() {
    fetch_cv.notify_one();
    decode_cv.notify_one();
    execute_cv.notify_one();
    mem_cv.notify_one();
    write_back_cv.notify_one();

    if (fetch_thread.joinable()) fetch_thread.join();
    if (decode_thread.joinable()) decode_thread.join();
    if (execute_thread.joinable()) execute_thread.join();
    if (mem_thread.joinable()) mem_thread.join();
    if (write_back_thread.joinable()) write_back_thread.join();

    LOG_INFO("Stopped CPU threads");
}

void CPURunner::fetch_thread_function() {
    std::unique_lock<std::mutex> lock(fetch_mutex);
    while (running) {
        LOG_DEBUG("Fetch thread waiting");
        fetch_cv.wait(lock, [this] { return cpu.can_fetch() || !running; });
        if (!running) break;
        LOG_DEBUG("Fetch thread running");
        cpu.fetch();
        decode_cv.notify_one();
    }
    LOG_INFO("Fetch thread exiting");
}

void CPURunner::decode_thread_function() {
    std::unique_lock<std::mutex> lock(decode_mutex);
    while (running) {
        LOG_DEBUG("Decode thread waiting");
        decode_cv.wait(lock, [this] { return cpu.can_decode() || !running; });
        if (!running) break;
        LOG_DEBUG("Decode thread running");
        cpu.decode();
        fetch_cv.notify_one();
        execute_cv.notify_one();
    }
    LOG_INFO("Decode thread exiting");
}

void CPURunner::execute_thread_function() {
    std::unique_lock<std::mutex> lock(execute_mutex);
    while (running) {
        LOG_DEBUG("Execute thread waiting");
        execute_cv.wait(lock, [this] { return cpu.can_execute() || !running; });
        if (!running) break;
        LOG_DEBUG("Execute thread running");
        cpu.execute();
        mem_cv.notify_one();
    }
}

void CPURunner::mem_thread_function() {
    std::unique_lock<std::mutex> lock(mem_mutex);
    while (running) {
        LOG_DEBUG("Mem thread waiting");
        mem_cv.wait(lock, [this] { return cpu.can_mem() || !running; });
        if (!running) break;
        LOG_DEBUG("Mem thread running");
        cpu.mem();
        write_back_cv.notify_one();
    }
}

void CPURunner::write_back_thread_function() {
    std::unique_lock<std::mutex> lock(write_back_mutex);
    while (running) {
        LOG_DEBUG("Write back thread waiting");
        write_back_cv.wait(lock,
                           [this] { return cpu.can_write_back() || !running; });
        if (!running) break;
        LOG_DEBUG("Write back thread running");
        cpu.write_back();
    }
}