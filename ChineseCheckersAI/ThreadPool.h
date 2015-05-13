#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    // Don't allow copies (the functions below are for the rule of 5)
    // copy ctor
    ThreadPool(const ThreadPool&) = delete;
    // move ctor
    ThreadPool(const ThreadPool&&) = delete;
    // copy assignment
    ThreadPool &operator=(const ThreadPool&) = delete;
    // move assignment
    ThreadPool &operator=(const ThreadPool&&) = delete;

    int getQueuedJobs();
    int getNumThreads();

    void queueJob(std::function<void(void*)> func, void* data);
private:
    std::queue<std::pair<std::function<void(void*)>, void*>> queuedJobs;
    std::mutex queuedJobsMutex;
    int activeJobs;
    bool running;
    std::vector<std::thread> threads;
    void runThreadLoop();
};