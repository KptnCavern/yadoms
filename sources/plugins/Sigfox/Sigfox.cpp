#include "stdafx.h"
#include "Sigfox.h"
#include <shared/event/EventTimer.h>
#include <plugin_cpp_api/ImplementationHelper.h>
#include <shared/plugin/yPluginApi/IExtraQuery.h>
#include <shared/Log.h>

// Use this macro to define all necessary to make your DLL a Yadoms valid plugin.
// Note that you have to provide some extra files, like package.json, and icon.png
// This macro also defines the static PluginInformations value that can be used by plugin to get information values

IMPLEMENT_PLUGIN(CSigfox)

CSigfox::CSigfox() :
   m_messageKeyword(boost::make_shared<yApi::historization::CText>("message",
                                                                   yApi::EKeywordAccessMode::kGetSet)),
   m_rssi(boost::make_shared<yApi::historization::CRssi>("rssi")),
   m_batteryLevel(boost::make_shared<yApi::historization::CBatteryLevel>("battery")),
   m_snr(boost::make_shared<specificHistorizers::CSNR>("snr")),
   m_keywordsData({ m_messageKeyword, m_rssi, m_snr }),
   m_keywordsService({ m_rssi, m_batteryLevel, m_snr })
{}

CSigfox::~CSigfox()
{}

// Event IDs
enum
{
   kEvtTimerRefreshCPULoad = yApi::IYPluginApi::kPluginFirstEventId, // Always start from shared::event::CEventHandler::kUserFirstId
   kRefreshStatesReceived,
   kConnectionRetryTimer,
   kAnswerTimeout
};

void CSigfox::doWork(boost::shared_ptr<yApi::IYPluginApi> api)
{
   YADOMS_LOG(information) << "Sigfox is starting...";
      
   try {

      m_isDeveloperMode = api->getYadomsInformation()->developperMode();

      api->setPluginState(yApi::historization::EPluginState::kRunning);
      YADOMS_LOG(information) << "Sigfox plugin is running..." ;
   }
   catch (...)
   {
      api->setPluginState(yApi::historization::EPluginState::kCustom, "initializationError");
      YADOMS_LOG(error) << "Sigfox plugin initialization error..." ;
   }

   while (true)
   {
      // Wait for an event
      switch (api->getEventHandler().waitForEvents())
      {
      case yApi::IYPluginApi::kEventStopRequested:
         {
            YADOMS_LOG(information) << "Stop requested" ;
            api->setPluginState(yApi::historization::EPluginState::kStopped);
            return;
         }
      case yApi::IYPluginApi::kEventExtraQuery:
      {
         // Command was received from Yadoms
         auto extraQuery = api->getEventHandler().getEventData<boost::shared_ptr<yApi::IExtraQuery> >();
         if (extraQuery)
         {
            if (extraQuery->getData().query() == "sigfox")
            {
               processIncomingMessage(api, extraQuery->getData().data());
            }
         }
         extraQuery->sendSuccess(shared::CDataContainer());
         break;
      }
      case yApi::IYPluginApi::kEventDeviceCommand:
      {
         // Command received from Yadoms
         break;
      }
      default:
         {
            YADOMS_LOG(error) << "Unknown message id" ;
            break;
         }
      }
   }
}

void CSigfox::processIncomingMessage(boost::shared_ptr<yApi::IYPluginApi> api, const shared::CDataContainer& newMessage) const
{
   if (m_isDeveloperMode)
      newMessage.printToLog(YADOMS_LOG(information));

   try {
      std::string deviceName = newMessage.get<std::string>("device");
      std::string type = newMessage.get<std::string>("type");

      declareDevice(api, deviceName);

      YADOMS_LOG(information) << "Message received for device " << deviceName;


      // Configuration sur le site https://backend.sigfox.com/

      // json string data format

      //{
      //   "type" : "data",
      //   "device" : "{device},
      //   "data" : "{data}"
      //}

      // json string service format

      //{
      //   "type" : "service",
      //   "device" : "{device},
      //   "rssi" : "{rssi}",
      //   "snr"  : "{snr"},
      //   "battery": "{batt}"
      //}

      if (type.compare("data") == 0)
      {
         m_messageKeyword->set(newMessage.get<std::string>("data"));
         m_rssi->set(static_cast<int>(newMessage.get<double>("rssi")));
         m_snr->set(newMessage.get<double>("snr"));

         api->historizeData(deviceName, m_keywordsData);
         YADOMS_LOG(information) << "historize a data message for " << deviceName;
      }

      if (type.compare("service") == 0)
      {
         m_rssi->set(newMessage.get<int>("rssi"));
         m_batteryLevel->set(newMessage.get<int>("battery"));
         m_snr->set(newMessage.get<double>("snr"));

         api->historizeData(deviceName, m_keywordsData);
         YADOMS_LOG(information) << "historize a service message for " << deviceName;
      }
   }
   catch (std::exception &e)
   {
      YADOMS_LOG(error) << "Error parsing the received message : " << e.what();
   }
}

void CSigfox::declareDevice(boost::shared_ptr<yApi::IYPluginApi> api, std::string deviceName) const
{
   // keywords list for declaration
   std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> > m_keywordsDeclaration({ m_messageKeyword, m_rssi, m_batteryLevel, m_snr });

   if (api->deviceExists(deviceName))
      return;

   api->declareDevice(deviceName, deviceName, m_keywordsDeclaration);
}