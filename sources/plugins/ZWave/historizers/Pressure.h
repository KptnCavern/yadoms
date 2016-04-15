#include "stdafx.h"
#include "../OpenZWaveSingleHistorizableData.h"

namespace historizers {

   class CPressure : public COpenZWaveSingleHistorizableData<double>
   {
   public:
      //-----------------------------------------------------
      ///\brief                     Constructor
      ///\param[in] keywordName     Yadoms keyword name
      ///\param[in] accessMode      The access mode
      ///\param[in] measureType     The measure type (normally kAbsolute)
      //-----------------------------------------------------
      CPressure(const std::string & name, shared::plugin::yPluginApi::EKeywordAccessMode accessMode);

      //-----------------------------------------------------
      ///\brief                     Constructor
      ///\param[in] keywordName     Yadoms keyword name
      ///\param[in] accessMode      The access mode
      ///\param[in] measureType     The measure type (normally kAbsolute)
      //-----------------------------------------------------
      CPressure(const std::string & name, shared::plugin::yPluginApi::EKeywordAccessMode accessMode, shared::plugin::yPluginApi::historization::EMeasureType measureType);

      //-----------------------------------------------------
      ///\brief                     Destructor
      //-----------------------------------------------------
      virtual ~CPressure();

      // COpenZWaveSingleHistorizableData<Poco::Int64> override ------------------------
      virtual void setWithUnits(double value, const std::string & unit);
      virtual double getWithUnits(const std::string & unit) const;
      // [END] - COpenZWaveSingleHistorizableData<double> override ------------------------
   };

} //namespace historizers 