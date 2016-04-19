//
// Instance.h
//
// yadoms-plugin instance handler
//
#pragma once

#include <shared/process/IProcess.h>
#include <shared/plugin/yPluginApi/IBindingQueryRequest.h>
#include <shared/plugin/information/IInformation.h>
#include <shared/plugin/yPluginApi/IExtraCommand.h>
#include "IInstance.h"
#include "IContextAccessor.h"

namespace pluginSystem
{
   //--------------------------------------------------------------
   /// \brief	this class is used to manage a plugin instance. 
   //--------------------------------------------------------------
   class CInstance : public IInstance
   {
   public:
      //--------------------------------------------------------------
      /// \brief	Constructor
      /// \param [in]	instanceInformation        Information on the instance
      /// \param [in]	pluginInformation          Information on the plugin
      /// \param [in]   process                    The instance process
      /// \param [in]   contextAccessor            The api context accessor
      //--------------------------------------------------------------
      CInstance(boost::shared_ptr<const database::entities::CPlugin> instanceInformation,
                const boost::shared_ptr<const shared::plugin::information::IInformation> pluginInformation,
                boost::shared_ptr<shared::process::IProcess> process,
                boost::shared_ptr<IContextAccessor> contextAccessor);

      //--------------------------------------------------------------
      /// \brief	Destructor
      //--------------------------------------------------------------
      virtual ~CInstance();


      //--------------------------------------------------------------
      /// \brief			            Post the plugin information
      /// \param  information       Plugin information
      //--------------------------------------------------------------
      void postPluginInformation(boost::shared_ptr<const shared::plugin::information::IInformation> information) const;

      //--------------------------------------------------------------
      /// \brief			            Post a device command to the plugin
      /// \param  information       Device command
      //--------------------------------------------------------------
      void postDeviceCommand(boost::shared_ptr<const shared::plugin::yPluginApi::IDeviceCommand> deviceCommand) const;

      //--------------------------------------------------------------
      /// \brief			            Post an extra command to the plugin
      /// \param  command           Extra command to post
      //--------------------------------------------------------------
      void postExtraCommand(boost::shared_ptr<const shared::plugin::yPluginApi::IExtraCommand> extraCommand) const;

      // IInstance Implementation
      void updateConfiguration(const shared::CDataContainer& newConfiguration) const override;
      void requestStop() override;
      void kill() override;
      boost::shared_ptr<const database::entities::CPlugin> about() const override;
      boost::shared_ptr<const shared::plugin::information::IInformation> aboutPlugin() const override;
      // [END] IInstance Implementation


   protected:
      void postStopRequest() const;

      //TODO
      //    //--------------------------------------------------------------
      //    /// \brief			            Post a command to the plugin
      //    /// \param  command           Command to post
      //    //--------------------------------------------------------------
      //    virtual void postCommand(boost::shared_ptr<const shared::plugin::yPluginApi::IDeviceCommand> command) const;

      //    //--------------------------------------------------------------
      //    /// \brief			            Post a manually device creation request to the plugin
      //    /// \param  request           Request data
      //    //--------------------------------------------------------------
      //    virtual void postManuallyDeviceCreationRequest(boost::shared_ptr<shared::plugin::yPluginApi::IManuallyDeviceCreationRequest> & request) const;

      ////--------------------------------------------------------------
      ///// \brief                 Post a custom query request to a plugin
      ///// \param [in] request    Request data
      ////--------------------------------------------------------------
      //    virtual void postBindingQueryRequest(boost::shared_ptr<shared::plugin::yPluginApi::IBindingQueryRequest> & request) const;

   private:
      //--------------------------------------------------------------
      /// \brief			      The plugin information
      //--------------------------------------------------------------
      const boost::shared_ptr<const database::entities::CPlugin> m_instanceInformation;

      //--------------------------------------------------------------
      /// \brief			      The plugin information
      //--------------------------------------------------------------
      const boost::shared_ptr<const shared::plugin::information::IInformation> m_pluginInformation;

      //-----------------------------------------------------
      ///\brief               The script process
      //-----------------------------------------------------
      boost::shared_ptr<shared::process::IProcess> m_process;

      //-----------------------------------------------------
      ///\brief               The api context accessor
      //-----------------------------------------------------
      boost::shared_ptr<IContextAccessor> m_contextAccessor;
   };
} // namespace pluginSystem
