#pragma once

#include "../../serialization/IPtreeToStringSerializer.h"


namespace shared { namespace plugin { namespace configuration
{

   //--------------------------------------------------------------
   /// \class Plugin configuration factory
   //--------------------------------------------------------------
   class CFactory
   {
      // This class can only be used by the configuration
      friend class CConfiguration;

   private:
      //--------------------------------------------------------------
      /// \brief	   Serializer creation
      /// \return    A new serializer instance
      //--------------------------------------------------------------
      static boost::shared_ptr<serialization::IPtreeToStringSerializer> createSerializer();
   };

} } } // namespace shared::plugin::configuration
