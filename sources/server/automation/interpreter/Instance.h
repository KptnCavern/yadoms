#pragma once
#include <shared/process/IProcess.h>
#include "IInstance.h"
#include "IIpcAdapter.h"

namespace automation
{
   namespace interpreter
   {
      //--------------------------------------------------------------
      /// \brief	this class is used to manage a interpreter instance. 
      //--------------------------------------------------------------
      class CInstance : public IInstance
      {
      public:
         CInstance(const boost::shared_ptr<const shared::script::yInterpreterApi::IInformation> interpreterInformation,
                   boost::shared_ptr<shared::process::IProcess> process,
                   boost::shared_ptr<IIpcAdapter> ipcAdapter);

         //--------------------------------------------------------------
         /// \brief	Destructor
         //--------------------------------------------------------------
         virtual ~CInstance();

         // IInstance Implementation
         boost::shared_ptr<const shared::script::yInterpreterApi::IInformation> aboutInterpreter() const override;
         bool isAvalaible() override;
         std::string loadScriptContent(const std::string& scriptPath) const override;
         void saveScriptContent(const std::string& scriptPath,
                                const std::string& scriptContent) const override;
         std::string startScript(int scriptInstanceId,
                                 const std::string& scriptPath,
                                 const std::string& yScriptApiId) const override;
         void stopScript(const std::string& scriptProcessId) const override;
         // [END] IInstance Implementation

      protected:
         bool CInstance::getAvalaibility() const;

      private:
         const boost::shared_ptr<const shared::script::yInterpreterApi::IInformation> m_interpreterInformation;

         boost::shared_ptr<shared::process::IProcess> m_process;
         boost::shared_ptr<IIpcAdapter> m_ipcAdapter;

         bool m_avalaible;
      };
   }
} // namespace automation::interpreter


