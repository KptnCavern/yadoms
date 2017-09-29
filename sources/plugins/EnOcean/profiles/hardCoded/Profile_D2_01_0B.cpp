#include "stdafx.h"
#include "Profile_D2_01_0B.h"
#include "../bitsetHelpers.hpp"
#include "../../message/RadioErp1SendMessage.h"
#include "../../message/ResponseReceivedMessage.h"
#include "Profile_D2_01_Common.h"
#include <shared/Log.h>

CProfile_D2_01_0B::CProfile_D2_01_0B(const std::string& deviceId,
                                     boost::shared_ptr<yApi::IYPluginApi> api)
   : m_deviceId(deviceId),
     m_channel(boost::make_shared<yApi::historization::CSwitch>("Channel", yApi::EKeywordAccessMode::kGetSet)),
     m_inputEnergy(boost::make_shared<yApi::historization::CEnergy>("Input energy")), //TODO dispo ?
     m_inputPower(boost::make_shared<yApi::historization::CPower>("Input power")), //TODO dispo ?
     m_loadEnergy(boost::make_shared<yApi::historization::CEnergy>("Load energy")),
     m_loadPower(boost::make_shared<yApi::historization::CPower>("Load power")),
     m_powerFailure(boost::make_shared<yApi::historization::CSwitch>("Power failure", yApi::EKeywordAccessMode::kGet)),
     m_historizers({m_channel, m_inputEnergy, m_inputPower, m_loadEnergy, m_loadPower, m_powerFailure })
{
}

CProfile_D2_01_0B::~CProfile_D2_01_0B()
{
}

const std::string& CProfile_D2_01_0B::profile() const
{
   static const std::string profile("D2-01-0B");
   return profile;
}

const std::string& CProfile_D2_01_0B::title() const
{
   static const std::string title("Electronic switch with energy measurement and local control");
   return title;
}

std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> CProfile_D2_01_0B::allHistorizers() const
{
   return m_historizers;
}

std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> CProfile_D2_01_0B::states(unsigned char rorg,
                                                                                                   const boost::dynamic_bitset<>& data,
                                                                                                   const boost::dynamic_bitset<>& status,
                                                                                                   const std::string& senderId,
                                                                                                   boost::shared_ptr<IMessageHandler> messageHandler) const
{
   // This device supports several RORG messages
   // We just use the VLD telegram
   if (rorg != CRorgs::ERorgIds::kVLD_Telegram)
      return std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>>();

   switch (bitset_extract(data, 4, 4))
   {
   case CProfile_D2_01_Common::kActuatorStatusResponse:
      {
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
            YADOMS_LOG(information) << "Profile " << profile() << " : received unsupported ioChannel value " << ioChannel;
            break;
         }

         auto powerFailureSupported = bitset_extract(data, 0, 1) ? true : false;
         auto powerFailureState = bitset_extract(data, 1, 1) ? true : false;
         if (powerFailureSupported)
         {
            m_powerFailure->set(powerFailureState);
            historizers.push_back(m_powerFailure);
         }

         return historizers;
      }
   case CProfile_D2_01_Common::kActuatorMeasurementResponse:
      {
         // Return only the concerned historizer
         std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> historizers;

         auto ioChannel = bitset_extract(data, 11, 5);
         auto unit = static_cast<CProfile_D2_01_Common::E_D2_01_MeasurementUnit>(bitset_extract(data, 8, 3));
         auto rawValue = bitset_extract(data, 16, 32);

         Poco::Int64 energyValueWh = 0;
         auto powerValueW = 0.0;
         switch (unit)
         {
         case CProfile_D2_01_Common::kEnergyWs:
            energyValueWh = static_cast<Poco::Int64>(rawValue) * 3600;
            break;
         case CProfile_D2_01_Common::kEnergyWh:
            energyValueWh = static_cast<Poco::Int64>(rawValue);
            break;
         case CProfile_D2_01_Common::kEnergyKWh:
            energyValueWh = static_cast<Poco::Int64>(rawValue) * 1000;
            break;
         case CProfile_D2_01_Common::kPowerW:
            powerValueW = static_cast<double>(rawValue);
            break;
         case CProfile_D2_01_Common::kPowerKW:
            powerValueW = static_cast<double>(rawValue) * 1000;
            break;
         default:
            YADOMS_LOG(information) << "Profile " << profile() << " : received unsupported unit value" << unit;
            break;
         }

         switch (ioChannel)
         {
         case 0: // Output channel
            {
               switch (unit)
               {
               case CProfile_D2_01_Common::kEnergyWs:
               case CProfile_D2_01_Common::kEnergyWh:
               case CProfile_D2_01_Common::kEnergyKWh:
                  m_loadEnergy->set(energyValueWh);
                  historizers.push_back(m_loadEnergy);
                  break;
               case CProfile_D2_01_Common::kPowerW:
               case CProfile_D2_01_Common::kPowerKW:
                  m_loadPower->set(powerValueW);
                  historizers.push_back(m_loadPower);

                  // Power is configured to be received automaticaly.
                  // As we can not receive both data (power + energy) automaticaly,
                  // we ask for Energy just after receiving Power.
                  CProfile_D2_01_Common::sendActuatorMeasurementQuery(messageHandler,
                                                                      senderId,
                                                                      m_deviceId); //TODO voir s'il faut le faire dans les autres profils
                  break;
               default:
                  YADOMS_LOG(information) << "Profile " << profile() << " : received unsupported unit value for output channel" << unit;
                  break;
               }
               break;
            }
         case 0x1F: // Input channel //TODO utile ?
            switch (unit)
            {
            case CProfile_D2_01_Common::kEnergyWs:
            case CProfile_D2_01_Common::kEnergyWh:
            case CProfile_D2_01_Common::kEnergyKWh:
               m_inputEnergy->set(energyValueWh);
               historizers.push_back(m_inputEnergy);
               break;
            case CProfile_D2_01_Common::kPowerW:
            case CProfile_D2_01_Common::kPowerKW:
               m_inputPower->set(powerValueW);
               historizers.push_back(m_inputPower);
               break;
            default:
               YADOMS_LOG(information) << "Profile " << profile() << " : received unsupported unit value for input channel" << unit;
               break;
            }
            break;
         default:
            YADOMS_LOG(information) << "Profile " << profile() << " : received unsupported ioChannel value " << ioChannel;
            break;
         }
         return historizers;
      }
   default:
      return std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>>();
   }
}

