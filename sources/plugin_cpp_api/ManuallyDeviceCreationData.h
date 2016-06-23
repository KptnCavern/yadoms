#pragma once

#include <shared/plugin/yPluginApi/IManuallyDeviceCreationData.h>

namespace plugin_cpp_api
{
   class CManuallyDeviceCreationData : public shared::plugin::yPluginApi::IManuallyDeviceCreationData
   {
   public:
      CManuallyDeviceCreationData(const std::string& deviceName,
                                  const std::string& configuration);
      virtual ~CManuallyDeviceCreationData();

      // IManuallyDeviceCreationData Implementation
      const std::string& getDeviceName() const override;
      const shared::CDataContainer& getConfiguration() const override;
      // [END] IManuallyDeviceCreationData Implementation

   private:
      const std::string m_deviceName;
      const shared::CDataContainer m_configuration;
   };
} // namespace plugin_cpp_api	


