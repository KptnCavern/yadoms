#include "stdafx.h"
#include "Rule.h"
#include <shared/Log.h>
#include "script/IProperties.h"

namespace automation
{
   CRule::CRule(boost::shared_ptr<const database::entities::CRule> ruleData,
                boost::shared_ptr<script::IManager> scriptManager,
                boost::shared_ptr<IRuleStateHandler> ruleStateHandler)
      : m_ruleData(ruleData),
        m_scriptManager(scriptManager),
        m_ruleStateHandler(ruleStateHandler)
   {
      start();
   }

   CRule::~CRule()
   {
   }

   void CRule::start()
   {
      if (!!m_process)
      {
         YADOMS_LOG(warning) << "Can not start rule " << m_ruleData->Name() << " : already started";
         return;
      }

      auto scriptProperties = m_scriptManager->createScriptProperties(m_ruleData);
      auto scriptLogger = m_scriptManager->createScriptLogger(scriptProperties->scriptPath());
      auto yScriptApi = m_scriptManager->createScriptContext(scriptLogger);
      auto stopNotifier = m_scriptManager->createStopNotifier(m_ruleStateHandler, m_ruleData->Id());

      auto scriptInterpreter = m_scriptManager->getAssociatedInterpreter(scriptProperties->interpreterName());

      scriptLogger->log("#### START ####");


      m_process = scriptInterpreter->createProcess(scriptProperties->scriptPath(), scriptLogger, yScriptApi, stopNotifier, shared::CDataContainer());
   }

   void CRule::requestStop()
   {
      m_process->kill();
   }
} // namespace automation	
	
	