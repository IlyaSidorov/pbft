#include "Database.h"
#include "DatabaseFactory.h"

namespace Pbft {

::std::mutex Database::mutex;
::std::unique_ptr<DatabaseInterface> Database::instance;

Database::Database(::std::shared_ptr<DatabaseFactoryInterface> factoryToSet)
    : factory(factoryToSet)
{
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
    return 0;
}

void Database::DeleteNode(NodeId id)
{

}

void Database::SetFaulty(NodeId id)
{

}

void Database::SetOperational(NodeId id)
{

}

void Database::TopUp(ClientId id, uint32_t sum)
{

}

void Database::Withdraw(ClientId id, uint32_t sum)
{

}

void Database::Transmit(ClientId sourceId, ClientId destinationId, uint32_t sum)
{

}

uint32_t Database::Balance(ClientId id) const
{
    return 0;
}

NodeId Database::GetFreeNodeId() const
{

}

}
