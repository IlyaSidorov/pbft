#pragma once

#include "NodeInterface.h"
#include "LinkInterface.h"
#include <boost/optional.hpp>
#include <mutex>

namespace Pbft {

class Node : public NodeInterface
{
public:
    Node(::std::shared_ptr<LinkInterface> linkToSet, NodeId idToSet);

private:
    virtual void SetNodeCount(uint32_t count) override;
    virtual void SetFaulty() override;
    virtual void SetOperational() override;

    void OnReceive(const Message& receivedMessage);
    void OnPrePrepare(const Message& receivedMessage);
    void OnPrepare(const Message& receivedMessage);
    void OnCommit(const Message& receivedMessage);

    bool TransactionCorrect(TransactionId id) const;
    bool MessageCorrect(const Message& receivedMessage) const;

    void InitiateTransaction(TransactionId transactionId);

    void ProcessCommand();
    void ProcessTopUpCommand();
    void ProcessWithdrawCommand();
    void ProcessTransmitCommand();
    void ProcessBalanceCommand();

    ::boost::optional<uint32_t> GetBalance(ClientId id) const;
    ::boost::optional<int32_t> CommandEffect(const Command& command, ClientId clientId) const;
    void ProcessSubtractingCommand(ClientId clientId, uint32_t sum);

    uint32_t NodeCount() const;
    bool Faulty() const;

    mutable ::std::mutex mutex;
    ::std::shared_ptr<LinkInterface> link;
    const NodeId id;
    ::boost::signals2::scoped_connection connection;
    uint32_t nodeCount{0u};
    bool faulty{false};
    ::boost::optional<Message> message;
    uint32_t lastMessageId{static_cast<uint32_t>(-1)};
    uint32_t messageCount{0u};
    // ::boost::multi_index_container must be used instead of the ::std::vector.
    // ::std::vector is taken for the implementation simplification
    ::std::vector<Command> commands;
};

}
