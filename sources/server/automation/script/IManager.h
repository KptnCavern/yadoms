#pragma once
#include "IProperties.h"
#include "../../database/common/requesters/Rule.h"
#include <shared/script/yScriptApi/IYScriptApi.h>
#include <server/automation/IRuleStateHandler.h>
#include <shared/process/IProcess.h>
#include <shared/script/IInterpreter.h>
#include <Poco/Logger.h>

namespace automation { namespace script
{
   //-----------------------------------------------------
   ///\brief The script factory interface
   //-----------------------------------------------------
   class IManager
   {
   public:
      //-----------------------------------------------------
      ///\brief               Destructor
      //-----------------------------------------------------
      virtual ~IManager() {}

      //-----------------------------------------------------
      ///\brief               Get available interpreters (as user-friendly names)
      ///\return              The list of available interpreters
      //-----------------------------------------------------
      virtual std::vector<std::string> getAvailableInterpreters() = 0;

      //-----------------------------------------------------
      ///\brief               Get the interpreter needed to run a script
      ///\param[in] interpreterName The interpreter name
      ///\return              The first interpreter found supporting this script
      ///\throw CScriptInterpreterNotFound No corresponding script interpreter was found
      //-----------------------------------------------------
      virtual boost::shared_ptr<shared::script::IInterpreter> getAssociatedInterpreter(const std::string& interpreterName) = 0;

      //-----------------------------------------------------
      ///\brief               Unload an interpreter (do nothing if interpreter not loaded)
      ///\param[in] interpreterName Interpreter to unload
      //-----------------------------------------------------
      virtual void unloadInterpreter(const std::string& interpreterName) = 0;


      //-----------------------------------------------------
      ///\brief               Create the properties for a script
      ///\param[in] ruleData  The rule raw data
      ///\return              A new script properties instance
      //-----------------------------------------------------
      virtual boost::shared_ptr<IProperties> createScriptProperties(boost::shared_ptr<const database::entities::CRule> ruleData) = 0;

      //-----------------------------------------------------
      ///\brief               Get the script file
      ///\param[in] ruleData  The rule raw data
      ///\return              The rule code
      ///\throw CInvalidParameter if rule ID or file not found
      //-----------------------------------------------------
      virtual std::string getScriptFile(boost::shared_ptr<const database::entities::CRule> ruleData) = 0;

      //-----------------------------------------------------
      ///\brief               Get the script template file
      ///\param[in] interpreterName Interpreter to unload
      ///\return              The template code
      //-----------------------------------------------------
      virtual std::string getScriptTemplateFile(const std::string& interpreterName) = 0;
      
      //-----------------------------------------------------
      ///\brief               Update the script file (create if necessary)
      ///\param[in] ruleData  The rule raw data
      ///\return              The rule code
      //-----------------------------------------------------
      virtual void updateScriptFile(boost::shared_ptr<const database::entities::CRule> ruleData, const std::string& code) = 0;

      //-----------------------------------------------------
      ///\brief               Delete the script file
      ///\param[in] ruleData  The rule raw data
      ///\param[in] doBackup  Do a backup if true
      //-----------------------------------------------------
      virtual void deleteScriptFile(boost::shared_ptr<const database::entities::CRule> ruleData, bool doBackup = true) = 0;

      //-----------------------------------------------------
      ///\brief               Get the script log file
      ///\param[in] ruleData  The rule raw data
      ///\return              The rule log file, empty if not available
      ///\throw CInvalidParameter if rule ID not found
      //-----------------------------------------------------
      virtual std::string getScriptLogFile(boost::shared_ptr<const database::entities::CRule> ruleData) = 0; 
      
      //-----------------------------------------------------
      ///\brief               Get the script log file name
      ///\param[in] ruleData  The rule raw data
      ///\return              The rule log file name
      //-----------------------------------------------------
      virtual boost::filesystem::path getScriptLogFileName(boost::shared_ptr<const database::entities::CRule> ruleData) = 0;

      //-----------------------------------------------------
      ///\brief               Create the script logger
      ///\param[in] ruleData  The rule raw data
      ///\return              A script logger instance
      //-----------------------------------------------------
      virtual Poco::Logger& createScriptLogger(boost::shared_ptr<const database::entities::CRule> ruleData) = 0;

      //-----------------------------------------------------
      ///\brief               Create the script context (IYScriptApi implementation)
      ///\param[in] scriptLogger The logger used for rule
      ///\return              A script context instance
      //-----------------------------------------------------
      virtual boost::shared_ptr<shared::script::yScriptApi::IYScriptApi> createScriptContext(Poco::Logger& scriptLogger) = 0;

      //-----------------------------------------------------
      ///\brief               Create the stop notifier
      ///\param[in] ruleStateHandler   The global state handler
      ///\param[in] ruleId    The rule ID
      ///\return              A stop notifier instance
      //-----------------------------------------------------
      virtual boost::shared_ptr<shared::process::IProcessObserver> createStopNotifier(boost::shared_ptr<IRuleStateHandler> ruleStateHandler, int ruleId) = 0;
   };

} } // namespace automation::script

