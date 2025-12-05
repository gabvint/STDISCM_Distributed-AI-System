#include "thread_pool.h"

ThreadPool::ThreadPool(std::size_t numThreads) : stop_(false) {
    for (std::size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back([this]() {
            for (;;) {
                std::function<void()> job;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cv_.wait(lock, [this]() { return stop_ || !jobs_.empty(); });
                    if (stop_ && jobs_.empty()) {
                        return;
                    }
                    job = std::move(jobs_.front());
                    jobs_.pop();
                }
                job();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cv_.notify_all();
    for (auto& w : workers_) {
        if (w.joinable()) {
            w.join();
        }
    }
}
