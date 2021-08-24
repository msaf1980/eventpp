#ifndef __EVENTPP_THREAD_DISPATCH_POLICY_HPP__
#define __EVENTPP_THREAD_DISPATCH_POLICY_HPP__

namespace eventpp {
class ThreadDispatchPolicy {
public:
    enum Policy {
        kRoundRobin,
        kIPAddressHashing,
    };

    ThreadDispatchPolicy() : policy_(kRoundRobin) {}

    void SetThreadDispatchPolicy(Policy v) {
        policy_ = v;
    }

    bool IsRoundRobin() const {
        return policy_ == kRoundRobin;
    }
protected:
    Policy policy_;
};
}
#endif // __EVENTPP_THREAD_DISPATCH_POLICY_HPP__