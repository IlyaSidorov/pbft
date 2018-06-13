#include "LinkProxy.h"

namespace Pbft {

LinkProxy::LinkProxy()
{
    try
    {
        service = ::std::make_unique<::boost::asio::io_service>();
        work = ::std::make_unique<::boost::asio::io_service::work>(*service);
        strand = ::std::make_unique<::boost::asio::io_service::strand>(*service);
        thread = ::std::make_unique<::std::thread>([this] {service->run(); });
    }
    catch (const std::exception&)
    {
        Uninitialize();
        throw;
    }
}

LinkProxy::~LinkProxy()
{
    Uninitialize();
}

void LinkProxy::Uninitialize()
{
    if (service)
    {
        service->stop();
    }

    if (thread)
    {
        thread->join();
    }

    thread.reset();
    strand.reset();
    work.reset();
    service.reset();
}

void LinkProxy::Send(const Message& message) const
{
    strand->post([message, this]{Receive(message);});
}

}
