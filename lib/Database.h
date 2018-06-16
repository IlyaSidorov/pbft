#pragma once

#include "DatabaseInterface.h"
#include "DatabaseFactoryInterface.h"
#include <future>

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
    virtual uint32_t Balance(ClientId id) override;

    NodeId FreeNodeId() const;
    void SetNodeCount() const;
    void ExecuteTransaction();
    void InitiateTransaction();
    void WaitResult();

    void OnReceive(const Message& receivedMessage);

    void CheckSucceededCommands() const;
    void CheckTopUpResult() const;
    void CheckWithdrawResult() const;
    void CheckTransmitResult() const;
    void CheckBalanceResult() const;

    static ::std::mutex mutex;
    static ::std::unique_ptr<DatabaseInterface> instance;
    ::std::shared_ptr<DatabaseFactoryInterface> factory;
    ::std::shared_ptr<LinkInterface> link;
    ::boost::signals2::scoped_connection connection;
    ::std::map<NodeId, ::std::shared_ptr<NodeInterface>> nodes;
    Message message;
    uint32_t messageCount{0u};
    ::std::list<Command> succeededCommands;
    ::std::unique_ptr<::std::promise<void>> promise;
};

}
