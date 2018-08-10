
// test1-strategy.hpp

#ifndef NDNSIM_TEST1_STRATEGY_HPP
#define NDNSIM_TEST1_STRATEGY_HPP

#include "face/face.hpp"
#include "fw/strategy.hpp"
#include "fw/algorithm.hpp"
#include "fw/retx-suppression-fixed.hpp"
#include "table/measurement-accessor.hpp"
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
			virtual void
			setMmtMap(const Face& outface, const shared_ptr<pit::Entry>& pitEntry);
			// ------------------------

		private:
			RetxSuppressionFixed m_retxSuppression;
			int counter;

			//------2018-08-08---------
			std::map<Face, measurement::Measurement> mmt_map;
			// -------------------------

		private:
			void
			afterReceiveNewInterest(const Face& inFace, const Interest& interest,
                          			const shared_ptr<pit::Entry>& pitEntry);

			void
			afterReceiveRetxInterest(const Face& inFace, const Interest& interest,
                          			 const shared_ptr<pit::Entry>& pitEntry);


		};
	}
}

#endif // NDNSIM_TEST1_STRATEGY_HPP
