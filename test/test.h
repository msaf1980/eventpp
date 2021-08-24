#ifndef __TEST_H__
#define __TEST_H__

#include "doctest.h"

#define H_ARRAYSIZE(a) \
        ((sizeof(a) / sizeof(*(a))) / \
         static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

#endif /* __TEST_H__ */
