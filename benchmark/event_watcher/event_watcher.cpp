#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>

#include <eventpp/event_loop.hpp>
#include <eventpp/event_loop_thread.hpp>
#include <eventpp/event_watcher.hpp>
#include <eventpp/event_watcher_pipe.hpp>

static void Handle(eventpp::EventLoop * ev_loop, std::atomic<int64_t> * counter)
{
    counter->fetch_sub(1);
}


static eventpp::PipeEventWatcher * ew_pipe;

static void initPipeEventWatcher(eventpp::EventLoopThread * ev_loop, std::atomic<int64_t> * counter)
{
    ew_pipe = new eventpp::PipeEventWatcher(ev_loop->loop(), std::bind(&Handle, ev_loop->loop(), counter));
    ew_pipe->Init();
    ew_pipe->AsyncWait();
    ev_loop->Start();
}

static void freePipeEventWatcher()
{
    delete ew_pipe;
}

int eventWatcherPipeBenchmark(int loop, eventpp::EventLoop * ev_loop, std::atomic<int64_t> * counter)
{
    for (int i = 0; i < loop; ++i)
    {
        ew_pipe->Notify();
    }
    ev_loop->Stop();
    return true;
}


static eventpp::EventWatcher * ew;

static void initEventWatcher(eventpp::EventLoopThread * ev_loop, std::atomic<int64_t> * counter)
{
    ew = new eventpp::EventWatcher(ev_loop->loop(), std::bind(&Handle, ev_loop->loop(), counter));
    ew->Init();
    ew->AsyncWait();
    ev_loop->Start();
}

static void freeEventWatcher()
{
    delete ew;
}

int eventWatcherBenchmark(int loop, eventpp::EventLoop * ev_loop, std::atomic<int64_t> * counter)
{
    for (int i = 0; i < loop; ++i)
    {
        ew->Notify();
    }
    ev_loop->Stop();
    return true;
}


typedef std::function<void(eventpp::EventLoopThread *, std::atomic<int64_t> *)> InitEventFunctor;
typedef std::function<void()> FreeEventFunctor;
typedef std::function<bool(int, eventpp::EventLoop *, std::atomic<int64_t> *)> BenchmarkFunctor;

void Benchmark(InitEventFunctor eventF, FreeEventFunctor freeF, BenchmarkFunctor f, int loop, const std::string & name)
{
    std::atomic<int64_t> counter(loop);
    eventpp::EventLoopThread ev_loop_th;

    eventF(&ev_loop_th, &counter);

    auto start
        = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    bool rc = f(loop, ev_loop_th.loop(), &counter);
    ev_loop_th.Join();

    if (counter.load() == loop)
    {
        std::cerr << name << " counter not decresed\n";
    }
    else if (rc)
    {
        auto end
            = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        auto cost = double(end - start) / 1000000.0;
        std::cout << name << " loop=" << loop << " cost=" << cost << "s op=" << double(end - start) * 1000.0 / loop
                  << "ns/op QPS=" << (loop / cost) / 1024.0 << "k\n";
    }
    else
    {
        std::cerr << name << " failed\n";
    }
    freeF();
}


/*
Linux 3.10.0-327.28.3.el7.x86_64 test result:

         gettimeofday_benchmark loop=10000000 cost=0.3642s op= 36.427ns/op QPS=26808.5k
         system_clock_benchmark loop=10000000 cost=1.2324s op=123.249ns/op QPS=7923.51k
         steady_clock_benchmark loop=10000000 cost=1.2244s op=122.441ns/op QPS=7975.82k
high_resolution_clock_benchmark loop=10000000 cost=1.2508s op=125.082ns/op QPS=7807.36k
*/

int main()
{
    int loop = 1000 * 1000 * 10;
    Benchmark(&initPipeEventWatcher, &freePipeEventWatcher, &eventWatcherPipeBenchmark, loop, "     PipeEventWatcher");
    Benchmark(&initEventWatcher, &freeEventWatcher, &eventWatcherBenchmark, loop, "         EventWatcher");
    return 0;
}
