#include "stdafx.h"
#include "Profile_D2_01_01.h"
#include "../bitsetHelpers.hpp"
#include "../../message/RadioErp1SendMessage.h"
#include "../../message/ResponseReceivedMessage.h"
#include "Profile_D2_01_Common.h"


CProfile_D2_01_01::CProfile_D2_01_01(const std::string& deviceId,
                                     boost::shared_ptr<yApi::IYPluginApi> api)
   : m_deviceId(deviceId),
     m_channel(boost::make_shared<yApi::historization::CSwitch>("Channel", yApi::EKeywordAccessMode::kGetSet)),
     m_historizers({m_channel})
{
}

CProfile_D2_01_01::~CProfile_D2_01_01()
{
}

const std::string& CProfile_D2_01_01::profile() const
{
   static const std::string profile("D2-01-01");
   return profile;
}

const std::string& CProfile_D2_01_01::title() const
{
   static const std::string title("Electronic switch with Local Control");
   return title;
}

std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> CProfile_D2_01_01::allHistorizers() const
{
   return m_historizers;
}

std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> CProfile_D2_01_01::states(unsigned char rorg,
                                                                                                   const boost::dynamic_bitset<>& data,
                                                                                                   const boost::dynamic_bitset<>& status) const
{
   // This device supports several RORG messages
   // We just use the VLD telegram
   if (rorg != CRorgs::ERorgIds::kVLD_Telegram)
      return std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>>();

   if (bitset_extract(data, 4, 4) != CProfile_D2_01_Common::kActuatorStatusResponse)
      return std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>>();

   // Return only the concerned historizer
   std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> historizers;

   auto ioChannel = bitset_extract(data, 11, 5);
   auto state = bitset_extract(data, 17, 1) ? true : false;
   switch (ioChannel)
   {
   case 0:
      m_channel->set(state);
      historizers.push_back(m_channel);
      break;
   default:
      std::cout << "Profile " << profile() << " : received unsupported ioChannel value " << ioChannel << std::endl;
      break;
   }
   return historizers;
}

void CProfile_D2_01_01::sendCommand(const std::string& keyword,
                                    const std::string& commandBody,
                                    const std::string& senderId,
                                    boost::shared_ptr<IMessageHandler> messageHandler) const
{
   message::CRadioErp1SendMessage command(CRorgs::kVLD_Telegram,
                                          senderId,
                                          m_deviceId,
                                          0);

   boost::dynamic_bitset<> userData(3 * 8);
   bitset_insert(userData, 4, 4, CProfile_D2_01_Common::kActuatorSetOutput);
   bitset_insert(userData, 11, 5, 0);
   bitset_insert(userData, 17, 7, commandBody == "1" ? 100 : 0);

   command.userData(bitset_to_bytes(userData));

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

   return;
}

void CProfile_D2_01_01::sendConfiguration(const shared::CDataContainer& deviceConfiguration,
                                          const std::string& senderId,
                                          boost::shared_ptr<IMessageHandler> messageHandler) const
{
   // Device supports no configuration
}
