#include "stdafx.h"
#include "Condition.h"
#include <boost/lexical_cast.hpp>
#include "KeywordException.hpp"

// Shortcut to yPluginApi namespace
namespace yApi = shared::plugin::yPluginApi;

CCondition::CCondition(std::string PluginName,
                       std::string KeyWordName)
   : m_PluginName(PluginName),
     m_condition(boost::make_shared<CConditionHistorizer>(KeyWordName, yApi::EKeywordAccessMode::kGet))
{
}

void CCondition::Initialize(boost::shared_ptr<yApi::IYPluginApi> api,
                            shared::CDataContainer details) const
{
   if (!api->keywordExists(m_PluginName, m_condition))
      api->declareKeyword(m_PluginName, m_condition, details);
}

CCondition::~CCondition()
{
}

void CCondition::AddUnit(const std::string& UnitName,
                         const std::string& UnitValue) const
{
   m_condition->AddUnit(UnitName, UnitValue);
}

void CCondition::SetPeriod(const shared::CDataContainer& ValueContainer,
                           const std::string& filterTime,
                           const std::string& filterWeatherCondition,
                           const std::string& filterTemp,
                           const std::string& filterPressure,
                           const std::string& filterVisibility,
                           const std::string& filterUV,
                           const std::string& filterDewPoint,
                           const std::string& filterMaxWind,
                           const std::string& filterAveWind,
                           const std::string& filterAveWindDegrees,
                           const std::string& filterAveHumidity,
                           const std::string& filterRainDay,
                           const std::string& filterFeelslike,
                           const std::string& filterWindChill) const
{
   std::string WeatherIconTemp;

   auto it = weatherunderground::helper::EEnumTypeNames.find(ValueContainer.get<std::string>(filterWeatherCondition));
   if (it != weatherunderground::helper::EEnumTypeNames.end())
   {
      WeatherIconTemp = static_cast<yApi::historization::EWeatherCondition>(it->second).toString();
   }
   else
      throw CKeywordException("Value " + ValueContainer.get<std::string>(filterWeatherCondition) + " could not be set");

   m_condition->SetPeriod(ValueContainer.get<std::string>(filterTime),
                          WeatherIconTemp,
                          ValueContainer.get<std::string>(filterTemp),
                          ValueContainer.get<std::string>(filterPressure),
                          ValueContainer.get<std::string>(filterVisibility),
                          ValueContainer.get<std::string>(filterUV),
                          ValueContainer.get<std::string>(filterDewPoint),
                          boost::lexical_cast<std::string>(ValueContainer.get<double>(filterMaxWind) / 3.6), // Transform from Km/h -> m/s
                          boost::lexical_cast<std::string>(ValueContainer.get<double>(filterAveWind) / 3.6), // Transform from Km/h -> m/s
                          ValueContainer.get<std::string>(filterAveWindDegrees),
                          ValueContainer.get<std::string>(filterAveHumidity),
                          ValueContainer.get<std::string>(filterRainDay),
                          ValueContainer.get<std::string>(filterFeelslike),
                          ValueContainer.get<std::string>(filterWindChill));
}

void CCondition::SetCityName(const std::string& CityName) const
{
   m_condition->SetCityName(CityName);
}

boost::shared_ptr<yApi::historization::IHistorizable> CCondition::GetHistorizable() const
{
   return m_condition;
}

