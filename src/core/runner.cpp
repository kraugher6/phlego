#include "core/runner.h"
#include "logger.h"
#include <csignal>

namespace phlego {

Runner::Runner(Pipeline& pipe) : pipeline(pipe), running(false) {}

void Runner::run() {
    running = true;
    LOG_INFO("Runner started (pipeline mode)");

    while (running) {
        if (!pipeline.tick()) {
            running = false;
            break;
        }
    }

    LOG_INFO("Runner stopped.");
}

void Runner::stop() {
    running = false;
}

} // namespace phlego
