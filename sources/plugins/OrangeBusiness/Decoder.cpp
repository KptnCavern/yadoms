#include "stdafx.h"
#include "Decoder.h"
#include <shared/Log.h>
#include "DefaultEquipment.h"
#include "UnauthorizedException.hpp"

CDecoder::CDecoder()
{}

CDecoder::~CDecoder()
{}

std::map<std::string, boost::shared_ptr<equipments::IEquipment>> CDecoder::decodeDevicesMessage(boost::shared_ptr<yApi::IYPluginApi> api,
                                    shared::CDataContainer& message)
{
	std::map<std::string, boost::shared_ptr<equipments::IEquipment>> equipmentList;
   message.printToLog(YADOMS_LOG(trace));

   auto errorcode = message.getWithDefault<std::string>("code","");

   if (errorcode == "UNAUTHORIZED")
      throw CUnauthorizedException(message.get<std::string>("message"));

   if (errorcode != "")
      throw shared::exception::CException(message.get<std::string>("message"));

   auto equipments = message.get<std::vector<shared::CDataContainer> >("data");
   std::vector<shared::CDataContainer>::iterator equipmentIterator;

   for (equipmentIterator = equipments.begin(); equipmentIterator != equipments.end(); ++equipmentIterator)
   {
      std::string name = (*equipmentIterator).get<std::string>("name");
      std::string devEUI = (*equipmentIterator).get<std::string>("devEUI");
      boost::shared_ptr<equipments::CDefaultEquipment> newEquipment(boost::make_shared<equipments::CDefaultEquipment>(name, devEUI, api));
	   equipmentList.insert(std::pair<std::string, boost::shared_ptr<equipments::IEquipment>>(name, newEquipment));
      YADOMS_LOG(information) << "create device name = " << name << " devEUI = " << devEUI;
   }

   return equipmentList;
}

bool CDecoder::isFrameComplete(shared::CDataContainer& message)
{
   int page = message.get<int>("page");
   int pageSize = message.get<int>("size");

   if (message.get<int>("totalCount") > (page * pageSize))
      return true;
   else
      return false;
}

shared::CDataContainer CDecoder::getLastData(shared::CDataContainer& message)
{
   auto commands = message.get<std::vector<shared::CDataContainer> >("data");
   shared::CDataContainer response;

   if (commands.size() > 0)
   {
      // // the lastest elements should be the first one presented
      response.set("data", commands[0].get<std::string>("data"));
	   response.set("date", commands[0].get<std::string>("creationTs"));
      response.set("id", commands[0].get<std::string>("id"));
   }
   return response;
}