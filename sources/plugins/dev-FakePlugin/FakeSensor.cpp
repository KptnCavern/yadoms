#include "stdafx.h"
#include "FakeSensor.h"
#include <shared/currentTime/Provider.h>

CFakeSensor::CFakeSensor(const std::string& deviceName)
   : m_deviceName(deviceName),
     m_temperature1(boost::make_shared<yApi::historization::CTemperature>("temp1")),
     m_temperature2(boost::make_shared<yApi::historization::CTemperature>("temp2")),
     m_batteryLevel(boost::make_shared<yApi::historization::CBatteryLevel>("Battery")),
     m_current(boost::make_shared<yApi::historization::CCurrent>("current",
                                                                 yApi::EKeywordAccessMode::kGet,
                                                                 yApi::historization::EMeasureType::kAbsolute,
                                                                 yApi::historization::typeInfo::CDoubleTypeInfo().setMin(0).setMax(5).setPrecision(0.1))),
     m_rssi(boost::make_shared<yApi::historization::CRssi>("rssi")),
     m_dateTime(boost::make_shared<yApi::historization::CDateTime>("dateTime",
                                                                   shared::plugin::yPluginApi::EKeywordAccessMode::kGet)),
     m_historizers({m_temperature1, m_temperature2, m_batteryLevel, m_rssi, m_dateTime, m_current}),
     m_dist(0, 20)
{
   m_temperature1->set(25.0);
   m_temperature2->set(10.0);
   m_batteryLevel->set(100);
   m_current->set(2);
   m_rssi->set(50);
   m_dateTime->set(shared::currentTime::Provider().now());
}

CFakeSensor::~CFakeSensor()
{
}

void CFakeSensor::declareDevice(boost::shared_ptr<yApi::IYPluginApi> api) const
{
   // Declare device and associated keywords (= values managed by this device)
   if (!api->deviceExists(m_deviceName))
      api->declareDevice(m_deviceName, getType(), getModel(), m_historizers);
}

void CFakeSensor::read()
{
   // Generate a random variation on temperature (+/- 0 to 1.0�)
   auto offset = static_cast<int>(m_dist(m_gen) - 10.0) / 10.0; // Random offset, value from -1.0 to 1.0
   auto temperature = m_temperature1->get() + offset;

   //we keep 2 decimals
   m_temperature1->set(static_cast<int>(temperature * 100) / 100.0);

   // Generate a random variation on temperature (+/- 0 to 2.0�)
   offset = static_cast<int>(m_dist(m_gen) - 20.0) / 20.0; // Random offset, value from -2.0 to 2.0
   temperature = m_temperature2->get() + offset;

   //we keep 2 decimals
   m_temperature2->set(static_cast<int>(temperature * 100) / 100.0);

   // Decrease battery level (min 20%)
   if (m_batteryLevel->get() > 20)
      m_batteryLevel->set(m_batteryLevel->get() - 1);

   //set the current date time onto m_datetime keyword
   m_dateTime->set(shared::currentTime::Provider().now());

   // Generate a random variation on temperature (+/- 0 to 1�)
   offset = static_cast<int>(m_dist(m_gen) - 10.0) / 10.0; // Random offset, value from -1.0 to 1.0
   auto current = m_current->get() + offset;
   if (current < 0)
      current = 0;
   if (current > 5)
      current = 5;
   m_current->set(static_cast<int>(current * 10) / 10.0);
}

void CFakeSensor::historizeData(boost::shared_ptr<yApi::IYPluginApi> api) const
{
   if (!api)
      throw shared::exception::CException("api must be defined");

   // If you need to historize several data, use the vector form of historizeData for performance
   api->historizeData(m_deviceName, m_historizers);
}

const std::string& CFakeSensor::getDeviceName() const
{
   return m_deviceName;
}

const std::string& CFakeSensor::getModel()
{
   static const std::string model("Fake sensor");
   return model;
}


const std::string& CFakeSensor::getType()
{
   static const std::string type("fakeSensorType");
   return type;
}

