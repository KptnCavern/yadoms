#include "stdafx.h"
#include "Clamp.h"
#include <shared/Log.h>

namespace equipments
{
   namespace subdevices
   {
      CClamp::CClamp(boost::shared_ptr<yApi::IYPluginApi> api,
                     std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> >& keywordsToDeclare,
                     const bool isInstantCurrentClampRegistered,
                     const std::string& deviceName,
                     const std::string& keywordName) :
         m_deviceName(deviceName),
         m_keywordName(keywordName),
         m_instantCurrentRegistered(isInstantCurrentClampRegistered)
      {
         initializeClamp(api, keywordsToDeclare, keywordName);
      }

      void CClamp::initializeClamp(boost::shared_ptr<yApi::IYPluginApi> api,
                                   std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> >& keywordsToDeclare,
                                   const std::string& keywordName)
      {
         if (m_instantCurrentRegistered)
         {
            boost::shared_ptr<yApi::historization::CCurrent> tempCurrent = boost::make_shared<yApi::historization::CCurrent>("I - " + keywordName,
                                                                                                                             yApi::EKeywordAccessMode::kGet);
            m_CurrentClamp = tempCurrent;
            keywordsToDeclare.push_back(tempCurrent);
         }
         else
         {
            if (m_CurrentClamp)
            {
               if (api->keywordExists(m_deviceName, m_CurrentClamp))
                  api->removeKeyword(m_deviceName, "I - " + keywordName);
            }
         }
         boost::shared_ptr<yApi::historization::CEnergy>  tempCounter = boost::make_shared<yApi::historization::CEnergy>("Wh - " + keywordName,
                                                                                                                         yApi::EKeywordAccessMode::kGet);
         m_CounterClamp = tempCounter;
         keywordsToDeclare.push_back(tempCounter);
      }

      void CClamp::updateConfiguration(boost::shared_ptr<yApi::IYPluginApi> api,
                                       const bool isInstantCurrentClampRegistered)
      {
         //TODO : A remplir avec la partie au-dessus
      }

      void CClamp::updateFromDevice(boost::shared_ptr<yApi::IYPluginApi> api,
                                    std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> >& keywordsToHistorize,
                                    const bool isInstantCurrentClampRegistered,
                                    const double& instantCurrentValue,
                                    const Poco::Int64& energyClampValue)
      {
         //TODO : If deviceName or contractName are different then create a new device
         //if ( m_instantCurrentRegistered!= isInstantCurrentClampRegistered)
         //   initializeClamp(api);

         if (isInstantCurrentClampRegistered)
         {
            m_CurrentClamp->set(instantCurrentValue);
            keywordsToHistorize.push_back(m_CurrentClamp);
            YADOMS_LOG(trace) << m_CurrentClamp->getKeyword() << " set to " << instantCurrentValue;
         }

         m_CounterClamp->set(energyClampValue);
         keywordsToHistorize.push_back(m_CounterClamp);
         YADOMS_LOG(trace) << m_CounterClamp->getKeyword() << " set to " << energyClampValue;
      }

      std::string CClamp::name()
      {
         return m_keywordName;
      }

      CClamp::~CClamp()
      {}
   }
}// namespace equipments::subdevices