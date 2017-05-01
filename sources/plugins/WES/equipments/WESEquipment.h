#pragma once

#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include "IEquipment.h"
#include "specificHistorizers/Analog.h"
#include "../IWESConfiguration.h"
#include "masterDeviceConfiguration.h"
#include "WESSubEquipments/TIC.h"
#include "WESSubEquipments/Pulse.h"
#include "WESSubEquipments/Clamp.h"

// Shortcut to yPluginApi namespace
namespace yApi = shared::plugin::yPluginApi;

#define WES_RELAY_QTY 2
#define WES_TIC_QTY   2
#define WES_INPUT_QTY 2
#define WES_PULSE_QTY 4
#define WES_CLAMP_QTY 4
#define WES_ANA_QTY   4

namespace equipments
{
   //-----------------------------------------------------
   ///\brief WES equipment
   //-----------------------------------------------------
   class CWESEquipment : public IEquipment
   {
   public:
      //-----------------------------------------------------
      ///\brief                          Constructor
      ///\param[in]   api               Yadoms API
      ///\param[in] device              The device name
      ///\param[in] deviceConfiguration The device configuration
      ///\param[in] pluginConfiguration The plugin configuration
      //-----------------------------------------------------
      CWESEquipment(boost::shared_ptr<yApi::IYPluginApi> api,
                    const std::string& device,
                    const shared::CDataContainer& deviceConfiguration,
                    const boost::shared_ptr<IWESConfiguration> pluginConfiguration);

      // IEquipment implementation
      std::string getDeviceName() const override;
      std::string getDeviceType() const override;
      bool isMasterDevice() const override;
      void updateFromDevice(boost::shared_ptr<yApi::IYPluginApi> api,
                            const boost::shared_ptr<IWESConfiguration> pluginConfiguration,
                            bool forceHistorization = false) override;
      void updateConfiguration(boost::shared_ptr<yApi::IYPluginApi> api,
                               const shared::CDataContainer& newConfiguration) override;
      void sendCommand(boost::shared_ptr<yApi::IYPluginApi> api,
                       std::string& keyword,
                       std::string& command) override;
      void remove(boost::shared_ptr<yApi::IYPluginApi> api) override;
      // [END] IEquipment implementation

      //-----------------------------------------------------
      ///\brief                     Destructor
      //-----------------------------------------------------
      virtual ~CWESEquipment();

   private:

      //-----------------------------------------------------
      ///\brief                          Constructor
      ///\param[in] keywordsToHistorize list of keywords to historize
      ///\param[in] keyword             The keyword to update, if any
      ///\param[in] newValue            The new value
      ///\param[in] forceHistorization  force historization if necessary
      //-----------------------------------------------------
      void updateSwitchValue(std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> >& keywordsToHistorize, 
                             boost::shared_ptr<yApi::historization::CSwitch> keyword, 
                             bool newValue, 
                             bool forceHistorization);

      //-----------------------------------------------------
      ///\brief                     The device name
      //-----------------------------------------------------
      std::string m_deviceName;

      //-----------------------------------------------------
      ///\brief                     The device type
      //-----------------------------------------------------
      std::string m_deviceType;

      //--------------------------------------------------------------
      /// \brief	configuration of the device
      //--------------------------------------------------------------
      CmasterDeviceConfiguration m_configuration;

      //--------------------------------------------------------------
      /// \brief	vector of all relays
      //--------------------------------------------------------------
      std::vector<boost::shared_ptr<yApi::historization::CSwitch> > m_relaysList;

      //--------------------------------------------------------------
      /// \brief	vector of inputs
      //--------------------------------------------------------------
      std::vector<boost::shared_ptr<yApi::historization::CSwitch> > m_inputList;

      //--------------------------------------------------------------
      /// \brief  TIC counters
      //--------------------------------------------------------------
      std::vector<boost::shared_ptr<equipments::subdevices::CTIC> > m_TICList;

      //--------------------------------------------------------------
      /// \brief	Pulse counters
      //--------------------------------------------------------------
      std::vector<boost::shared_ptr<equipments::subdevices::CPulse> > m_PulseList;

      //--------------------------------------------------------------
      /// \brief	Clamps
      //--------------------------------------------------------------
      std::vector<boost::shared_ptr<equipments::subdevices::CClamp> > m_ClampList;

      //--------------------------------------------------------------
      /// \brief	Analog Values
      //--------------------------------------------------------------
      std::vector<boost::shared_ptr<specificHistorizers::CAnalog> > m_AnalogList;
   };
} // namespace equipments