#include "Database.h"
#include "DatabaseFactory.h"
#include "Utilities.h"
#include <sstream>

namespace Pbft {

::std::mutex Database::mutex;
::std::unique_ptr<DatabaseInterface> Database::instance;

Database::Database(::std::shared_ptr<DatabaseFactoryInterface> factoryToSet)
    : factory(factoryToSet)
{
    link = factory->CreateLink();
    connection = link->Receive.connect([this](const Message& receivedMessage){OnReceive(receivedMessage);});
}

DatabaseInterface& Database::Instance()
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    if (!instance)
    {
        instance = ::std::make_unique<Database>(::std::make_shared<DatabaseFactory>());
    }

    return *instance;
}

NodeId Database::CreateNode()
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    const auto id(FreeNodeId());
    if (nodes.empty())
    {
        nodes.emplace(id, factory->CreateNode(link, id));
    }
    else
    {
        nodes.emplace(id, factory->CreateNode(link, id, nodes.begin()->second->Commands()));
    }

    SetNodeCount();
    return id;
}

void Database::DeleteNode(NodeId id)
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    nodes.erase(id);
    SetNodeCount();
}

void Database::SetFaulty(NodeId id)
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    nodes.at(id)->SetFaulty();
}

void Database::SetOperational(NodeId id)
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    nodes.at(id)->SetOperational();
}

void Database::TopUp(ClientId id, uint32_t sum)
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    message.command.id = CommandId::TopUp;
    message.command.topUp.id = id;
    message.command.topUp.sum = sum;
    ExecuteTransaction();
}

void Database::Withdraw(ClientId id, uint32_t sum)
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    message.command.id = CommandId::Withdraw;
    message.command.withdraw.id = id;
    message.command.withdraw.sum = sum;
    ExecuteTransaction();
}

void Database::Transmit(ClientId sourceId, ClientId destinationId, uint32_t sum)
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    message.command.id = CommandId::Transmit;
    message.command.transmit.sourceId = sourceId;
    message.command.transmit.destinationId = destinationId;
    message.command.transmit.sum = sum;
    ExecuteTransaction();
}

uint32_t Database::Balance(ClientId id)
{
    ::std::lock_guard<::std::mutex> lock(mutex);
    message.command.id = CommandId::Balance;
    message.command.balance.id = id;
    ExecuteTransaction();
    return message.command.balance.sum;
}

NodeId Database::FreeNodeId() const
{
    auto id(0u);
    for (; id <= nodes.size(); ++id)
    {
        if (nodes.find(id) == nodes.end())
        {
            break;
        }
    }

    return id;
}

void Database::SetNodeCount() const
{
    ::std::for_each(nodes.begin(), nodes.end(), [this](const decltype(nodes)::value_type& element){
        element.second->SetNodeCount(nodes.size());});
}

void Database::ExecuteTransaction()
{
    InitiateTransaction();
    WaitResult();
}

void Database::InitiateTransaction()
{
    ++message.id;
    messageCount = 0;
    succeededCommands.clear();
    promise = ::std::make_unique<::std::promise<void>>();
    link->Send(message);
}

void Database::WaitResult()
{
    // No timeout is possible in current implementation for the simplification
    promise->get_future().get();
}

void Database::OnReceive(const Message& receivedMessage)
{
    if ((receivedMessage.transactionId == TransactionId::Result) && (receivedMessage.id == message.id))
    {
        if (receivedMessage.resultId == ResultId::Success)
        {
            ++succeededCommands[receivedMessage.command];
        }

        if (++messageCount == nodes.size())
        {
            ProcessSucceededCommands();
        }
    }
}

void Database::ProcessSucceededCommands()
{
    try
    {
        CheckSucceededCommandsReceived();

        auto count(0u);
        auto command(GetCommandWithMaximumReplication(count));
        CheckCommandWithMaximumReplication(command, count);

        promise->set_value();
    }
    catch (const ::std::exception&)
    {
        promise->set_exception(::std::current_exception());
    }
}

void Database::CheckSucceededCommandsReceived()
{
    if (succeededCommands.empty())
    {
        throw ::std::runtime_error("Succeeded commands has not been received");
    }
}

const Command& Database::GetCommandWithMaximumReplication(uint32_t& count) const
{
    using ElementType = decltype(succeededCommands)::value_type;
    const auto element(::std::max_element(succeededCommands.begin(), succeededCommands.end(), [](
        const ElementType& left, const ElementType& right) {return (left.second < right.second);}));

    count = element->second;
    return element->first;
}

void Database::CheckCommandWithMaximumReplication(const Command& command, uint32_t count)
{
    if (!Utilities::TransactionConfirmed(nodes.size(), count))
    {
        ::std::stringstream errorStream;
        errorStream << "Transaction not confirmed: " << count << " commands succeeded of " << nodes.size();
        throw ::std::runtime_error(errorStream.str());
    }

    RegisterResult(command);

    if (message.command != command)
    {
        throw ::std::runtime_error("Invalid command received");
    }
}

void Database::RegisterResult(const Command& command)
{
    if ((message.command.id == CommandId::Balance) && (command.id == CommandId::Balance))
    {
        message.command.balance.sum = command.balance.sum;
    }
}

}
