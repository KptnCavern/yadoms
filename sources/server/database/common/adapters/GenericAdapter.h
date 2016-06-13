#pragma once

#include "IResultAdapter.h"


namespace database { 
namespace common { 
namespace adapters { 


   //--------------------------------------------------------------
   /// \Brief		Adapter for generic table results
   //--------------------------------------------------------------
   class CGenericAdapter : public IResultAdapterEx< std::map<std::string, std::string> >
   {
   public:
      //--------------------------------------------------------------
      /// \Brief		Constructor
      //--------------------------------------------------------------
      CGenericAdapter();
   
      //--------------------------------------------------------------
      /// \Brief		Destructor
      //--------------------------------------------------------------
      virtual ~CGenericAdapter();
   
      // ISQLiteResultAdapter implementation
      bool adapt(boost::shared_ptr<IResultHandler> resultHandler);
      // [END] ISQLiteResultAdapter implementation

      std::vector<std::map<std::string, std::string> > getResults();
   private:
      //--------------------------------------------------------------
      /// \Brief		Contains the list of Plugin entities
      //--------------------------------------------------------------
      std::vector<std::map<std::string, std::string> > m_results;
   };

} //namespace adapters
} //namespace common
} //namespace database 

