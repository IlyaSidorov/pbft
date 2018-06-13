#pragma once

#include "DatabaseInterface.h"
#include "DatabaseFactoryInterface.h"
#include <mutex>

namespace Pbft {

class Database : public DatabaseInterface
{
public:
    explicit Database(::std::shared_ptr<DatabaseFactoryInterface> factoryToSet);

    static DatabaseInterface& Instance();

private:
    virtual NodeId CreateNode() override;
    virtual void DeleteNode(NodeId id) override;
    virtual void SetFaulty(NodeId id) override;
    virtual void SetOperational(NodeId id) override;

    virtual void TopUp(ClientId id, uint32_t sum) override;
    virtual void Withdraw(ClientId id, uint32_t sum) override;
    virtual void Transmit(ClientId sourceId, ClientId destinationId, uint32_t sum) override;
    virtual uint32_t Balance(ClientId id) const override;

    NodeId GetFreeNodeId() const;

    static ::std::mutex mutex;
    static ::std::unique_ptr<DatabaseInterface> instance;
    ::std::shared_ptr<DatabaseFactoryInterface> factory;

};

}
