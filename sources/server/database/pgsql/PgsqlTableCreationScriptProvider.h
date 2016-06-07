#pragma once

#include "database/ITableCreationScriptProvider.h"


namespace database { namespace pgsql {

   class CPgsqlTableCreationScriptProvider : public ITableCreationScriptProvider
   {
   public:
      //--------------------------------------------------------------
      /// \brief       Constructor
      //--------------------------------------------------------------
      CPgsqlTableCreationScriptProvider();

      //--------------------------------------------------------------
      /// \brief       Destructor
      //--------------------------------------------------------------
      virtual ~CPgsqlTableCreationScriptProvider();

      // ITableCreationScriptProvider implementation
      virtual std::string getTableConfiguration();
      virtual std::string getTableDevice();
      virtual std::string getTablePlugin();
      virtual std::string getTableKeyword();
      virtual std::string getTablePage();
      virtual std::string getTableWidget();
      virtual std::string getTablePluginEventLogger();
      virtual std::string getTableEventLogger();
      virtual std::string getTableAcquisition();
      virtual std::string getTableAcquisitionSummary();
      virtual std::string getTableRule();
      virtual std::string getTableRecipient();
      virtual std::string getTableRecipientField();
      virtual void getTableAcquisitionIndexes(std::vector<std::string> & indexScripts);
      // [END] ITableCreationScriptProvider implementation
   };
 
} //namespace pgsql
} //namespace database 
