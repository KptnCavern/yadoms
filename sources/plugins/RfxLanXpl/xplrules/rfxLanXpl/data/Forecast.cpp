#include "stdafx.h"
#include "Forecast.h"
#include <shared/plugin/yPluginApi/StandardUnits.h>

namespace xplrules
{
   namespace rfxLanXpl
   {
      namespace data
      {
         DECLARE_ENUM_IMPLEMENTATION(EForecast,
            ((Sunny)("sunny"))
            ((PartlyCloudy)("partly cloudy"))
            ((Cloudy)("cloudy"))
            ((Rain)("rain"))
         );


         static const shared::plugin::yPluginApi::CStandardCapacity ForecastCapacity("forecast",
                                                                                     shared::plugin::yPluginApi::CStandardUnits::NoUnits,
                                                                                     shared::plugin::yPluginApi::EKeywordDataType::kNoData);


         CForecast::CForecast(const std::string& keywordName)
            : CSingleHistorizableData<EForecast>(keywordName,
                                                 ForecastCapacity,
                                                 shared::plugin::yPluginApi::EKeywordAccessMode::kGet)
         {
         }

         CForecast::~CForecast()
         {
         }
      }
   }
} // namespace xplrules::rfxLanXpl::data


