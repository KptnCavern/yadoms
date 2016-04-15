#include "stdafx.h"
#include "Distance.h"
#include <shared/plugin/yPluginApi/historization/Distance.h>

namespace historizers {

   CDistance::CDistance(const std::string & name, shared::plugin::yPluginApi::EKeywordAccessMode accessMode)
      : COpenZWaveSingleHistorizableData<double>(boost::shared_ptr< shared::plugin::yPluginApi::historization::CSingleHistorizableData<double> >(new shared::plugin::yPluginApi::historization::CDistance(name, accessMode)))
   {

   }

   CDistance::~CDistance()
   {
   }

   // COpenZWaveSingleHistorizableData<Poco::Int64> override ------------------------
   void CDistance::setWithUnits(double value, const std::string & unit)
   {
      if (unit == "cm")
         set(value / 100.0);
      else if (unit == "ft")
         set(value / 3.280839895);
      else
         set(value);
   }

   double CDistance::getWithUnits(const std::string & unit) const
   {
      if (unit == "cm")
         return get() * 100.0;
      if (unit == "ft")
         return get() * 3.280839895;
      return get();
   }
   // [END] - COpenZWaveSingleHistorizableData<double> override ------------------------



} //namespace historizers 