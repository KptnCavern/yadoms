#include "stdafx.h"
#include "EnOcean.h"
#include <plugin_cpp_api/ImplementationHelper.h>
#include "EnOceanFactory.h"
#include <shared/communication/PortException.hpp>
#include "ProtocolException.hpp"
#include "profiles/generated-manufacturers.h"
#include "4BSTeachinVariant2.h"
#include "profiles/bitsetHelpers.hpp"
#include <shared/exception/EmptyResult.hpp>
#include "ProfileHelper.h"


IMPLEMENT_PLUGIN(CEnOcean)


// Event IDs
enum
{
   //TODO faire le m�nage dans les enum
   kEvtPortConnection = yApi::IYPluginApi::kPluginFirstEventId, // Always start from yApi::IYPluginApi::kPluginFirstEventId
   kEvtPortDataReceived,
   kProtocolErrorRetryTimer,
   kAnswerTimeout,
};


CEnOcean::CEnOcean()
   : m_sentCommand(message::CO_WR_SLEEP)
{
}

CEnOcean::~CEnOcean()
{
}

void CEnOcean::doWork(boost::shared_ptr<yApi::IYPluginApi> api)
{
   m_api = api;

   m_api->setPluginState(yApi::historization::EPluginState::kCustom, "connecting");

   std::cout << "EnOcean is starting..." << std::endl;

   // Load configuration values (provided by database)
   m_configuration.initializeWith(m_api->getConfiguration());

   // Load known devices
   loadAllDevices();

   // Create the connection
   createConnection();

   // the main loop
   while (true)
   {
      // Wait for an event
      switch (m_api->getEventHandler().waitForEvents())
      {
      case yApi::IYPluginApi::kEventStopRequested:
         {
            std::cout << "Stop requested" << std::endl;
            m_api->setPluginState(yApi::historization::EPluginState::kStopped);
            return;
         }

      case yApi::IYPluginApi::kEventUpdateConfiguration:
         {
            // Configuration was updated
            m_api->setPluginState(yApi::historization::EPluginState::kCustom, "updateConfiguration");
            auto newConfigurationData = m_api->getEventHandler().getEventData<shared::CDataContainer>();
            std::cout << "Update configuration..." << std::endl;
            BOOST_ASSERT(!newConfigurationData.empty()); // newConfigurationData shouldn't be empty, or kEventUpdateConfiguration shouldn't be generated

            // Close connection
            CEnOceanConfiguration newConfiguration;
            newConfiguration.initializeWith(newConfigurationData);

            // If port has changed, destroy and recreate connection (if any)
            auto needToReconnect = !connectionsAreEqual(m_configuration, newConfiguration) && !!m_port;

            if (needToReconnect)
               destroyConnection();

            // Update configuration
            m_configuration.initializeWith(newConfigurationData);

            if (needToReconnect)
               createConnection();
            else
               m_api->setPluginState(yApi::historization::EPluginState::kRunning);

            break;
         }

      case yApi::IYPluginApi::kEventDeviceCommand:
         {
            // Command received from Yadoms
            auto command(m_api->getEventHandler().getEventData<boost::shared_ptr<const yApi::IDeviceCommand>>());
            std::cout << "Command received : " << yApi::IDeviceCommand::toString(command) << std::endl;

            processDeviceCommand(command);

            break;
         }

      case yApi::IYPluginApi::kEventManuallyDeviceCreation:
         {
            auto creation(api->getEventHandler().getEventData<boost::shared_ptr<yApi::IManuallyDeviceCreationRequest>>());
            try
            {
               creation->sendSuccess(processManuallyDeviceCreation(creation));
            }
            catch (std::exception& ex)
            {
               std::cerr << "Unable to create device : " << ex.what() << std::endl;
               creation->sendError(ex.what());
            }
            break;
         }

      case yApi::IYPluginApi::kEventDeviceRemoved:
         {
            auto device = api->getEventHandler().getEventData<boost::shared_ptr<const yApi::IDeviceRemoved>>();
            std::cout << device->device() << " was removed" << std::endl;
            processDeviceRemmoved(device);
            break;
         }

      case yApi::IYPluginApi::kSetDeviceConfiguration:
         {
            // Yadoms sent the new device configuration. Plugin must apply this configuration to device.
            auto deviceConfiguration = api->getEventHandler().getEventData<boost::shared_ptr<const yApi::ISetDeviceConfiguration>>();
            processDeviceConfiguration(deviceConfiguration);
            break;
         }

      case kEvtPortConnection:
         {
            if (m_api->getEventHandler().getEventData<bool>())
               processConnectionEvent();
            else
               processUnConnectionEvent();

            break;
         }
      case kEvtPortDataReceived:
         {
            const auto message(m_api->getEventHandler().getEventData<const message::CReceivedEsp3Packet>());
            processDataReceived(message);
            break;
         }
      case kAnswerTimeout:
         {
            std::cout << "No answer received from EnOcean dongle (timeout)" << std::endl;
            protocolErrorProcess();
            break;
         }
      case kProtocolErrorRetryTimer:
         {
            createConnection();
            break;
         }
      default:
         {
            std::cerr << "Unknown message id" << std::endl;
            break;
         }
      }
   }
}

