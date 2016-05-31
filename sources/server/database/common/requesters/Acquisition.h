#pragma once

#include "server/database/IAcquisitionRequester.h"
#include "server/database/IDataProvider.h"
#include "server/database/IDatabaseRequester.h"

namespace database { namespace common { namespace requesters { 

   class CKeyword;

   //--------------------------------------------------------------
   /// \Brief		   Acquisition requester for SQLite database
   //--------------------------------------------------------------
   class CAcquisition: public IAcquisitionRequester
   {
   public:
      //--------------------------------------------------------------
      /// \Brief		   Constructor
      /// \param [in]	pDatabaseHandler: the database handler
      //--------------------------------------------------------------
      CAcquisition(boost::shared_ptr<IDatabaseRequester> databaseRequester, boost::shared_ptr<CKeyword> keywordRequester);

      //--------------------------------------------------------------
      /// \Brief		   Destructor
      //--------------------------------------------------------------
      virtual ~CAcquisition();

      // IAcquisitionRequester implementation
      virtual boost::shared_ptr<entities::CAcquisition> saveData(const int keywordId, const std::string & data, boost::posix_time::ptime & dataTime);
      virtual boost::shared_ptr<entities::CAcquisition> incrementData(const int keywordId, const std::string & increment, boost::posix_time::ptime & dataTime);
      virtual boost::shared_ptr<entities::CAcquisitionSummary> saveSummaryData(const int keywordId, database::entities::EAcquisitionSummaryType curType, boost::posix_time::ptime & dataTime);
      virtual void getKeywordsHavingDate(const boost::posix_time::ptime & timeFrom, const boost::posix_time::ptime & timeTo, std::vector<int> & results);
      virtual bool summaryDataExists(const int keywordId, database::entities::EAcquisitionSummaryType curType, boost::posix_time::ptime & date);
      virtual void removeKeywordData(const int keywordId);
      virtual boost::shared_ptr<entities::CAcquisition> getAcquisitionByKeywordAndDate(const int keywordId, boost::posix_time::ptime time);
         virtual boost::shared_ptr<entities::CAcquisition> getKeywordLastData(const int keywordId, bool throwIfNotExists = true);
      virtual std::vector<boost::tuple<boost::posix_time::ptime, std::string>  > getKeywordData(int keywordId, boost::posix_time::ptime timeFrom, boost::posix_time::ptime timeTo);
      virtual std::vector<boost::shared_ptr<entities::CAcquisitionSummary> > getKeywordDataByDay(int keywordId, boost::posix_time::ptime timeFrom, boost::posix_time::ptime timeTo);
      virtual std::vector<boost::shared_ptr<entities::CAcquisitionSummary> > getKeywordDataByHour(int keywordId, boost::posix_time::ptime timeFrom, boost::posix_time::ptime timeTo);
      virtual std::vector<boost::shared_ptr<entities::CAcquisitionSummary> > getKeywordDataByMonth(int keywordId, boost::posix_time::ptime timeFrom, boost::posix_time::ptime timeTo);
      virtual std::vector<boost::shared_ptr<entities::CAcquisitionSummary> > getKeywordDataByYear(int keywordId, boost::posix_time::ptime timeFrom, boost::posix_time::ptime timeTo);
      virtual std::string getKeywordHighchartData(int keywordId, boost::posix_time::ptime timeFrom, boost::posix_time::ptime timeTo);
      virtual std::string getKeywordHighchartDataByHour(int keywordId, boost::posix_time::ptime timeFrom, boost::posix_time::ptime timeTo);
      virtual std::string getKeywordHighchartDataByDay(int keywordId, boost::posix_time::ptime timeFrom, boost::posix_time::ptime timeTo);
      virtual std::string getKeywordHighchartDataByMonth(int keywordId, boost::posix_time::ptime timeFrom, boost::posix_time::ptime timeTo);
      virtual std::string getKeywordHighchartDataByYear(int keywordId, boost::posix_time::ptime timeFrom, boost::posix_time::ptime timeTo);
      virtual int purgeAcquisitions(boost::posix_time::ptime purgeDate);
      // [END] IAcquisitionRequester implementation

   private:
      //--------------------------------------------------------------
      /// \brief                    Get the data  by type (avg, min, max)
      /// \param [in] keywordId     keywordId Id
      /// \param [in] timeFrom      The start date (optionnal)
      /// \param [in] timeTo        The end date (optionnal)
      /// \return                   CAcquisitionSummary data
      /// \throw                    CInvalidParameter if deviceId is unknown
      //--------------------------------------------------------------
      std::vector<boost::shared_ptr<entities::CAcquisitionSummary > > getKeywordSummaryDataByType(const entities::EAcquisitionSummaryType & type, int keywordId, boost::posix_time::ptime timeFrom, boost::posix_time::ptime timeTo);
      //--------------------------------------------------------------
      /// \brief                    Get the data  by type (avg, min, max)
      /// \param [in] keywordId     keywordId Id
      /// \param [in] timeFrom      The start date (optionnal)
      /// \param [in] timeTo        The end date (optionnal)
      /// \return                   CAcquisitionSummary data
      /// \throw                    CInvalidParameter if deviceId is unknown
      //--------------------------------------------------------------
      std::vector<boost::shared_ptr<entities::CAcquisitionSummary > > getKeywordSummaryDataByType(const entities::EAcquisitionSummaryType & type, int keywordId, const Poco::DateTime & timeFrom, const Poco::DateTime & timeTo);

      //--------------------------------------------------------------
      /// \brief                    Get the summary data (highchart js format) : [[date,value],[date,value],...] by acquisition type
      /// \param [in] type          Type of data to select
      /// \param [in] keywordId     keywordId Id
      /// \param [in] timeFrom      The start date (optionnal)
      /// \param [in] timeTo        The end date (optionnal)
      /// \return                   Map of data : (date, value). One value per hour
      /// \throw                    CInvalidParameter if deviceId is unknown
      //--------------------------------------------------------------
      std::string getKeywordHighchartDataByType(const entities::EAcquisitionSummaryType & type, int keywordId, boost::posix_time::ptime timeFrom, boost::posix_time::ptime timeTo);

      //--------------------------------------------------------------
      /// \Brief		   Pointer to keyword requester
      //--------------------------------------------------------------
      boost::shared_ptr<CKeyword> m_keywordRequester;

      //--------------------------------------------------------------
      /// \Brief		   Reference to IDatabaseRequester
      //--------------------------------------------------------------
      boost::shared_ptr<IDatabaseRequester> m_databaseRequester;
   };

} } } //namespace database::common::requesters

