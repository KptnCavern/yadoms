#pragma once
#include "IFactory.h"
#include <shared/process/IProcess.h>
#include <shared/process/ILogger.h>
#include <shared/process/ICommandLine.h>
#include <IPathProvider.h>
#include "IIpcAdapter.h"
#include "yInterpreterApiImplementation.h"
#include "InstanceStateHandler.h"

namespace automation
{
   namespace interpreter
   {
      //--------------------------------------------------------------
      /// \brief	IFactory implementation
      //--------------------------------------------------------------
      class CFactory : public IFactory
      {
      public:
         //--------------------------------------------------------------
         /// \brief	Constructor
         /// \param[in] pathProvider      The path provider
         //--------------------------------------------------------------
         explicit CFactory(const IPathProvider& pathProvider);

         //--------------------------------------------------------------
         /// \brief	Destructor
         //--------------------------------------------------------------
         virtual ~CFactory();

         // IFactory Implementation
         boost::shared_ptr<interpreter::IInstance> createInterpreterInstance(const std::string& interpreterFileName,
                                                                             boost::function2<void, bool, const std::string&> onInstanceStateChangedFct) const override;
         boost::filesystem::path interpreterLogFile(const std::string& interpreterFileName) const override;
         // [END] IFactory Implementation

      protected:
         boost::shared_ptr<const shared::script::yInterpreterApi::IInformation> createInterpreterInformation(const std::string& interpreterFileName) const;
         boost::shared_ptr<shared::process::ILogger> createProcessLogger(const std::string& interpreterFileName) const;
         boost::shared_ptr<interpreter::IIpcAdapter> createInterpreterRunningContext(boost::shared_ptr<const shared::script::yInterpreterApi::IInformation> interpreterInformation) const;
         boost::shared_ptr<interpreter::CYInterpreterApiImplementation> createInterpreterApiImplementation(boost::shared_ptr<const shared::script::yInterpreterApi::IInformation> interpreterInformation) const;
         boost::shared_ptr<shared::process::ICommandLine> createCommandLine(const boost::shared_ptr<const shared::script::yInterpreterApi::IInformation> interpreterInformation,
                                                                            const std::string& messageQueueId) const;
         boost::shared_ptr<interpreter::CInstanceStateHandler> createInstanceStateHandler(boost::shared_ptr<const shared::script::yInterpreterApi::IInformation> interpreterInformation,
                                                                                          boost::function2<void, bool, const std::string&> onInstanceStateChangedFct) const;
         boost::shared_ptr<shared::process::IProcess> createInstanceProcess(boost::shared_ptr<shared::process::ICommandLine> commandLine,
                                                                            boost::shared_ptr<shared::process::ILogger> logger,
                                                                            boost::shared_ptr<interpreter::CInstanceStateHandler> instanceStateHandler) const;
      private:
         const IPathProvider& m_pathProvider;//TODO utile ?
      };
   }
} // namespace automation::interpreter


