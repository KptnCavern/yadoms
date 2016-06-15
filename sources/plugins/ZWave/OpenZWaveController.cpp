#include "stdafx.h"
#include "OpenZWaveController.h"
#include <shared/Log.h>
#include "ZWave.h"
#include "OpenZWaveCommandClass.h"
#include <shared/plugin/yPluginApi/StandardCapacities.h>
#include <shared/plugin/yPluginApi/historization/Dimmable.h>
#include <command_classes/SwitchBinary.h>
#include <Options.h>
#include <Manager.h>
#include <Notification.h>
#include <platform/Log.h>
#include "KeywordContainer.h"
#include "OpenZWaveNodeKeywordFactory.h"
#include "OpenZWaveHelpers.h"
#include "ZWaveInternalState.h"

COpenZWaveController::COpenZWaveController()
   :m_homeId(0), m_initFailed(false), m_nodesQueried(false), m_lastSuccessfullySentCommand(OpenZWave::Driver::ControllerCommand_None), m_handler(NULL), m_configuration(NULL)
{

}


COpenZWaveController::~COpenZWaveController()
{

}

void onGlobalNotification(OpenZWave::Notification const* _notification, void* _context)
{
   try
   {
      YADOMS_LOG(debug) << "OpenZWave notification : " << _notification->GetAsString();

      COpenZWaveController * pPlugin = static_cast<COpenZWaveController *>(_context);
      if (pPlugin != NULL)
         pPlugin->onNotification(_notification, _context);
   }
   catch (OpenZWave::OZWException & ex)
   {
      YADOMS_LOG(fatal) << "OpenZWave exception (OnGlobalNotification) : " << ex.what();
   }
   catch (std::exception & ex)
   {
      YADOMS_LOG(fatal) << "OpenZWave std::exception (OnGlobalNotification) : " << ex.what();
   }
   catch (...)
   {
      YADOMS_LOG(fatal) << "OpenZWave unknown exception (OnGlobalNotification)";
   }
}


void COpenZWaveController::configure(CZWaveConfiguration * configuration, shared::event::CEventHandler * handler)
{
   BOOST_ASSERT(configuration != NULL);
   BOOST_ASSERT(handler != NULL);
   boost::lock_guard<boost::mutex> lock(m_treeMutex);
   m_configuration = configuration;
   m_handler = handler;
}

