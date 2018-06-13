#include "DatabaseFactory.h"
#include "LinkProxy.h"
#include "Node.h"
#include "Transaction.h"

namespace Pbft {

::std::unique_ptr<LinkInterface> DatabaseFactory::CreateLink() const
{
    return ::std::make_unique<LinkProxy>();
}

::std::unique_ptr<NodeInterface> DatabaseFactory::CreateNode() const
{
    return ::std::make_unique<Node>();
}

::std::unique_ptr<TransactionInterface> DatabaseFactory::CreateTransaction() const
{
    return ::std::make_unique<Transaction>();
}

}
