#include "stdafx.h"
#include "Profile_D2_01_02.h"
#include "../bitsetHelpers.hpp"
#include "../../message/RadioErp1SendMessage.h"
#include "Profile_D2_01_Common.h"
#include <shared/Log.h>

CProfile_D2_01_02::CProfile_D2_01_02(const std::string& deviceId,
                                     boost::shared_ptr<yApi::IYPluginApi> api)
   : m_deviceId(deviceId),
     m_dimmer(boost::make_shared<yApi::historization::CDimmable>("Dimmer", yApi::EKeywordAccessMode::kGetSet)),
     m_historizers({m_dimmer})
{
}

CProfile_D2_01_02::~CProfile_D2_01_02()
{
}

const std::string& CProfile_D2_01_02::profile() const
{
   static const std::string profile("D2-01-02");
   return profile;
}

const std::string& CProfile_D2_01_02::title() const
{
   static const std::string title("Electronic dimmer with local Control");
   return title;
}

std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> CProfile_D2_01_02::allHistorizers() const
{
   return m_historizers;
}

std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> CProfile_D2_01_02::states(unsigned char rorg,
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
   int state = bitset_extract(data, 17, 7);
   switch (ioChannel)
   {
   case 0:
      m_dimmer->set(state);
      historizers.push_back(m_dimmer);
      break;
   default:
      YADOMS_LOG(information) << "Profile " << profile() << " : received unsupported ioChannel value " << ioChannel;
      break;
   }
   return historizers;
}

void CProfile_D2_01_02::sendCommand(const std::string& keyword,
                                    const std::string& commandBody,
                                    const std::string& senderId,
                                    boost::shared_ptr<IMessageHandler> messageHandler) const
{
   CProfile_D2_01_Common::sendActuatorSetOutputCommandDimming(messageHandler,
                                                              senderId,
                                                              m_deviceId,
                                                              std::stoul(commandBody));
}

void CProfile_D2_01_02::sendConfiguration(const shared::CDataContainer& deviceConfiguration,
                                          const std::string& senderId,
                                          boost::shared_ptr<IMessageHandler> messageHandler) const
{
   // Device supports no configuration
}

