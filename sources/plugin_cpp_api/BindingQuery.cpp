#include "stdafx.h"
#include "BindingQuery.h"


namespace plugin_cpp_api
{
   CBindingQuery::CBindingQuery(const toPlugin::BindingQuery& msg,
                                boost::function1<void, const shared::CDataContainer&> sucessCallback,
                                boost::function1<void, const std::string&> errorCallback)
      : m_query(msg.query()),
        m_sucessCallback(sucessCallback),
        m_errorCallback(errorCallback)
   {
   }

   CBindingQuery::~CBindingQuery()
   {
   }

   const shared::plugin::yPluginApi::IBindingQueryData& CBindingQuery::getData()
   {
      return m_query;
   }

   void CBindingQuery::sendSuccess(const shared::CDataContainer& result)
   {
      m_sucessCallback(result);
   }

   void CBindingQuery::sendError(const std::string& errorMessage)
   {
      m_errorCallback(errorMessage);
   }
} // namespace plugin_cpp_api	


