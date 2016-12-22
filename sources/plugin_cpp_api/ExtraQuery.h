#pragma once
#include <toPlugin.pb.h>
#include <shared/plugin/yPluginApi/IExtraQuery.h>
#include "ExtraQueryData.h"


namespace plugin_cpp_api
{
   //-----------------------------------------------------
   ///\brief The IExtraQuery implementation
   //-----------------------------------------------------
   class CExtraQuery : public shared::plugin::yPluginApi::IExtraQuery
   {
   public:
      explicit CExtraQuery(const plugin_IPC::toPlugin::ExtraQuery& msg,
                           boost::function1<void, const shared::CDataContainer&> sucessCallback,
                           boost::function1<void, const std::string&> errorCallback);
      virtual ~CExtraQuery();

      // IExtraQuery implementation
      const shared::plugin::yPluginApi::IExtraQueryData& getData() const override;
      void sendSuccess(const shared::CDataContainer& result) override;
      void sendError(const std::string& errorMessage) override;
      // [END] IExtraQuery implementation

   private:
      CExtraQueryData m_data;
      boost::function1<void, const shared::CDataContainer&> m_sucessCallback;
      boost::function1<void, const std::string&> m_errorCallback;
   };
} // namespace plugin_cpp_api	


