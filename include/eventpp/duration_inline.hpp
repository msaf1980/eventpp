#ifndef __EVENTPP_DURATION_INLINE_HPP__
#define __EVENTPP_DURATION_INLINE_HPP__

namespace eventpp {
    
inline Duration::Duration()
    : ns_(0) {}

inline Duration::Duration(const struct timeval& t)
    : ns_(t.tv_sec * duration::kSecond + t.tv_usec * duration::kMicrosecond) {}

inline Duration::Duration(int64_t nanoseconds)
    : ns_(nanoseconds) {}

inline Duration::Duration(int nanoseconds)
    : ns_(nanoseconds) {}

inline Duration::Duration(double seconds)
    : ns_((int64_t)(seconds * duration::kSecond)) {}

inline int64_t Duration::Nanoseconds() const {
    return ns_;
}

inline double Duration::Seconds() const {
    return double(ns_) / duration::kSecond;
}

inline double Duration::Milliseconds() const {
    return double(ns_) / duration::kMillisecond;
}

inline double Duration::Microseconds() const {
    return double(ns_) / duration::kMicrosecond;
}

inline double Duration::Minutes() const {
    return double(ns_) / duration::kMinute;
}

inline double Duration::Hours() const {
    return double(ns_) / duration::kHour;
}

inline bool Duration::IsZero() const {
    return ns_ == 0;
}

inline struct timeval Duration::TimeVal() const {
    struct timeval t;
    To(&t);
    return t;
}

inline void Duration::To(struct timeval* t) const {
    t->tv_sec = (long)(ns_ / duration::kSecond);
    t->tv_usec = (long)(ns_ % duration::kSecond) / (long)duration::kMicrosecond;
}

inline bool Duration::operator<(const Duration& rhs) const {
    return ns_ < rhs.ns_;
}

inline bool Duration::operator<=(const Duration& rhs) const {
    return ns_ <= rhs.ns_;
}

inline bool Duration::operator>(const Duration& rhs) const {
    return ns_ > rhs.ns_;
}

inline bool Duration::operator>=(const Duration& rhs) const {
    return ns_ >= rhs.ns_;
}

inline bool Duration::operator==(const Duration& rhs) const {
    return ns_ == rhs.ns_;
}

inline Duration Duration::operator+=(const Duration& rhs) {
    ns_ += rhs.ns_;
    return *this;
}

inline Duration Duration::operator-=(const Duration& rhs) {
    ns_ -= rhs.ns_;
    return *this;
}

inline Duration Duration::operator*=(int n) {
    ns_ *= n;
    return *this;
}

inline Duration Duration::operator/=(int n) {
    ns_ /= n;
    return *this;
}
} // namespace eventpp


#endif // __EVENTPP_DURATION_INLINE_HPP__