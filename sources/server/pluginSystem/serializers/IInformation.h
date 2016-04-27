#pragma once

#include <pluginInformation.pb.h>


namespace pluginSystem
{
   namespace serializers
   {
      //--------------------------------------------------------------
      /// \brief		Serializer of IInformation
      //--------------------------------------------------------------
      class IInformation
      {
      public:
         virtual ~IInformation()
         {
         }

         virtual void toPb(pbPluginInformation::Information* pb) const = 0;
      };
   }
} // namespace pluginSystem::serializers
