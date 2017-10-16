/**
 * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 9/8/17 Carlos Brito *
 * * * * * * * * * * * * * * * * * * *
 * 
 * @file
 * @brief Contains class definitions.
 */
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include "DeviceRecordDAO.hpp"
#include "db/exceptions.hpp"
#include "db/util.hpp"

using namespace std;
using namespace solarplant::db::util;

namespace solarplant
{
namespace db
{
namespace dao
{
DeviceRecordDAO::DeviceRecordDAO(const std::shared_ptr<sql::Connection> conn)
        : GenericDAO( conn )
{
    assert(conn != nullptr);
}

entity::DeviceRecord DeviceRecordDAO::get(const DeviceRecordDAO::key_type & id)
{
    unique_ptr<sql::Statement>       stmt;
    unique_ptr<sql::ResultSet>       res;
    entity::DeviceRecord             deviceRecord;

    if ( conn == nullptr || !conn->isValid())
    {
        throw sql::SQLException( "Invalid connection! Attempting to query the db will result in a segfault" );
    }

    try
    {
        // Build condition.
        std::string cond;
        size_t idx = 0;
        cond += " WHERE ";

        static_for<0, n>([&](auto i) 
        {
            if constexpr (i < n - 1)
                cond += pk_columns[i] + "=" + quote(to_string(std::get<i.value>(id)), '\'') + " AND ";
            else
                cond += pk_columns[n - 1] + "=" + quote(to_string(std::get<i.value>(id)), '\'');
        });
        
        // Execute statement
        stmt = unique_ptr<sql::Statement>( conn->createStatement() );
        res  = unique_ptr<sql::ResultSet>(
                stmt->executeQuery("SELECT * FROM " + TABLE_NAME + cond + ";")
        );

        // Advance to first entry
        res->next();

        // Build deviceRecord
        deviceRecord.device_id  = res->getInt( "device_id" );
        deviceRecord.index      = res->getInt( "idx" );
        deviceRecord.t          = boost::posix_time::time_from_string(res->getString( "t" ));
        deviceRecord.cid        = res->getString( "cid" );
        deviceRecord.lif        = res->getString( "lif" );
        deviceRecord.lid        = res->getString( "lid" );

    } catch ( const sql::SQLException &e )
    {
        throw e; // Rethrow
    }

    return deviceRecord;
}

void DeviceRecordDAO::save(const entity::DeviceRecord & deviceRecord)
{
    unique_ptr<sql::PreparedStatement> stmt;
    unique_ptr<sql::ResultSet>         res;

    if ( conn == nullptr || !conn->isValid())
        throw sql::SQLException( "Invalid or NULL connection to database." );

    std::vector<std::string> values{
        to_string(deviceRecord.device_id),
        to_string(deviceRecord.index),
        to_string(deviceRecord.t),
        to_string(deviceRecord.cid),
        to_string(deviceRecord.lif),
        to_string(deviceRecord.lid)
    };

    try
    {
        stmt = unique_ptr<sql::PreparedStatement>(
            conn->prepareStatement(
                "INSERT INTO " + TABLE_NAME + "(" + util::as_comma_list(util::as_quote_vector(columns, '`')) + ") " +
                "VALUES (" + util::as_comma_list(util::as_quote_vector(values)) + ");"
        ));

        stmt->execute();
    } catch ( const sql::SQLException &e )
    {
        throw e; // Rethrow
    }

}
}
}
}