void CEnOcean::loadAllDevices()
{
   auto devices = m_api->getAllDevices();
   for (const auto& deviceId : devices)
   {
      try
      {
         auto deviceConfiguration = m_api->getDeviceConfiguration(deviceId);
         if (deviceId == "pluginState" || deviceConfiguration.empty())
            continue; // Not configured device

         CProfileHelper profileHelper(deviceConfiguration.get<std::string>("profile.activeSection"));
         auto device = createDevice(deviceId,
                                    profileHelper);

         m_devices[deviceId] = device;
      }
      catch (shared::exception::CEmptyResult&)
      {
         // Don't add a not configured device to the m_devices list
      }
      catch (std::exception& e)
      {
         // Don't add a wrong configured device to the m_devices list
         std::cerr << "Error loading device from database : device " << deviceId << " is malformed (" << e.what() << "), will be ignored" << std::endl;
      }
   }
}

void CEnOcean::createConnection()
{
   m_api->setPluginState(yApi::historization::EPluginState::kCustom, "connecting");

   // Create the port instance
   m_port = CEnOceanFactory::constructPort(m_configuration,
                                           m_api->getEventHandler(),
                                           kEvtPortConnection,
                                           kEvtPortDataReceived);
   m_port->start();
}

void CEnOcean::destroyConnection()
{
   m_port.reset();
}

bool CEnOcean::connectionsAreEqual(const CEnOceanConfiguration& conf1,
                                   const CEnOceanConfiguration& conf2)
{
   return (conf1.getSerialPort() == conf2.getSerialPort());
}

boost::shared_ptr<IType> CEnOcean::createDevice(const std::string& deviceId,
                                                const CProfileHelper& profileHelper) const
{
   return CRorgs::createRorg(profileHelper.rorg())->createFunc(profileHelper.func())->createType(profileHelper.type(),
                                                                                                 deviceId,
                                                                                                 m_api);
}

std::string CEnOcean::generateModel(const std::string& model,
                                    const std::string& manufacturer,
                                    const CProfileHelper& profile)
{
   if (!model.empty())
      return model;

   std::string generatedModel;

   if (!manufacturer.empty())
      generatedModel += manufacturer + std::string(" - ");

   generatedModel += createDevice(std::string(), profile)->title();

   return generatedModel;
}

void CEnOcean::processDeviceCommand(boost::shared_ptr<const shared::plugin::yPluginApi::IDeviceCommand> command)
{
   if (!m_port)
   {
      std::cout << "Unable to process command : dongle is not ready" << std::endl;
      return;
   }

   if (m_devices.find(command->getDevice()) == m_devices.end())
   {
      std::cerr << "Unable to process command : device " << command->getDevice() << " unknown" << std::endl;
      return;
   }

   m_devices[command->getDevice()]->sendCommand(command->getKeyword(),
                                                command->getBody());
}


