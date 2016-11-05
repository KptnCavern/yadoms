#pragma once

#include "server/database/IDeviceRequester.h"
#include "server/database/IDatabaseRequester.h"

namespace database
{
   namespace common
   {
      namespace requesters
      {
         //--------------------------------------------------------------
         /// \Brief		   Device requester for SQLite database
         //--------------------------------------------------------------
         class CDevice: public IDeviceRequester
         {
         public:
            //--------------------------------------------------------------
            /// \Brief		   Constructor
            /// \param [in]	pDatabaseHandler: the database handler
            //--------------------------------------------------------------
            explicit CDevice(boost::shared_ptr<IDatabaseRequester> databaseRequester);

            //--------------------------------------------------------------
            /// \Brief		   Destructor
            //--------------------------------------------------------------
            virtual ~CDevice();

            // IDeviceRequester implementation
            bool deviceExists(const int deviceId) const override;
            bool deviceExists(const int pluginId,
                              const std::string& deviceName) const override;
            boost::shared_ptr<entities::CDevice> getDevice(int deviceId) const override;
            boost::shared_ptr<entities::CDevice> getDevice(const int pluginId,
                                                           const std::string& name) const override;
            boost::shared_ptr<entities::CDevice> createDevice(int pluginId,
                                                              const std::string& name,
                                                              const std::string& friendlyName,
                                                              const std::string& model,
                                                              const shared::CDataContainer& details) override;
            std::vector<boost::shared_ptr<entities::CDevice>> getDevices() const override;
            std::vector<std::string> getDevicesNames(int pluginId) const override;
            std::vector<boost::shared_ptr<entities::CDevice>> getDevices(int pluginId) const override;
            std::vector<boost::shared_ptr<entities::CDevice>> getDevicesIdFromFriendlyName(const std::string& friendlyName) const override;
            std::vector<boost::shared_ptr<entities::CDevice>> getDeviceWithCapacity(const std::string& capacityName,
                                                                                    const shared::plugin::yPluginApi::EKeywordAccessMode& accessMode) const override;
            std::vector<boost::shared_ptr<entities::CDevice>> getDeviceWithCapacityType(const shared::plugin::yPluginApi::EKeywordAccessMode& capacityAccessMode,
                                                                                        const shared::plugin::yPluginApi::EKeywordDataType& capacityType) const override;
            void updateDeviceFriendlyName(int deviceId, const std::string& newFriendlyName) override;
            void updateDeviceConfiguration(int deviceId, const shared::CDataContainer & configuration) override;
            
            void removeDevice(int deviceId) override;
            void removeDevice(int pluginId,
                              const std::string& deviceName) override;
            void removeAllDeviceForPlugin(int pluginId) override;
            // [END] IDeviceRequester implementation

         private:
            //--------------------------------------------------------------
            /// \Brief		   Reference to IDatabaseRequester
            //--------------------------------------------------------------
            boost::shared_ptr<IDatabaseRequester> m_databaseRequester;
         };
      } //namespace requesters
   } //namespace common
} //namespace database 


