#ifndef RUNNER_H
#define RUNNER_H

#include "core/pipeline.h"
#include <atomic>

namespace phlego {

class Runner {
public:
    Runner(Pipeline& pipe);
    
    void run();
    void stop();
    bool is_running() const { return running; }

private:
    Pipeline& pipeline;
    std::atomic<bool> running;
};

} // namespace phlego

#endif
