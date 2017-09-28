#include "stdafx.h"
#include "DefaultEquipment.h"
#include <shared/DataContainer.h>

namespace equipments
{
   CDefaultEquipment::CDefaultEquipment(const std::string& name,
                                        const std::string& devEUID) :
      m_name(name),
      m_devEUI(devEUID),
      m_batteryLevel(boost::make_shared<yApi::historization::CBatteryLevel>("battery")),
      m_messageKeyword(boost::make_shared<yApi::historization::CText>("message",
                                                                      yApi::EKeywordAccessMode::kGetSet))
   {}

   std::string CDefaultEquipment::getName() const
   {
      return m_name;
   }

   std::string CDefaultEquipment::getEUI() const
   {
      return m_devEUI;
   }

   CDefaultEquipment::~CDefaultEquipment()
   {}

   void CDefaultEquipment::createDevice(boost::shared_ptr<yApi::IYPluginApi> api)
   {
      std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> keywordsToDeclare = { m_batteryLevel, m_messageKeyword };

      // Save names into details
      shared::CDataContainer details;
      details.set("devEUI", m_devEUI);

      api->declareDevice(m_name, "Orange Business", "Orange Business", keywordsToDeclare, details);
   }

   void CDefaultEquipment::updateData(boost::shared_ptr<yApi::IYPluginApi> api,
                                      int batteryLevel,
                                      std::string& data)
   {
      std::vector<boost::shared_ptr<const yApi::historization::IHistorizable>> keywordsToHistorize = { m_batteryLevel, m_messageKeyword };

      m_batteryLevel->set(batteryLevel);
      m_messageKeyword->set(data);

      api->historizeData(m_name, keywordsToHistorize);
   }
}