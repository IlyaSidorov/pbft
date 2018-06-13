#pragma once

#include "DatabaseTypes.h"

namespace Pbft {

class BackdoorConnectionInterface
{
public:
    BackdoorConnectionInterface(const BackdoorConnectionInterface&) = delete;
    virtual ~BackdoorConnectionInterface() = default;

    BackdoorConnectionInterface& operator=(const BackdoorConnectionInterface&) = delete;

    virtual NodeId CreateNode() = 0;
    virtual void DeleteNode(NodeId id) = 0;
    virtual void SetFaulty(NodeId id) = 0;
    virtual void SetOperational(NodeId id) = 0;

protected:
    BackdoorConnectionInterface() = default;
};

}
