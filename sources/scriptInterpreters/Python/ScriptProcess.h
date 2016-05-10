#pragma once
#include "IPythonExecutable.h"
#include <shared/process/ILogger.h>
#include <shared/process/IProcess.h>
#include "IScriptFile.h"
#include "ContextAccessor.h"
#include <shared/process/ICommandLine.h>
#include <shared/process/IProcessObserver.h>

//--------------------------------------------------------------
/// \brief	Python process
//--------------------------------------------------------------
class CScriptProcess : public shared::process::IProcess
{
public:
   //--------------------------------------------------------------
   /// \brief	Constructor
   /// \param[in] executable  Python executable to call to start script
   /// \param[in] scriptFile The script file to execute
   /// \param[in] yScriptApi The context, used by script to interact with Yadoms
   /// \param[in] scriptLogger The script logger
   /// \param[in] stopNotifier The stop notifier
   //--------------------------------------------------------------
   CScriptProcess(boost::shared_ptr<IPythonExecutable> executable,
                  boost::shared_ptr<const IScriptFile> scriptFile,
                  boost::shared_ptr<shared::script::yScriptApi::IYScriptApi> yScriptApi,
                  boost::shared_ptr<shared::process::ILogger> scriptLogger,
                  boost::shared_ptr<shared::process::IProcessObserver> stopNotifier);

   //--------------------------------------------------------------
   /// \brief	Destructor
   //--------------------------------------------------------------
   virtual ~CScriptProcess();

   // shared::process::IProcess Implementation
   void kill() override;
   int getReturnCode() const override;
   std::string getError() const override;
   // [END] shared::process::IProcess Implementation

protected:
   //--------------------------------------------------------------
   /// \brief	Start a module
   //--------------------------------------------------------------
   void start();


   boost::shared_ptr<shared::process::ICommandLine> createCommandLine(const std::string& apiIdentifier) const;

private:
   //--------------------------------------------------------------
   /// \brief	The Python executable to call to start script
   //--------------------------------------------------------------
   boost::shared_ptr<IPythonExecutable> m_executable;

   //--------------------------------------------------------------
   /// \brief	The script file to execute
   //--------------------------------------------------------------
   boost::shared_ptr<const IScriptFile> m_scriptFile;

   //--------------------------------------------------------------
   /// \brief	The script api context
   //--------------------------------------------------------------
   boost::shared_ptr<shared::script::yScriptApi::IYScriptApi> m_yScriptApi;

   //--------------------------------------------------------------
   /// \brief	The script logger
   //--------------------------------------------------------------
   boost::shared_ptr<shared::process::ILogger> m_scriptLogger;

   //--------------------------------------------------------------
   /// \brief	Object to notify when process stops
   //--------------------------------------------------------------
   boost::shared_ptr<shared::process::IProcessObserver> m_stopNotifier;

   //--------------------------------------------------------------
   /// \brief	The context accessor
   //--------------------------------------------------------------
   boost::shared_ptr<CContextAccessor> m_contextAccessor;

   //--------------------------------------------------------------
   /// \brief	The process
   //--------------------------------------------------------------
   boost::shared_ptr<IProcess> m_process;
};



