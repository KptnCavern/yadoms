#include "stdafx.h"
#include "Color.h"
#include <shared/plugin/yPluginApi/StandardUnits.h>
#include <shared/plugin/yPluginApi/StandardCapacities.h>

//TODO V2 : To be copy in a library teleInfo
namespace teleInfo
{
   namespace specificHistorizers
   {
      DECLARE_ENUM_IMPLEMENTATION(ETeleInfoColor,
         ((NOTDEFINED))
         ((BLUE))
         ((WHITE))
         ((RED))
      );

      DECLARE_CAPACITY(ColorCapacity, "ForecastPrice", shared::plugin::yPluginApi::CStandardUnits::NoUnits, shared::plugin::yPluginApi::EKeywordDataType::kEnum);

      CColor::CColor(const std::string& keywordName)
         : CSingleHistorizableData<ETeleInfoColor>(keywordName,
                                                   ColorCapacity(),
                                                   shared::plugin::yPluginApi::EKeywordAccessMode::kGet)
      {
      }

      CColor::~CColor()
      {
      }
   }
} // namespace teleInfo::specificHistorizer


