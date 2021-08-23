#ifndef __EVENTPP_TIMESTAMP_INLINE_HPP__
#define __EVENTPP_TIMESTAMP_INLINE_HPP__

#include "gettimeofday.h"

namespace eventpp {
inline Timestamp::Timestamp()
    : ns_(0) {}

inline Timestamp::Timestamp(int64_t nanoseconds)
    : ns_(nanoseconds) {}

inline bool Timestamp::IsEpoch() const {
    return ns_ == 0;
}

inline Timestamp::Timestamp(const struct timeval& t)
    : ns_(int64_t(t.tv_sec) * duration::kSecond + t.tv_usec * duration::kMicrosecond) {}

inline Timestamp Timestamp::Now() {
    // gettimeofday have a higher performance than std::chrono::system_clock or std::chrono::high_resolution_clock
    // Detail benchmark can see benchmark/gettimeofday/gettimeofday.cc
#if 1
    return Timestamp(int64_t(eventpp::utcmicrosecond() * duration::kMicrosecond));
#else
    return Timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
#endif
}

inline void Timestamp::Add(Duration d) {
    ns_ += d.Nanoseconds();
}

inline void Timestamp::To(struct timeval* t) const {
    t->tv_sec = (long)(ns_ / duration::kSecond);
    t->tv_usec = (long)(ns_ % duration::kSecond) / (long)duration::kMicrosecond;
}

inline struct timeval Timestamp::TimeVal() const {
    struct timeval t;
    To(&t);
    return t;
}

inline int64_t Timestamp::Unix() const {
    return ns_ / duration::kSecond;
}

inline int64_t Timestamp::UnixNano() const {
    return ns_;
}

inline int64_t Timestamp::UnixMicro() const {
    return ns_ / duration::kMicrosecond;
}

inline bool Timestamp::operator< (const Timestamp& rhs) const {
    return ns_ < rhs.ns_;
}

inline bool Timestamp::operator==(const Timestamp& rhs) const {
    return ns_ == rhs.ns_;
}

inline Timestamp Timestamp::operator+=(const Duration& rhs) {
    ns_ += rhs.Nanoseconds();
    return *this;
}

inline Timestamp Timestamp::operator+(const Duration& rhs) const {
    Timestamp temp(*this);
    temp += rhs;
    return temp;
}

inline Timestamp Timestamp::operator-=(const Duration& rhs) {
    ns_ -= rhs.Nanoseconds();
    return *this;
}

inline Timestamp Timestamp::operator-(const Duration& rhs) const {
    Timestamp temp(*this);
    temp -= rhs;
    return temp;
}

inline Duration Timestamp::operator-(const Timestamp& rhs) const {
    int64_t ns = ns_ - rhs.ns_;
    return Duration(ns);
}
} // namespace eventpp


#endif // __EVENTPP_TIMESTAMP_INLINE_HPP__
