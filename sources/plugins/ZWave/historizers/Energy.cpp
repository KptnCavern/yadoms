#include "stdafx.h"
#include "Energy.h"
#include <shared/plugin/yPluginApi/historization/Energy.h>

namespace historizers {

   CEnergy::CEnergy(const std::string & name, shared::plugin::yPluginApi::EKeywordAccessMode accessMode)
      : COpenZWaveSingleHistorizableData<double>(boost::shared_ptr< shared::plugin::yPluginApi::historization::CSingleHistorizableData<double> >(new shared::plugin::yPluginApi::historization::CEnergyDouble(name, accessMode)))
   {
   }

   CEnergy::CEnergy(const std::string & name, shared::plugin::yPluginApi::EKeywordAccessMode accessMode, shared::plugin::yPluginApi::historization::EMeasureType measureType)
      : COpenZWaveSingleHistorizableData<double>(boost::shared_ptr< shared::plugin::yPluginApi::historization::CSingleHistorizableData<double> >(new shared::plugin::yPluginApi::historization::CEnergyDouble(name, accessMode, measureType)))
   {

   }

   CEnergy::~CEnergy()
   {
   }

} //namespace historizers 