void CEnOcean::send(const message::CSendMessage& sendMessage) const
{
   if (!m_port)
   {
      std::cerr << "Send message failed : dongle is not ready" << std::endl;
      return;
   }

   m_port->send(sendMessage.buildBuffer());
}

void CEnOcean::processConnectionEvent()
{
   std::cout << "EnOcean port opened" << std::endl;

   try
   {
      requestDongleVersion();
      //TODO
   }
   catch (CProtocolException& e)
   {
      std::cerr << "Protocol error : " << e.what();
      protocolErrorProcess();
   }
   catch (shared::communication::CPortException& e)
   {
      std::cerr << "Error connecting to EnOcean dongle : " << e.what();
      // Disconnection will be notified, we just have to wait...
   }
}

void CEnOcean::protocolErrorProcess()
{
   // Retry full connection
   processUnConnectionEvent();
   m_api->getEventHandler().createTimer(kProtocolErrorRetryTimer, shared::event::CEventTimer::kOneShot, boost::posix_time::seconds(30));
}

void CEnOcean::processUnConnectionEvent()
{
   std::cout << "EnOcean connection was lost" << std::endl;
   m_api->setPluginState(yApi::historization::EPluginState::kError, "connectionFailed");

   destroyConnection();
}

std::string CEnOcean::processManuallyDeviceCreation(boost::shared_ptr<const shared::plugin::yPluginApi::IManuallyDeviceCreationRequest> creation)
{
   auto configuration = creation->getData().getConfiguration();
   auto deviceId = configuration.get<std::string>("id");
   auto selectedProfile = CProfileHelper(configuration.get<std::string>("profile.activeSection"));

   // Declare the device
   auto device = declareDevice(deviceId,
                               selectedProfile,
                               configuration.get<std::string>("manufacturer"),
                               configuration.get<std::string>("model"));

   // Send configuration to device
   try
   {
      device->sendConfiguration(configuration.get<shared::CDataContainer>("profile.content." + selectedProfile.profile() + ".content"));
   }
   catch (std::exception& e)
   {
      throw std::runtime_error(std::string("Unable to configure device : ") + e.what());
   }

   return deviceId;
}

void CEnOcean::processDeviceRemmoved(boost::shared_ptr<const shared::plugin::yPluginApi::IDeviceRemoved> deviceRemoved)
{
   if (m_devices.find(deviceRemoved->device()) == m_devices.end())
      return;

   m_devices.erase(deviceRemoved->device());
}

void CEnOcean::processDeviceConfiguration(boost::shared_ptr<const yApi::ISetDeviceConfiguration> setDeviceConfigurationData)
{
   auto deviceId = setDeviceConfigurationData->device();
   auto configuration = setDeviceConfigurationData->configuration();

   try
   {
      auto selectedProfile = CProfileHelper(configuration.get<std::string>("profile.activeSection"));
      auto manufacturer = configuration.get<std::string>("manufacturer");
      auto model = generateModel(configuration.get<std::string>("model"),
                                 manufacturer,
                                 selectedProfile);

      std::cout << "Device \"" << deviceId << "\" is configurated as " << selectedProfile.profile() << std::endl;

      if (m_devices.find(deviceId) == m_devices.end() || m_devices[deviceId]->profile() != selectedProfile.profile())
      {
         // Device was not configured or profile changed, recreate local device and all keywords
         if (m_devices.find(deviceId) != m_devices.end())
            for (const auto& keyword : m_api->getAllKeywords(deviceId))
               m_api->removeKeyword(deviceId, keyword);

         // Don't recreate device in Yadoms, unless it will change of ID
         auto device = createDevice(deviceId,
                                    selectedProfile);
         m_api->declareKeywords(deviceId,
                                device->allHistorizers());
         m_devices[deviceId] = device;
      }

      // Send configuration to device
      try
      {
         m_devices[deviceId]->sendConfiguration(configuration.get<shared::CDataContainer>("profile.content." + m_devices[deviceId]->profile() + ".content"));
      }
      catch (std::exception& e)
      {
         throw std::runtime_error(std::string("Unable to configure device : ") + e.what());
      }
   }
   catch (shared::exception::CEmptyResult&)
   {
      std::cerr << "Unable to configure device : unknown device \"" << deviceId << "\"" << std::endl;
   }
   catch (std::exception& e)
   {
      std::cerr << "Unable to configure device : " << e.what() << std::endl;
   }
   //TODO ajouter un catch si la regex ne se passe pas bien
}

