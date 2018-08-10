
// test1-strategy.cpp

#include "test1-strategy.hpp"
#include "core/logger.hpp"

using namespace std;

namespace nfd {
	namespace fw {
		Test1Strategy::Test1Strategy(Forwarder& forwarder, const Name& name) 
			: Strategy(forwarder)
		{
			//NFD_LOG_INFO("Test1Strategy Constructor!");
			cout << "---Test1Strategy Constructor!---" << endl;
			counter = 0;
			this->setInstanceName(makeInstanceName(name, getStrategyName()));
		}

		Test1Strategy::~Test1Strategy()
		{
			//NFD_LOG_INFO("Test1Strategy Destructor!");
			cout << "---Test1Strategy Destructor!---" << endl;
		}

		const Name&
		Test1Strategy::getStrategyName()
		{
			//NFD_LOG_INFO("GetStrategyName()...");
			static Name strategyName("ndn:/localhost/nfd/strategy/test1-strategy/%FD%01");
			return strategyName;
		}	

		void
		Test1Strategy::afterReceiveInterest(const Face& inFace, const Interest& interest,
									const shared_ptr<pit::Entry>& pitEntry)
		{
			//NFD_LOG_INFO("afterReceiveInterest()...");
			//cout << "---Test1Strategy::afterReceiveInterest()---" << endl;

			RetxSuppressionResult suppressResult = m_retxSuppression.decidePerPitEntry(*pitEntry);
			
			switch (suppressResult) 
			{
				// New Interest
				case RetxSuppressionResult::NEW:
					this->afterReceiveNewInterest(inFace, interest, pitEntry);
					break;
				// time-out
				case RetxSuppressionResult::FORWARD:
					this->afterReceiveRetxInterest(inFace, interest, pitEntry);
					break;
				// not time-out
				case RetxSuppressionResult::SUPPRESS:
					cout << "---RetxSuppressionResult::SUPPRESS---" << endl;
					cout << interest << "Interest From" << inFace.getId() << "not time-out, SUPPRESS!" << endl;
					//NFD_LOG_INFO(interest << "Interest From" << inFace.getId() << "not time-out, SUPPRESS!");
					break;
				default:
					cout << "---SWITCH ERROR!---" << endl;
					//NFD_LOG_INFO("SWITCH DEFALUT ERROR!");

			}
		}

		// forwarding NEW Interest
		void
		Test1Strategy::afterReceiveNewInterest(const Face& inFace, const Interest& interest,
                                        const shared_ptr<pit::Entry>& pitEntry)
		{
			//NFD_LOG_INFO("afterReceiveNewInterest()...");
			//NFD_LOG_INFO("NEW Interest: " << interest << "is Received --- Multicast to All Face.");
			cout << "---Test1Strategy::afterReceiveNewInterest()---" << endl;
			//cout << "NEW Interest: " << interest << "is Received --- Multicast to All Face." << endl;
			
			const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
			const fib::NextHopList& nexthopList = fibEntry.getNextHops();
			// fib::NextHopList::const_iterator it = nexthops.end();

			// multicast forwarding
			int faceNumber = 1;
			for(const fib::NextHop& nexthop : nexthopList)
			{
				Face& outFace = nexthop.getFace();
				// except incoming face
				if(&outFace == &inFace || wouldViolateScope(inFace, interest, outFace))
				{
					continue;
				}
				//NFD_LOG_INFO("Forwarding NEW Interest: " << pitEntry->getInterest() << " To Face: " 
				//	<< outFace.getId());
				cout << faceNumber << " -> Forwarding NEW Interest: " << pitEntry->getName() << " To Face: " 
					<< outFace.getId() << endl;
				this->sendInterest(pitEntry, outFace, interest);
				faceNumber++;
			}
		}

		// forwarding Retrasmission Interest --- pitEntry time-out OR data loss
		// Upstream may be congested
		void 
		Test1Strategy::afterReceiveRetxInterest(const Face& inFace, const Interest& interest,
                                         const shared_ptr<pit::Entry>& pitEntry)
		{
			//NFD_LOG_INFO("afterReceiveRetxInterest()...");
			cout << "---Test1Strategy::afterReceiveRetxInterest()---" << endl;
			//cout << "Congestion may be happening..." << endl;

			// const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
			// const fib::NextHopList& nexthopList = fibEntry.getNextHops();

			// temporary using afterReceiveNewInterest()
			this->afterReceiveNewInterest(inFace, interest, pitEntry);

			// need to inform CS to store
		}

		void
		Test1Strategy::beforeSatisfyInterest(const shared_ptr<pit::Entry>& pitEntry,
                                      const Face& inFace, const Data& data)
		{
			cout << "---Test1Strategy::beforeSatisfyInterest()---" << endl;
			cout << "Number otf Data is: " << ++counter << endl;
			cout << "Data Prefix: " << data << endl;
		}
	}
}