void CProfile_D2_01_0B::sendCommand(const std::string& keyword,
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
   YADOMS_LOG(error) << "Fail to send state to " << m_deviceId << " : no answer to Actuator Set Output command";

   auto response = boost::make_shared<message::CResponseReceivedMessage>(answer);

   if (response->returnCode() != message::CResponseReceivedMessage::RET_OK)
   YADOMS_LOG(error) << "Fail to send state to " << m_deviceId << " : Actuator Set Output command returns " << response->returnCode();
}

void CProfile_D2_01_0B::sendConfiguration(const shared::CDataContainer& deviceConfiguration,
                                          const std::string& senderId,
                                          boost::shared_ptr<IMessageHandler> messageHandler) const
{
   auto localControl = deviceConfiguration.get<std::string>("localControl") == "enable";
   auto taughtInAllDevices = deviceConfiguration.get<std::string>("taughtIn") == "allDevices";
   auto userInterfaceDayMode = deviceConfiguration.get<std::string>("userInterfaceMode") == "dayMode";
   auto defaultState = deviceConfiguration.get<CProfile_D2_01_Common::EDefaultState>("defaultState");
   auto powerFailureDetection = deviceConfiguration.get<std::string>("powerFailureDetection") == "enable";

   CProfile_D2_01_Common::sendActuatorSetLocalCommand(messageHandler,
                                                      senderId,
                                                      m_deviceId,
                                                      localControl,
                                                      taughtInAllDevices,
                                                      userInterfaceDayMode,
                                                      powerFailureDetection,
                                                      defaultState,
                                                      0.0,
                                                      0.0,
                                                      0.0);


   auto minEnergyMeasureRefreshTime = deviceConfiguration.get<double>("minEnergyMeasureRefreshTime");
   auto maxEnergyMeasureRefreshTime = deviceConfiguration.get<double>("maxEnergyMeasureRefreshTime");

   if (minEnergyMeasureRefreshTime > maxEnergyMeasureRefreshTime)
   {
      std::ostringstream oss;
      oss << "Min refresh time (" << minEnergyMeasureRefreshTime << ") is over max refresh time (" << maxEnergyMeasureRefreshTime << ") for device " << m_deviceId << " (" << profile() << ")";
      YADOMS_LOG(error) << oss.str();
      throw std::logic_error(oss.str());
   }

   // Configure for automatic power measure
   // At each power measure receive, we ask for energy measure
   CProfile_D2_01_Common::sendActuatorSetMeasurementCommand(messageHandler,
                                                            senderId,
                                                            m_deviceId,
                                                            true,
                                                            0, // TODO corriger les autres appels � sendActuatorSetMeasurementCommand
                                                            minEnergyMeasureRefreshTime,
                                                            maxEnergyMeasureRefreshTime);
}
