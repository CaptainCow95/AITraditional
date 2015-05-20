#include "ThreadPool.h"

ThreadPool::ThreadPool()
{
    activeJobs = 0;
    running = true;

    // spawn the same number of threads as cpu cores, defaulting to 8 if it fails.
    unsigned numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0)
    {
        numThreads = 8;
    }

    for (size_t i = 0; i < numThreads; ++i)
    {
        threads.push_back(std::thread(&ThreadPool::runThreadLoop, this));
    }
}

ThreadPool::~ThreadPool()
{
    running = false;
    for (size_t i = 0; i < threads.size(); ++i)
    {
        threads[i].join();
    }
}

size_t ThreadPool::getQueuedJobs()
{
    queuedJobsMutex.lock();
    size_t returnValue = queuedJobs.size() + activeJobs;
    queuedJobsMutex.unlock();
    return returnValue;
}

size_t ThreadPool::getNumThreads()
{
    return threads.size();
}

void ThreadPool::queueJob(std::function<void(void*)> func, void* data)
{
    queuedJobsMutex.lock();
    queuedJobs.push(std::pair<std::function<void(void*)>, void*>(func, data));
    queuedJobsMutex.unlock();
}

void ThreadPool::runThreadLoop()
{
    while (running)
    {
        queuedJobsMutex.lock();

        if (queuedJobs.size() > 0)
        {
            auto data = queuedJobs.front();
            ++activeJobs;
            queuedJobs.pop();
            queuedJobsMutex.unlock();

            data.first(data.second);

            queuedJobsMutex.lock();
            --activeJobs;
            queuedJobsMutex.unlock();
            continue;
        }

        queuedJobsMutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}