IZWaveController::E_StartResult COpenZWaveController::start()
{

   try
   {
      // Create the OpenZWave Manager.
      // The first argument is the path to the config files (where the manufacturer_specific.xml file is located
      // The second argument is the path for saved Z-Wave network state and the log file.  If you leave it NULL 
      // the log file will appear in the program's working directory.
      boost::filesystem::path full_path(m_configuration->getPath());
      boost::filesystem::path folder = full_path.parent_path();
      folder /= "config";

      boost::filesystem::path dataFolder = full_path.parent_path();
      dataFolder /= "data";
      if (!boost::filesystem::exists(dataFolder)) 
      {
         boost::system::error_code returnedError;

         boost::filesystem::create_directories(dataFolder, returnedError);
         if (returnedError)
         {
            //did not successfully create directories
            YADOMS_LOG(error) << "Fali to create folder : " << dataFolder.string();
         }
      }

      OpenZWave::Options::Create(folder.string(), dataFolder.string(), "");
      OpenZWave::Options::Get()->AddOptionInt("SaveLogLevel", OpenZWave::LogLevel_Error);
      OpenZWave::Options::Get()->AddOptionInt("QueueLogLevel", OpenZWave::LogLevel_Error);
      OpenZWave::Options::Get()->AddOptionInt("DumpTrigger", OpenZWave::LogLevel_Error);
      OpenZWave::Options::Get()->AddOptionInt("PollInterval", 500);
      OpenZWave::Options::Get()->AddOptionBool("IntervalBetweenPolls", true);
      OpenZWave::Options::Get()->AddOptionBool("ValidateValueChanges", true);
      OpenZWave::Options::Get()->Lock();


      OpenZWave::Manager::Create();

      // Add a callback handler to the manager.  The second argument is a context that
      // is passed to the OnNotification method.  If the OnNotification is a method of
      // a class, the context would usually be a pointer to that class object, to
      // avoid the need for the notification handler to be a static.
      OpenZWave::Manager::Get()->AddWatcher(onGlobalNotification, this);

      // Add a Z-Wave Driver
      
      //this part wait infinitely for serial port open success (configuration can be changed by another thread)
      bool serialPortOpened = false;
      int remainingTries = 3;
      while (!serialPortOpened)
      {
         //lock access to configuration
         boost::lock_guard<boost::mutex> lock(m_treeMutex);

         //open the port
         if (OpenZWave::Manager::Get()->AddDriver(m_configuration->getSerialPort()))
         {
            //ok
            serialPortOpened = true;
         }
         else
         {
            //fail to open : then unlock mutex to allow configuration to be changed, then wait 1 sec
            m_treeMutex.unlock();
            remainingTries--;
            if (remainingTries <= 0)
            {
               YADOMS_LOG(error) << "Fail to open serial port : " << m_configuration->getSerialPort();
               return IZWaveController::kSerialPortError;
            }
            else
            {
               YADOMS_LOG(warning) << "Fail to open serial port : " << m_configuration->getSerialPort() << ". Attemps remaining : " << remainingTries;
               boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
            }
         }
      }
         


      // Now we just wait for either the AwakeNodesQueried or AllNodesQueried notification,
      // then write out the config file.
      // In a normal app, we would be handling notifications and building a UI for the user.

      // Since the configuration file contains command class information that is only 
      // known after the nodes on the network are queried, wait until all of the nodes 
      // on the network have been queried (at least the "listening" ones) before
      // writing the configuration file.  (Maybe write again after sleeping nodes have
      // been queried as well.)
      while (!m_nodesQueried && !m_initFailed)
      {
         boost::this_thread::sleep(boost::posix_time::milliseconds(200));
      }

      if (!m_initFailed)
      {
         RequestConfigurationParameters();

         OpenZWave::Manager::Get()->WriteConfig(m_homeId);
         return IZWaveController::kSuccess;
      }
      else 
      {
         YADOMS_LOG(error) << "Fail to initialize controller";
         return IZWaveController::kControllerError;
      }

      
   }
   catch (OpenZWave::OZWException & ex)
   {
      YADOMS_LOG(fatal) << "Fail to start OpenZWave controller : OpenZWave exception : " << ex.what();
   }
   catch (std::exception & ex)
   {
      YADOMS_LOG(fatal) << "Fail to start OpenZWave controller : std::exception : " << ex.what();
   }
   catch (...)
   {
      YADOMS_LOG(fatal) << "Fail to start OpenZWave controller unknown exception";
   }

   return IZWaveController::kUnknownError;
}

void COpenZWaveController::stop()
{
   try
   {
      OpenZWave::Manager::Destroy();
   }
   catch (OpenZWave::OZWException & ex)
   {
      YADOMS_LOG(fatal) << "Fail to stop OpenZWave controller : OpenZWave exception : " << ex.what();
   }
   catch (std::exception & ex)
   {
      YADOMS_LOG(fatal) << "Fail to stop OpenZWave controller : std::exception : " << ex.what();
   }
   catch (...)
   {
      YADOMS_LOG(fatal) << "Fail to stop OpenZWave controller unknown exception";
   }
}


boost::shared_ptr<COpenZWaveNode> COpenZWaveController::getNode(OpenZWave::Notification const* _notification)
{
   uint32 const homeId = _notification->GetHomeId();
   uint8 const nodeId = _notification->GetNodeId();
   return getNode(homeId, nodeId);
}

