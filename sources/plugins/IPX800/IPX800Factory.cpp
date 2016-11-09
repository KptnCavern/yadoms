#include "stdafx.h"
#include "IPX800Factory.h"
#include "equipments/IPX800Equipment.h"
#include "equipments/X8RExtension.h"
#include "equipments/X8DExtension.h"
#include "equipments/X24DExtension.h"

CIPX800Factory::CIPX800Factory(boost::shared_ptr<yApi::IYPluginApi> api,
                               const std::string& device,
                               const IIPX800Configuration& configuration):
   m_ioManager(boost::make_shared<CIOManager>(device, configuration.getIPAddress(), configuration.isPasswordActivated(), configuration.getPassword()))
{
   //TODO : Faire les remplissages du X-8D et X-24D interdisants les slots pour les autres equipments.
   for (int counter = 0; counter < 6; ++counter)
   {
      X8RSlotused[counter] = false;
      X8DSlotused[counter] = false;
   }

   for (int counter = 0; counter < 2; ++counter)
      X24DSlotused[counter] = false;

   // Creation of the IPX800 device
   m_devicesList.push_back(boost::make_shared<equipments::CIPX800Equipment>(api, device));

   std::vector<std::string> devices = api->getAllDevices();
   std::vector<std::string>::iterator devicesIterator;

   // Create all extensions devices
   for (devicesIterator = devices.begin(); devicesIterator != devices.end(); ++devicesIterator)
   {
      boost::shared_ptr<equipments::IEquipment> extension;
      std::string model = api->getDeviceModel(*devicesIterator);
      std::cout << "Name : " << (*devicesIterator) << std::endl;
      std::cout << "Model : " << model << std::endl;

      if (model == "X-8R")
      {
         auto details = api->getDeviceDetails((*devicesIterator));
         int position = details.get<int>("position");
         extension = boost::make_shared<equipments::CX8RExtension>(api, (*devicesIterator), position);
         X8RSlotused[position-1] = true;
      }
      else if (model == "X-8D")
      {
         auto details = api->getDeviceDetails((*devicesIterator));
         int position = details.get<int>("position");
         extension = boost::make_shared<equipments::CX8DExtension>(api, (*devicesIterator), position);
         X8DSlotused[position-1] = true;
         
         // The corresponding slot for X-24D could not be installed
         X24DSlotused[position/3-1] = true;
      }
      else if (model == "X-24D")
      {
         auto details = api->getDeviceDetails((*devicesIterator));
         int position = details.get<int>("position");
         extension = boost::make_shared<equipments::CX24DExtension>(api, (*devicesIterator), position);
         X24DSlotused[position] = true;

         // The corresponding slots for X8D could not be installed
         X8DSlotused[position * 3] = true;
         X8DSlotused[position * 3 + 1] = true;
         X8DSlotused[position * 3 + 2] = true;
      }

      m_devicesList.push_back(extension);
   }

   m_ioManager->Initialize(api, m_devicesList);
}

std::string CIPX800Factory::createDeviceManually(boost::shared_ptr<yApi::IYPluginApi> api,
                                                 const yApi::IManuallyDeviceCreationData& data)
{
   boost::shared_ptr<equipments::IEquipment> extension;

   if (data.getConfiguration().get<bool>("type.content.X8R.radio"))
   {
      int position = data.getConfiguration().get<int>("type.content.X8R.content.Position");
      extension = boost::make_shared<equipments::CX8RExtension>(api, data.getDeviceName(), position);
      X8RSlotused[position] = true;
   }
   else if (data.getConfiguration().get<bool>("type.content.X8D.radio"))
   {
      int position = data.getConfiguration().get<int>("type.content.X8D.content.Position");
      extension = boost::make_shared<equipments::CX8DExtension>(api, data.getDeviceName(), position);
      X8DSlotused[position] = true;
   }
   else if (data.getConfiguration().get<bool>("type.content.X24D.radio"))
   {
      int position = data.getConfiguration().get<int>("type.content.X24D.content.Position");
      extension = boost::make_shared<equipments::CX24DExtension>(api, data.getDeviceName(), position);
      X24DSlotused[position] = true;
   }

   m_devicesList.push_back(extension);

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
         ev.set(boost::lexical_cast<std::string>(counter), "");
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
      if (!X24DSlotused[counter])
         ev.set(boost::lexical_cast<std::string>(counter), "");
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

void CIPX800Factory::OnConfigurationUpdate(boost::shared_ptr<yApi::IYPluginApi> api,
                                            const IIPX800Configuration& configuration)
{
/*
   for (int counter=0; counter<NB_INPUTS; ++counter)
      m_DigitalInput[counter]->ConfigurePullResistance( configuration.PullResistanceState(counter) );
      */
}

CIPX800Factory::~CIPX800Factory()
{}