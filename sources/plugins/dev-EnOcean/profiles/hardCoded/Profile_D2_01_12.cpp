#include "stdafx.h"
#include "Profile_D2_01_12.h"
#include "../bitsetHelpers.hpp"
#include "../../message/RadioErp1SendMessage.h"
#include "../../message/ResponseReceivedMessage.h"

DECLARE_ENUM_IMPLEMENTATION_NESTED(CProfile_D2_01_12::EDefaultState, EDefaultState,
   ((off))
   ((on))
   ((previousState))
   ((notUsed))
);

DECLARE_ENUM_IMPLEMENTATION_NESTED(CProfile_D2_01_12::EConnectedSwitchsType, EConnectedSwitchsType,
   ((switch))
   ((pushButton))
   ((autodetection))
);

const CRorgs::ERorgIds CProfile_D2_01_12::m_rorg(CRorgs::kVLD_Telegram);

CProfile_D2_01_12::CProfile_D2_01_12(const std::string& deviceId,
                                     boost::shared_ptr<yApi::IYPluginApi> api)
   : m_deviceId(deviceId),
     m_channel1(boost::make_shared<yApi::historization::CSwitch>("Channel 1", yApi::EKeywordAccessMode::kGetSet)),
     m_channel2(boost::make_shared<yApi::historization::CSwitch>("Channel 2", yApi::EKeywordAccessMode::kGetSet)),
     m_historizers({m_channel1 , m_channel2})
{
}

CProfile_D2_01_12::~CProfile_D2_01_12()
{
}

const std::string& CProfile_D2_01_12::profile() const
{
   static const std::string profile("D2-01-12");
   return profile;
}

const std::string& CProfile_D2_01_12::title() const
{
   static const std::string title("Slot-in module with 2 channels");
   return title;
}

std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> CProfile_D2_01_12::allHistorizers() const
{
   return m_historizers;
}

std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> CProfile_D2_01_12::states(unsigned char rorg,
                                                                                                   const boost::dynamic_bitset<>& data,
                                                                                                   const boost::dynamic_bitset<>& status) const
{
   // This device supports several RORG messages
   // We just use the VLD telegram
   if (rorg != CRorgs::ERorgIds::kVLD_Telegram)
      return std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>>();

   if (bitset_extract(data, 4, 4) != kActuatorStatusResponse)
      return std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>>();

   // Return only the concerned historizer
   std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> historizers;

   auto ioChannel = bitset_extract(data, 11, 5);
   auto state = bitset_extract(data, 17, 1) ? true : false;
   switch (ioChannel)
   {
   case 0:
      m_channel1->set(state);
      historizers.push_back(m_channel1);
      break;
   case 1:
      m_channel2->set(state);
      historizers.push_back(m_channel2);
      break;
   default:
      std::cout << "Profile " << profile() << " : received unsupported ioChannel value " << ioChannel << std::endl;
      break;
   }
   return historizers;
}

void CProfile_D2_01_12::sendCommand(const std::string& keyword,
                                    const std::string& commandBody,
                                    boost::shared_ptr<IMessageHandler> messageHandler) const
{
   message::CRadioErp1SendMessage command(m_rorg,
                                          "00000000",//TODO mettre constant
                                          0);
   boost::dynamic_bitset<> data(4 * 8);

   //TOFIX : marche p�
   bitset_insert(data, 4, 4, kActuatorSetOutput);
   bitset_insert(data, 11, 5, (keyword == m_channel1->getKeyword()) ? 0 : 1);
   bitset_insert(data, 17, 7, commandBody == "true" ? 1 : 0);

   command.userData(bitset_to_bytes(data));

   boost::shared_ptr<const message::CEsp3ReceivedPacket> answer;
   if (!messageHandler->send(command,
                             [](boost::shared_ptr<const message::CEsp3ReceivedPacket> esp3Packet)
                             {
                                return esp3Packet->header().packetType() == message::RESPONSE;
                             },
                             [&](boost::shared_ptr<const message::CEsp3ReceivedPacket> esp3Packet)
                             {
                                answer = esp3Packet;
                             }))
      std::cerr << "Fail to send state to " << m_deviceId << " : no answer to Actuator Set Output command" << std::endl;

   auto response = boost::make_shared<message::CResponseReceivedMessage>(answer);

   if (response->returnCode() != message::CResponseReceivedMessage::RET_OK)
      std::cerr << "Fail to send state to " << m_deviceId << " : Actuator Set Output command returns " << response->returnCode() << std::endl;
}

