
// test1-strategy.hpp

#ifndef NDNSIM_TEST1_STRATEGY_HPP
#define NDNSIM_TEST1_STRATEGY_HPP

#include "face/face.hpp"
#include "fw/strategy.hpp"
#include "fw/algorithm.hpp"
#include <map>



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

			// --------------------------------


		private:
			int counter;
			static const time::nanoseconds MEASUREMENTS_LIFETIME;

			//------2018-08-08---------
			// std::map<std::String, measurements::Measurements> mmt_map;

			// -------------------------
		


		};
	}
}

#endif // NDNSIM_TEST1_STRATEGY_HPP
