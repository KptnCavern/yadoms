#include "stdafx.h"
#include "CustomEnumHistorizer.h"
#include <shared/plugin/yPluginApi/KeywordAccessMode.h>
#include <shared/plugin/yPluginApi/StandardUnits.h>
#include <shared/plugin/yPluginApi/StandardCapacity.h>
#include "CustomEnumTypeInfo.h"


namespace pluginSystem
{
   namespace internalPlugin
   {
      DECLARE_CAPACITY(CustomEnumCapacity, "CustomEnum", yApi::CStandardUnits::NoUnit(), yApi::EKeywordDataType::kEnum);


      CCustomEnumHistorizer::CCustomEnumHistorizer(const std::string& keywordName,
                                                   const shared::plugin::yPluginApi::EKeywordAccessMode& keywordAccessMode,
                                                   const std::vector<std::string>& values)
         : m_keywordName(keywordName),
           m_keywordAccessMode(keywordAccessMode),
           m_typeInfo(CCustomEnumTypeInfo(CustomEnumCapacity().getName(),
                                          values)),
           m_value(values.front())
      {
      }

      CCustomEnumHistorizer::~CCustomEnumHistorizer()
      {
      }

      const std::string& CCustomEnumHistorizer::getKeyword() const
      {
         return m_keywordName;
      }

      const yApi::CStandardCapacity& CCustomEnumHistorizer::getCapacity() const
      {
         return CustomEnumCapacity();
      }

      const yApi::EKeywordAccessMode& CCustomEnumHistorizer::getAccessMode() const
      {
         return m_keywordAccessMode;
      }

      std::string CCustomEnumHistorizer::formatValue() const
      {
         return shared::CStringExtension::cultureInvariantToString(m_value);
      }

      const yApi::historization::EMeasureType& CCustomEnumHistorizer::getMeasureType() const
      {
         return yApi::historization::EMeasureType::kAbsolute;
      }

      shared::CDataContainer CCustomEnumHistorizer::getTypeInfo() const
      {
         return m_typeInfo.serialize();
      }
   }
} // namespace pluginSystem::internalPlugin