void CEnOcean::processDataReceived(const message::CReceivedEsp3Packet& message)
{
   try
   {
      switch (message.header().packetType())
      {
      case message::RADIO_ERP1:
         processRadioErp1(message);
         break;//TODO
      case message::RESPONSE:
         processResponse(message);
         break;
      case message::RADIO_SUB_TEL: break;//TODO
      case message::EVENT:
         processEvent(message);
         break;//TODO
      case message::COMMON_COMMAND: break;//TODO
      case message::SMART_ACK_COMMAND: break;//TODO
      case message::REMOTE_MAN_COMMAND: break;//TODO
      case message::RADIO_MESSAGE: break;//TODO
      case message::RADIO_ERP2: break;//TODO
      case message::RADIO_802_15_4: break;//TODO
      case message::COMMAND_2_4: break;//TODO
      case message::MESSAGES: break;//TODO
      default:
         throw CProtocolException((boost::format("Unknown packet type %1%") % message.header().packetType()).str());
      }
   }
   catch (CProtocolException& e)
   {
      std::cerr << "Error processing received message : " << e.what() << std::endl;
   }
   catch (std::exception& e)
   {
      std::cerr << "Error processing received message : " << e.what() << std::endl;
   }
}


void CEnOcean::processRadioErp1(const message::CReceivedEsp3Packet& esp3Packet)
{
   message::CRadioErp1Message erp1Message(esp3Packet);

   // Create associated RORG object
   auto erp1Data = bitset_from_bytes(erp1Message.data());
   auto rorg = CRorgs::createRorg(erp1Message.rorg());
   auto deviceId = erp1Message.senderIdAsString();

   if (rorg->isTeachIn(erp1Data))
   {
      // Teachin telegram

      if (m_devices.find(deviceId) != m_devices.end())
      {
         std::cout << "Device " << deviceId << " already exists, teachin message ignored." << std::endl;
         return;
      }

      if (!rorg->isEepProvided(erp1Data))
      {
         if (m_api->deviceExists(deviceId))
         {
            std::cout << "Device " << deviceId << " already declared, teachin message ignored." << std::endl;
            return;
         }

         declareDeviceWithoutProfile(deviceId);
         return;
      }

      if (rorg->id() != CRorgs::k4BS_Telegram)
         throw std::domain_error((boost::format("Teach-in telegram is only supported for 4BS telegram for now. Please report to Yadoms-team. Telegram \"%1%\"") % erp1Message.dump()).str());

      // Special-case of 4BS teachin mode Variant 2 (profile is provided in the telegram)
      C4BSTeachinVariant2 teachInData(erp1Data);

      try
      {
         declareDevice(deviceId,
                       CProfileHelper(rorg->id(),
                                      teachInData.funcId(),
                                      teachInData.typeId()),
                       CManufacturers::name(teachInData.manufacturerId()));
      }
      catch (std::exception& e)
      {
         std::cerr << "Unable to declare device : " << e.what() << std::endl;
      }
   }
   else
   {
      // Data telegram

      // Declare device if unknown
      if (m_devices.find(deviceId) == m_devices.end())
      {
         if (m_api->deviceExists(deviceId))
         {
            std::cout << "Device " << deviceId << " already declared but not configured, message ignored." << std::endl;
            return;
         }

         declareDeviceWithoutProfile(deviceId);
         return;
      }

      auto device = m_devices[deviceId];

      auto keywordsToHistorize = device->states(erp1Data);
      if (keywordsToHistorize.empty())
      {
         std::cout << "Received message for id#" << deviceId << ", but nothing to historize" << std::endl;
         return;
      }

      std::cout << "Received message for id#" << deviceId << " : " << std::endl;
      for (const auto& kw: keywordsToHistorize)
         std::cout << "  - " << kw->getKeyword() << " = " << kw->formatValue() << std::endl;;

      m_api->historizeData(deviceId, keywordsToHistorize);
   }
}

