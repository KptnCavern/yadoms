#include "stdafx.h"
#include "OpenZWaveNodeConfiguration.h"
#include "OpenZWaveNodeKeywordFactory.h"
#include "OpenZWaveHelpers.h"
#include "ConfigurationSchemaFactory.h"
#include <shared/exception/NotSupported.hpp>
#include <shared/Log.h>

COpenZWaveNodeConfiguration::COpenZWaveNodeConfiguration(const uint32 homeId, const uint8 nodeId)
   : m_homeId(homeId), m_nodeId(nodeId)
{
}


COpenZWaveNodeConfiguration::~COpenZWaveNodeConfiguration()
{
}

void COpenZWaveNodeConfiguration::registerConfiguration(OpenZWave::ValueID& value)
{
   std::string keyword = COpenZWaveHelpers::GenerateKeywordName(value);

   if (m_configurationItems.find(keyword) == m_configurationItems.end())
      m_configurationItems[keyword] = COpenZWaveNodeKeywordFactory::createKeyword(value, m_homeId, m_nodeId, false);

}

boost::shared_ptr<IOpenZWaveNodeKeyword> COpenZWaveNodeConfiguration::getConfigurationItem(OpenZWave::ValueID& value)
{
   std::string keyword = COpenZWaveHelpers::GenerateKeywordName(value);

   if (m_configurationItems.find(keyword) == m_configurationItems.end())
   {
      registerConfiguration(value);
      if (m_configurationItems.find(keyword) != m_configurationItems.end())
      {
         return boost::shared_ptr<IOpenZWaveNodeKeyword>();
      }
   }
   return m_configurationItems[keyword];
}

bool COpenZWaveNodeConfiguration::sendCommand(const std::string& keyword, const std::string& commandData)
{
   if (m_configurationItems.find(keyword) != m_configurationItems.end())
      return m_configurationItems[keyword]->sendCommand(commandData);

   throw shared::exception::CException("The keyword is not registered for this zwave node");
}

shared::CDataContainer COpenZWaveNodeConfiguration::generateConfigurationSchema()
{
   shared::CDataContainer result;
   for (auto i = m_configurationItems.begin(); i != m_configurationItems.end(); ++i)
   {
      if (i->second != NULL)
      {
         try
         {
            auto itemSchema = CConfigurationSchemaFactory::generateForHistorizer(i->second);
            result.set(CConfigurationSchemaFactory::generateValidKeyName(i->first), itemSchema);
         }
         catch (shared::exception::CNotSupported &)
         {
            YADOMS_LOG(information) << "Fail to generate configuration schema for : " << i->first << " : historizer not supported" ;
         }
         catch (std::exception & ex)
         {
            YADOMS_LOG(information) << "Exception in generating configuration schema for : " << i->first << " : " << ex.what() ;
         }
         
      }
   }

   return result;
}

shared::CDataContainer COpenZWaveNodeConfiguration::saveValuesToDatabase()
{
   shared::CDataContainer result;
   for (auto i = m_configurationItems.begin(); i != m_configurationItems.end(); ++i)
   {
      if (i->second != NULL)
      {
         try
         {
            auto itemSchema = CConfigurationSchemaFactory::generateForHistorizer(i->second);
            result.set(CConfigurationSchemaFactory::generateValidKeyName(i->first), i->second->getLastKeywordValue()->formatValue());
         }
         catch (shared::exception::CNotSupported &)
         {
            YADOMS_LOG(information) << "Fail to getting last value for : " << i->first << " : historizer not supported" ;
         }
         catch (std::exception & ex)
         {
            YADOMS_LOG(information) << "Exception in getting last value for : " << i->first << " : " << ex.what() ;
         }

      }
   }

   return result;
}

void COpenZWaveNodeConfiguration::setConfigurationValues(const shared::CDataContainer &configuration)
{
   shared::CDataContainer result;
   for (auto i = m_configurationItems.begin(); i != m_configurationItems.end(); ++i)
   {
      if (i->second != NULL)
      {
         try
         {
            auto keywordKey = CConfigurationSchemaFactory::generateValidKeyName(i->first);
            if (configuration.containsValue(keywordKey))
            {
               sendCommand(i->first, configuration.get<std::string>(keywordKey));
            }
         }
         catch (shared::exception::CNotSupported &)
         {
            YADOMS_LOG(information) << "Fail to set configuration value for : " << i->first << " : historizer not supported" ;
         }
         catch (std::exception & ex)
         {
            YADOMS_LOG(information) << "Exception in setting configuration value for : " << i->first << " : " << ex.what() ;
         }

      }
   }
}

void COpenZWaveNodeConfiguration::updateNodeConfiguration(const std::string& keyword, const std::string& value, shared::CDataContainer & configuration)
{
   configuration.set(CConfigurationSchemaFactory::generateValidKeyName(keyword), value);
}