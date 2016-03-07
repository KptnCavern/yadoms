#include "stdafx.h"
#include "EventLogger.h"
#include <shared/currentTime/Provider.h>
#include <shared/exception/NotImplemented.hpp>
#include <shared/exception/InvalidParameter.hpp>
#include <shared/exception/EmptyResult.hpp>
#include "database/common/adapters/SingleValueAdapter.hpp"
#include "database/common/adapters/DatabaseAdapters.h"
#include "database/common/DatabaseTables.h"
#include "database/common/Query.h"


namespace database { namespace common { namespace requesters { 

   CEventLogger::CEventLogger(boost::shared_ptr<IDatabaseRequester> databaseRequester)
      :m_databaseRequester(databaseRequester)
   {
   }

   CEventLogger::~CEventLogger()
   {
   }

   // IEventLoggerRequester implementation

   int CEventLogger::addEvent(const entities::ESystemEventCode & eventCode, const std::string & who, const std::string & what)
   {
      boost::posix_time::ptime insertDate = shared::currentTime::Provider::now();

      CQuery qInsert = m_databaseRequester->newQuery();
      qInsert. InsertInto(CEventLoggerTable::getTableName(), CEventLoggerTable::getCodeColumnName(), CEventLoggerTable::getWhoColumnName(), CEventLoggerTable::getWhatColumnName(), CEventLoggerTable::getDateColumnName()).
         Values(eventCode, who, what, insertDate);

      if(m_databaseRequester->queryStatement(qInsert) <= 0)
         throw shared::exception::CEmptyResult("No lines affected");

      CQuery qSelect = m_databaseRequester->newQuery();
      qSelect. Select(CEventLoggerTable::getIdColumnName()).
         From(CEventLoggerTable::getTableName()).
         Where(CEventLoggerTable::getCodeColumnName(), CQUERY_OP_EQUAL, eventCode).
         And(CEventLoggerTable::getDateColumnName(), CQUERY_OP_EQUAL, insertDate).
         OrderBy(CEventLoggerTable::getIdColumnName(), CQUERY_ORDER_DESC);

      adapters::CSingleValueAdapter<int> adapter;
      m_databaseRequester->queryEntities(&adapter, qSelect);
      if(adapter.getResults().size() >= 1)
         return adapter.getResults()[0];
      
      throw shared::exception::CEmptyResult("Cannot retrieve inserted ");
   }


   std::vector<boost::shared_ptr<entities::CEventLogger> > CEventLogger::getEvents()
   {
      CQuery qSelect = m_databaseRequester->newQuery();
      qSelect. Select().
         From(CEventLoggerTable::getTableName()).
         OrderBy(CEventLoggerTable::getIdColumnName(), CQUERY_ORDER_DESC);

      adapters::CEventLoggerAdapter adapter;
      m_databaseRequester->queryEntities(&adapter, qSelect);
      return adapter.getResults();
   }

   boost::shared_ptr<entities::CEventLogger>  CEventLogger::getEvent(const int eventId)
   {
      CQuery qSelect = m_databaseRequester->newQuery();
      qSelect. Select().
         From(CEventLoggerTable::getTableName()).
         Where(CEventLoggerTable::getIdColumnName(), CQUERY_OP_EQUAL, eventId).
         Limit(1);

      adapters::CEventLoggerAdapter adapter;
      m_databaseRequester->queryEntities(&adapter, qSelect);
      if(adapter.getResults().empty())
         return boost::shared_ptr<entities::CEventLogger>(); 
      return adapter.getResults()[0];
   }

   boost::shared_ptr<entities::CEventLogger>  CEventLogger::getLastEvent()
   {
      CQuery qSelect = m_databaseRequester->newQuery();
      qSelect. Select().
         From(CEventLoggerTable::getTableName()).
         OrderBy(CEventLoggerTable::getIdColumnName(), CQUERY_ORDER_DESC).
         Limit(1);

      adapters::CEventLoggerAdapter adapter;
      m_databaseRequester->queryEntities(&adapter, qSelect);

      std::vector<boost::shared_ptr<entities::CEventLogger> > list = adapter.getResults();
      if(list.empty())
         return boost::shared_ptr<entities::CEventLogger>(); 
      return adapter.getResults()[0];
   }

   std::vector<boost::shared_ptr<entities::CEventLogger> >  CEventLogger::getEventsFrom(const int eventId)
   {
      CQuery qSelect = m_databaseRequester->newQuery();
      qSelect. Select().
         From(CEventLoggerTable::getTableName()).
         Where(CEventLoggerTable::getIdColumnName(), CQUERY_OP_SUP, eventId).
         OrderBy(CEventLoggerTable::getIdColumnName(), CQUERY_ORDER_DESC, CEventLoggerTable::getDateColumnName(), CQUERY_ORDER_DESC);

      adapters::CEventLoggerAdapter adapter;
      m_databaseRequester->queryEntities(&adapter, qSelect);
      return adapter.getResults();
   }

   std::vector<boost::shared_ptr<entities::CEventLogger> >  CEventLogger::getEventsRange(const int offset, const int count)
   {
      CQuery qSelect = m_databaseRequester->newQuery();
      qSelect. Select().
         From(CEventLoggerTable::getTableName()).
         OrderBy(CEventLoggerTable::getIdColumnName(), CQUERY_ORDER_DESC, CEventLoggerTable::getDateColumnName(), CQUERY_ORDER_DESC).
         Limit(count, offset);

      adapters::CEventLoggerAdapter adapter;
      m_databaseRequester->queryEntities(&adapter, qSelect);
      return adapter.getResults();
   }

   // [END] IEventLoggerRequester implementation

} //namespace requesters
} //namespace common
} //namespace database 

