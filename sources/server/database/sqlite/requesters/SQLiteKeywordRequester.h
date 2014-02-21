#pragma once

#include "server/database/IKeywordRequester.h"

namespace server { 
   namespace database { 
      namespace sqlite { 

         class CSQLiteDataProvider;
         class CSQLiteRequester;

         namespace requesters { 

            //--------------------------------------------------------------
            /// \Brief		   Keyword requester for SQLite database
            //--------------------------------------------------------------
            class CSQLiteKeywordRequester: public IKeywordRequester
            {
            public:
               //--------------------------------------------------------------
               /// \Brief		   Constructor
               /// \param [in]	pDatabaseHandler: the database handler
               //--------------------------------------------------------------
               CSQLiteKeywordRequester(const CSQLiteDataProvider & databaseHandler, boost::shared_ptr<CSQLiteRequester> & databaseRequester);

               //--------------------------------------------------------------
               /// \Brief		   Destructor
               //--------------------------------------------------------------
               virtual ~CSQLiteKeywordRequester();

               // IKeywordRequester implementation
               virtual void addKeyword(boost::shared_ptr<server::database::entities::CKeyword> newKeyword);
               virtual boost::shared_ptr<server::database::entities::CKeyword> getKeyword(const std::string & keyword);
               virtual std::vector<boost::shared_ptr<server::database::entities::CKeyword> > getKeywords();
               virtual void removeKeyword(const std::string & keyword);
               // [END] IKeywordRequester implementation

            private:
               //--------------------------------------------------------------
               /// \Brief		   Reference to SQLiteDatabseHandler
               //--------------------------------------------------------------
               const CSQLiteDataProvider & m_databaseHandler;

               //--------------------------------------------------------------
               /// \Brief		   Reference to SQLiteRequester
               //--------------------------------------------------------------
               boost::shared_ptr<CSQLiteRequester> m_databaseRequester;
            };

         } //namespace requesters
      } //namespace sqlite
   } //namespace database 
} //namespace server
