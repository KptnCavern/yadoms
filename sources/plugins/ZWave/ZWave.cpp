#include "stdafx.h"
#include "ZWave.h"
#include <plugin_cpp_api/ImplementationHelper.h>
#include <shared/exception/Exception.hpp>
#include "ZWaveControllerFactory.h"
#include "KeywordContainer.h"
#include "ZWaveInternalState.h"

// Use this macro to define all necessary to make your DLL a Yadoms valid plugin.
// Note that you have to provide some extra files, like package.json, and icon.png
// This macro also defines the static PluginInformations value that can be used by plugin to get information values
IMPLEMENT_PLUGIN(CZWave)


CZWave::CZWave()
{
}

CZWave::~CZWave()
{
}


void CZWave::doWork(boost::shared_ptr<yApi::IYPluginApi> api)
{
   try
   {
      api->setPluginState(yApi::historization::EPluginState::kCustom, "Initialization");

      // Load configuration values (provided by database)
      m_configuration.initializeWith(api->getConfiguration());

      m_configuration.setPath(api->getInformation()->getPath().string());

      // the main loop
      api->setPluginState(yApi::historization::EPluginState::kCustom, "Configuring");
      m_controller = CZWaveControllerFactory::Create();
      m_controller->configure(&m_configuration, &api->getEventHandler());

      std::cout << "CZWave is initializing" << std::endl;
      context->setPluginState(yApi::historization::EPluginState::kCustom, "Initialization");
      IZWaveController::E_StartResult initResult = m_controller->start();
      if (initResult == IZWaveController::kSuccess)
      {
         std::cout << "CZWave is running" << std::endl;
         context->setPluginState(yApi::historization::EPluginState::kRunning);
         while (1)
         {
            // Wait for an event
            switch (api->getEventHandler().waitForEvents())
            {
            case yApi::IYPluginApi::kEventStopRequested:
            {
               std::cout << "Stop requested" << std::endl;
               api->setPluginState(yApi::historization::EPluginState::kStopped);
               StopController();
               return;
            }
            case yApi::IYPluginApi::kEventDeviceCommand:
            {
               // Command was received from Yadoms
               auto command = api->getEventHandler().getEventData<boost::shared_ptr<const yApi::IDeviceCommand> >();

               std::cout << "Command received from Yadoms :" << yApi::IDeviceCommand::toString(command) << std::endl;
               try
               {
                  m_controller->sendCommand(command->getDevice(), command->getKeyword(), command->getBody());
               }
               catch (shared::exception::CException& ex)
               {
                  std::cerr << "Fail to send command : " << ex.what() << std::endl;
               }
               catch (std::exception& ex)
               {
                  std::cerr << "Fail to send command. exception : " << ex.what() << std::endl;
               }
               catch (...)
               {
                  std::cerr << "Fail to send command. unknown exception" << std::endl;
               }
               break;
            }
            case yApi::IYPluginApi::kEventExtraCommand:
            {
               // Command was received from Yadoms
               auto extraCommand = api->getEventHandler().getEventData<boost::shared_ptr<const yApi::IExtraCommand> >();

               if (extraCommand)
               {
                  std::cout << "Extra command received : " << extraCommand->getCommand() << std::endl;

                  if (extraCommand->getCommand() == "inclusionMode")
                  {
                     m_controller->startInclusionMode();
                  }
                  else if (extraCommand->getCommand() == "exclusionMode")
                  {
                     m_controller->startExclusionMode();
                  }
                  else if (extraCommand->getCommand() == "hardReset")
                  {
                     m_controller->hardResetController();
                  }
                  else if (extraCommand->getCommand() == "softReset")
                  {
                     m_controller->softResetController();
                  }
                  else if (extraCommand->getCommand() == "testNetwork")
                  {
                     m_controller->testNetwork(extraCommand->getData().get<int>("frameCount"));
                  }
                  else if (extraCommand->getCommand() == "healNetowrk")
                  {
                     m_controller->healNetwork();
                  }
                  else if (extraCommand->getCommand() == "cancelCommand")
                  {
                     m_controller->cancelCurrentCommand();
                  }
               }
               break;
            }

            case yApi::IYPluginApi::kEventUpdateConfiguration:
            {
               // Configuration was updated
               api->setPluginState(yApi::historization::EPluginState::kCustom, "updateConfiguration");
               auto newConfiguration = api->getEventHandler().getEventData<shared::CDataContainer>();
               std::cout << "Update configuration..." << std::endl;
               BOOST_ASSERT(!newConfiguration.empty()); // newConfigurationValues shouldn't be empty, or kEventUpdateConfiguration shouldn't be generated

               if (m_controller)
                  m_controller->stop();

               m_configuration.initializeWith(newConfiguration);

               m_controller->configure(&m_configuration, &api->getEventHandler());

               if (m_controller->start())
                  api->setPluginState(yApi::historization::EPluginState::kRunning);
               else
                  api->setPluginState(yApi::historization::EPluginState::kError);
               break;
            }
            case kDeclareDevice:
            {
               try
               {
                  auto deviceData = api->getEventHandler().getEventData<shared::CDataContainer>();
                  if (!api->deviceExists(deviceData.get<std::string>("name")))
                     api->declareDevice(deviceData.get<std::string>("name"),
                        deviceData.get<std::string>("friendlyName"),
                        std::vector<boost::shared_ptr<const shared::plugin::yPluginApi::historization::IHistorizable> >(),
                        deviceData.get<shared::CDataContainer>("details"));
               }
               catch (shared::exception::CException& ex)
               {
                  std::cerr << "Fail to declare device : " << ex.what() << std::endl;
               }
               catch (std::exception& ex)
               {
                  std::cerr << "Fail to declare device. exception : " << ex.what() << std::endl;
               }
               catch (...)
               {
                  std::cerr << "Fail to declare device. unknown exception" << std::endl;
               }
               break;
            }

            case kUpdateKeyword:
            {
               try
               {
                  auto keywordData = api->getEventHandler().getEventData<boost::shared_ptr<CKeywordContainer> >();

                  auto deviceId = keywordData->getDeviceId();
                  auto keywordId = keywordData->getKeyword();

                  if (!api->keywordExists(deviceId, keywordId))
                     api->declareKeyword(deviceId, keywordId);

                  api->historizeData(deviceId, keywordId);
               }
               catch (shared::exception::CException& ex)
               {
                  std::cerr << "Fail to update keyword : " << ex.what() << std::endl;
               }
               catch (std::exception& ex)
               {
                  std::cerr << "Fail to update keyword. exception : " << ex.what() << std::endl;
               }
               catch (...)
               {
                  std::cerr << "Fail to update keyword. unknown exception" << std::endl;
               }
            }
            break;

            case kInternalStateChange:
            {
               auto s = EZWaveInteralState(api->getEventHandler().getEventData<std::string>());

               switch (s)
               {
               case EZWaveInteralState::kCompletedValue:
               case EZWaveInteralState::kRunningValue:
                  api->setPluginState(yApi::historization::EPluginState::kRunning);
                  break;
               default:
                  api->setPluginState(yApi::historization::EPluginState::kCustom, s.toString());
                  break;
               }
               break;
            }
            default:
            {
               std::cerr << "Unknown message id" << std::endl;
               break;
            }
            }
         }
         switch (initResult)
         {
         case IZWaveController::kSerialPortError:
            context->setPluginState(yApi::historization::EPluginState::kError, "failToAccessSerialPort");
            break;

         case IZWaveController::kControllerError:
            context->setPluginState(yApi::historization::EPluginState::kError, "failToStartController");
            break;

         default:
         case IZWaveController::kUnknownError:
            context->setPluginState(yApi::historization::EPluginState::kError, "failToStart");
            break;
         }
      }

      std::cerr << "Cannot start ZWave controller. Plugin ends." << std::endl;
   }
   catch (shared::exception::CException& ex)
   {
      std::cerr << "The ZWave plugin fails. shared exception : " << ex.what() << std::endl;
      api->setPluginState(yApi::historization::EPluginState::kError, (boost::format("Shared exception : %1%") % ex.what()).str());
   }
   catch (std::exception& ex)
   {
      std::cerr << "The ZWave plugin fails. exception : " << ex.what() << std::endl;
      api->setPluginState(yApi::historization::EPluginState::kError, (boost::format("Exception : %1%") % ex.what()).str());
   }
   catch (...)
   {
      std::cerr << "The ZWave plugin fails. unknown exception" << std::endl;
      api->setPluginState(yApi::historization::EPluginState::kError, "Unknown exception");
   }

   StopController();
}

void CZWave::StopController() const
{
   std::cout << "Ending ZWave plugin" << std::endl;
   try
   {
      if (m_controller)
         m_controller->stop();
   }
   catch (std::exception& ex)
   {
      std::cerr << "The ZWave plugin fail to stop. exception : " << ex.what() << std::endl;
   }
   catch (...)
   {
      std::cerr << "The ZWave plugin fail to stop. unknown exception" << std::endl;
   }
}

