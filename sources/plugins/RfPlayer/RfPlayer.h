#pragma once
#include <plugin_cpp_api/IPlugin.h>
#include "Configuration.h"
#include <shared/communication/IAsyncPort.h>
#include "Transceiver.h"
#include "IMessageHandler.h"
#include "Dongle.h"
#include <shared/communication/AsyncPortConnectionNotification.h>

// Shortcut to yPluginApi namespace
namespace yApi = shared::plugin::yPluginApi;

//--------------------------------------------------------------
/// \brief	This class is the ZWave plugin entry point
//--------------------------------------------------------------
class CRfPlayer : public plugin_cpp_api::IPlugin
{
public:
   //--------------------------------------------------------------
   /// \brief	Constructor
   //--------------------------------------------------------------
   CRfPlayer();

   //--------------------------------------------------------------
   /// \brief	Destructor
   //--------------------------------------------------------------
   virtual ~CRfPlayer();

   // IPlugin implementation
   void doWork(boost::shared_ptr<yApi::IYPluginApi> api) override;
   // [END] IPlugin implementation

private:
   //--------------------------------------------------------------
   /// \brief	The plugin configuration
   //--------------------------------------------------------------
   CConfiguration m_configuration;

   //--------------------------------------------------------------
   /// \brief  The communication port
   //--------------------------------------------------------------
   boost::shared_ptr<shared::communication::IAsyncPort> m_port;

   //--------------------------------------------------------------
   /// \brief  The ziblue transceiver
   //--------------------------------------------------------------
   boost::shared_ptr<CTransceiver> m_transceiver;

   //--------------------------------------------------------------
   /// \brief  The message handler
   //--------------------------------------------------------------
   boost::shared_ptr<IMessageHandler> m_messageHandler;

   //--------------------------------------------------------------
   /// \brief  The Ziblue dongle
   //--------------------------------------------------------------
   boost::shared_ptr<CDongle> m_dongle;

   //--------------------------------------------------------------
   /// \brief  Developer mode
   //--------------------------------------------------------------
   bool m_isDeveloperMode;

   //--------------------------------------------------------------
   /// \brief  Retry timer after error
   //--------------------------------------------------------------
   boost::shared_ptr<shared::event::CEventTimer> m_protocolErrorRetryTimer;


   //--------------------------------------------------------------
   /// \brief	                     Create the connection to the RfPlayer RFP1000
   /// \param [in] eventHandler     Event handler to be notified on events on the connection
   //--------------------------------------------------------------
   void createConnection(shared::event::CEventHandler& eventHandler);

   //--------------------------------------------------------------
   /// \brief	                     Destroy connection
   //--------------------------------------------------------------
   void destroyConnection();

   //--------------------------------------------------------------
   /// \brief	                     Check if connections are the same between the 2 configurations
   /// \param [in] conf1            First configuration to compare
   /// \param [in] conf2            Second configuration to compare
   /// \return                      true is connection data are all the same in the both configurations
   //--------------------------------------------------------------
   static bool connectionsAreEqual(const CConfiguration& conf1, const CConfiguration& conf2);

   void processZiBlueConnectionEvent(boost::shared_ptr<yApi::IYPluginApi> api);
   void errorProcess(boost::shared_ptr<yApi::IYPluginApi> api);
   static void processZiBlueUnConnectionEvent(boost::shared_ptr<yApi::IYPluginApi> api, boost::shared_ptr<shared::communication::CAsyncPortConnectionNotification> notification);
   void processZiBlueBinaryFrameReceived(boost::shared_ptr<yApi::IYPluginApi> api, boost::shared_ptr<frames::incoming::CBinaryFrame> data) const;
   void processZiBlueAsciiFrameReceived(boost::shared_ptr<yApi::IYPluginApi> api, boost::shared_ptr<frames::incoming::CAsciiFrame> data) const;
   void initZiBlue(boost::shared_ptr<yApi::IYPluginApi> api);
   void updateDongleConfiguration(boost::shared_ptr<yApi::IYPluginApi> api);
   void processFirmwareUpdate(boost::shared_ptr<yApi::IYPluginApi> & api, boost::shared_ptr<yApi::IExtraQuery> & extraQuery);
};



