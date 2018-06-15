#include "DatabaseFactory.h"
#include "LinkProxy.h"
#include "Node.h"

namespace Pbft {

::std::unique_ptr<LinkInterface> DatabaseFactory::CreateLink() const
{
    return ::std::make_unique<LinkProxy>();
}

::std::unique_ptr<NodeInterface> DatabaseFactory::CreateNode(::std::shared_ptr<LinkInterface> linkToSet,
    NodeId idToSet) const
{
    return CreateNode(linkToSet, idToSet, {});
}

::std::unique_ptr<NodeInterface> DatabaseFactory::CreateNode(::std::shared_ptr<LinkInterface> linkToSet,
    NodeId idToSet, const ::std::vector<Command>& commands) const
{
    return ::std::make_unique<Node>(linkToSet, idToSet, commands);
}

}
