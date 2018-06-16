#include "ConnectionFactory.h"
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <string>

void WaitInput()
{
#ifdef _WIN32
    system("pause");
#else
    system("read");
#endif
}

bool Exit(::std::mutex& mutex, ::std::condition_variable& condition, const bool& exitCommand,
    const::std::chrono::milliseconds& timeout)
{
    ::std::unique_lock<::std::mutex> lock(mutex);
    condition.wait_for(lock, ::std::chrono::milliseconds(timeout), [&exitCommand]{return exitCommand;});
    return exitCommand;
}

void RequestExit(::std::mutex& mutex, ::std::condition_variable& condition, bool& exitCommand)
{
    ::std::unique_lock<::std::mutex> lock(mutex);
    exitCommand = true;
    condition.notify_all();
}

::std::mutex display;
void OutputBalance(::Pbft::ClientId id, ::std::string message, uint32_t sum)
{
    ::std::lock_guard<::std::mutex> lock(display);
    ::std::cout << "Client " << id << " balance " << message << ": " << ::std::setw(10) << sum <<
        ::std::endl;
}

void Client0Activity(::std::shared_ptr<::Pbft::ClientConnectionInterface> client, ::Pbft::ClientId id,
    ::Pbft::ClientId destinationId, ::std::mutex& mutex, ::std::condition_variable& condition,
        const bool& exitCommand)
{
    for (;;)
    {
        if (Exit(mutex, condition, exitCommand, ::std::chrono::milliseconds(200ll)))
        {
            break;
        }

        try
        {
        }
        catch (const ::std::exception& ex)
        {
            ::std::cout << "Exception: " << ex.what() << ::std::endl;
            WaitInput();
        }

        OutputBalance(id, "before TopUp", client->Balance());
        const auto sum(10);
        client->TopUp(sum);
        OutputBalance(id, "after TopUp", client->Balance());

        OutputBalance(id, "before Transmit", client->Balance());
        client->Transmit(destinationId, sum);
        OutputBalance(id, "after Transmit", client->Balance());
    }
}

void Client1Activity(::std::shared_ptr<::Pbft::ClientConnectionInterface> client, ::Pbft::ClientId id,
    ::Pbft::ClientId sourceId, ::std::mutex& mutex, ::std::condition_variable& condition,
        const bool& exitCommand)
{
    for (;;)
    {
        if (Exit(mutex, condition, exitCommand, ::std::chrono::milliseconds(100ll)))
        {
            break;
        }

        const auto balance(client->Balance());
        OutputBalance(id, "before Withdraw", client->Balance());
        client->Withdraw(balance);
        OutputBalance(id, "after Withdraw", client->Balance());
    }
}

void AdministratorActivity()
{
}

void main()
{
    ::std::shared_ptr<::Pbft::BackdoorConnectionInterface> backdoor(::Pbft::ConnectionFactory::
        CreateBackdoorConnection());

    backdoor->CreateNode();
    backdoor->CreateNode();
    backdoor->CreateNode();
    backdoor->CreateNode();

    ::std::condition_variable condition;
    ::std::mutex mutex;

    auto clientId0(0u);
    ::std::shared_ptr<::Pbft::ClientConnectionInterface> client0(::Pbft::ConnectionFactory::
        CreateClientConnection(clientId0));
    auto clientId1(1u);
    ::std::shared_ptr<::Pbft::ClientConnectionInterface> client1(::Pbft::ConnectionFactory::
        CreateClientConnection(clientId1));

    auto exitCommand(false);
    ::std::thread client0Activity([&client0, &clientId0, &clientId1, &mutex, &condition, &exitCommand]{
        Client0Activity(client0, clientId0, clientId1, mutex, condition, exitCommand);});
    ::std::thread client1Activity([&client1, &clientId1, &clientId0, &mutex, &condition, &exitCommand] {
        Client0Activity(client1, clientId1, clientId0, mutex, condition, exitCommand);});

    WaitInput();
    RequestExit(mutex, condition, exitCommand);

    client0Activity.join();
    client1Activity.join();
}
