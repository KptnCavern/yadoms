#include "stdafx.h"
#include "ForecastDays.h"
#include <shared/exception/Exception.hpp>
#include "Keywords/KeywordHelpers.h"

CForecastDays::CForecastDays(boost::shared_ptr<yApi::IYPluginApi> api,
                             IWUConfiguration& wuConfiguration,
                             boost::shared_ptr<const shared::ILocation> location,
                             const std::string& deviceName,
                             const std::string& stationName)
   : m_localisation(stationName),
     m_deviceName(deviceName),
     m_forecast(boost::make_shared<CForecast>(m_deviceName, "Forecast", weatherunderground::helper::EPeriod::kDay)),
     m_temp(boost::make_shared<yApi::historization::CTemperature>("low_temperature")),
     m_isDeveloperMode(false),
     m_url ("http://api.wunderground.com/api/" + wuConfiguration.getAPIKey() + "/forecast/q/" + boost::lexical_cast<std::string>(location->latitude()) + "," + boost::lexical_cast<std::string>(location->longitude()) + ".json"),
     m_location(location)
{
   try
   {
      InitializeForecastDays(api, wuConfiguration);
   }
   catch (shared::exception::CException& e)
   {
      std::cout << "Configuration or initialization error of Forecast module :" << e.what() << std::endl;
      throw;
   }
}

void CForecastDays::InitializeForecastDays(boost::shared_ptr<yApi::IYPluginApi> api, IWUConfiguration& wuConfiguration)
{
   if (wuConfiguration.isForecast10DaysEnabled())
   {
      m_keywords.clear();

      m_keywords.push_back(m_forecast->getHistorizable());

      m_forecast->addUnit(shared::plugin::yPluginApi::CStandardCapacities::Temperature.getName(),
                          shared::plugin::yPluginApi::CStandardCapacities::Temperature.getUnit());
      m_forecast->addUnit(shared::plugin::yPluginApi::CStandardCapacities::Speed.getName(),
                          shared::plugin::yPluginApi::CStandardCapacities::Speed.getUnit());
      m_forecast->addUnit(shared::plugin::yPluginApi::CStandardCapacities::Humidity.getName(),
                          shared::plugin::yPluginApi::CStandardCapacities::Humidity.getUnit());
      m_forecast->addUnit(shared::plugin::yPluginApi::CStandardCapacities::Rain.getName(),
                          shared::plugin::yPluginApi::CStandardCapacities::Rain.getUnit());

      if (wuConfiguration.isRainIndividualKeywordsEnabled())
      {
         for (auto counter = 0; counter < NB_RAIN_FORECAST_DAY; ++counter)
         {
            std::stringstream TempString;
            TempString << m_prefix << "Rain_Day_" << counter;
            m_forecastRain[counter] = boost::make_shared<yApi::historization::CRainRate>(TempString.str());
            m_keywords.push_back(m_forecastRain[counter]);
         }
      }

      m_keywords.push_back(m_temp);

      //TODO : Pr�voir la cr�ation et la suppression des keywords en plus

      // Declare keywords
      std::string m_type = "forecast";
      shared::CDataContainer details;
      details.set("type", m_type);
      api->declareDevice(m_deviceName, m_type, m_keywords, details);
   }
}

void CForecastDays::onPluginUpdate(boost::shared_ptr<yApi::IYPluginApi> api,
                                   IWUConfiguration& wuConfiguration)
{
   try
   {
      m_url.str("");
      m_url << "http://api.wunderground.com/api/" << wuConfiguration.getAPIKey() << "/forecast/q/" << boost::lexical_cast<std::string>(m_location->latitude()) << "," << boost::lexical_cast<std::string>(m_location->longitude()) << ".json";
   }
   catch (shared::exception::CException& e)
   {
      std::cout << "Configuration or initialization error in forecast module :" << e.what() << std::endl;
      throw;
   }
}

void CForecastDays::parse(boost::shared_ptr<yApi::IYPluginApi> api,
                          const shared::CDataContainer dataToParse,
                          IWUConfiguration& wuConfiguration)
{
   try
   {
      if (wuConfiguration.isForecast10DaysEnabled())
      {
         auto result = dataToParse.get<std::vector<shared::CDataContainer> >("forecast.simpleforecast.forecastday");
         std::vector<shared::CDataContainer>::iterator i;

         m_forecast->clearAllPeriods();

         unsigned char counter = 0;

         for (i = result.begin(); i != result.end(); ++i)
         {
            m_forecast->addPeriod(*i,
                                    "date.year",
                                    "date.month",
                                    "date.day",
                                    "icon",
                                    "high.celsius",
                                    "low.celsius",
                                    "maxwind.kph",
                                    "avewind.kph",
                                    "avewind.degrees",
                                    "avehumidity",
                                    "qpf_allday.mm",
                                    "snow_allday.cm");

            if (counter == 0)
            {
               double temp = 0;
               if (convertDouble(temp, *i, "low.celsius"))
                  m_temp->set(temp);
            }

            if (wuConfiguration.isRainIndividualKeywordsEnabled())
            {
               if (counter < NB_RAIN_FORECAST_DAY)
               {
                  double rainRate1h = 0;
                  if (convertDouble(rainRate1h, *i, "qpf_allday.mm"))
                     m_forecastRain[counter]->set(rainRate1h);
               }
            }

            ++counter;
         }
      }
      api->historizeData(m_deviceName, m_keywords);

      std::cout << "Refresh Forecast Information" << std::endl;
   }
   catch (shared::exception::CException& e)
   {
      std::cout << "Error during parsing the element ! : " << e.what() << std::endl;
   }
}

void CForecastDays::setCityName(const std::string& CityName)
{
   m_forecast->setCityName(CityName);
}

CForecastDays::~CForecastDays()
{
}

std::string CForecastDays::getUrl() const
{
   return m_url.str();
}

std::string CForecastDays::getName() const
{
   return m_deviceName;
}