#pragma once
#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include "WeatherUndergroundHelpers.h"
#include <shared/DataContainer.h>
#include "WUConfiguration.h"
#include "Keywords/Forecast.h"

namespace yApi = shared::plugin::yPluginApi;

//--------------------------------------------------------------
/// \brief	Forecast X Days Module
/// \note   This class consist of the traitment of the forecast information during X days from the web site
//--------------------------------------------------------------
class CForecastDays
{
public:
   //--------------------------------------------------------------
   /// \brief	  Constructor
   /// \param[in] api              pointer to the API
   /// \param[in] wuConfiguration  The Configuration of the module
   //--------------------------------------------------------------
   CForecastDays(boost::shared_ptr<yApi::IYPluginApi> api,
                 IWUConfiguration& wuConfiguration);

   //--------------------------------------------------------------
   /// \brief	  Parse the answer from the web Site
   /// \param[in] api             pointer to the API
   /// \param[in] wuConfiguration The configuration of the module
   /// \param[in] dataToParse     data to parse
   //--------------------------------------------------------------
   void parse(boost::shared_ptr<yApi::IYPluginApi> api,
              const IWUConfiguration& wuConfiguration,
              const shared::CDataContainer dataToParse) const;

   //--------------------------------------------------------------
   /// \brief	  Update the configuration when something change from the HMI
   /// \param[in] api                pointer to the API
   /// \param[in] wuConfiguration    The Plugin configuration
   //--------------------------------------------------------------
   void onUpdate(boost::shared_ptr<yApi::IYPluginApi> api,
                 IWUConfiguration& wuConfiguration);

   //--------------------------------------------------------------
   /// \brief	  Set the city Name
   /// \param[in] wuConfiguration    The Plugin configuration
   //--------------------------------------------------------------
   void setCityName(const std::string& CityName) const;

   //--------------------------------------------------------------
   /// \brief	  Return the url
   /// \return    The url string
   //--------------------------------------------------------------
   std::string getUrl() const;

   //--------------------------------------------------------------
   /// \brief	    Destructor
   //--------------------------------------------------------------
   virtual ~CForecastDays();

private:

   //--------------------------------------------------------------
   /// \brief	  Initialise Forecast 10 Days variables
   /// \param[in] api                pointer to the API
   /// \param[in] wuConfiguration    The Plugin configuration
   //--------------------------------------------------------------
   void InitializeForecastDays(boost::shared_ptr<yApi::IYPluginApi> api,
                               IWUConfiguration& wuConfiguration);

   //--------------------------------------------------------------
   /// \brief	    Your Location to received custom information from the web site
   //--------------------------------------------------------------
   std::string m_localisation;

   //--------------------------------------------------------------
   /// \brief	    Your Location to received custom information from the web site
   //--------------------------------------------------------------
   std::string m_countryOrState;

   //--------------------------------------------------------------
   /// \brief	    url code between 3 or 10 days
   //--------------------------------------------------------------
   std::string m_prefix;

   //--------------------------------------------------------------
   /// \brief	    The Device Name
   //--------------------------------------------------------------
   std::string m_deviceName;

   //--------------------------------------------------------------
   /// \brief	    The url link to access properly the web site
   //--------------------------------------------------------------
   std::stringstream m_url;

   //--------------------------------------------------------------
   /// \brief	    Keywords
   //--------------------------------------------------------------
   boost::shared_ptr<CForecast> m_forecast;

   //--------------------------------------------------------------
   /// \brief	    Tab of rain keywords for 3 next days
   //--------------------------------------------------------------
   boost::shared_ptr<yApi::historization::CRainRate> m_forecastRain[NB_RAIN_FORECAST_DAY];

   //--------------------------------------------------------------
   /// \brief	    the temperature for the next day
   //--------------------------------------------------------------
   boost::shared_ptr<yApi::historization::CTemperature> m_temp;

   //--------------------------------------------------------------
   /// \brief  Keywords list
   //--------------------------------------------------------------
   std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> > m_keywords;

   //--------------------------------------------------------------
   /// \brief  developerMode state
   //--------------------------------------------------------------
   bool m_isDeveloperMode;
};