#include "Connection.h"

namespace Pbft {

Connection::Connection(DatabaseInterface& databaseToSet) : database(databaseToSet)
{
}

DatabaseInterface& Connection::Database() const
{
    return database;
}

}
