#pragma once

#include "ClientTypes.h"

namespace Pbft {

enum class TransactionId
{
    PrePrepare,
    Prepare,
    Commit
};

using NodeId = uint32_t;
using MessageId = uint32_t;

enum class CommandId
{
    TopUp,
    Withdraw,
    Transmit,
    Balance
};

struct TopUpCommand
{
    bool operator==(const TopUpCommand& right) const
    {
        return ((id == right.id) && (sum == right.sum));
    }

    bool operator!=(const TopUpCommand& right) const
    {
        return !operator==(right);
    }

    ClientId id;
    uint32_t sum;
};

struct WithdrawCommand
{
    bool operator==(const WithdrawCommand& right) const
    {
        return ((id == right.id) && (sum == right.sum));
    }

    bool operator!=(const WithdrawCommand& right) const
    {
        return !operator==(right);
    }

    ClientId id;
    uint32_t sum;
};

struct TransmitCommand
{
    bool operator==(const TransmitCommand& right) const
    {
        return ((sourceId == right.sourceId) && (destinationId == right.destinationId) && (sum == right.sum));
    }

    bool operator!=(const TransmitCommand& right) const
    {
        return !operator==(right);
    }

    ClientId sourceId;
    ClientId destinationId;
    uint32_t sum;
};

struct BalanceCommand
{
    bool operator==(const BalanceCommand& right) const
    {
        return ((id == right.id) && (sum == right.sum));
    }

    bool operator!=(const BalanceCommand& right) const
    {
        return !operator==(right);
    }

    ClientId id;
    uint32_t sum;
};

struct Command
{
    bool operator==(const Command& right) const
    {
        if (id != right.id)
        {
            return false;
        }

        return
            (((id == CommandId::TopUp) && (topUp == right.topUp)) ||
             ((id == CommandId::Withdraw) && (withdraw == right.withdraw)) ||
             ((id == CommandId::Transmit) && (transmit == right.transmit)) ||
             ((id == CommandId::Balance) && (balance == right.balance)));
    }

    bool operator!=(const Command& right) const
    {
        return !operator==(right);
    }

    CommandId id;
    union
    {
        TopUpCommand topUp;
        WithdrawCommand withdraw;
        TransmitCommand transmit;
        BalanceCommand balance;
    };
};

enum class ResultId
{
    Success,
    Failure
};

struct Message
{
    TransactionId transactionId;
    NodeId nodeId;
    MessageId id;
    Command command;
    ResultId resultId;
};

}
