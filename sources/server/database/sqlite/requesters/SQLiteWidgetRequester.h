#pragma once

#include "server/database/IWidgetRequester.h"

namespace server { 
   namespace database { 
      namespace sqlite { 

         class CSQLiteDataProvider;
         class CSQLiteRequester;

         namespace requesters { 

            //--------------------------------------------------------------
            /// \Brief		   Widget requester for SQLite database
            //--------------------------------------------------------------
            class CSQLiteWidgetRequester: public IWidgetRequester
            {
            public:
               //--------------------------------------------------------------
               /// \Brief		   Constructor
               /// \param [in]	pDatabaseHandler: the database handler
               //--------------------------------------------------------------
               CSQLiteWidgetRequester(const CSQLiteDataProvider & databaseHandler, boost::shared_ptr<CSQLiteRequester> & databaseRequester);

               //--------------------------------------------------------------
               /// \Brief		   Destructor
               //--------------------------------------------------------------
               virtual ~CSQLiteWidgetRequester();

               // IWidgetRequester implementation
               virtual int addWidget(const server::database::entities::CWidget & newWidget);
               virtual boost::shared_ptr<server::database::entities::CWidget> getWidget(int widgetId);
               virtual std::vector<boost::shared_ptr<server::database::entities::CWidget> > getWidgets();
               virtual std::vector<boost::shared_ptr<server::database::entities::CWidget> > getWidgetsForPage(int pageId);
               virtual void updateWidgetConfiguration(int widgetId, const std::string& newConfiguration);
               virtual void updateWidgetSize(int widgetId, int sizeX, int sizeY);
               virtual void updateWidgetPosition(int widgetId, int positionX, int positionY);
               virtual void removeWidget(int widgetId);
               virtual void removeWidgetsInPage(int pageId);
               virtual void removeAllWidgets();
               // [END] IWidgetRequester implementation

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
