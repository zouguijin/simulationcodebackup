
// test1-strategy.hpp

#ifndef NDNSIM_TEST1_STRATEGY_HPP
#define NDNSIM_TEST1_STRATEGY_HPP

#include "face/face.hpp"
#include "fw/strategy.hpp"
#include "fw/algorithm.hpp"
#include <map>
#include <chrono>
#include <vector>



namespace nfd {
	namespace fw {
		class Test1Strategy : public Strategy {
		public:
			Test1Strategy(Forwarder& forwarder, const Name& name = getStrategyName());

			virtual
			~Test1Strategy() override;

			virtual void
			afterReceiveInterest(const Face& inFace, const Interest& interest,
									const shared_ptr<pit::Entry>& pitEntry) override;

			virtual void
			beforeSatisfyInterest(const shared_ptr<pit::Entry>& pitEntry,
                                      const Face& inFace, const Data& data);

			static const Name&
			getStrategyName();

			//------2018-08-08---------
			void
			setMmtEntry(const Face& outface, const shared_ptr<pit::Entry>& pitEntry);			

			static void 
			getcurrentTime();

			static std::string
			getOutFaceUri(std::string originStr);

			// ------------------------

			// ----------2018-08-14------------
			std::string
			getMostContentPrefix(const Face& outface);

			static std::chrono::system_clock::time_point
			// static void
			getCurrentTimePoint();

			bool
			isCongested(const Face& outFace, const Data& data); 

			bool
			isFaceCounterTimeOut(const Face& outFace); 

			void
			init();
			// {
				// Face Uri: "07" "0a" "0c" "0e"
				// 07LinkCapacity: 10000000 bps 0aLinkCapacity: 3000000 bps 
				// 0cLinkCapacity: 1000000  bps 0eLinkCapacity: 5000000 bps
				// FACECOUNTER_DURATION = 2s
			// }

			// --------------------------------


		private:
			int counter;
			static const time::nanoseconds MEASUREMENTS_LIFETIME;

			//------2018-08-14---------
			// ------global parameter--------
			// 4 parameters to store link capacity according to Face Uri:
			// 07LinkCapacity 0aLinkCapacity 0cLinkCapacity 0eLinkCapacity
			// 
			// 4 parameters to store InData's amount according to Face Uri: (InData - xxInData == diff)
			// 07InDataCounter 0aInDataCounter 0cInDataCounter 0eInDataCounter
			std::map<std::string, std::vector<int>> mapFaceLinkData;
			// 4 Time Point according to Face Uri: 
			// 07TimePoint 0aTimePoint 0cTimePoint 0eTimePoint
			// WHEN now() - timePoint == duration, THEN flowRate = diff*Data_Size / duration (bit/s)
			std::map<std::string, std::chrono::system_clock::time_point> mapFaceTimePoint;

			std::vector<std::string> prefixVec;

			int FACECOUNTER_DURATION;
			int DATA_SIZE;

			// -------------------------
		


		};
	}
}

#endif // NDNSIM_TEST1_STRATEGY_HPP