boost::shared_ptr<COpenZWaveNode> COpenZWaveController::getNode(const uint32 homeId, const uint8 nodeId)
{
   for (NodeListType::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
   {
      if ((*it)->match(homeId, nodeId))
         return *it;
   }
   return boost::shared_ptr<COpenZWaveNode>();
}

void COpenZWaveController::RequestConfigurationParameters()
{
   for (NodeListType::iterator it = m_nodes.begin(); it != m_nodes.end(); ++it)
   {
      OpenZWave::Manager::Get()->RequestAllConfigParams((*it)->getHomeId(), (*it)->getNodeId());
   }
}







void COpenZWaveController::onNotification(OpenZWave::Notification const* _notification, void* _context)
{
   // Must do this inside a critical section to avoid conflicts with the main thread
   boost::lock_guard<boost::mutex> lock(m_treeMutex);

   //get all glocbal informations (for all notifications)
   OpenZWave::ValueID vID = _notification->GetValueID();
   ECommandClass commandClass((int)vID.GetCommandClassId());

   switch (_notification->GetType())
   {
   case OpenZWave::Notification::Type_ValueAdded:
   {
      boost::shared_ptr<COpenZWaveNode> node = getNode(_notification);
      if (node)
         node->registerKeyword(vID, m_configuration->getIncludeSystemKeywords());
      break;
   }

   case OpenZWave::Notification::Type_ValueRemoved:
      break;

   case OpenZWave::Notification::Type_ValueChanged:
   {
      if (!OpenZWave::Manager::Get()->GetChangeVerified(vID))
      {
         OpenZWave::Manager::Get()->SetChangeVerified(vID, true);
      }
      else
      {
         boost::shared_ptr<COpenZWaveNode> node = getNode(_notification);
         if (node)
         {
            boost::shared_ptr<IOpenZWaveNodeKeyword> kw = node->getKeyword(vID, m_configuration->getIncludeSystemKeywords());
            if (kw)
            {
               boost::shared_ptr<shared::plugin::yPluginApi::historization::IHistorizable> historizedData = node->updateKeywordValue(vID, m_configuration->getIncludeSystemKeywords());
               boost::shared_ptr<CKeywordContainer> d(new CKeywordContainer(COpenZWaveHelpers::GenerateDeviceName(node->getHomeId(), node->getNodeId()), *historizedData.get()));

               YADOMS_LOG(debug) << "===================================";
               YADOMS_LOG(debug) << "OpenZWave notification [Type_ValueChanged] : instance=" << (int)vID.GetInstance();
               YADOMS_LOG(debug) << "OpenZWave notification [Type_ValueChanged] : node=" << (int)vID.GetNodeId();
               YADOMS_LOG(debug) << "OpenZWave notification [Type_ValueChanged] : valeur=" << historizedData->formatValue();

               if (m_handler != NULL)
                  m_handler->postEvent(CZWave::kUpdateKeyword, d);
            }
         }
      }
      break;
   }

   case OpenZWave::Notification::Type_ValueRefreshed:
   {
      // One of the node values has changed
      boost::shared_ptr<COpenZWaveNode> node = getNode(_notification);
      if (node)
      {
         boost::shared_ptr<IOpenZWaveNodeKeyword> kw = node->getKeyword(vID, m_configuration->getIncludeSystemKeywords());
         if (kw)
         {
            boost::shared_ptr<shared::plugin::yPluginApi::historization::IHistorizable> historizedData = node->updateKeywordValue(vID, m_configuration->getIncludeSystemKeywords());
            boost::shared_ptr<CKeywordContainer> d(new CKeywordContainer(COpenZWaveHelpers::GenerateDeviceName(node->getHomeId(), node->getNodeId()), *historizedData.get()));

            YADOMS_LOG(debug) << "===================================";
            YADOMS_LOG(debug) << "OpenZWave notification [Type_ValueRefreshed] : instance=" << (int)vID.GetInstance();
            YADOMS_LOG(debug) << "OpenZWave notification [Type_ValueRefreshed] : node=" << (int)vID.GetNodeId();
            YADOMS_LOG(debug) << "OpenZWave notification [Type_ValueRefreshed] : valeur=" << historizedData->formatValue();

            if (m_handler != NULL)
               m_handler->postEvent(CZWave::kUpdateKeyword, d);
         }

         

         
      }
      break;
   }

   case OpenZWave::Notification::Type_Group:
   {
      // One of the node's association groups has changed
      int groupIdx = _notification->GetGroupIdx();
      std::string groupLabel = OpenZWave::Manager::Get()->GetGroupLabel(_notification->GetHomeId(), _notification->GetNodeId(), groupIdx);
      YADOMS_LOG(debug) << "Group change : id= " << groupIdx << " label=" << groupLabel; 
      break;

   }

   case OpenZWave::Notification::Type_NodeAdded:
   {
      m_nodes.push_back( boost::shared_ptr<COpenZWaveNode>( new COpenZWaveNode(_notification->GetHomeId(), _notification->GetNodeId())));
      break;
   }



   case OpenZWave::Notification::Type_NodeProtocolInfo:
   {
      boost::shared_ptr<COpenZWaveNode> node = getNode(_notification);
      if (node)
      {
         //OpenZWave::Manager::Get()->GetControllerInterfaceType()
         std::string sNodeType = OpenZWave::Manager::Get()->GetNodeType(node->getHomeId(), node->getNodeId());
         YADOMS_LOG(information) << "ZWave : NodeProtocolInfo : " << sNodeType;
         
      }
      break;
   }

   case OpenZWave::Notification::Type_NodeNaming:
   {
      boost::shared_ptr<COpenZWaveNode> nodeInfo = getNode(_notification);
      if (nodeInfo)
      {
         std::string sNodeName = OpenZWave::Manager::Get()->GetNodeName(nodeInfo->getHomeId(), nodeInfo->getNodeId());
         std::string sNodeManufacturer = OpenZWave::Manager::Get()->GetNodeManufacturerName(nodeInfo->getHomeId(), nodeInfo->getNodeId());
         std::string sNodeProductName = OpenZWave::Manager::Get()->GetNodeProductName(nodeInfo->getHomeId(), nodeInfo->getNodeId());
         std::string sNodeProductType = OpenZWave::Manager::Get()->GetNodeProductType(nodeInfo->getHomeId(), nodeInfo->getNodeId());
         std::string sNodeProductId = OpenZWave::Manager::Get()->GetNodeProductId(nodeInfo->getHomeId(), nodeInfo->getNodeId());
         std::string sNodeLocation = OpenZWave::Manager::Get()->GetNodeLocation(nodeInfo->getHomeId(), nodeInfo->getNodeId());
         std::string sNodeType = OpenZWave::Manager::Get()->GetNodeType(nodeInfo->getHomeId(), nodeInfo->getNodeId());
         std::string id = COpenZWaveHelpers::GenerateDeviceName(nodeInfo->getHomeId(), nodeInfo->getNodeId());

         YADOMS_LOG(debug) << "ZWave : NodeNaming : id = " << id;
         YADOMS_LOG(debug) << "ZWave : NodeNaming : name = " << sNodeName;
         YADOMS_LOG(debug) << "ZWave : NodeNaming : manufacturer = " << sNodeManufacturer;
         YADOMS_LOG(debug) << "ZWave : NodeNaming : productName = " << sNodeProductName;
         YADOMS_LOG(debug) << "ZWave : NodeNaming : productType = " << sNodeProductType;
         YADOMS_LOG(debug) << "ZWave : NodeNaming : productId = " << sNodeProductId;

         shared::CDataContainer d;
         d.set("name", id);

         if (sNodeName.empty())
            d.set("friendlyName", sNodeProductName);
         else
            d.set("friendlyName", sNodeName);

         shared::CDataContainer details;
         details.set("Manufacturer", sNodeManufacturer);
         details.set("Product", sNodeProductName);
         details.set("ProductId", sNodeProductId);
         details.set("ProductType", sNodeProductType);
         details.set("Location", sNodeLocation);
         details.set("Type", sNodeType);
         d.set("details", details);

         if (m_handler != NULL)
            m_handler->postEvent(CZWave::kDeclareDevice, d);

      }
      break;
   }


   case OpenZWave::Notification::Type_NodeRemoved:
   {
      uint32 const homeId = _notification->GetHomeId();
      uint8 const nodeId = _notification->GetNodeId();

      for (NodeListType::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
      {
         if ((*i)->match(homeId, nodeId))
         {
            m_nodes.erase(i);
            break;
         }
      }
      break;
   }

   case OpenZWave::Notification::Type_NodeEvent:
   {
      // We have received an event from the node, caused by a
      // basic_set or hail message.

      break;
   }

   case OpenZWave::Notification::Type_PollingDisabled:
   {
      break;
   }

   case OpenZWave::Notification::Type_PollingEnabled:
   {
      break;
   }

   case OpenZWave::Notification::Type_DriverReady:
   {
      if (m_handler != NULL)
         m_handler->postEvent<std::string>(CZWave::kInternalStateChange, EZWaveInteralState::kDriverReady);
      
      m_homeId = _notification->GetHomeId();
      break;
   }

   case OpenZWave::Notification::Type_DriverFailed:
   {
      if (m_handler != NULL)
         m_handler->postEvent<std::string>(CZWave::kInternalStateChange, EZWaveInteralState::kDriverFailed);

      m_initFailed = true;
      break;
   }

   case OpenZWave::Notification::Type_AwakeNodesQueried:
   case OpenZWave::Notification::Type_AllNodesQueried:
   case OpenZWave::Notification::Type_AllNodesQueriedSomeDead:
   {
      m_nodesQueried = true;
      break;
   }

   case OpenZWave::Notification::Type_ControllerCommand:
      switch (_notification->GetEvent())
      {
      case OpenZWave::Driver::ControllerState_Normal:
         //str = "ControlerCommand - Normal";
         if (m_handler != NULL)
            m_handler->postEvent<std::string>(CZWave::kInternalStateChange, EZWaveInteralState::kRunning);
         break;
      case OpenZWave::Driver::ControllerState_Starting:
         //str = "ControllerComand - Starting";
         break;
      case OpenZWave::Driver::ControllerState_Cancel:
         //str = "ControllerCommand - Canceled";
         m_lastSuccessfullySentCommand = OpenZWave::Driver::ControllerCommand_None;
         if (m_handler != NULL)
            m_handler->postEvent<std::string>(CZWave::kInternalStateChange, EZWaveInteralState::kRunning);

         break;
      case OpenZWave::Driver::ControllerState_Error:
         switch (_notification->GetNotification())
         {
         case OpenZWave::Driver::ControllerError_None:
            break;
         case OpenZWave::Driver::ControllerError_ButtonNotFound:
            YADOMS_LOG(error) << "Command controller error : ButtonNotFound";
            break;
         case OpenZWave::Driver::ControllerError_NodeNotFound:
            YADOMS_LOG(error) << "Command controller error : NodeNotFound";
            break;
         case OpenZWave::Driver::ControllerError_NotBridge:
            YADOMS_LOG(error) << "Command controller error : NotBridge";
            break;
         case OpenZWave::Driver::ControllerError_NotSUC:
            YADOMS_LOG(error) << "Command controller error : NotSUC";
            break;
         case OpenZWave::Driver::ControllerError_NotSecondary:
            YADOMS_LOG(error) << "Command controller error : NotSecondary";
            break;
         case OpenZWave::Driver::ControllerError_IsPrimary:
            YADOMS_LOG(error) << "Command controller error : IsPrimary";
            break;
         case OpenZWave::Driver::ControllerError_NotFound:
            YADOMS_LOG(error) << "Command controller error : NotFound";
            break;
         case OpenZWave::Driver::ControllerError_Busy:
            YADOMS_LOG(error) << "Command controller error : Busy";
            break;
         case OpenZWave::Driver::ControllerError_Failed:
            YADOMS_LOG(error) << "Command controller error : Failed";
            break;
         case OpenZWave::Driver::ControllerError_Disabled:
            YADOMS_LOG(error) << "Command controller error : Disabled";
            break;
         case OpenZWave::Driver::ControllerError_Overflow:
            YADOMS_LOG(error) << "Command controller error : Overflow";
            break;
         }
         break;
      case OpenZWave::Driver::ControllerState_Waiting:
         //str = "ControllerCommand - Waiting";
         if (m_handler != NULL)
         {
            switch (m_lastSuccessfullySentCommand)
            {
            case OpenZWave::Driver::ControllerCommand_AddDevice:
               m_handler->postEvent<std::string>(CZWave::kInternalStateChange, EZWaveInteralState::kWaitingInclusion);
               break;
            case OpenZWave::Driver::ControllerCommand_RemoveDevice:
               m_handler->postEvent<std::string>(CZWave::kInternalStateChange, EZWaveInteralState::kWaitingExclusion);
               break;
            default:
               m_handler->postEvent<std::string>(CZWave::kInternalStateChange, EZWaveInteralState::kWaiting);
               break;
            }
         }
            
         break;
      case OpenZWave::Driver::ControllerState_Sleeping:
         //str = "ControllerCommand - Sleeping";
         break;
      case OpenZWave::Driver::ControllerState_InProgress:
         //str = "ControllerCommand - InProgress";
         break;
      case OpenZWave::Driver::ControllerState_Completed:
         //str = "ControllerCommand - Completed";
         m_lastSuccessfullySentCommand = OpenZWave::Driver::ControllerCommand_None;
         if (m_handler != NULL)
            m_handler->postEvent<std::string>(CZWave::kInternalStateChange, EZWaveInteralState::kCompleted);
         break;
      case OpenZWave::Driver::ControllerState_Failed:
         //str = "ControllerCommand - Failed";
         break;
      case OpenZWave::Driver::ControllerState_NodeOK:
         //str = "ControllerCommand - NodeOK";
         break;
      case OpenZWave::Driver::ControllerState_NodeFailed:
         break;
      }
      break;



   case OpenZWave::Notification::Type_DriverReset:
      break;

   case OpenZWave::Notification::Type_NodeQueriesComplete:
      break;

   default:
      break;

   }

}


void COpenZWaveController::retreiveOpenZWaveIds(const std::string & device, const std::string & keyword, uint32 & homeId, uint8 & nodeId, uint8 & instance)
{
   std::vector<std::string> splittedDevice;
   boost::split(splittedDevice, device, boost::is_any_of("."), boost::token_compress_on);
   if (splittedDevice.size() < 2)
   {
      throw shared::exception::CException("The device id is invalid : not matching pattern : <homeId>.<nodeId>");
   }
   homeId = boost::lexical_cast<uint32>(splittedDevice[0]);
   nodeId = static_cast<uint8>(atoi(splittedDevice[1].c_str())); //dont use lexical cast for uint8, because it realize a string to char conversion: "2" is transform in '2' = 0x32
   
   //instance <class>.<keyword>.<instance>
   std::vector<std::string> splittedKeyword;
   boost::split(splittedKeyword, keyword, boost::is_any_of("."), boost::token_compress_on);
   if (splittedKeyword.size() > 2)
      instance = static_cast<uint8>(atoi(splittedKeyword[2].c_str())); //dont use lexical cast for uint8, because it realize a string to char conversion: "2" is transform in '2' = 0x32
   else
      instance = 1; //default instance value in OpenZWave
}

void COpenZWaveController::sendCommand(const std::string & device, const std::string & keyword, const std::string & value)
{
   boost::lock_guard<boost::mutex> lock(m_treeMutex);

   uint32 homeId;
   uint8 nodeId;
   uint8 instance;
   ECommandClass keywordClass;

   retreiveOpenZWaveIds(device, keyword, homeId, nodeId, instance);

   boost::shared_ptr<COpenZWaveNode> node = getNode(homeId, nodeId);
   if (node)
   {
      if (!node->sendCommand(keyword, value))
      {
         throw shared::exception::CException( (boost::format("Fail to send command keyword[%1%] = %2% ") % keyword % value).str());
      }
   }
}

void COpenZWaveController::startInclusionMode()
{
   if (OpenZWave::Manager::Get()->AddNode(m_homeId))
   {
      m_lastSuccessfullySentCommand = OpenZWave::Driver::ControllerCommand_AddDevice;
   }
   else
   {
      YADOMS_LOG(error) << "Fail to start inclusion mode";
   }
}

void COpenZWaveController::startExclusionMode()
{
   if(OpenZWave::Manager::Get()->RemoveNode(m_homeId))
   {
      m_lastSuccessfullySentCommand = OpenZWave::Driver::ControllerCommand_RemoveDevice;
   }
   else
   {
      YADOMS_LOG(error) << "Fail to start exclusion mode";
   }

}

void COpenZWaveController::hardResetController()
{
   OpenZWave::Manager::Get()->ResetController(m_homeId);
}

void COpenZWaveController::softResetController()
{
   OpenZWave::Manager::Get()->SoftReset(m_homeId);
}

void COpenZWaveController::testNetwork(int count)
{
   OpenZWave::Manager::Get()->TestNetwork(m_homeId, count); //send count frames to each node
}

void COpenZWaveController::cancelCurrentCommand() 
{
   if(OpenZWave::Manager::Get()->CancelControllerCommand(m_homeId))
   {
      m_lastSuccessfullySentCommand = OpenZWave::Driver::ControllerCommand_None;
   }
   else
   {
      YADOMS_LOG(error) << "Fail to cancel last command";
   }
}

void COpenZWaveController::healNetwork()
{
   OpenZWave::Manager::Get()->HealNetwork(m_homeId, true);
}