void CProfile_D2_01_12::sendConfiguration(const shared::CDataContainer& deviceConfiguration,
                                          boost::shared_ptr<IMessageHandler> messageHandler) const
{
   auto localControl = deviceConfiguration.get<std::string>("localControl") == "enable";
   auto taughtInAllDevices = deviceConfiguration.get<std::string>("taughtIn") == "allDevices";
   auto userInterfaceDayMode = deviceConfiguration.get<std::string>("userInterfaceMode") == "dayMode";
   auto defaultState = deviceConfiguration.get<EDefaultState>("defaultState");
   auto connectedSwitchsType = deviceConfiguration.get<EConnectedSwitchsType>("connectedSwitchsType");
   auto autoOffTimerSeconds = deviceConfiguration.get<unsigned int>("autoOffTimer");
   auto delayRadioOffTimerSeconds = deviceConfiguration.get<unsigned int>("delayRadioOffTimer");
   auto switchingStateToggle = deviceConfiguration.get<std::string>("switchingState") == "tooggle";

   // CMD 0x2 - Actuator Set Local
   {
      message::CRadioErp1SendMessage command(m_rorg,
                                             "00000000",//TODO mettre constant
                                             0);
      boost::dynamic_bitset<> data(4 * 8);

      bitset_insert(data, 4, 4, kActuatorSetLocal);
      bitset_insert(data, 0, !taughtInAllDevices);
      bitset_insert(data, 10, localControl);
      bitset_insert(data, 11, 5, 0x1E); // Use all output channels supported by the device
      bitset_insert(data, 24, !userInterfaceDayMode);
      bitset_insert(data, 26, 2, defaultState);

      command.userData(bitset_to_bytes(data));

      boost::shared_ptr<const message::CEsp3ReceivedPacket> answer;
      if (!messageHandler->send(command,
                                [](boost::shared_ptr<const message::CEsp3ReceivedPacket> esp3Packet)
                                {
                                   return esp3Packet->header().packetType() == message::RESPONSE;
                                },
                                [&](boost::shared_ptr<const message::CEsp3ReceivedPacket> esp3Packet)
                                {
                                   answer = esp3Packet;
                                }))
         std::cerr << "Fail to send configuration to " << m_deviceId << " : no answer to Actuator Set Local command" << std::endl;

      auto response = boost::make_shared<message::CResponseReceivedMessage>(answer);

      if (response->returnCode() != message::CResponseReceivedMessage::RET_OK)
         std::cerr << "Fail to send configuration to " << m_deviceId << " : Actuator Set Local command returns " << response->returnCode() << std::endl;
   }

   // CMD 0xB - Actuator Set External Interface Settings
   {
      message::CRadioErp1SendMessage command(m_rorg,
                                             "00000000",//TODO mettre constant
                                             0);
      boost::dynamic_bitset<> data(7 * 8);

      bitset_insert(data, 4, 4, kActuatorSetExternalInterfaceSettings);
      bitset_insert(data, 11, 5, 0x1E); // Use all output channels supported by the device
      bitset_insert(data, 16, 16, autoOffTimerSeconds);
      bitset_insert(data, 32, 16, delayRadioOffTimerSeconds);
      bitset_insert(data, 48, 2, connectedSwitchsType);
      bitset_insert(data, 50, !switchingStateToggle);

      command.userData(bitset_to_bytes(data));

      boost::shared_ptr<const message::CEsp3ReceivedPacket> answer;
      if (!messageHandler->send(command,
                                [](boost::shared_ptr<const message::CEsp3ReceivedPacket> esp3Packet)
                                {
                                   return esp3Packet->header().packetType() == message::RESPONSE;
                                },
                                [&](boost::shared_ptr<const message::CEsp3ReceivedPacket> esp3Packet)
                                {
                                   answer = esp3Packet;
                                }))
         std::cerr << "Fail to send configuration to " << m_deviceId << " : no answer to Actuator Set External Interface Settings command" << std::endl;

      auto response = boost::make_shared<message::CResponseReceivedMessage>(answer);

      if (response->returnCode() != message::CResponseReceivedMessage::RET_OK)
         std::cerr << "Fail to send configuration to " << m_deviceId << " : Actuator Set External Interface Settings command returns " << response->returnCode() << std::endl;
   }
}
