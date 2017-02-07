#include "stdafx.h"
#include "Type3RemoteCode.h"
#include <shared/plugin/yPluginApi/StandardCapacities.h>
#include <shared/plugin/yPluginApi/StandardUnits.h>

namespace specificHistorizers {
      
   DECLARE_ENUM_IMPLEMENTATION(EType3RemoteCodeValues,
      ((Left))
      ((Right))
   )


   const yApi::CStandardCapacity& Type3RemoteCodeCapacity = yApi::CStandardCapacity("remoteCode", yApi::CStandardUnits::NoUnits, yApi::EKeywordDataType::kEnum);


   CType3RemoteCode::CType3RemoteCode(const std::string& keywordName)
      : CSingleHistorizableData<EType3RemoteCodeValues>(keywordName, Type3RemoteCodeCapacity, yApi::EKeywordAccessMode::kGetSet)
   {
   }

   CType3RemoteCode::~CType3RemoteCode()
   {
   }

} //namespace specificHistorizers