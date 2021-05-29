// Everything assumes ZII (zero-initialization-is-initialization), everything is either POD or POD with functions (no constructors of any kind)
#pragma once
#include "common.h"

template <int SZ>
struct StaticArena {
    uint8_t data[SZ];
    size_t  top;
};

struct ArenaAllocator {
    uint8_t *base_ptr;
    size_t  &top;
    size_t   size;

    template <int SZ>
    static inline ArenaAllocator from_static(StaticArena<SZ> &arena)
    {
        return {
            arena.data,
            arena.top,
            SZ
        };
    }

    inline uint8_t *push_bytes(size_t bytes)
    {
        assert(top + bytes <= size);
        top += bytes;
        uint8_t *ptr = &base_ptr[top];

        return ptr;
    }

    template <typename T>
    inline T &push() {
        T *placed_obj = (T *)push_bytes(sizeof(T));

        return *placed_obj;
    }

    template <typename T>
    inline T &push(const T &obj) {
        T &placed_obj = push<T>();
        placed_obj = obj;

        return placed_obj;
    }

    inline void reset() {
        top = 0;
    }
};
