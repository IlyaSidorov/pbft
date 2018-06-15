#include "Node.h"
#include "Utilities.h"

namespace Pbft {

Node::Node(::std::shared_ptr<LinkInterface> linkToSet, NodeId idToSet, const ::std::vector<Command>&
    commandsToSet)
    : link(linkToSet)
    , id(idToSet)
    , commands(commandsToSet)
{
    connection = link->Receive.connect([this](const Message& receivedMessage){OnReceive(receivedMessage);});
}

const ::std::vector<Command>& Node::Commands() const
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    return commands;
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
    InitiateTransaction(TransactionId::Prepare);
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
        InitiateTransaction(TransactionId::Commit);
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
        ProcessCommand();
        InitiateTransaction(TransactionId::Result);
        message = {};
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

void Node::InitiateTransaction(TransactionId transactionId)
{
    messageCount = 0;

    auto messageToSend(*message);
    messageToSend.transactionId = transactionId;
    link->Send(messageToSend);
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
    commands.emplace_back(message->command);
    message->resultId = Faulty() ? ResultId::Failure : ResultId::Success;
}

void Node::ProcessWithdrawCommand()
{
    ProcessSubtractingCommand(message->command.withdraw.id, message->command.withdraw.sum);
}

void Node::ProcessTransmitCommand()
{
    ProcessSubtractingCommand(message->command.transmit.sourceId, message->command.transmit.sum);
}

void Node::ProcessBalanceCommand()
{
    message->resultId = ResultId::Success;
    if (Faulty())
    {
        message->command.balance.sum = Utilities::Random<decltype(message->command.balance.sum)>();
    }
    else if (auto balance = GetBalance(message->command.balance.id))
    {
        message->command.balance.sum = *balance;
    }
    else
    {
        message->resultId = ResultId::Failure;
    }
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
        return {};
    }

    return balance;
}

::boost::optional<int32_t> Node::CommandEffect(const Command& command, ClientId clientId) const
{
    switch (command.id)
    {
        case CommandId::TopUp:
        {
            if (command.topUp.id == clientId)
            {
                return static_cast<int32_t>(command.topUp.sum);
            }
            
            break;
        }
        case CommandId::Withdraw:
        {
            if (command.withdraw.id == clientId)
            {
                return -static_cast<int32_t>(command.withdraw.sum);
            }

            break;
        }
        case CommandId::Transmit:
        {
            if (command.transmit.sourceId == clientId)
            {
                return -static_cast<int32_t>(command.transmit.sum);
            }
            else if (command.transmit.destinationId == clientId)
            {
                return static_cast<int32_t>(command.transmit.sum);
            }

            break;
        }
    }

    return {};
}

void Node::ProcessSubtractingCommand(ClientId clientId, uint32_t sum)
{
    message->resultId = ResultId::Failure;
    if (auto balance = GetBalance(clientId))
    {
        if (*balance >= sum)
        {
            commands.emplace_back(message->command);

            if (!Faulty())
            {
                message->resultId = ResultId::Success;
            }
        }
    }
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

}
