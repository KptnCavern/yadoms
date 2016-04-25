﻿#include "stdafx.h"
#include "DateTimeNotifier.h"
#include <shared/Log.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Timezone.h>
#include <Poco/LocalDateTime.h>
#include <Poco/Util/TimerTaskAdapter.h>
#include "notification/Helpers.hpp"

#include "notification/Helpers.hpp"
#include <shared/dateTime/DateTimeContainer.h>



namespace dateTime {
   
   Poco::Timespan CDateTimeNotifier::m_oneMinuteOffset(0, 0, 1, 0, 0);

   CDateTimeNotifier::CDateTimeNotifier()
   {
      //create the summary computing task
      m_dateTimeTask = new Poco::Util::TimerTaskAdapter<CDateTimeNotifier>(*this, &dateTime::CDateTimeNotifier::taskRun);
   }

   CDateTimeNotifier::~CDateTimeNotifier()
   {
      //stop();
   }
   
   void CDateTimeNotifier::start()
   {
      //iniitalize the current minute (with second=0 and millisecond=0)
      Poco::DateTime now;
      m_nextMinute = Poco::DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute());
      //schedule next minute
      scheduleOnNextMinute();
   }

   void CDateTimeNotifier::scheduleOnNextMinute()
   {
      //in case of system date/time change, then reset the nextMinute to match systemDateTime
      Poco::DateTime now;
      if (abs((now - m_nextMinute).totalSeconds()) > 10)
         m_nextMinute = Poco::DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute());

      //reschedule the task
      m_nextMinute += m_oneMinuteOffset;//+1minute

      YADOMS_LOG(information) << "DateTimeNotifier : schedule the task @" << Poco::DateTimeFormatter::format(Poco::LocalDateTime(m_nextMinute), "%H:%M:%S.%i");
      m_dateTimeTimer.schedule(m_dateTimeTask, m_nextMinute.timestamp());
   }


   void CDateTimeNotifier::taskRun(Poco::Util::TimerTask& task)
   {
      //use try catch, to allow rescheduling in case of any problem
      try
      {
         //do the job
         boost::shared_ptr<shared::dateTime::CDateTimeContainer> notif(new shared::dateTime::CDateTimeContainer(m_nextMinute));
         notification::CHelpers::postNotification(notif);

         if (!task.isCancelled())
         {
            //re-schedule next minute
            scheduleOnNextMinute();
         }
      }
      catch (...)
      {
         //re-schedule next minute
         scheduleOnNextMinute();
      }
   }


   void CDateTimeNotifier::stop()
   {
      m_dateTimeTimer.cancel(true);
      if(m_dateTimeTask)
         m_dateTimeTask->cancel();
   }
} //namespace dateTime