#pragma once
#include <server/dataAccessLayer/IEventLogger.h>
#include <shared/event/EventHandler.hpp>
#include "IInstanceStartErrorObserver.h"
#include "IInstanceStateHandler.h"
#include <shared/plugin/yPluginApi/historization/Text.h>
#include <server/database/IPluginRequester.h>
#include <server/database/IKeywordRequester.h>
#include <server/database/IPluginEventLoggerRequester.h>
#include <server/dataAccessLayer/IDeviceManager.h>
#include <server/dataAccessLayer/IAcquisitionHistorizer.h>
#include <shared/process/IProcessObserver.h>

namespace pluginSystem
{
   //-----------------------------------------------------
   ///\brief The instance error handler
   //-----------------------------------------------------
   class CInstanceStateHandler : public IInstanceStartErrorObserver, public IInstanceStateHandler, public shared::process::IProcessObserver
   {
   public:
      //-----------------------------------------------------
      ///\brief               Constructor
      ///\param[in] instanceData             Plugin instance data
      ///\param[in] pluginInformation        Plugin information
      ///\param[in] pluginRequester          Database requester
      ///\param[in] eventLogger              Main event logger
      ///\param[in] pluginEventLoggerRequester Plugin event logger
      ///\param[in] acquisitionHistorizer    Acquisition recorder
      ///\param[in] managerEventHandler      The manager event handler
      ///\param[in] deviceManager            Device manager
      ///\param[in] keywordRequester         Keyword requester
      ///\param[in] instanceStopEventId      Id of the instance stop event
      //-----------------------------------------------------
      CInstanceStateHandler(boost::shared_ptr<const database::entities::CPlugin> instanceData,
                            boost::shared_ptr<const shared::plugin::information::IInformation> pluginInformation,
                            boost::shared_ptr<database::IPluginRequester> pluginRequester,
                            boost::shared_ptr<dataAccessLayer::IEventLogger> eventLogger,
                            boost::shared_ptr<database::IPluginEventLoggerRequester> pluginEventLoggerRequester,
                            boost::shared_ptr<dataAccessLayer::IAcquisitionHistorizer> acquisitionHistorizer,
                            boost::shared_ptr<shared::event::CEventHandler> managerEventHandler,
                            boost::shared_ptr<dataAccessLayer::IDeviceManager> deviceManager,
                            boost::shared_ptr<database::IKeywordRequester> keywordRequester,
                            int instanceStopEventId);

      //-----------------------------------------------------
      ///\brief               Destructor
      //-----------------------------------------------------
      virtual ~CInstanceStateHandler();

      // IInstanceStartErrorObserver Implementation
      void signalStartError(const std::string& error) override;
      // [END] IInstanceStartErrorObserver Implementation

      // IInstanceStateHandler Implementation
      void setState(const shared::plugin::yPluginApi::historization::EPluginState& state, const std::string & customMessageId = std::string()) override;
      // [END] IInstanceStateHandler Implementation

      // process::IProcessObserver Implementation
      void onStart() override;
      void onFinish(int returnCode, const std::string& error) override;
      // [END] process::IProcessObserver Implementation




   protected:
      void signalNormalStop();
      void signalError(const std::string& error);

      // Get the plugin state device/keywords (create if not exists)
      int pluginStateDeviceId() const;
      int pluginStateKeywordId();
      int pluginStateMessageIdKeywordId();

      //-----------------------------------------------------
      ///\brief Record a plugin major event (recorded in Yadoms database)
      ///\param    [in]    severity           The message severity
      ///\param    [in]    message            The message
      //-----------------------------------------------------      
      enum PluginEventSeverity { kInfo, kError };
      virtual void recordPluginEvent(PluginEventSeverity severity, const std::string & message);//TODO � virer ?

   private:
      //-----------------------------------------------------
      ///\brief               The instance data
      //-----------------------------------------------------
      boost::shared_ptr<const database::entities::CPlugin> m_instanceData;

      //-----------------------------------------------------
      ///\brief               The plugin information
      //-----------------------------------------------------
      boost::shared_ptr<const shared::plugin::information::IInformation> m_pluginInformation;

      //-----------------------------------------------------
      ///\brief               The plugin data accessor
      //-----------------------------------------------------
      boost::shared_ptr<database::IPluginRequester> m_pluginRequester;

      //-----------------------------------------------------
      ///\brief               The event logger
      //-----------------------------------------------------
      boost::shared_ptr<dataAccessLayer::IEventLogger> m_eventLogger;

      //--------------------------------------------------------------
      /// \brief			The plugin event logger requester
      //--------------------------------------------------------------
      boost::shared_ptr<database::IPluginEventLoggerRequester> m_pluginEventLoggerRequester;

      //--------------------------------------------------------------
      /// \brief			The Acquisition historizer
      //--------------------------------------------------------------
      boost::shared_ptr<dataAccessLayer::IAcquisitionHistorizer> m_acquisitionHistorizer;

      //--------------------------------------------------------------
      /// \brief			The plugin manager event handler
      //--------------------------------------------------------------
      boost::shared_ptr<shared::event::CEventHandler> m_managerEventHandler;

      //--------------------------------------------------------------
      /// \brief			The device requester
      //--------------------------------------------------------------
      boost::shared_ptr<dataAccessLayer::IDeviceManager> m_deviceManager;

      //--------------------------------------------------------------
      /// \brief			The keyword requester
      //--------------------------------------------------------------
      boost::shared_ptr<database::IKeywordRequester> m_keywordRequester;

      //--------------------------------------------------------------
      /// \brief	      The plugin state keyword
      /// \note         Use it by pluginState() method
      //--------------------------------------------------------------
      boost::shared_ptr<shared::plugin::yPluginApi::historization::CPluginState> m_pluginStateKeyword;
      int m_pluginStateKeywordId;
      boost::shared_ptr<shared::plugin::yPluginApi::historization::CText> m_pluginStateMessageIdKeyword;
      int m_pluginStateMessageIdKeywordId;

      //-----------------------------------------------------
      ///\brief               Stop event ID
      //-----------------------------------------------------
      int m_instanceStopEventId;
   };
	
} // namespace pluginSystem
	
	