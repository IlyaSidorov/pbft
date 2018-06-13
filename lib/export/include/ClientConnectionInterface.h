#pragma once

#include "ClientTypes.h"

namespace Pbft {

class ClientConnectionInterface
{
public:
    ClientConnectionInterface(const ClientConnectionInterface&) = delete;
    virtual ~ClientConnectionInterface() = default;

    ClientConnectionInterface& operator=(const ClientConnectionInterface&) = delete;

    virtual void TopUp(uint32_t sum) = 0;
    virtual void Withdraw(uint32_t sum) = 0;
    virtual void Transmit(ClientId destinationId, uint32_t sum) = 0;
    virtual uint32_t Balance() const = 0;

protected:
    ClientConnectionInterface() = default;
};

}
