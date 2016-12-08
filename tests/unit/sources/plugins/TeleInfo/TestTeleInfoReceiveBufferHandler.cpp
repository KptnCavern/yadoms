#include "stdafx.h"
#include <boost/test/unit_test.hpp>

// Includes needed to compile tested classes
#include "../../../../sources/plugins/TeleInfo/TeleInfoReceiveBufferHandler.h"
#include <../../../../sources/shared/shared/communication/Buffer.hpp>
#include <../../../../sources/shared/shared/communication/IBufferLogger.h>

#include "../../mock/shared/currentTime/DefaultCurrentTimeMock.h"

// Includes needed to compile the test
#include "../../testCommon/fileSystem.h"

enum
{
   kSTX = 0x02,
   kETX = 0x03,

   kStartMessage = 0x0a,
   kEndMessage = 0x0d
};

class BufferLoggerMock : public shared::communication::IBufferLogger
{
public:
	explicit BufferLoggerMock() {}
	virtual ~BufferLoggerMock(){}
	void logReceived(const shared::communication::CByteBuffer& data) {}
	void logSent(const shared::communication::CByteBuffer& data) {}
	std::string msgToString(const shared::communication::CByteBuffer& data) const { return ""; }
};

// A Mock just to gain public visibility of normally protected methods
class CTeleInfoReceiveBufferHandlerMock : public CTeleInfoReceiveBufferHandler
{
public:
   CTeleInfoReceiveBufferHandlerMock(shared::event::CEventHandler& receiveDataEventHandler,
                                     int receiveDataEventId,
                                     const boost::posix_time::time_duration suspendDelay,
									 boost::shared_ptr<shared::communication::IBufferLogger> logger)
      : CTeleInfoReceiveBufferHandler(receiveDataEventHandler,
                                      receiveDataEventId,
                                      suspendDelay,
									  logger)
   {
   }

   static bool isCheckSumOk(const std::string& message)
   {
      return CTeleInfoReceiveBufferHandler::isCheckSumOk(message);
   }

   static  boost::shared_ptr<std::map<std::string, std::string>> getMessages(boost::shared_ptr<const std::vector<unsigned char>> frame)
   {
	   return CTeleInfoReceiveBufferHandler::getMessages(frame);
   }


};

std::string toMessage(const std::string& content)
{
   std::string message;
   message.push_back(kStartMessage);
   for (const auto car:content)
      message.push_back(car);
   message.push_back(kEndMessage);
   return message;
}

std::vector<unsigned char> normalizeFrame(const std::string& content)
{
   auto outString = content;
   boost::replace_all(outString, "<etx>", std::string(1, kETX));
   boost::replace_all(outString, "<stx>", std::string(1, kSTX));
   boost::replace_all(outString, "<lf>", std::string(1, kStartMessage));
   boost::replace_all(outString, "<cr>", std::string(1, kEndMessage));

   std::vector<unsigned char> outVector;
   for (const auto car : outString)
      outVector.push_back(static_cast<unsigned char>(car));

   return outVector;
}

BOOST_AUTO_TEST_SUITE(TestTeleInfoReceiveBufferHandler)

   BOOST_AUTO_TEST_CASE(CheckCrc)
   {
      shared::event::CEventHandler eventHandler;
      CTeleInfoReceiveBufferHandlerMock bufferHandler(eventHandler,
                                                      shared::event::kUserFirstId,
                                                      boost::posix_time::seconds(30),
													  boost::make_shared<BufferLoggerMock>());


      // Too small message
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk(""), false);
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk("1"), false);
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk("12"), false);
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk("123"), false);
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk("123"), false);
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk(std::string{ kStartMessage, ' ', kEndMessage }), false);

      // Empty message
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk(std::string{ kStartMessage, ' ', 0x20, kEndMessage }), true);
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk(std::string{ kStartMessage, ' ', 0x21, kEndMessage }), false);

      // Message OK
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk(toMessage("OPTARIF BASE 0")), true);
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk(toMessage("MOTDETAT 000000 B")), true);
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk(toMessage("ADCO 031428097115 @")), true);
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk(toMessage("PTEC TH.. $")), true);

      // Wrong CRC
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk(toMessage("OPTARIF BASE 1")), false);
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk(toMessage("MOTDETAT 000000 Z")), false);
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk(toMessage("ADCO 031428097115 5")), false);
      BOOST_CHECK_EQUAL(bufferHandler.isCheckSumOk(toMessage("PTEC TH.. @")), false);
   }

