#include "stdafx.h"
#include "MemoryLoad.h"
#include <shared/exception/Exception.hpp>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <shared/DataContainer.h>

#define NB_LINE_TO_PARSE 10

CMemoryLoad::CMemoryLoad(const std::string& keywordName)
   : m_keyword(boost::make_shared<yApi::historization::CLoad>(keywordName))
{
}

CMemoryLoad::~CMemoryLoad()
{
}

void CMemoryLoad::ReadFromFile(unsigned long long *dmemTotal,
                               unsigned long long *dmemFree,
                               unsigned long long *dbuffer,
                               unsigned long long *dcached)
{
   // Enum type, declare keys labels
   static const shared::CDataContainer::EnumValuesNames EEnumTypeNames = boost::assign::map_list_of
      ("MemTotal", kmemTotal)
      ("MemFree" , kmemFree)
      ("Buffers" , kbuffer)
      ("Cached"  , kcached);

   std::ifstream procFile("/proc/meminfo");
   
   boost::regex reg("^(.*): *(\\d+)");

   std::string line;
   unsigned long long memTab[4];
   unsigned char counter = 0;

   while ( std::getline(procFile, line) && (counter < NB_LINE_TO_PARSE) ) 
   {
      boost::smatch match;
      if ( boost::regex_search( line, match, reg ) ) 
      {
         shared::CDataContainer::EnumValuesNames::const_iterator it = EEnumTypeNames.find(boost::lexical_cast<std::string>(match[1]));

         if (it != EEnumTypeNames.end())
            memTab[ it->second ]  = boost::lexical_cast<long long>(match[2]);
      }
      counter++;
   }

   *dmemTotal = memTab[ 0 ];
   *dmemFree  = memTab[ 1 ];
   *dbuffer   = memTab[ 2 ];
   *dcached   = memTab[ 3 ];

   procFile.close();
}

void CMemoryLoad::read()
{
   unsigned long long memTotal  =0;
   unsigned long long memFree   =0;
   unsigned long long memBuffer =0;
   unsigned long long memCached =0;

   ReadFromFile ( &memTotal, &memFree, &memBuffer, &memCached );

   // as described here :
   // http://blog.guillaume.fenollar.fr/2013/11/comprendre-conso-memoire-vive-ram-linux.html

   std::cout << "memTotal :" << memTotal << std::endl;
   std::cout << "memFree  :" << memFree << std::endl;
   std::cout << "memBuffer:" << memBuffer << std::endl;
   std::cout << "memCached:" << memCached << std::endl;

   float MemoryLoad = static_cast<float>(((memTotal-memFree)-memBuffer-memCached) * 100 / (float) memTotal );

   m_keyword->set(MemoryLoad);

   std::cout << "Memory Load : " << m_keyword->formatValue() << std::endl;
}
