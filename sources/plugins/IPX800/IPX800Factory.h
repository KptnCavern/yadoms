#pragma once
#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include <shared/event/EventHandler.hpp>
#include "IIPX800Configuration.h"
#include "IOManager.h"
#include "specificHistorizers/Analog.h"
#include <shared/plugin/yPluginApi/IManuallyDeviceCreationRequest.h>
#include "specificHistorizers/inputOutput.h"
#include "specificHistorizers/counter.h"

// Shortcut to yPluginApi namespace
namespace yApi = shared::plugin::yPluginApi;

//--------------------------------------------------------------
/// \brief	Class where all Keywords are created and updated
/// \note   
//--------------------------------------------------------------
class CIPX800Factory
{
public:
   //--------------------------------------------------------------
   /// \brief	    Constructor
   /// \param[in] api                 yPluginApi API
   /// \param[in] device              The device name
   //--------------------------------------------------------------
   CIPX800Factory(boost::shared_ptr<yApi::IYPluginApi> api,
                  const std::string& device,
                  const IIPX800Configuration& configuration);

   //--------------------------------------------------------------
   /// \brief	    Destructor
   //--------------------------------------------------------------
   virtual ~CIPX800Factory();

   //--------------------------------------------------------------
   /// \brief	    OnConfigurationUpdate
   /// \param[in] api                  yPluginApi API
   /// \param[in] IIPX800Configuration configuration
   //--------------------------------------------------------------
   void OnConfigurationUpdate(boost::shared_ptr<yApi::IYPluginApi> api,
                              const IIPX800Configuration& configuration);

   //--------------------------------------------------------------
   /// \brief	    bindSlotsX8R
   //--------------------------------------------------------------
   shared::CDataContainer bindSlotsX8R();

   //--------------------------------------------------------------
   /// \brief	    bindSlotsX8D
   //--------------------------------------------------------------
   shared::CDataContainer bindSlotsX8D();

   //--------------------------------------------------------------
   /// \brief	    bindSlotsX24D
   //--------------------------------------------------------------
   shared::CDataContainer bindSlotsX24D();

   //--------------------------------------------------------------
   /// \brief	    createDeviceManually
   /// \param[in] api                  yPluginApi API
   //--------------------------------------------------------------
   std::string createDeviceManually(boost::shared_ptr<yApi::IYPluginApi> api, 
                                    const yApi::IManuallyDeviceCreationData& data);

   //--------------------------------------------------------------
   /// \brief	    getIOManager
   /// \return     the IOManager
   //--------------------------------------------------------------
   boost::shared_ptr<CIOManager> getIOManager(void);

   //--------------------------------------------------------------
   /// \brief	                     Process a command received from Yadoms
   /// \param [in] api              Plugin execution context (Yadoms API)
   /// \param [in] receivedValue    The received value from the interrupt
   //--------------------------------------------------------------
   void removeDevice(boost::shared_ptr<yApi::IYPluginApi> api, std::string deviceRemoved);

private:

   //--------------------------------------------------------------
   /// \brief	Manager for all IOs
   //--------------------------------------------------------------
   boost::shared_ptr<CIOManager> m_ioManager;

   //--------------------------------------------------------------
   /// \brief	X8R Slots used
   //--------------------------------------------------------------
   bool X8RSlotused[6];

   //--------------------------------------------------------------
   /// \brief	X8D et X24D Slots used
   //--------------------------------------------------------------
   bool X8DSlotused[6];
   
   //--------------------------------------------------------------
   /// \brief	All extensions
   //--------------------------------------------------------------
   std::vector<boost::shared_ptr<equipments::IEquipment> > m_devicesList;
};