void CEnOcean::declareDeviceWithoutProfile(const std::string& deviceId) const
{
   std::cout << "New device declared : " << std::endl;
   std::cout << "  - Id           : " << deviceId << std::endl;
   std::cout << "  - Profile      : Unknown. No historization until user enter profile." << std::endl;

   m_api->declareDevice(deviceId,
                        std::string(),
                        std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>>());
}

std::string CEnOcean::extractSenderId(const std::vector<unsigned char>& data,
                                      int startIndex)
{
   return std::to_string(
      (data[startIndex + 3] << 24) +
      (data[startIndex + 2] << 16) +
      (data[startIndex + 1] << 8) +
      (data[startIndex]));
}

void CEnOcean::processResponse(const message::CReceivedEsp3Packet& esp3Packet) const
{
   switch (m_sentCommand)
   {
   case message::CO_RD_VERSION: processDongleVersionResponse(esp3Packet);
      break;
   default:
      std::cout << "Receive response not supported (last command sent was " << m_sentCommand << ")" << std::endl;
   }
}

void CEnOcean::processDongleVersionResponse(const message::CReceivedEsp3Packet& esp3Packet) const
{
   if (esp3Packet.header().dataLength() != 33) //TODO on peut pas mieux faire que cette valeur en dur ?
      throw CProtocolException((boost::format("Invalid data length %1%, expected 33. Request was CO_RD_VERSION.") % esp3Packet.header().dataLength()).str());

   auto returnCode = static_cast<message::EReturnCode>(esp3Packet.data()[0]);

   if (returnCode == message::RET_NOT_SUPPORTED)
   {
      std::cout << "CO_RD_VERSION request returned not supported" << std::endl;
      return;
   }

   if (returnCode != message::RET_OK)
      throw CProtocolException((boost::format("Unexpected return code %1%. Request was CO_RD_VERSION.") % returnCode).str());

   struct Version
   {
      unsigned int m_main;
      unsigned int m_beta;
      unsigned int m_alpha;
      unsigned int m_build;

      std::string toString() const
      {
         std::ostringstream str;
         str << m_main << '.' << m_beta << '.' << m_alpha << '.' << m_build;
         return str.str();
      }
   };
   Version appVersion;
   appVersion.m_main = esp3Packet.data()[1];
   appVersion.m_beta = esp3Packet.data()[2];
   appVersion.m_alpha = esp3Packet.data()[3];
   appVersion.m_build = esp3Packet.data()[4];

   Version apiVersion;
   apiVersion.m_main = esp3Packet.data()[5];
   apiVersion.m_beta = esp3Packet.data()[6];
   apiVersion.m_alpha = esp3Packet.data()[7];
   apiVersion.m_build = esp3Packet.data()[8];

   auto chipId = esp3Packet.data()[9] << 24
      | esp3Packet.data()[10] << 16
      | esp3Packet.data()[11] << 8
      | esp3Packet.data()[12];

   auto chipVersion = esp3Packet.data()[13] << 24
      | esp3Packet.data()[14] << 16
      | esp3Packet.data()[15] << 8
      | esp3Packet.data()[16];

   std::string appDescription(esp3Packet.data().begin() + 17,
                              esp3Packet.data().end());
   appDescription.erase(appDescription.find_last_not_of('\0') + 1);

   std::cout << "EnOcean dongle Version " << appVersion.toString() <<
      " " << appDescription << "" <<
      ", api " << apiVersion.toString() <<
      ", chipId " << std::hex << chipId <<
      ", chipVersion " << std::hex << chipVersion <<
      std::endl;

   m_api->setPluginState(yApi::historization::EPluginState::kRunning);
}

