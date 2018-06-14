#pragma once

#include <cstdint>

namespace Pbft {

class Utilities
{
public:
    Utilities() = delete;
    Utilities(const Utilities&) = delete;

    Utilities& operator=(const Utilities&) = delete;

    static bool TransactionConfirmed(uint32_t nodeCount, uint32_t messageCount)
    {
        return (messageCount >= (((nodeCount - 1) / 3) + 1));
    }
};

}
