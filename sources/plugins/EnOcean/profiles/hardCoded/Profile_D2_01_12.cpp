#include "stdafx.h"
#include "Profile_D2_01_12.h"
#include "../bitsetHelpers.hpp"
#include "../../message/RadioErp1SendMessage.h"
#include "../../message/ResponseReceivedMessage.h"
#include "Profile_D2_01_Common.h"
#include <shared/Log.h>

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
                                                                                                   const boost::dynamic_bitset<>& status,
                                                                                                   const std::string& senderId,
                                                                                                   boost::shared_ptr<IMessageHandler> messageHandler) const
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
      m_channel1->set(state);
      historizers.push_back(m_channel1);
      break;
   case 1:
      m_channel2->set(state);
      historizers.push_back(m_channel2);
      break;
   default:
      YADOMS_LOG(information) << "Profile " << profile() << " : received unsupported ioChannel value " << ioChannel;
      break;
   }
   return historizers;
}

void CProfile_D2_01_12::sendCommand(const std::string& keyword,
                                    const std::string& commandBody,
                                    const std::string& senderId,
                                    boost::shared_ptr<IMessageHandler> messageHandler) const
{
   CProfile_D2_01_Common::sendActuatorSetOutputCommandSwitching(messageHandler,
                                                                senderId,
                                                                m_deviceId,
                                                                commandBody == "1",
                                                                keyword == m_channel1->getKeyword() ? 0 : 1);
}

void CProfile_D2_01_12::sendConfiguration(const shared::CDataContainer& deviceConfiguration,
                                          const std::string& senderId,
                                          boost::shared_ptr<IMessageHandler> messageHandler) const
{
   auto localControl = deviceConfiguration.get<std::string>("localControl") == "enable";
   auto taughtInAllDevices = deviceConfiguration.get<std::string>("taughtIn") == "allDevices";
   auto userInterfaceDayMode = deviceConfiguration.get<std::string>("userInterfaceMode") == "dayMode";
   auto defaultState = deviceConfiguration.get<CProfile_D2_01_Common::EDefaultState>("defaultState");
   auto connectedSwitchsType = deviceConfiguration.get<CProfile_D2_01_Common::EConnectedSwitchsType>("connectedSwitchsType");
   auto autoOffTimerSeconds = deviceConfiguration.get<double>("autoOffTimer");
   auto delayRadioOffTimerSeconds = deviceConfiguration.get<double>("delayRadioOffTimer");
   auto switchingStateToggle = deviceConfiguration.get<std::string>("switchingState") == "tooggle";

   // CMD 0x2 - Actuator Set Local
   CProfile_D2_01_Common::sendActuatorSetLocalCommand(messageHandler,
                                                      senderId,
                                                      m_deviceId,
                                                      localControl,
                                                      taughtInAllDevices,
                                                      userInterfaceDayMode,
                                                      false,
                                                      defaultState,
                                                      0.0,
                                                      0.0,
                                                      0.0);

   // CMD 0xB - Actuator Set External Interface Settings
   CProfile_D2_01_Common::sendActuatorSetExternalInterfaceSettingsCommand(messageHandler,
                                                                          senderId,
                                                                          m_deviceId,
                                                                          connectedSwitchsType,
                                                                          autoOffTimerSeconds,
                                                                          delayRadioOffTimerSeconds,
                                                                          switchingStateToggle);
}

