#pragma once
#include <shared/process/IExternalProcessLogger.h>
#include <shared/script/yInterpreterApi/IYInterpreterApi.h>


// Shortcut to yInterpreterApi namespace
namespace yApi = shared::script::yInterpreterApi;

class CScriptLogger : public shared::process::IExternalProcessLogger
{
public:
   CScriptLogger(boost::shared_ptr<yApi::IYInterpreterApi> api,
                 int scriptInstanceId);
   virtual ~CScriptLogger();

   // IExternalProcessLogger Implementation
   void init() override;
   void information(const std::string& line) override;
   void error(const std::string& line) override;
   // [END] IExternalProcessLogger Implementation

private:
   boost::shared_ptr<yApi::IYInterpreterApi> m_api;
   const int m_scriptInstanceId;
};

