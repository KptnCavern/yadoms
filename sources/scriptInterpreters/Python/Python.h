#pragma once
#include <shared/script/IInterpreter.h>
#include "IPythonExecutable.h"

//--------------------------------------------------------------
/// \brief	Python interpreter
//--------------------------------------------------------------
class CPython : public shared::script::IInterpreter
{
public:
   //--------------------------------------------------------------
   /// \brief	Constructor
   //--------------------------------------------------------------
   CPython();

   //--------------------------------------------------------------
   /// \brief	Destructor
   //--------------------------------------------------------------
   virtual ~CPython();

   // IInterpreter implementation
   std::string name() const override;
   std::string type() const override;
   bool isAvailable() const override;
   std::string loadScriptContent(const std::string& scriptPath) const override;
   void saveScriptContent(const std::string& scriptPath, const std::string& content) const override;
   boost::shared_ptr<shared::process::IProcess> createProcess(const std::string& scriptPath,
                                                              boost::shared_ptr<shared::process::ILogger> scriptLogger,
                                                              boost::shared_ptr<shared::script::yScriptApi::IYScriptApi> yScriptApi,
                                                              boost::shared_ptr<shared::process::IEndOfProcessObserver> stopNotifier,
                                                              const shared::CDataContainer& scriptConfiguration) const override;
  // [END] IInterpreter implementation

private:
   //--------------------------------------------------------------
   /// \brief	The Python executable accessor
   //--------------------------------------------------------------
   boost::shared_ptr<IPythonExecutable> m_executable;
};



