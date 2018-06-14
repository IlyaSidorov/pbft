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

void Node::OnReceive(const Message& receivedMessage)
{
    switch (receivedMessage.transactionId)
    {
        case TransactionId::PrePrepare:
        {
            OnPrePrepare(receivedMessage);
            break;
        }
        case TransactionId::Prepare:
        {
            OnPrepare(receivedMessage);
            break;
        }
        case TransactionId::Commit:
        {
            OnCommit(receivedMessage);
            break;
        }
    }
}

void Node::OnPrePrepare(const Message& receivedMessage)
{
    if ((static_cast<int32_t>(receivedMessage.id) - static_cast<int32_t>(lastMessageId)) <= 0)
    {
        return;
    }

    message = receivedMessage;
    message->nodeId = id;
    lastMessageId = receivedMessage.id;
    messageCount = 0;

    auto messageToSend(*message);
    messageToSend.transactionId = TransactionId::Prepare;
    link->Send(messageToSend);
}

void Node::OnPrepare(const Message& receivedMessage)
{
    if ((!TransactionCorrect(TransactionId::PrePrepare)) || (!MessageCorrect(receivedMessage)))
    {
        return;
    }

    if (Utilities::TransactionConfirmed(NodeCount(), ++messageCount))
    {
        message->transactionId = TransactionId::Prepare;
        messageCount = 0;

        ProcessCommand();

        auto messageToSend(*message);
        messageToSend.transactionId = TransactionId::Commit;
        link->Send(messageToSend);
    }
}

void Node::OnCommit(const Message& receivedMessage)
{
    if ((!TransactionCorrect(TransactionId::Prepare)) || (!MessageCorrect(receivedMessage)))
    {
        return;
    }

    if (Utilities::TransactionConfirmed(NodeCount(), ++messageCount))
    {
        message == ::boost::none;
    }
}

bool Node::TransactionCorrect(TransactionId id) const
{
    return (message && (message->transactionId == id));
}

bool Node::MessageCorrect(const Message& receivedMessage) const
{
    return ((message->id == receivedMessage.id) && (message->command == receivedMessage.command));
}

uint32_t Node::NodeCount() const
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    return nodeCount;
}

bool Node::Faulty() const
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    return faulty;
}

void Node::ProcessCommand()
{
    switch (message->command.id)
    {
        case CommandId::TopUp:
        {
            ProcessTopUpCommand();
            break;
        }
        case CommandId::Withdraw:
        {
            ProcessWithdrawCommand();
            break;
        }
        case CommandId::Transmit:
        {
            ProcessTransmitCommand();
            break;
        }
        case CommandId::Balance:
        {
            ProcessBalanceCommand();
            break;
        }
    }
}

void Node::ProcessTopUpCommand()
{
    if (Faulty())
    {
        message->resultId = ResultId::Failure;
        return;
    }

    commands.emplace_back(message->command);
    message->resultId = ResultId::Success;
}

void Node::ProcessWithdrawCommand()
{

}

void Node::ProcessTransmitCommand()
{

}

void Node::ProcessBalanceCommand()
{

}

::boost::optional<uint32_t> Node::GetBalance(ClientId id) const
{
    auto clientFound(false);
    auto balance(0u);
    for (const auto& command : commands)
    {
        if (auto effect = CommandEffect(command, id))
        {
            clientFound = true;
            balance += static_cast<uint32_t>(*effect);
        }
    }

    if (!clientFound)
    {
        return ::boost::none;
    }

    return balance;
}

::boost::optional<int32_t> Node::CommandEffect(const Command& command, ClientId clientId) const
{
    auto destinationId(0u);
    auto destinationSum(0);
    switch (command.id)
    {
        case CommandId::TopUp:
        {
            destinationId = command.topUp.id;
            destinationSum = static_cast<int32_t>(command.topUp.sum);
            break;
        }
        case CommandId::Withdraw:
        {
            destinationId = command.withdraw.id;
            destinationSum = -static_cast<int32_t>(command.withdraw.sum);
            break;
        }
        case CommandId::Transmit:
        {
            destinationId = command.transmit.destinationId;
            destinationSum = static_cast<int32_t>(command.transmit.sum);
            break;
        }
    }

    if (destinationId != clientId)
    {
        return ::boost::none;
    }

    return destinationSum;
}

}
