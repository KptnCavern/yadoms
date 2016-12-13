#pragma once
#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include <shared/DataContainer.h>
#include "WUConfiguration.h"
#include "IdeviceConfiguration.h"
#include "Keywords/Moon.h"

namespace yApi = shared::plugin::yPluginApi;

//--------------------------------------------------------------
/// \brief	Astronomy Module
/// \note   This class consist of the traitment of the astronomy information from the web site
//--------------------------------------------------------------
class CAstronomy
{
public:
   //--------------------------------------------------------------
   /// \brief	  Constructor
   /// \param[in] api                  pointer to the API
   /// \param[in] wuConfiguration      The Configuration of the module
   /// \param[in] deviceconfiguration  The Configuration of the device
   //--------------------------------------------------------------
   CAstronomy(boost::shared_ptr<yApi::IYPluginApi> api,
              IWUConfiguration& wuConfiguration,
              boost::shared_ptr<IdeviceConfiguration> deviceConfiguration);

   //--------------------------------------------------------------
   /// \brief	  Parse the answer from the web Site
   /// \param[in] api             pointer to the API
   /// \param[in] wuConfiguration The configuration of the module
   /// \param[in] dataToParse     received data to parse
   //--------------------------------------------------------------
   void parse(boost::shared_ptr<yApi::IYPluginApi> api,
              const shared::CDataContainer dataToParse);

   //--------------------------------------------------------------
   /// \brief	  Update the configuration when something change from the HMI
   /// \param[in] api                  pointer to the API
   /// \param[in] wuConfiguration      The Plugin configuration
   //--------------------------------------------------------------
   void onPluginUpdate(boost::shared_ptr<yApi::IYPluginApi> api,
                       IWUConfiguration& wuConfiguration);

   //--------------------------------------------------------------
   /// \brief	  Update the configuration when something change from the HMI
   /// \param[in] api                  pointer to the API
   /// \param[in] deviceconfiguration  The Configuration of the device
   //--------------------------------------------------------------
   void onDeviceUpdate(boost::shared_ptr<yApi::IYPluginApi> api,
                       boost::shared_ptr<IdeviceConfiguration> deviceConfiguration);

   //--------------------------------------------------------------
   /// \brief	  Return the url
   /// \return    The url string
   //--------------------------------------------------------------
   std::string getUrl() const;

   //--------------------------------------------------------------
   /// \brief	    Destructor
   //--------------------------------------------------------------
   virtual ~CAstronomy();

private:

   //--------------------------------------------------------------
   /// \brief	  Initialise Astronomy keywords
   /// \param[in] api                  pointer to the API
   //--------------------------------------------------------------
   void initializeKeywords(boost::shared_ptr<yApi::IYPluginApi> api);

   //--------------------------------------------------------------
   /// \brief	    Your Location to received custom information from the web site
   //--------------------------------------------------------------
   std::string m_localisation;

   //--------------------------------------------------------------
   /// \brief	    Your Location to received custom information from the web site
   //--------------------------------------------------------------
   std::string m_countryOrState;

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
   boost::shared_ptr<CMoon> m_moonCharacteristics;

   //--------------------------------------------------------------
   /// \brief	    The device Configuration
   //--------------------------------------------------------------   
   boost::shared_ptr<IdeviceConfiguration> m_deviceConfiguration;

   //--------------------------------------------------------------
   /// \brief  Keywords list
   //--------------------------------------------------------------
   std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> > m_keywords;
};

