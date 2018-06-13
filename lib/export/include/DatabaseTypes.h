#pragma once

#include <boost/any.hpp>
#include <cstdint>

namespace Pbft {

using NodeId = uint32_t;

enum class TransactionPhase
{
    Preprepare,
    Prepare,
    Commit
};

using MessageId = uint32_t;
struct Message
{
    TransactionPhase phase;
    MessageId id;
    ::boost::any data;
};

}
