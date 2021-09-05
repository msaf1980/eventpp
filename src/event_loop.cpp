#include "pch.h"

// #include "evpp/inner_pre.h"

// #include "evpp/event_watcher.h"
#include <eventpp/event_loop.hpp>
#include <eventpp/invoke_timer.hpp>

#include <eventpp/trace.hpp>

namespace eventpp
{
EventLoop::EventLoop(bool preciser_timer, Duration shutdown_timeout) : evbase_destroy_(true), notified_(false), pending_functor_count_(0), shutdown_timeout_(shutdown_timeout)
{
#if LIBEVENT_VERSION_NUMBER >= 0x02001500
    if (preciser_timer)
    {
        struct event_config * cfg = event_config_new();
        if (cfg)
        {
            // Does not cache time to get a preciser timer
            event_config_set_flag(cfg, EVENT_BASE_FLAG_NO_CACHE_TIME);
            evbase_ = event_base_new_with_config(cfg);
            event_config_free(cfg);
        }
        else
        {
            throw std::bad_alloc();
        }
    }
    else
#endif
    {
        evbase_ = event_base_new();
        if (evbase_ == nullptr)
        {
            throw std::bad_alloc();
        }
    }

    try
    {
        Init();
    }
    catch(...)
    {
        Free();

        throw;
    }
}

EventLoop::EventLoop(struct event_base * base) : evbase_(base), evbase_destroy_(false), notified_(false), pending_functor_count_(0)
{
    Init();

    // When we build an EventLoop instance from an existing event_base
    // object, we will never call EventLoop::Run() method.
    // So we need to watch the task queue here.
    if (!watcher_->AsyncWait())
    {
        throw std::runtime_error("PipeEventWatcher init failed");
    }
    status_.store(kRunning);
}

EventLoop::~EventLoop()
{
    Free();
}

void EventLoop::Init()
{
    // protect from duplicate init
    ServerStatus::Status status = kNull;
    if (status_.compare_exchange_weak(status, kInitializing, std::memory_order_release, std::memory_order_relaxed))
    {
#ifdef H_HAVE_BOOST
        const size_t kPendingFunctorCount = 1024 * 16;
        this->pending_functors_ = new boost::lockfree::queue<Functor *>(kPendingFunctorCount);
#elif defined(H_HAVE_CAMERON314_CONCURRENTQUEUE)
        this->pending_functors_ = new moodycamel::ConcurrentQueue<Functor>();
#else
        this->pending_functors_ = new std::vector<Functor>();
#endif

        tid_ = std::this_thread::get_id(); // The default thread id

        if (!InitNotifyPipeWatcher())
        {
            throw std::runtime_error("EventLoop::InitNotifyPipeWatcher() failed");
        }

        status_.store(kInitialized);

    }
}

bool EventLoop::InitNotifyPipeWatcher()
{
    // Initialized task queue notify pipe watcher
    watcher_.reset(new EventWatcher(this, std::bind(&EventLoop::DoPendingFunctors, this)));
    return watcher_->Init();
}

void EventLoop::Free()
{
    watcher_.reset();

    if (evbase_ != nullptr && evbase_destroy_)
    {
        event_base_free(evbase_);
        evbase_ = nullptr;
    }

    delete pending_functors_;
    pending_functors_ = nullptr;
}

void EventLoop::Run()
{
    // protect from duplicate run
    ServerStatus::Status status = kInitialized;
    if (status_.compare_exchange_weak(status, kStarting, std::memory_order_release, std::memory_order_relaxed))
    {
        tid_ = std::this_thread::get_id(); // The actual thread id

        if (!watcher_->AsyncWait())
        {
            throw std::runtime_error("EventLoop::watcher::AsyncWait()) failed");
        }

        // After everything have initialized, we set the status to kRunning
        status_.store(kRunning);

        int serrno = 0;
        int rc = event_base_dispatch(evbase_);
        if (rc == -1)
        {
            serrno = errno;
        }

        // shutdown
        // DLOG_TRACE << "Shutdown loop with pipe " << watcher_->wfd() << std:: endl;

        // Make sure watcher_ does construct, initialize and destruct in the same thread.
        watcher_.reset();

        status_.store(kStopped);

        // check event_base_dispatch result
        if (rc == 1)
        {
            throw std::runtime_error("event_base_dispatch error: no event registered");
        }
        else if (rc == -1)
        {
            throw std::system_error(serrno, std::generic_category(), "event_base_dispatch error");
        }
    }
}

void EventLoop::Stop()
{
    ServerStatus::Status status = kRunning;
    if (status_.compare_exchange_weak(status, kStopping, std::memory_order_release, std::memory_order_relaxed))
    {
        notified_ = false;
        // DLOG_TRACE << "Initiate shutdown loop " << watcher_->wfd() << std::endl;
        QueueInLoop(std::bind(&EventLoop::StopInLoop, this));
    }
}

void EventLoop::FlushPendingFunctors()
{
    while (1)
    {
        DoPendingFunctors();
        if (IsPendingQueueEmpty())
        {
            break;
        }
    }
}

void EventLoop::StopInLoop()
{
    assert(status_.load() == kStopping);

    FlushPendingFunctors();

    auto timeout = shutdown_timeout_.TimeVal();

    event_base_loopexit(evbase_, &timeout);

    // DLOG_TRACE << "Shutdown loop " << watcher_->wfd() << std::endl;

    FlushPendingFunctors();
}

void EventLoop::AfterFork()
{
    int rc = event_reinit(evbase_);
    if (rc != 0)
    {
        throw std::runtime_error("event_reinit failed");
    }

    // We create EventLoopThread and initialize it in father process,
    // but we use it in child process.
    // If we have only one child process, everything goes well.
    //
    // But if we have multi child processes, something goes wrong.
    // Because EventLoop::watcher_ is created and initialized in father process
    // all children processes inherited father's pipe.
    //
    // When we use the pipe to do a notification in one child process
    // the notification may be received by another child process randomly.
    //
    // So we need to reinitialize the watcher_
    InitNotifyPipeWatcher();
}

InvokeTimerPtr EventLoop::RunAfter(double delay_ms, const Functor & f)
{
    return RunAfter(Duration(delay_ms / 1000.0), f);
}

InvokeTimerPtr EventLoop::RunAfter(double delay_ms, Functor && f)
{
    return RunAfter(Duration(delay_ms / 1000.0), std::move(f));
}

InvokeTimerPtr EventLoop::RunAfter(Duration delay, const Functor & f)
{
    std::shared_ptr<InvokeTimer> t = InvokeTimer::Create(this, delay, f, false);
    t->Start();
    return t;
}

InvokeTimerPtr EventLoop::RunAfter(Duration delay, Functor && f)
{
    std::shared_ptr<InvokeTimer> t = InvokeTimer::Create(this, delay, std::move(f), false);
    t->Start();
    return t;
}

InvokeTimerPtr EventLoop::RunEvery(Duration interval, const Functor & f)
{
    std::shared_ptr<InvokeTimer> t = InvokeTimer::Create(this, interval, f, true);
    t->Start();
    return t;
}

InvokeTimerPtr EventLoop::RunEvery(Duration interval, Functor && f)
{
    std::shared_ptr<InvokeTimer> t = InvokeTimer::Create(this, interval, std::move(f), true);
    t->Start();
    return t;
}

void EventLoop::RunInLoop(const Functor & functor)
{
    if (IsRunning() && IsInLoopThread())
    {
        functor();
    }
    else
    {
        QueueInLoop(functor);
    }
}

void EventLoop::RunInLoop(Functor && functor)
{
    if (IsRunning() && IsInLoopThread())
    {
        functor();
    }
    else
    {
        QueueInLoop(std::move(functor));
    }
}

void EventLoop::QueueInLoop(const Functor & cb)
{
    // DLOG_TRACE << "pending_functor_count_=" << pending_functor_count_ << " PendingQueueSize=" << GetPendingQueueSize()
    //            << " notified_=" << notified_.load();
    {
#ifdef H_HAVE_BOOST
        auto f = new Functor(cb);
        while (!pending_functors_->push(f))
        {
        }
#elif defined(H_HAVE_CAMERON314_CONCURRENTQUEUE)
        while (!pending_functors_->enqueue(cb))
        {
        }
#else
        std::lock_guard<std::mutex> lock(mutex_);
        pending_functors_->emplace_back(cb);
#endif
    }
    ++pending_functor_count_;
    // DLOG_TRACE << "queued a new Functor. pending_functor_count_=" << pending_functor_count_ << " PendingQueueSize=" << GetPendingQueueSize()
    //            << " notified_=" << notified_.load();

    bool notified = false;
    if (notified_.compare_exchange_weak(notified, true, std::memory_order_release, std::memory_order_relaxed))
    {
        // DLOG_TRACE << "call watcher_->Nofity() notified_.store(true)";

        // We must set notified_ to true before calling `watcher_->Nodify()`
        // otherwise there is a change that:
        //  1. We called watcher_- > Nodify() on thread1
        //  2. On thread2 we watched this event, so wakeup the CPU changed to run this EventLoop on thread2 and executed all the pending task
        //  3. Then the CPU changed to run on thread1 and set notified_ to true
        //  4. Then, some thread except thread2 call this QueueInLoop to push a task into the queue, and find notified_ is true, so there is no change to wakeup thread2 to execute this task

        // Sometimes one thread invoke EventLoop::QueueInLoop(...), but anther
        // thread is invoking EventLoop::Stop() to stop this loop. At this moment
        // this loop maybe is stopping and the watcher_ object maybe has been
        // released already.
        if (watcher_)
        {
            watcher_->Notify();
        }
        // else
        // {
        //     DLOG_TRACE << "status=" << StatusToString();
        //     assert(!IsRunning());
        // }
    }
    // else
    // {
    //     DLOG_TRACE << "No need to call watcher_->Nofity()";
    // }
}

void EventLoop::QueueInLoop(Functor && cb)
{
    // DLOG_TRACE << "pending_functor_count_=" << pending_functor_count_ << " PendingQueueSize=" << GetPendingQueueSize()
    //            << " notified_=" << notified_.load();
    {
#ifdef H_HAVE_BOOST
        auto f = new Functor(std::move(cb)); // TODO Add test code for it
        while (!pending_functors_->push(f))
        {
        }
#elif defined(H_HAVE_CAMERON314_CONCURRENTQUEUE)
        while (!pending_functors_->enqueue(std::move(cb)))
        {
        }
#else
        std::lock_guard<std::mutex> lock(mutex_);
        pending_functors_->emplace_back(std::move(cb));
#endif
    }
    ++pending_functor_count_;
    // DLOG_TRACE << "queued a new Functor. pending_functor_count_=" << pending_functor_count_ << " PendingQueueSize=" << GetPendingQueueSize()
    //            << " notified_=" << notified_.load();
    bool notified = false;
    if (notified_.compare_exchange_weak(notified, true, std::memory_order_release, std::memory_order_relaxed))
    {
        // DLOG_TRACE << "call watcher_->Nofity() notified_.store(true)";
        if (watcher_)
        {
            watcher_->Notify();
        }
        // else
        // {
        //     DLOG_TRACE << "watcher_ is empty, maybe we call EventLoop::QueueInLoop on a stopped EventLoop. status=" << StatusToString();
        //     assert(!IsRunning());
        // }
    }
    // else
    // {
    //     DLOG_TRACE << "No need to call watcher_->Nofity()";
    // }
}

void EventLoop::DoPendingFunctors()
{
    // DLOG_TRACE << "pending_functor_count_=" << pending_functor_count_ << " PendingQueueSize=" << GetPendingQueueSize()
    //            << " notified_=" << notified_.load();

    notified_.store(false);

#ifdef H_HAVE_BOOST
    Functor * f = nullptr;
    while (pending_functors_->pop(f))
    {
        (*f)();
        delete f;
        --pending_functor_count_;
    }
#elif defined(H_HAVE_CAMERON314_CONCURRENTQUEUE)
    Functor f;
    while (pending_functors_->try_dequeue(f))
    {
        f();
        --pending_functor_count_;
    }
#else
    std::vector<Functor> functors;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_functors_->swap(functors);
        // DLOG_TRACE << "pending_functor_count_=" << pending_functor_count_ << " PendingQueueSize=" << GetPendingQueueSize()
        //            << " notified_=" << notified_.load();
    }
    // DLOG_TRACE << "pending_functor_count_=" << pending_functor_count_ << " PendingQueueSize=" << GetPendingQueueSize()
    //            << " notified_=" << notified_.load();
    for (size_t i = 0; i < functors.size(); ++i)
    {
        functors[i]();
        --pending_functor_count_;
    }
    // DLOG_TRACE << "pending_functor_count_=" << pending_functor_count_ << " PendingQueueSize=" << GetPendingQueueSize()
    //            << " notified_=" << notified_.load();
#endif
}

size_t EventLoop::GetPendingQueueSize()
{
#ifdef H_HAVE_BOOST
    return static_cast<size_t>(pending_functor_count_.load());
#elif defined(H_HAVE_CAMERON314_CONCURRENTQUEUE)
    return pending_functors_->size_approx();
#else
    return pending_functors_->size();
#endif
}

bool EventLoop::IsPendingQueueEmpty()
{
#ifdef H_HAVE_BOOST
    return pending_functors_->empty();
#elif defined(H_HAVE_CAMERON314_CONCURRENTQUEUE)
    return pending_functors_->size_approx() == 0;
#else
    return pending_functors_->empty();
#endif
}

}
