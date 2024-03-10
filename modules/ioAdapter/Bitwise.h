#pragma once

#include <cstdint>

class Bitwise
{
    public:
        static uint32_t shift(const int bit)
        {
            auto n = 1u << bit;
            return static_cast<uint32_t>(1u << bit);
        }

        static bool getBitState(const uint32_t value, const int bit)
        {
            const uint32_t mask = shift(bit);
            return value & mask;
        }

        static void setBit(uint32_t& value, const int bit)
        {
            const uint32_t mask = shift(bit);
            value |= mask;
        }

        static void clearBit(uint32_t& value, const int bit)
        {
            const uint32_t mask = shift(bit);
            value &= ~mask;
        }

};
