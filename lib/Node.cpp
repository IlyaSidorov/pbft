#include "Node.h"
#include "Utilities.h"

namespace Pbft {

Node::Node(::std::shared_ptr<LinkInterface> linkToSet, NodeId idToSet)
    : link(linkToSet)
    , id(idToSet)
{
}

void Node::SetNodeCount(uint32_t count)
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    nodeCount = count;
}

void Node::SetFaulty()
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    faulty = true;
}

void Node::SetOperational()
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    faulty = false;
}

void Node::OnReceive(const Message& messageReceived)
{
    switch (messageReceived.transactionId)
    {
        case TransactionId::PrePrepare:
        {
            OnPrePrepare(messageReceived);
            break;
        }
        case TransactionId::Prepare:
        {
            OnPrepare(messageReceived);
            break;
        }
        case TransactionId::Commit:
        {
            OnCommit(messageReceived);
            break;
        }
    }
}

void Node::OnPrePrepare(const Message& messageReceived)
{
    if ((static_cast<int32_t>(messageReceived.id) - static_cast<int32_t>(lastMessageId)) <= 0)
    {
        return;
    }

    message = messageReceived;
    message->nodeId = id;
    lastMessageId = messageReceived.id;
    messageCount = 0;

    auto messageToSend(*message);
    messageToSend.transactionId = TransactionId::Prepare;
    link->Send(messageToSend);
}

void Node::OnPrepare(const Message& messageReceived)
{
    if ((!TransactionCorrect(TransactionId::PrePrepare)) || (!MessageCorrect(messageReceived)))
    {
        return;
    }

    if (Utilities::TransactionConfirmed(nodeCount, ++messageCount))
    {
        message->transactionId = TransactionId::Prepare;
        messageCount = 0;

        ProcessCommand();

        auto messageToSend(*message);
        messageToSend.transactionId = TransactionId::Commit;
        link->Send(messageToSend);
    }
}

void Node::OnCommit(const Message& messageReceived)
{
    if ((!TransactionCorrect(TransactionId::Prepare)) || (!MessageCorrect(messageReceived)))
    {
        return;
    }

    if (Utilities::TransactionConfirmed(nodeCount, ++messageCount))
    {
        message == boost::none;
    }
}

bool Node::TransactionCorrect(TransactionId id) const
{
    return (message && (message->transactionId == id));
}

bool Node::MessageCorrect(const Message& messageReceived) const
{
    return ((message->id == messageReceived.id) && (message->command == messageReceived.command));
}

void Node::ProcessCommand()
{
    switch (messageReceived.transactionId)
    {
        case TransactionId::PrePrepare:
        {
            OnPrePrepare(messageReceived);
            break;
        }
        case TransactionId::Prepare:
        {
            OnPrepare(messageReceived);
            break;
        }
        case TransactionId::Commit:
        {
            OnCommit(messageReceived);
            break;
        }
    }
}

}
