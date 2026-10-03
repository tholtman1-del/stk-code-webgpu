#include "ge_wgpu_texture_loader.hpp"

#include <algorithm>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace GE
{
namespace GEWGPUTextureLoader
{
namespace
{
std::mutex g_mutex;
std::condition_variable g_cv;
std::deque<std::pair<const void*, std::function<void()> > > g_tasks;
std::vector<std::thread> g_threads;
bool g_quit = false;

// ----------------------------------------------------------------------------
void loop()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(g_mutex);
            g_cv.wait(lock, []() { return g_quit || !g_tasks.empty(); });
            if (g_tasks.empty())
                return;
            task = std::move(g_tasks.front().second);
            g_tasks.pop_front();
        }
        task();
    }
}   // loop

}   // anonymous namespace

// ----------------------------------------------------------------------------
void init()
{
    if (!g_threads.empty())
        return;
    // Leave a core for the main thread, which still reads the files and
    // uploads (Emscripten's thread pool size is raised accordingly)
    unsigned count = std::thread::hardware_concurrency();
    count = std::min(std::max(count, 2u) - 1, 4u);
    g_quit = false;
    for (unsigned i = 0; i < count; i++)
        g_threads.emplace_back(loop);
}   // init

// ----------------------------------------------------------------------------
void destroy()
{
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_quit = true;
    }
    g_cv.notify_all();
    for (std::thread& t : g_threads)
        t.join();
    g_threads.clear();
}   // destroy

// ----------------------------------------------------------------------------
void add(const void* owner, std::function<void()> task)
{
    if (g_threads.empty())
    {
        task();
        return;
    }
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_tasks.emplace_back(owner, std::move(task));
    }
    g_cv.notify_one();
}   // add

// ----------------------------------------------------------------------------
bool runNow(const void* owner)
{
    std::function<void()> task;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto it = std::find_if(g_tasks.begin(), g_tasks.end(),
            [owner](const std::pair<const void*, std::function<void()> >& t)
            { return t.first == owner; });
        if (it == g_tasks.end())
            return false;
        task = std::move(it->second);
        g_tasks.erase(it);
    }
    task();
    return true;
}   // runNow

}

}