BOOST_AUTO_TEST_CASE(getMessages)
{
	const boost::shared_ptr<const std::vector<unsigned char>> frame = boost::make_shared<const std::vector<unsigned char>>(normalizeFrame("5 F<cr><lf>PAPP 00490 .<cr><lf>MOTDETAT 000000 B<cr><etx><stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006251693 +<cr><lf>PTEC TH.. $<cr><lf>IINST 002 Y<cr><lf>IMAX 025 F<cr><lf>PAPP 00490 .<cr><lf>MOTDETAT 000000 B<cr><etx>"));
	const std::map<std::string, std::string> expectedMap = {
		{ "ADCO", "031428097115" },
		{ "OPTARIF", "BASE" },
		{ "ISOUSC", "30" },
		{ "BASE", "006251693" },
		{ "PTEC", "TH.." },
		{ "IINST", "002" },
		{ "IMAX", "025" },
		{ "PAPP", "00490" },
		{ "MOTDETAT", "000000" } };

	shared::event::CEventHandler evtHandler;
	CTeleInfoReceiveBufferHandlerMock bufferHandler(evtHandler,
												    shared::event::kUserFirstId,
												    boost::posix_time::seconds(0),
													boost::make_shared<BufferLoggerMock>());

	const auto out = bufferHandler.getMessages(frame);
	BOOST_CHECK_EQUAL(*out == expectedMap, true);
}

   BOOST_AUTO_TEST_CASE(Nominal)
   {
      const auto frame = normalizeFrame("<stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx>");
      const std::map<std::string, std::string> expectedMap = {
         { "ADCO", "031428097115" },
         { "OPTARIF", "BASE" },
         { "ISOUSC", "30" },
         { "BASE", "006238747" },
         { "PTEC", "TH.." },
         { "IINST", "008" },
         { "IMAX", "025" },
         { "PAPP", "01940" },
         { "MOTDETAT", "000000" } };

      shared::event::CEventHandler evtHandler;
      CTeleInfoReceiveBufferHandler bufferHandler(evtHandler,
                                                  shared::event::kUserFirstId,
                                                  boost::posix_time::seconds(0),
												  boost::make_shared<BufferLoggerMock>());
      bufferHandler.push(shared::communication::CByteBuffer(frame));
      BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kUserFirstId);
      const auto out = evtHandler.getEventData<boost::shared_ptr<std::map<std::string, std::string>>>();
      BOOST_CHECK_EQUAL(*out == expectedMap, true);
   }

   BOOST_AUTO_TEST_CASE(twoframesOnePush)
   {
	   const auto frame = normalizeFrame("<stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx><stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx>");
	   const std::map<std::string, std::string> expectedMap = {
		   { "ADCO", "031428097115" },
		   { "OPTARIF", "BASE" },
		   { "ISOUSC", "30" },
		   { "BASE", "006238747" },
		   { "PTEC", "TH.." },
		   { "IINST", "008" },
		   { "IMAX", "025" },
		   { "PAPP", "01940" },
		   { "MOTDETAT", "000000" } };

	   shared::event::CEventHandler evtHandler;
	   CTeleInfoReceiveBufferHandler bufferHandler(evtHandler,
												   shared::event::kUserFirstId,
												   boost::posix_time::seconds(0),
												   boost::make_shared<BufferLoggerMock>());
	   bufferHandler.push(shared::communication::CByteBuffer(frame));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kUserFirstId);
	   const auto out = evtHandler.getEventData<boost::shared_ptr<std::map<std::string, std::string>>>();
	   BOOST_CHECK_EQUAL(*out == expectedMap, true);
   }

   BOOST_AUTO_TEST_CASE(twoframesTwoPush)
   {
	   const auto frame1 = normalizeFrame("<stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx><stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx>");
	   const std::map<std::string, std::string> expectedMap = {
		   { "ADCO", "031428097115" },
		   { "OPTARIF", "BASE" },
		   { "ISOUSC", "30" },
		   { "BASE", "006238747" },
		   { "PTEC", "TH.." },
		   { "IINST", "008" },
		   { "IMAX", "025" },
		   { "PAPP", "01940" },
		   { "MOTDETAT", "000000" } };

	   shared::event::CEventHandler evtHandler;
	   CTeleInfoReceiveBufferHandler bufferHandler(evtHandler,
												   shared::event::kUserFirstId,
												   boost::posix_time::seconds(0),
												   boost::make_shared<BufferLoggerMock>());
	   bufferHandler.push(shared::communication::CByteBuffer(frame1));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kUserFirstId);
	   const auto out = evtHandler.getEventData<boost::shared_ptr<std::map<std::string, std::string>>>();
	   BOOST_CHECK_EQUAL(*out == expectedMap, true);
	   bufferHandler.push(shared::communication::CByteBuffer(frame1));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kUserFirstId);
	   const auto out1 = evtHandler.getEventData<boost::shared_ptr<std::map<std::string, std::string>>>();
	   BOOST_CHECK_EQUAL(*out1 == expectedMap, true);
   }

   BOOST_AUTO_TEST_CASE(extraCharactersbetweenCRetx)
   {                                                                                                                                                                                                                                          //*******//
	   const auto frame = normalizeFrame("<stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr>D4R } $<etx>");

	   shared::event::CEventHandler evtHandler;
	   CTeleInfoReceiveBufferHandler bufferHandler(evtHandler,
												   shared::event::kUserFirstId,
												   boost::posix_time::seconds(0),
												   boost::make_shared<BufferLoggerMock>());
	   bufferHandler.push(shared::communication::CByteBuffer(frame));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kNoEvent);
   }

   BOOST_AUTO_TEST_CASE(extraCharactersbetweenstxLF)
   {                                         //*******//                                                                                                                                                                                                
	   const auto frame = normalizeFrame("<stx>D4R } $<lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx>");
	   const std::map<std::string, std::string> expectedMap = {
		   { "ADCO", "031428097115" },
		   { "OPTARIF", "BASE" },
		   { "ISOUSC", "30" },
		   { "BASE", "006238747" },
		   { "PTEC", "TH.." },
		   { "IINST", "008" },
		   { "IMAX", "025" },
		   { "PAPP", "01940" },
		   { "MOTDETAT", "000000" } };

	   shared::event::CEventHandler evtHandler;
	   CTeleInfoReceiveBufferHandler bufferHandler(evtHandler,
												   shared::event::kUserFirstId,
												   boost::posix_time::seconds(0),
												   boost::make_shared<BufferLoggerMock>());
	   bufferHandler.push(shared::communication::CByteBuffer(frame));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kNoEvent);
   }

   BOOST_AUTO_TEST_CASE(extraCharactersbeforestx)
   {                                    //******************************************************************************************************************************************************************************************************//                                                                                                                                                                                                
	   const auto frame = normalizeFrame("<lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx><stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx>");
	   const std::map<std::string, std::string> expectedMap = {
		   { "ADCO", "031428097115" },
		   { "OPTARIF", "BASE" },
		   { "ISOUSC", "30" },
		   { "BASE", "006238747" },
		   { "PTEC", "TH.." },
		   { "IINST", "008" },
		   { "IMAX", "025" },
		   { "PAPP", "01940" },
		   { "MOTDETAT", "000000" } };

	   shared::event::CEventHandler evtHandler;
	   CTeleInfoReceiveBufferHandler bufferHandler(evtHandler,
												   shared::event::kUserFirstId,
												   boost::posix_time::seconds(0),
												   boost::make_shared<BufferLoggerMock>());
	   bufferHandler.push(shared::communication::CByteBuffer(frame));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kUserFirstId);
	   const auto out = evtHandler.getEventData<boost::shared_ptr<std::map<std::string, std::string>>>();
	   BOOST_CHECK_EQUAL(*out == expectedMap, true);
   }

   BOOST_AUTO_TEST_CASE(extraCharactersbeforestx2)
   {
	   const auto frame = normalizeFrame("T 000000 B<cr><etx><stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006251729 +<cr><lf>PTEC TH.. $<cr><lf>IINST 002 Y<cr><lf>IMAX 025 F<cr><lf>PAPP 00460 +<cr><lf>MOTDETAT 000000 B<cr><etx><stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006251729 +<cr><lf>PTEC TH.. $<cr><lf>IINST 002 Y<cr><lf>IMAX 025 F<cr><lf>PAPP 00450 *<cr><lf>MOTDETAT 000000 B<cr><etx>");
	   const std::map<std::string, std::string> expectedMap = {
		   { "ADCO", "031428097115" },
		   { "OPTARIF", "BASE" },
		   { "ISOUSC", "30" },
		   { "BASE", "006251729" },
		   { "PTEC", "TH.." },
		   { "IINST", "002" },
		   { "IMAX", "025" },
		   { "PAPP", "00450" },
		   { "MOTDETAT", "000000" } };

	   shared::event::CEventHandler evtHandler;
	   CTeleInfoReceiveBufferHandler bufferHandler(evtHandler,
												   shared::event::kUserFirstId,
												   boost::posix_time::seconds(0),
												   boost::make_shared<BufferLoggerMock>());
	   bufferHandler.push(shared::communication::CByteBuffer(frame));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kUserFirstId);
	   const auto out = evtHandler.getEventData<boost::shared_ptr<std::map<std::string, std::string>>>();
	   BOOST_CHECK_EQUAL(*out == expectedMap, true);
   }

   BOOST_AUTO_TEST_CASE(OneCRCFailed)
   {                                                                                     //|// Here the CRC Error
	   const auto frame = normalizeFrame("<stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE Z<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx>");

	   shared::event::CEventHandler evtHandler;
	   CTeleInfoReceiveBufferHandler bufferHandler(evtHandler,
												   shared::event::kUserFirstId,
												   boost::posix_time::seconds(0),
												   boost::make_shared<BufferLoggerMock>());
	   bufferHandler.push(shared::communication::CByteBuffer(frame));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kNoEvent);
   }

   BOOST_AUTO_TEST_CASE(MissingCR)
   {                                                                                                                                                    //|// Missing CR
	   const auto frame = normalizeFrame("<stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx>");

	   shared::event::CEventHandler evtHandler;
	   CTeleInfoReceiveBufferHandler bufferHandler(evtHandler,
												   shared::event::kUserFirstId,
												   boost::posix_time::seconds(0),
												   boost::make_shared<BufferLoggerMock>());
	   bufferHandler.push(shared::communication::CByteBuffer(frame));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kNoEvent);
   }

   BOOST_AUTO_TEST_CASE(Multiframe)
   {
	   const auto frame1 = normalizeFrame("<stx><lf>ADCO 031428097115 @<cr><lf>OPTA");
	   const auto frame2 = normalizeFrame("RIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE ");
	   const auto frame3 = normalizeFrame("006238747 0<cr><lf>PTEC TH.. $<cr><lf>I");
	   const auto frame4 = normalizeFrame("INST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 0");
	   const auto frame5 = normalizeFrame("1940 /<cr><lf>MOTDETAT 000000 B<cr><etx>");

	   const std::map<std::string, std::string> expectedMap = {
		   { "ADCO", "031428097115" },
		   { "OPTARIF", "BASE" },
		   { "ISOUSC", "30" },
		   { "BASE", "006238747" },
		   { "PTEC", "TH.." },
		   { "IINST", "008" },
		   { "IMAX", "025" },
		   { "PAPP", "01940" },
		   { "MOTDETAT", "000000" } };

	   shared::event::CEventHandler evtHandler;
	   CTeleInfoReceiveBufferHandler bufferHandler(evtHandler,
												   shared::event::kUserFirstId,
												   boost::posix_time::seconds(0),
												   boost::make_shared<BufferLoggerMock>());

	   bufferHandler.push(shared::communication::CByteBuffer(frame1));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kNoEvent);
	   bufferHandler.push(shared::communication::CByteBuffer(frame2));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kNoEvent);
	   bufferHandler.push(shared::communication::CByteBuffer(frame3));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kNoEvent);
	   bufferHandler.push(shared::communication::CByteBuffer(frame4));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kNoEvent);
	   bufferHandler.push(shared::communication::CByteBuffer(frame5));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kUserFirstId);
	   const auto out = evtHandler.getEventData<boost::shared_ptr<std::map<std::string, std::string>>>();
	   BOOST_CHECK_EQUAL(*out == expectedMap, true);
   }

   BOOST_AUTO_TEST_CASE(final)
   {
	   const auto frame1 = normalizeFrame("<stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr>");
	   const auto frame2 = normalizeFrame("<etx>");
	   
	   const std::map<std::string, std::string> expectedMap = {
		   { "ADCO", "031428097115" },
		   { "OPTARIF", "BASE" },
		   { "ISOUSC", "30" },
		   { "BASE", "006238747" },
		   { "PTEC", "TH.." },
		   { "IINST", "008" },
		   { "IMAX", "025" },
		   { "PAPP", "01940" },
		   { "MOTDETAT", "000000" } };

	   shared::event::CEventHandler evtHandler;
	   CTeleInfoReceiveBufferHandler bufferHandler(evtHandler,
												   shared::event::kUserFirstId,
												   boost::posix_time::seconds(0),
												   boost::make_shared<BufferLoggerMock>());
	   bufferHandler.push(shared::communication::CByteBuffer(frame1));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kNoEvent);
	   bufferHandler.push(shared::communication::CByteBuffer(frame2));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kUserFirstId);
	   const auto out = evtHandler.getEventData<boost::shared_ptr<std::map<std::string, std::string>>>();
	   BOOST_CHECK_EQUAL(*out == expectedMap, true);
   }

   BOOST_AUTO_TEST_CASE(threeImmediateframesTwoPushdelay30s)
   {
	   shared::currentTime::Provider().setProvider(boost::make_shared<CDefaultCurrentTimeMock>());

	   const auto frame1 = normalizeFrame("<stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx><stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx>");
	   const std::map<std::string, std::string> expectedMap = {
		   { "ADCO", "031428097115" },
		   { "OPTARIF", "BASE" },
		   { "ISOUSC", "30" },
		   { "BASE", "006238747" },
		   { "PTEC", "TH.." },
		   { "IINST", "008" },
		   { "IMAX", "025" },
		   { "PAPP", "01940" },
		   { "MOTDETAT", "000000" } };

	   shared::event::CEventHandler evtHandler;
	   CTeleInfoReceiveBufferHandler bufferHandler(evtHandler,
												   shared::event::kUserFirstId,
												   boost::posix_time::seconds(30),
												   boost::make_shared<BufferLoggerMock>());
	   bufferHandler.push(shared::communication::CByteBuffer(frame1));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kUserFirstId);
	   const auto out = evtHandler.getEventData<boost::shared_ptr<std::map<std::string, std::string>>>();
	   BOOST_CHECK_EQUAL(*out == expectedMap, true);
	   bufferHandler.push(shared::communication::CByteBuffer(frame1));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kNoEvent);
	   bufferHandler.push(shared::communication::CByteBuffer(frame1));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kNoEvent);
   }

   BOOST_AUTO_TEST_CASE(threeframesat30sTwoPushdelay30s)
   {
	   auto timeMock = boost::make_shared<CDefaultCurrentTimeMock>();
	   shared::currentTime::Provider().setProvider(timeMock);

	   const auto frame1 = normalizeFrame("<stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx><stx><lf>ADCO 031428097115 @<cr><lf>OPTARIF BASE 0<cr><lf>ISOUSC 30 9<cr><lf>BASE 006238747 0<cr><lf>PTEC TH.. $<cr><lf>IINST 008 _<cr><lf>IMAX 025 F<cr><lf>PAPP 01940 /<cr><lf>MOTDETAT 000000 B<cr><etx>");
	   const std::map<std::string, std::string> expectedMap = {
		   { "ADCO", "031428097115" },
		   { "OPTARIF", "BASE" },
		   { "ISOUSC", "30" },
		   { "BASE", "006238747" },
		   { "PTEC", "TH.." },
		   { "IINST", "008" },
		   { "IMAX", "025" },
		   { "PAPP", "01940" },
		   { "MOTDETAT", "000000" } };

	   shared::event::CEventHandler evtHandler;
	   CTeleInfoReceiveBufferHandler bufferHandler(evtHandler,
												   shared::event::kUserFirstId,
												   boost::posix_time::seconds(30),
												   boost::make_shared<BufferLoggerMock>());
	   bufferHandler.push(shared::communication::CByteBuffer(frame1));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kUserFirstId);
	   const auto out = evtHandler.getEventData<boost::shared_ptr<std::map<std::string, std::string>>>();
	   BOOST_CHECK_EQUAL(*out == expectedMap, true);
	   timeMock->sleep(boost::posix_time::seconds(30));
	   bufferHandler.push(shared::communication::CByteBuffer(frame1));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kUserFirstId);
	   const auto out1 = evtHandler.getEventData<boost::shared_ptr<std::map<std::string, std::string>>>();
	   BOOST_CHECK_EQUAL(*out1 == expectedMap, true);
	   timeMock->sleep(boost::posix_time::seconds(30));
	   bufferHandler.push(shared::communication::CByteBuffer(frame1));
	   BOOST_CHECK_EQUAL(evtHandler.waitForEvents(boost::date_time::min_date_time), shared::event::kUserFirstId);
	   const auto out2 = evtHandler.getEventData<boost::shared_ptr<std::map<std::string, std::string>>>();
	   BOOST_CHECK_EQUAL(*out2 == expectedMap, true);
   }

   BOOST_AUTO_TEST_SUITE_END()