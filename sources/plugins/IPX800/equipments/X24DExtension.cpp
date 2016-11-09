#include "stdafx.h"
#include "X24DExtension.h"
#include <shared/DataContainer.h>
#include "../specificHistorizers/inputOutput.h"

namespace equipments
{

   CX24DExtension::CX24DExtension(boost::shared_ptr<yApi::IYPluginApi> api,
                                  const std::string& device,
                                  const int& position):
      m_deviceName(device),
      m_deviceType("X-24D")
   {
      shared::CDataContainer details;
      details.set("provider", "IPX800");
      details.set("shortProvider", "ipx");
      details.set("type", m_deviceType);
      details.set("position", boost::lexical_cast<std::string>(position));

      // Relay Configuration
      for (int counter = 0; counter<X24D_DI_QTY; ++counter)
      {
         std::stringstream name, hardwareName;
         name << "D" << std::setfill('0') << std::setw(2) << boost::lexical_cast<int>(counter + 1);
         hardwareName << "D" << boost::lexical_cast<int>(position*24 + counter + 1);
         boost::shared_ptr<specificHistorizers::CInputOuput> temp = boost::make_shared<specificHistorizers::CInputOuput>(name.str(),
                                                                                                                         hardwareName.str(),
                                                                                                                         yApi::EKeywordAccessMode::kGet);
         m_keywordList.push_back(temp);
      }

      std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> > keywordsToDeclare;
      keywordsToDeclare.insert(keywordsToDeclare.end(), m_keywordList.begin(), m_keywordList.end());

      //D�claration of all IOs
      api->declareDevice(device, m_deviceType, keywordsToDeclare, details);
   }

   std::string CX24DExtension::getDeviceName() const
   {
      return m_deviceName;
   }

   std::string CX24DExtension::getDeviceType() const
   {
      return m_deviceType;
   }

   shared::CDataContainer CX24DExtension::buildMessageToDevice(boost::shared_ptr<yApi::IYPluginApi> api, boost::shared_ptr<const yApi::IDeviceCommand> command)
   {
      throw shared::exception::CException("Extension module X-24D have no keyword to set");
   }

   void CX24DExtension::updateFromDevice(const std::string& type, 
                                         boost::shared_ptr<yApi::IYPluginApi> api,
                                        shared::CDataContainer& values)
   {
      if (type == "D")
      {
         std::vector<boost::shared_ptr<specificHistorizers::CInputOuput> >::const_iterator diIterator;
         std::vector<boost::shared_ptr<const yApi::historization::IHistorizable> > keywordsToHistorize;

         try {
            for (diIterator = m_keywordList.begin(); diIterator != m_keywordList.end(); ++diIterator)
            {
               std::cout << "Set IO : " << (*diIterator)->getHardwareName() << std::endl;
               bool newValue = values.get<bool>((*diIterator)->getHardwareName());

               //historize only for new value
               if ((*diIterator)->get() != newValue)
               {
                  (*diIterator)->set(newValue);
                  keywordsToHistorize.push_back((*diIterator));
               }
            }
         }
         catch (shared::exception::CException& e)
         {
            std::cout << "error retrieve value :" << e.what() << std::endl;
         }

         api->historizeData(m_deviceName, keywordsToHistorize);
      }
   }

   void CX24DExtension::historizePendingCommand(boost::shared_ptr<yApi::IYPluginApi> api,
                                                boost::shared_ptr<const yApi::IDeviceCommand> command)
   {
      throw shared::exception::CException("Extension module X-24D have no keyword to set");
   }

   // TODO : A remplir
   CX24DExtension::~CX24DExtension()
   {}
}// namespace equipments