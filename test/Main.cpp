#include "ConnectionFactory.h"
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

namespace {

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
void ShowMessage(const ::std::string& message)
{
    ::std::lock_guard<::std::mutex> lock(display);
    ::std::cout << message << ::std::endl;
}

void ShowClientMessage(::Pbft::ClientId id, const ::std::string& message)
{
    ::std::stringstream stream;
    stream << "Client " << id << " -> " << message;
    ShowMessage(stream.str());
}

void ShowAdministratorMessage(const ::std::string& message)
{
    ShowMessage("Administrator -> " + message);
}

void ShowAdministratorActivityMessage(const ::std::string& begin, uint32_t count, const ::std::string& end)
{
    ::std::stringstream stream;
    stream << begin << " " << count << " " << end;
    ShowAdministratorMessage(stream.str());
}

void ShowBalance(::Pbft::ClientId id, const ::std::string& message, uint32_t sum)
{
    ::std::stringstream stream;
    ::std::string extendedMessage(message + ": ");
    stream << "Balance" << ::std::setw(20) << extendedMessage << ::std::setw(10) << sum;
    ShowClientMessage(id, stream.str());
}

void Client0Activity(::std::shared_ptr<::Pbft::ClientConnectionInterface> client, ::Pbft::ClientId id,
    ::Pbft::ClientId destinationId, ::std::mutex& mutex, ::std::condition_variable& condition,
        const bool& exitCommand)
{
    while (!Exit(mutex, condition, exitCommand, ::std::chrono::milliseconds(400ll)))
    {
        try
        {
            ShowBalance(id, "before TopUp", client->Balance());
        }
        catch (const ::std::exception& ex)
        {
            ShowClientMessage(id, ex.what());
        }

        try
        {
            const auto sum(10);
            client->TopUp(sum);
            ShowBalance(id, "after TopUp", client->Balance());

            ShowBalance(id, "before Transmit", client->Balance());
            client->Transmit(destinationId, sum);
            ShowBalance(id, "after Transmit", client->Balance());
        }
        catch (const ::std::exception& ex)
        {
            ShowClientMessage(id, ex.what());
        }
    }
}

void Client1Activity(::std::shared_ptr<::Pbft::ClientConnectionInterface> client, ::Pbft::ClientId id,
    ::Pbft::ClientId sourceId, ::std::mutex& mutex, ::std::condition_variable& condition,
        const bool& exitCommand)
{
    while (!Exit(mutex, condition, exitCommand, ::std::chrono::milliseconds(800ll)))
    {
        try
        {
            const auto balance(client->Balance());
            ShowBalance(id, "before Withdraw", client->Balance());
            client->Withdraw(balance);
            ShowBalance(id, "after Withdraw", client->Balance());
        }
        catch (const ::std::exception& ex)
        {
            ShowClientMessage(id, ex.what());
        }
    }
}

void MakeNodesFaulty(::std::shared_ptr<::Pbft::BackdoorConnectionInterface> backdoor,
    const ::std::vector<::Pbft::NodeId>& nodeIds, uint32_t count)
{
    ShowAdministratorActivityMessage("Make", count, "nodes faulty");
    for (const auto& id : nodeIds)
    {
        if (count > 0)
        {
            --count;
            backdoor->SetFaulty(id);
        }
        else
        {
            backdoor->SetOperational(id);
        }
    }
}

void CreateNodes(::std::shared_ptr<::Pbft::BackdoorConnectionInterface> backdoor,
    ::std::vector<::Pbft::NodeId>& nodeIds, uint32_t count)
{
    ShowAdministratorActivityMessage("Create", count, "nodes");
    while (count-- > 0)
    {
        nodeIds.emplace_back(backdoor->CreateNode());
    }
}

void DeleteNodes(::std::shared_ptr<::Pbft::BackdoorConnectionInterface> backdoor,
    ::std::vector<::Pbft::NodeId>& nodeIds, uint32_t count)
{
    ShowAdministratorActivityMessage("Delete", count, "nodes");
    while (count-- > 0)
    {
        backdoor->DeleteNode(nodeIds.back());
        nodeIds.pop_back();
    }
}

void AdministratorActivity(::std::shared_ptr<::Pbft::BackdoorConnectionInterface> backdoor,
    ::std::mutex& mutex, ::std::condition_variable& condition, const bool& exitCommand)
{
    ::std::vector<::Pbft::NodeId> nodeIds;
    const auto nodeCount0(0u);
    const auto nodeCount1(1u);
    const auto nodeCount2(2u);
    const auto nodeCount3(3u);
    const auto nodeCount4(4u);
    const auto nodeCount5(5u);
    ::std::vector<::std::function<void()>> activities
    {
        [&backdoor, &nodeIds, nodeCount4]{CreateNodes(backdoor, nodeIds, nodeCount4);},
        [&backdoor, &nodeIds, nodeCount1]{MakeNodesFaulty(backdoor, nodeIds, nodeCount1);},
        [&backdoor, &nodeIds, nodeCount2]{MakeNodesFaulty(backdoor, nodeIds, nodeCount2);},
        [&backdoor, &nodeIds, nodeCount3]{MakeNodesFaulty(backdoor, nodeIds, nodeCount3);},
        [&backdoor, &nodeIds, nodeCount3]{CreateNodes(backdoor, nodeIds, nodeCount3);},
        [&backdoor, &nodeIds, nodeCount4]{MakeNodesFaulty(backdoor, nodeIds, nodeCount4);},
        [&backdoor, &nodeIds, nodeCount5]{MakeNodesFaulty(backdoor, nodeIds, nodeCount5);},
        [&backdoor, &nodeIds, nodeCount4]{MakeNodesFaulty(backdoor, nodeIds, nodeCount4);},
        [&backdoor, &nodeIds, nodeCount3]{MakeNodesFaulty(backdoor, nodeIds, nodeCount3);},
        [&backdoor, &nodeIds, nodeCount3]{DeleteNodes(backdoor, nodeIds, nodeCount3);},
        [&backdoor, &nodeIds, nodeCount2]{MakeNodesFaulty(backdoor, nodeIds, nodeCount2);},
        [&backdoor, &nodeIds, nodeCount1]{MakeNodesFaulty(backdoor, nodeIds, nodeCount1);},
        [&backdoor, &nodeIds, nodeCount0]{MakeNodesFaulty(backdoor, nodeIds, nodeCount0);},
        [&backdoor, &nodeIds, nodeCount4]{DeleteNodes(backdoor, nodeIds, nodeCount4);}
    };

    auto currentActivity(0u);
    while (!Exit(mutex, condition, exitCommand, ::std::chrono::milliseconds(2000ll)))
    {
        try
        {
            activities[currentActivity]();
        }
        catch (const ::std::exception& ex)
        {
            ShowAdministratorMessage(ex.what());
        }

        if (++currentActivity == activities.size())
        {
            currentActivity = 0;
        }
    }
}

}

