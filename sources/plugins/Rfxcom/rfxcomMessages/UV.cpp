#include "stdafx.h"
#include "UV.h"
#include <shared/exception/InvalidParameter.hpp>

// Shortcut to yPluginApi namespace
namespace yApi = shared::plugin::yPluginApi;

namespace rfxcomMessages
{
   CUV::CUV(boost::shared_ptr<yApi::IYPluginApi> api,
            const RBUF& rbuf,
            size_t rbufSize,
            boost::shared_ptr<const ISequenceNumberProvider> seqNumberProvider)
      : m_uv("uv"),
        m_temperature("temperature"),
        m_batteryLevel("battery"),
        m_rssi("rssi")
   {
      CheckReceivedMessage(rbuf,
                           rbufSize,
                           pTypeUV,
                           DONT_CHECK_SUBTYPE,
                           GET_RBUF_STRUCT_SIZE(UV),
                           DONT_CHECK_SEQUENCE_NUMBER);

      m_subType = rbuf.UV.subtype;

      m_id = rbuf.UV.id1 | (rbuf.UV.id2 << 8);

      m_uv.set(rbuf.UV.uv / 10);
      if (m_subType == sTypeUV3)
         m_temperature.set(NormalizeTemperature(rbuf.UV.temperatureh, rbuf.UV.temperaturel, rbuf.UV.tempsign == 1));
      m_batteryLevel.set(NormalizeBatteryLevel(rbuf.UV.battery_level));
      m_rssi.set(NormalizeRssiLevel(rbuf.UV.rssi));

      Init(api);
   }

   CUV::~CUV()
   {
   }

   void CUV::Init(boost::shared_ptr<yApi::IYPluginApi> api)
   {
      // Build device description
      buildDeviceModel();
      buildDeviceName();

      // Create device and keywords if needed
      if (!api->deviceExists(m_deviceName))
      {
         shared::CDataContainer details;
         details.set("type", pTypeUV);
         details.set("subType", m_subType);
         details.set("id", m_id);
         api->declareDevice(m_deviceName, m_deviceModel, details);

         if (m_subType == sTypeUV3)
            api->declareKeyword(m_deviceName, m_temperature);

         api->declareKeyword(m_deviceName, m_uv);
         api->declareKeyword(m_deviceName, m_batteryLevel);
         api->declareKeyword(m_deviceName, m_rssi);
      }
   }

   boost::shared_ptr<std::queue<shared::communication::CByteBuffer>> CUV::encode(boost::shared_ptr<ISequenceNumberProvider> seqNumberProvider) const
   {
      throw shared::exception::CInvalidParameter("UV is a read-only message, can not be encoded");
   }

   void CUV::historizeData(boost::shared_ptr<yApi::IYPluginApi> api) const
   {
      if (m_subType == sTypeUV3)
         api->historizeData(m_deviceName, m_temperature);

      api->historizeData(m_deviceName, m_uv);
      api->historizeData(m_deviceName, m_batteryLevel);
      api->historizeData(m_deviceName, m_rssi);
   }

   const std::string& CUV::getDeviceName() const
   {
      return m_deviceName;
   }

   void CUV::buildDeviceName()
   {
      std::ostringstream ssdeviceName;
      ssdeviceName << static_cast<unsigned int>(m_subType) << "." << static_cast<unsigned int>(m_id);
      m_deviceName = ssdeviceName.str();
   }

   void CUV::buildDeviceModel()
   {
      std::ostringstream ssModel;

      switch (m_subType)
      {
      case sTypeUV1: ssModel << "UVN128, UV138";
         break;
      case sTypeUV2: ssModel << "UVN800";
         break;
      case sTypeUV3: ssModel << "TFA";
         break;
      default: ssModel << boost::lexical_cast<std::string>(m_subType);
         break;
      }

      m_deviceModel = ssModel.str();
   }
} // namespace rfxcomMessages


