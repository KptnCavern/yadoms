#include "stdafx.h"
#include "OpenZWaveHelpers.h"
#include <Manager.h>
#include "OpenZWaveCommandClass.h"

std::string COpenZWaveHelpers::GenerateKeywordName(OpenZWave::ValueID& value)
{
   ECommandClass commandClass(static_cast<int>(value.GetCommandClassId()));
   auto vLabel = OpenZWave::Manager::Get()->GetValueLabel(value);
   return (boost::format("%1%.%2%.%3%") % commandClass.toString() % vLabel % static_cast<int>(value.GetInstance())).str();
}

std::string COpenZWaveHelpers::GenerateDeviceName(Poco::UInt32 homeId,
                                                  Poco::UInt8 nodeId)
{
   return (boost::format("%1%.%2%") % homeId % static_cast<int>(nodeId)).str();
}

