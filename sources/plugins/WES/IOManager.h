#pragma once
#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include <shared/event/EventHandler.hpp>
#include "equipments/IEquipment.h"
#include "IWESConfiguration.h"

// Shortcut to yPluginApi namespace
namespace yApi = shared::plugin::yPluginApi;

//--------------------------------------------------------------
/// \brief	Class where all keywords are updated
/// \note   
//--------------------------------------------------------------
class CIOManager
{
public:
   //--------------------------------------------------------------
   /// \brief	   Constructor
   /// \param[in] device            The device name
   /// \param[in] socket            The IP Address with the socket number of the WES
   /// \param[in] passwordActivated If the password is used
   /// \param[in] password          password used to access the equipment
   //--------------------------------------------------------------
   explicit CIOManager(const std::string device, 
                       Poco::Net::SocketAddress socket,
                       bool passwordActivated, 
                       std::string password);

   //--------------------------------------------------------------
   /// \brief	    Initialize all elements
   /// \param[in]  extensionList   Set a new list of IOs
   //--------------------------------------------------------------
   void Initialize(std::vector<boost::shared_ptr<equipments::IEquipment> >& extensionList);

   //--------------------------------------------------------------
   /// \brief	    Destructor
   //--------------------------------------------------------------
   virtual ~CIOManager();

   //--------------------------------------------------------------
   /// \brief	                     Process a command received from Yadoms
   /// \param [in] api              Plugin execution context (Yadoms API)
   /// \param [in] command          The received command
   //--------------------------------------------------------------
   void onCommand(boost::shared_ptr<yApi::IYPluginApi> api,
                  boost::shared_ptr<const yApi::IDeviceCommand> command);

   //--------------------------------------------------------------
   /// \brief	                     Process a command received from Yadoms
   /// \param [in] api                   Plugin execution context (Yadoms API)
   /// \param [in] receivedValue         The received value from the interrupt
   /// \param [in] forceHistorization    force the historization of all keywords
   //--------------------------------------------------------------
   void readIOFromDevice(boost::shared_ptr<yApi::IYPluginApi> api, 
                         const std::string& type,
                         bool forceHistorization = false);

   //--------------------------------------------------------------
   /// \brief	                     Process a command received from Yadoms
   /// \param [in] api                   Plugin execution context (Yadoms API)
   /// \param [in] forceHistorization    force the historization of all keywords
   //--------------------------------------------------------------
   void readAllIOFromDevice(boost::shared_ptr<yApi::IYPluginApi> api, bool forceHistorization = false);

   //--------------------------------------------------------------
   /// \brief	                     Process a command received from Yadoms
   /// \param [in] api              Plugin execution context (Yadoms API)
   /// \param [in] deviceRemoved    the name of the device removed
   //--------------------------------------------------------------
   void removeDevice(boost::shared_ptr<yApi::IYPluginApi> api, std::string deviceRemoved);

   //--------------------------------------------------------------
   /// \brief	    OnConfigurationUpdate
   /// \param[in] api                  yPluginApi API
   /// \param[in] configuration        the new plugin configuration
   //--------------------------------------------------------------
   void OnConfigurationUpdate(boost::shared_ptr<yApi::IYPluginApi> api,
                              const IWESConfiguration& configuration);

private:

   //--------------------------------------------------------------
   /// \brief	The plugin name
   //--------------------------------------------------------------
   std::string m_deviceName;

   //--------------------------------------------------------------
   /// \brief	socket and address used to access the equipment
   //--------------------------------------------------------------
   Poco::Net::SocketAddress m_socketAddress;

   //--------------------------------------------------------------
   /// \brief	Password activation
   //--------------------------------------------------------------
   bool m_isPasswordActivated;

   //--------------------------------------------------------------
   /// \brief	Password
   //--------------------------------------------------------------
   std::string m_password;

   //--------------------------------------------------------------
   /// \brief	All extensions
   //--------------------------------------------------------------
   std::vector<boost::shared_ptr<equipments::IEquipment> > m_devicesList;
};