#pragma once

#include "LinkInterface.h"
#include "NodeInterface.h"
#include <memory>

namespace Pbft {

class DatabaseFactoryInterface
{
public:
    DatabaseFactoryInterface(const DatabaseFactoryInterface&) = delete;
    virtual ~DatabaseFactoryInterface() = default;

    DatabaseFactoryInterface& operator=(const DatabaseFactoryInterface&) = delete;

    virtual ::std::unique_ptr<LinkInterface> CreateLink() const = 0;
    virtual ::std::unique_ptr<NodeInterface> CreateNode() const = 0;

protected:
    DatabaseFactoryInterface() = default;
};

}
