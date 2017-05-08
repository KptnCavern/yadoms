#pragma once
#include <shared/Export.h>
#include <shared/enumeration/EnumHelpers.hpp>
#include <shared/plugin/yPluginApi/historization/SingleHistorizableData.hpp>

//TODO_V2 : To be copy in a library teleInfo
namespace specificHistorizers
{
   //-----------------------------------------------------
   ///\brief  Status of the TeleInfo
   //-----------------------------------------------------   
   DECLARE_ENUM_HEADER(EdeviceStatus,
      ((Undefined))
      ((Ok))
      ((Error))
      ((TimeOut))
      );

   //-----------------------------------------------------
   ///\brief A Status object
   //-----------------------------------------------------
   class CdeviceStatus : public shared::plugin::yPluginApi::historization::CSingleHistorizableData<EdeviceStatus>
   {
   public:
      //-----------------------------------------------------
      ///\brief                     Constructor
      ///\param[in] keywordName     Yadoms keyword name
      //-----------------------------------------------------
      explicit CdeviceStatus(const std::string& keywordName);

      //-----------------------------------------------------
      ///\brief                     Destructor
      //-----------------------------------------------------
      virtual ~CdeviceStatus();
   };
} // namespace specificHistorizers