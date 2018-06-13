#pragma once

#include "LinkInterface.h"

namespace Pbft {

class Node
{
public:
    Node(::std::shared_ptr<LinkInterface> linkToSet);

private:
    void OnReceive();

    ::std::shared_ptr<LinkInterface> link;
};

}