void CEnOcean::processEvent(const message::CReceivedEsp3Packet& esp3Packet)
{
   //TODO tout revoir pour utiliser le code g�n�r� (si dispo pour les events)
   if (esp3Packet.header().dataLength() < 1)
      throw CProtocolException((boost::format("RadioERP1 message : wrong data size (%1%, < 1)") % esp3Packet.header().dataLength()).str());

   enum
      {
         SA_RECLAIM_NOT_SUCCESSFUL = 0x01,
         SA_CONFIRM_LEARN = 0x02,
         SA_LEARN_ACK = 0x03,
         CO_READY = 0x04,
         CO_EVENT_SECUREDEVICES = 0x05,
         CO_DUTYCYCLE_LIMIT = 0x06,
         CO_TRANSMIT_FAILED = 0x07
      };

   auto eventCode = esp3Packet.data()[0];

   switch (eventCode)
   {
   case SA_RECLAIM_NOT_SUCCESSFUL: break; //TODO
   case SA_CONFIRM_LEARN: break; //TODO
   case SA_LEARN_ACK: break; //TODO
   case CO_READY: break; //TODO
   case CO_EVENT_SECUREDEVICES: break; //TODO
   case CO_DUTYCYCLE_LIMIT: break; //TODO
   case CO_TRANSMIT_FAILED: break; //TODO
   default:
      throw CProtocolException((boost::format("Unknown event code %1%") % eventCode).str());
   }

   //TODO
}

boost::shared_ptr<IType> CEnOcean::declareDevice(const std::string& deviceId,
                                                 const CProfileHelper& profile,
                                                 const std::string& manufacturer,
                                                 const std::string& model)
{
   auto device = createDevice(deviceId,
                              profile);

   auto modelLabel = generateModel(model,
                                   manufacturer,
                                   profile);

   auto keywordsToDeclare = device->allHistorizers();
   if (keywordsToDeclare.empty())
   {
      std::stringstream s;
      s << "Can not declare device id#" << deviceId
         << " (" << profile.profile()
         << ") : no keyword to declare" << std::endl;
      throw std::logic_error(s.str());
   }

   if (m_devices.find(deviceId) != m_devices.end())
      throw std::logic_error("Device " + deviceId + " already exist");

   m_api->declareDevice(deviceId,
                        modelLabel,
                        keywordsToDeclare);

   std::cout << "New device declared : " << std::endl;
   std::cout << "  - Id           : " << deviceId << std::endl;
   std::cout << "  - Profile      : " << profile.profile() << std::endl;
   std::cout << "  - Manufacturer : " << manufacturer << std::endl;
   std::cout << "  - Model        : " << modelLabel << std::endl;
   std::cout << "  - RORG         : " << CRorgs::createRorg(profile.rorg())->title() << std::endl;
   std::cout << "  - FUNC         : " << CRorgs::createRorg(profile.rorg())->createFunc(profile.func())->title() << std::endl;
   std::cout << "  - TYPE         : " << device->title() << std::endl;

   m_devices[deviceId] = device;
   return device;
}

void CEnOcean::requestDongleVersion()
{
   message::CCommandSendMessage sendMessage;
   sendMessage.appendData({message::CO_RD_VERSION});
   m_sentCommand = message::CO_RD_VERSION;

   send(sendMessage);
}
