#pragma once

#include <shared/plugin/yPluginApi/IYPluginApi.h>
#include <plugin_IPC/plugin_IPC.h>


namespace plugin_cpp_api
{
   //-----------------------------------------------------
   ///\brief The Yadoms plugin API implementation for the CPP plugin API
   //-----------------------------------------------------
   class CApiImplementation : public shared::plugin::yPluginApi::IYPluginApi
   {
   public:
      //-----------------------------------------------------
      ///\brief               Constructor
      //-----------------------------------------------------
      CApiImplementation();

      //-----------------------------------------------------
      ///\brief               Destructor
      //-----------------------------------------------------
      virtual ~CApiImplementation();

      void setSendingMessageQueue(boost::shared_ptr<boost::interprocess::message_queue> sendMessageQueue);

      bool stopRequested() const;

      // shared::script::yScriptApi::IYScriptApi implementation
      void setPluginState(const shared::plugin::yPluginApi::historization::EPluginState& state,
                           const std::string& customMessageId = shared::CStringExtension::EmptyString,
                           const std::map<std::string, std::string> & customMessageDataParams = std::map<std::string, std::string>()) override;

      bool deviceExists(const std::string& device) const override;
      shared::CDataContainer getDeviceDetails(const std::string& device) const override;
      void declareDevice(const std::string& device,
                         const std::string& model,
                         boost::shared_ptr<const shared::plugin::yPluginApi::historization::IHistorizable> keyword,
                         const shared::CDataContainer& details = shared::CDataContainer::EmptyContainer) override;
      void declareDevice(const std::string& device,
                         const std::string& model,
                         const std::vector<boost::shared_ptr<const shared::plugin::yPluginApi::historization::IHistorizable> >& keywords,
                         const shared::CDataContainer& details = shared::CDataContainer::EmptyContainer) override;
      bool keywordExists(const std::string& device,
                         const std::string& keyword) const override;
      bool keywordExists(const std::string& device,
                         boost::shared_ptr<const shared::plugin::yPluginApi::historization::IHistorizable> keyword) const override;
      void declareKeyword(const std::string& device,
                          boost::shared_ptr<const shared::plugin::yPluginApi::historization::IHistorizable> keyword,
                          const shared::CDataContainer& details = shared::CDataContainer::EmptyContainer) override;
      std::string getRecipientValue(int recipientId,
                                    const std::string& fieldName) const override;
      std::vector<int> findRecipientsFromField(const std::string& fieldName,
                                               const std::string& expectedFieldValue) const override;
      bool recipientFieldExists(const std::string& fieldName) const override;
      void historizeData(const std::string& device,
                         boost::shared_ptr<const shared::plugin::yPluginApi::historization::IHistorizable> data) override;
      void historizeData(const std::string& device,
                         const std::vector<boost::shared_ptr<const shared::plugin::yPluginApi::historization::IHistorizable> >& dataVect) override;
      boost::shared_ptr<const shared::plugin::information::IInformation> getInformation() const override;
      shared::CDataContainer getConfiguration() override;
      const boost::filesystem::path& getDataPath() const override;
      shared::event::CEventHandler& getEventHandler() override;
      bool isDeveloperMode() const override;
      // [END] shared::script::yScriptApi::IYScriptApi implementation

      void onReceive(boost::shared_ptr<const unsigned char[]> message, size_t messageSize);

      void waitInitialized() const;

   protected:
      //--------------------------------------------------------------
      /// \brief	Send a request
      /// \param[in] request Request to send
      //--------------------------------------------------------------
      void send(const toYadoms::msg& msg) const;
      void send(const toYadoms::msg& msg,
                boost::function1<bool, const toPlugin::msg&> checkExpectedMessageFunction,
                boost::function1<void, const toPlugin::msg&> onReceiveFunction) const;

      void processSystem(const toPlugin::System& msg);
      void processInit(const toPlugin::Init& msg);
      void processUpdateConfiguration(const toPlugin::Configuration& msg);
      void processBindingQuery(const toPlugin::BindingQuery& msg);
      void processDeviceConfigurationSchemaRequest(const toPlugin::DeviceConfigurationSchemaRequest& msg);
      void processSetDeviceConfiguration(const toPlugin::SetDeviceConfiguration& msg);
      void processDeviceCommand(const toPlugin::DeviceCommand& msg);
      void processExtraCommand(const toPlugin::ExtraCommand& msg);
      void processManuallyDeviceCreation(const toPlugin::ManuallyDeviceCreation& msg);

      void setInitialized();

      static void fillHistorizable(boost::shared_ptr<const shared::plugin::yPluginApi::historization::IHistorizable> in,
                                   toYadoms::Historizable* out);
      static void fillCapacity(const shared::plugin::yPluginApi::CStandardCapacity& in,
                               toYadoms::Capacity* out);

   private:
      mutable std::condition_variable m_initializationCondition;
      mutable std::mutex m_initializationConditionMutex;
      bool m_initialized;

      bool m_stopRequested;

      shared::event::CEventHandler m_pluginEventHandler;


      // The message queue buffer, localy used but defined here to be allocated only once
      boost::shared_ptr<unsigned char[]> m_mqBuffer;

      // The send mutex. Protect m_mqBuffer and m_sendMessageQueue
      mutable boost::recursive_mutex m_sendMutex;

      boost::shared_ptr<boost::interprocess::message_queue> m_sendMessageQueue;

      mutable boost::recursive_mutex m_onReceiveHookMutex;
      mutable boost::function1<bool, const toPlugin::msg&> m_onReceiveHook;

      boost::shared_ptr<shared::plugin::information::IInformation> m_pluginInformation;
      boost::shared_ptr<const boost::filesystem::path> m_dataPath;
   };
} // namespace plugin_cpp_api	


