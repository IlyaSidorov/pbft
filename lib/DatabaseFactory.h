#pragma once

#include "DatabaseFactoryInterface.h"

namespace Pbft {

class DatabaseFactory : public DatabaseFactoryInterface
{
private:
    virtual ::std::unique_ptr<LinkInterface> CreateLink() const override;
    virtual ::std::unique_ptr<NodeInterface> CreateNode(::std::shared_ptr<LinkInterface> linkToSet,
        NodeId idToSet) const override;
    virtual ::std::unique_ptr<NodeInterface> CreateNode(::std::shared_ptr<LinkInterface> linkToSet,
        NodeId idToSet, const ::std::vector<Command>& commands) const override;
};

}
