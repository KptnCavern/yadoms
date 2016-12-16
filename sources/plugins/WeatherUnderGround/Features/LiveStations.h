#pragma once
#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include <shared/DataContainer.h>

namespace yApi = shared::plugin::yPluginApi;

//--------------------------------------------------------------
/// \brief	Live Station return by WeatherUnderground
/// \note   This class consist of the traitment of all stations returned by the WeatherUnderground website
//--------------------------------------------------------------
class CLiveStations
{
public:
   //--------------------------------------------------------------
   /// \brief	  Constructor
   /// \param[in] api              pointer to the API
   //--------------------------------------------------------------
   CLiveStations(boost::shared_ptr<yApi::IYPluginApi> api);

   //--------------------------------------------------------------
   /// \brief	  Constructor
   /// \param[in] newLocation       enter directly a location
   //--------------------------------------------------------------
   CLiveStations(boost::shared_ptr<const shared::ILocation> location);

   //--------------------------------------------------------------
   /// \brief get All forecast stations around the location
   /// \param[in] api       pointer to the API
   /// \param[in] apikey    api key to weatherUnderground
   //--------------------------------------------------------------
   void getAllStations(boost::shared_ptr<yApi::IYPluginApi> api,
                       const std::string& apikey);

   //--------------------------------------------------------------
   /// \brief	    bindAvailableStations
   /// \return all forecast available stations
   //--------------------------------------------------------------
   shared::CDataContainer bindAvailableStations();


   //--------------------------------------------------------------
   /// \brief	    getStationLocation
   /// \param[in] selection      list selection return by the web site
   /// \return                   the location (longitude, latitude) structure of the selected forecast station
   //--------------------------------------------------------------
   boost::shared_ptr<const shared::ILocation> getStationLocation(int selection);

   //--------------------------------------------------------------
   /// \brief	    getStationName
   /// \param[in] selection      list selection return by the web site
   /// \return                   the forecast station name
   //--------------------------------------------------------------
   std::string getStationName(int selection);

   //--------------------------------------------------------------
   /// \brief	    Destructor
   //--------------------------------------------------------------
   virtual ~CLiveStations();

private:

   //--------------------------------------------------------------
   /// \brief	    GPS Location
   //--------------------------------------------------------------
   boost::shared_ptr<const shared::ILocation> m_location;

   //--------------------------------------------------------------
   /// \brief	    list of all Stations return by WeatherUnderground
   //--------------------------------------------------------------
   std::vector<shared::CDataContainer> m_stations;
};

