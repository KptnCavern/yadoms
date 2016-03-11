#pragma once

#include "../common/Query.h"


namespace database { namespace sqlite { 


   //
   /// \brief  Class used to create queries
   //
   class CSQLiteQuery : public common::CQuery
   {
   public:
      //
      /// \brief           Constructor
      //
      CSQLiteQuery();

      //
      /// \brief           Destructor
      //
      virtual ~CSQLiteQuery();

      // Overriding common::CQuery
      virtual const std::string functionDateToIsoString(const std::string &sqlPart);
      // [END] - Overriding common::CQuery 

   };


} //namespace sqlite
} //namespace database 

