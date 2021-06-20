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

/*
 * Packed array based on a simple C-array,
 * with the first element being used for a uint32_t count.
 */
template <typename T, int Capacity>
inline T *packed_array_add(T (&arr)[Capacity])
{
    static_assert(sizeof(T) >= sizeof(uint32_t), "");

    uint32_t &count = *(uint32_t*)&arr[0];
    if(count + 1 < Capacity) {
        return &arr[1 + count++];
    }
    else {
        return nullptr;
    }
}

template <typename T, int Capacity>
inline void packed_array_remove(T (&arr)[Capacity], const uint32_t *deletion_list, size_t deletion_count)
{
    static_assert(sizeof(T) >= sizeof(uint32_t), "");

    uint32_t &proj_count = *(uint32_t*)&arr[0];
    for(uint32_t i = 0; i < deletion_count; ++i) {
        uint32_t idx = deletion_list[deletion_count - 1 - i];

        if(idx == proj_count) {
            --proj_count;
        }
        else {
            arr[idx] = arr[proj_count--];
        }
    }
}

template <typename T, int Capacity>
inline uint32_t packed_array_count(T (&arr)[Capacity])
{
    static_assert(sizeof(T) >= sizeof(uint32_t), "");

    return *(uint32_t*)&arr[0];
}

template <typename T, int Capacity>
inline void packed_array_clear(T (&arr)[Capacity])
{
    static_assert(sizeof(T) >= sizeof(uint32_t), "");

    auto &count = *(uint32_t*)&arr[0];
    count = 0;
}

template <typename Func, typename T, int Capacity>
inline void packed_array_iterate(T (&arr)[Capacity], Func func)
{
    static_assert(sizeof(T) >= sizeof(uint32_t), "");

    auto count = *(uint32_t*)&arr[0];
    for(uint32_t i = 1; i <= count; ++i) {
        func(i);
    }
}
