#pragma once

#include "DatabaseFactoryInterface.h"

namespace Pbft {

class DatabaseFactory : public DatabaseFactoryInterface
{
private:
    virtual ::std::unique_ptr<LinkInterface> CreateLink() const override;
    virtual ::std::unique_ptr<NodeInterface> CreateNode() const override;
};

}
