#pragma once

namespace Pbft {

class TransactionInterface
{
public:
    TransactionInterface(const TransactionInterface&) = delete;
    virtual ~TransactionInterface() = default;

    TransactionInterface& operator=(const TransactionInterface&) = delete;

protected:
    TransactionInterface() = default;
};

}
