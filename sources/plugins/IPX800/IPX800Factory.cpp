#include "stdafx.h"
#include "IPX800Factory.h"
#include "equipments/IPX800Equipment.h"
#include "equipments/X8RExtension.h"
#include "equipments/X8DExtension.h"
#include "equipments/X24DExtension.h"
#include "equipments/manuallyDeviceCreationException.hpp"

CIPX800Factory::CIPX800Factory(boost::shared_ptr<yApi::IYPluginApi> api,
                               const std::string& device,
                               const IIPX800Configuration& configuration):
   m_ioManager(boost::make_shared<CIOManager>(device, 
                                              configuration.getIPAddressWithSocket(),
                                              configuration.isPasswordActivated(), 
                                              configuration.getPassword()))
{
   for (int counter = 0; counter < 6; ++counter)
   {
      X8RSlotused[counter] = false;
      X8DSlotused[counter] = false;
   }

   std::vector<std::string> devices = api->getAllDevices();
   std::vector<std::string>::iterator devicesIterator;

   // Creation of the IPX800 module
   // This module is created by default
   boost::shared_ptr<equipments::IEquipment> extension;
   extension = boost::make_shared<equipments::CIPX800Equipment>(api, device);
   m_devicesList.push_back(extension);
   //

   // Create all extensions devices
   for (devicesIterator = devices.begin(); devicesIterator != devices.end(); ++devicesIterator)
   {
      std::string model = api->getDeviceModel(*devicesIterator);
      std::cout << "Name : " << (*devicesIterator) << std::endl;
      std::cout << "Model : " << model << std::endl;

      if (model == "X-8R")
      {
         auto details = api->getDeviceDetails((*devicesIterator));
         int position = details.get<int>("position");
         extension = boost::make_shared<equipments::CX8RExtension>(api, (*devicesIterator), position);
         m_devicesList.push_back(extension);
         X8RSlotused[position-1] = true;
      }
      else if (model == "X-8D")
      {
         auto details = api->getDeviceDetails((*devicesIterator));
         int position = details.get<int>("position");
         extension = boost::make_shared<equipments::CX8DExtension>(api, (*devicesIterator), position);
         m_devicesList.push_back(extension);
         X8DSlotused[position-1] = true;
      }
      else if (model == "X-24D")
      {
         auto details = api->getDeviceDetails((*devicesIterator));
         int position = details.get<int>("position");
         extension = boost::make_shared<equipments::CX24DExtension>(api, (*devicesIterator), position);
         m_devicesList.push_back(extension);

         // The corresponding slots for X-8D could not be installed
         X8DSlotused[position * 3] = true;
         X8DSlotused[position * 3 + 1] = true;
         X8DSlotused[position * 3 + 2] = true;
      }  
   }

   m_ioManager->Initialize(m_devicesList);
}

std::string CIPX800Factory::createDeviceManually(boost::shared_ptr<yApi::IYPluginApi> api,
                                                 const yApi::IManuallyDeviceCreationData& data)
{
   boost::shared_ptr<equipments::IEquipment> extension;

   try {

      if (data.getConfiguration().get<bool>("type.content.X8R.radio"))
      {
         int position = data.getConfiguration().get<int>("type.content.X8R.content.Position");
         extension = boost::make_shared<equipments::CX8RExtension>(api, data.getDeviceName(), position);
         X8RSlotused[position-1] = true;
      }
      else if (data.getConfiguration().get<bool>("type.content.X8D.radio"))
      {
         int position = data.getConfiguration().get<int>("type.content.X8D.content.Position");
         extension = boost::make_shared<equipments::CX8DExtension>(api, data.getDeviceName(), position);
         X8DSlotused[position-1] = true;
      }
      else if (data.getConfiguration().get<bool>("type.content.X24D.radio"))
      {
         int position = data.getConfiguration().get<int>("type.content.X24D.content.Position");
         extension = boost::make_shared<equipments::CX24DExtension>(api, data.getDeviceName(), position);

         // The corresponding slots for X-8D could not be installed
         X8DSlotused[(position-1) * 3] = true;
         X8DSlotused[(position-1) * 3 + 1] = true;
         X8DSlotused[(position-1) * 3 + 2] = true;
      }

      if (extension)
         m_devicesList.push_back(extension);
   }
   catch (shared::exception::CException& e)
   {
      throw CManuallyDeviceCreationException(e.what());
   }

   // Update IO list
   m_ioManager->Initialize(m_devicesList);

   return extension->getDeviceName();
}

shared::CDataContainer CIPX800Factory::bindSlotsX8R()
{
   shared::CDataContainer ev;

   for (int counter = 0; counter < 6; ++counter)
   {
      // Add only not used slots
      if (!X8RSlotused[counter])
         ev.set(boost::lexical_cast<std::string>(counter + 1), "");
   }

   shared::CDataContainer en;
   en.set("type", "enum");
   en.set("values", ev);
   en.set("defaultValue", "1");

   return en;
}

shared::CDataContainer CIPX800Factory::bindSlotsX8D()
{
   shared::CDataContainer ev;

   for (int counter = 0; counter < 6; ++counter)
   {
      // Add only not used slots
      if (!X8DSlotused[counter])
         ev.set(boost::lexical_cast<std::string>(counter + 1), "");
   }

   shared::CDataContainer en;
   en.set("type", "enum");
   en.set("values", ev);
   en.set("defaultValue", "SLOT1");

   return en;
}

shared::CDataContainer CIPX800Factory::bindSlotsX24D()
{
   shared::CDataContainer ev;

   for (int counter = 0; counter < 2; ++counter)
   {
      // Add only not used slots
      if (!X8DSlotused[counter*3] && !X8DSlotused[counter * 3 + 1] && !X8DSlotused[counter * 3 + 2])
         ev.set(boost::lexical_cast<std::string>(counter + 1), "");
   }

   shared::CDataContainer en;
   en.set("type", "enum");
   en.set("values", ev);
   en.set("defaultValue", "SLOT1");

   return en;
}

boost::shared_ptr<CIOManager> CIPX800Factory::getIOManager(void)
{
   return m_ioManager;
}

void CIPX800Factory::removeDevice(boost::shared_ptr<yApi::IYPluginApi> api, std::string deviceRemoved)
{
   for (unsigned char counter = 0; counter < m_devicesList.size(); ++counter)
   {
      std::cout << "device Name : " << m_devicesList[counter]->getDeviceName() << std::endl;
      std::cout << "deviceRemoved : " << deviceRemoved << std::endl;

      // Deletion from the list of the device
      if (m_devicesList[counter]->getDeviceName() == deviceRemoved)
      {

         // If it's an extension, we delete the extension.
         // If it's the IPX800, we delete all elements
         if (m_devicesList[counter]->getDeviceType() != "IPX800")
         {
            // free slot(s) associated to this device for future configurations
            int position = m_devicesList[counter]->getSlot();
            
            if (m_devicesList[counter]->getDeviceType() == "X-8R") X8RSlotused[position - 1] = false;
            if (m_devicesList[counter]->getDeviceType() == "X-8D") X8DSlotused[position - 1] = false;
            if (m_devicesList[counter]->getDeviceType() == "X-24D")
            { 
               X8DSlotused[(position - 1) * 3] = false;
               X8DSlotused[(position - 1) * 3 + 1] = false;
               X8DSlotused[(position - 1) * 3 + 2] = false;
            }
            
            // remove the extension
            m_devicesList.erase(m_devicesList.begin() + counter);
         }
         else // If IPX800, we delete all extensions
            m_devicesList.clear();
      }
   }
}

CIPX800Factory::~CIPX800Factory()
{}