void main()
{
    auto clientId0(0u);
    ::std::shared_ptr<::Pbft::ClientConnectionInterface> client0(::Pbft::ConnectionFactory::
        CreateClientConnection(clientId0));
    auto clientId1(1u);
    ::std::shared_ptr<::Pbft::ClientConnectionInterface> client1(::Pbft::ConnectionFactory::
        CreateClientConnection(clientId1));
    ::std::shared_ptr<::Pbft::BackdoorConnectionInterface> backdoor(::Pbft::ConnectionFactory::
        CreateBackdoorConnection());

    ::std::condition_variable condition;
    ::std::mutex mutex;
    auto exitCommand(false);
    ::std::thread client0Activity([&client0, &clientId0, &clientId1, &mutex, &condition, &exitCommand]{
        Client0Activity(client0, clientId0, clientId1, mutex, condition, exitCommand);});
    ::std::thread client1Activity([&client1, &clientId1, &clientId0, &mutex, &condition, &exitCommand]{
        Client1Activity(client1, clientId1, clientId0, mutex, condition, exitCommand);});
    ::std::thread administratorActivity([&backdoor, &mutex, &condition, &exitCommand]{
        AdministratorActivity(backdoor, mutex, condition, exitCommand);});

    WaitInput();
    RequestExit(mutex, condition, exitCommand);

    administratorActivity.join();
    client1Activity.join();
    client0Activity.join();
}
