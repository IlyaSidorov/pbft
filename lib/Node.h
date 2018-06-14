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

    void OnReceive(const Message& messageReceived);
    void OnPrePrepare(const Message& messageReceived);
    void OnPrepare(const Message& messageReceived);
    void OnCommit(const Message& messageReceived);

    bool TransactionCorrect(TransactionId id) const;
    bool MessageCorrect(const Message& messageReceived) const;
    void ProcessCommand();

    ::std::mutex mutex;
    ::std::shared_ptr<LinkInterface> link;
    const NodeId id;
    uint32_t nodeCount{0u};
    ::boost::optional<Message> message;
    uint32_t lastMessageId{static_cast<uint32_t>(-1)};
    uint32_t messageCount{0u};
    bool faulty{false};
};

}
