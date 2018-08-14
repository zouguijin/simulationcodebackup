
// test1-strategy.cpp

#include "test1-strategy.hpp"
#include "core/logger.hpp"
#include "core/common.hpp"

#include <chrono>
#include <ctime>
#include <boost/algorithm/string.hpp>
#include <vector>

#include "table/measurements-entry.hpp"
#include "table/name-tree.hpp"

using namespace std;
using namespace boost;

namespace nfd {
	namespace fw {

		// const time::nanoseconds Test1Strategy::MEASUREMENTS_LIFETIME = time::seconds(4);

		Test1Strategy::Test1Strategy(Forwarder& forwarder, const Name& name) 
			: Strategy(forwarder)
		{
			//NFD_LOG_INFO("Test1Strategy Constructor!");
			cout << "---Test1Strategy Constructor!---" << endl;

			getcurrentTime();

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

			cout << "---Test1Strategy::afterReceiveNewInterest()---" << endl;
			//cout << "NEW Interest: " << interest << "is Received --- Multicast to All Face." << endl;
			
			const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
			const fib::NextHopList& nexthopList = fibEntry.getNextHops();
			// fib::NextHopList::const_iterator it = nexthops.end();

			// multicast forwarding
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
				cout 
					<< " NEW Interest: " << pitEntry->getName() << endl
					// << " To Face: " << outFace.getId() 
					// << " Face Uri: " << outFace.getLocalUri() << endl
					// << " Scheme: " << outFace.getLocalUri().getScheme() << endl
					// << " Host: " << outFace.getLocalUri().getHost() << endl
					// << " Port: " << outFace.getLocalUri().getPort() << endl
					// << " Path: " << outFace.getLocalUri().getPath() << endl
					// << " toString() " << outFace.getLocalUri().toString()
					// << endl 
					<< " InData: " << outFace.getCounters().nInData << endl;
				
				
				string outFaceUri = getOutFaceUri(outFace.getLocalUri().getHost());
				// cout << "outFaceUri: " << outFaceUri << endl;

				this->setMmtEntry(outFace, pitEntry);
				string mostContentPrefix = this->getMostContentPrefix(outFace);
				cout << "MOST_CONTENT_PREFIX: " << mostContentPrefix << endl;

				this->sendInterest(pitEntry, outFace, interest);
			}
		}

		void
		Test1Strategy::beforeSatisfyInterest(const shared_ptr<pit::Entry>& pitEntry,
                                      const Face& inFace, const Data& data)
		{
			//cout << "---Test1Strategy::beforeSatisfyInterest()---" << endl;
			// cout << "Number of Data is: " << ++counter << endl;
			//cout << "Data Prefix: " << data << endl;
			cout << "";
		}

		void
		Test1Strategy::setMmtEntry(const Face& outFace, const shared_ptr<pit::Entry>& pitEntry) 
		{
			string outFaceHost = outFace.getLocalUri().getHost(); // mark: node+outFace
			string outFaceUri = getOutFaceUri(outFaceHost);
			string pitEntryUri = pitEntry->getName().toUri(); // mark: prefix
			string mmtEntryUri = outFaceUri + pitEntryUri;

			cout << "mmtEntryUri: " << mmtEntryUri << endl; // 0c/bupt/zou/A/%FE%00 

			const Name& mmtEntryName(mmtEntryUri); // string -> Name
			
			measurements::Entry* mmtEntry = this->getMeasurements().get(mmtEntryName); // insert into mmt
			// this->getMeasurements().extendLifetime(*mmtEntry, MEASUREMENTS_LIFETIME);
			
		}

		string 
		Test1Strategy::getMostContentPrefix(const Face& outFace) 
		{
			const string& prefixA("/bupt/zou/A");
			const string& prefixB("/bupt/zou/B");
			const string& prefixC("/bupt/zou/C");

			vector<string> prefixVec;
			prefixVec.push_back(prefixA);
			// prefixVec.push_back(prefixA);
			// prefixVec.push_back(prefixA);

			unsigned int MOST_CONTENT_COUNTER = 0;
			string MOST_CONTENT_PREFIX = "";

			const string OUTFACE_HOST = outFace.getLocalUri().getHost();
			const string OUTFACE_URI = getOutFaceUri(OUTFACE_HOST);

			for(int i = 0; i < prefixVec.size(); i++)
			{
				string uriCurrent = OUTFACE_URI + prefixVec.at(i);
				string uriTestCurrent = "/" + uriCurrent;

				cout << "uriCurrent: " << uriCurrent << endl;
				cout << "uriTestCurrent: " << uriTestCurrent << endl;
				cout << "mmt size: " << this->getMeasurements().m_measurements.size() << endl;

				Name name(uriCurrent);
				name_tree::NameTree& nt = this->getMeasurements().m_measurements.m_nameTree;
								
				auto&& enumerable = nt.partialEnumerate(name);
				unsigned int tmpCounter = 0;
				for (const name_tree::Entry& nte : enumerable) 
				{
					cout << "Loop: " << endl;
					if(name.isPrefixOf(nte.getName()) && uriTestCurrent.compare(nte.getName().toUri()) != 0) 
					{

						cout << "---bupt/zou/A--- child: " << nte.getName().toUri() << endl;
						tmpCounter++;
					}

				}
				cout << "Counter: " << tmpCounter << endl;
				if(tmpCounter > MOST_CONTENT_COUNTER)
				{
					MOST_CONTENT_COUNTER = tmpCounter;
					MOST_CONTENT_PREFIX = prefixVec.at(i);
				}
				cout << "MOST_CONTENT_COUNTER: " << MOST_CONTENT_COUNTER << endl;
				cout << "MOST_CONTENT_PREFIX: " << MOST_CONTENT_PREFIX << endl;
			}
			return MOST_CONTENT_PREFIX;
		}


		void 
		Test1Strategy::getcurrentTime()
		{
		     std::chrono::time_point<std::chrono::system_clock> 
		     	currentTimeP = std::chrono::system_clock::now();
		     std::time_t 
		     	currentTime = std::chrono::system_clock::to_time_t(currentTimeP);
		     cout << std::ctime(&currentTime) << endl;
		}

		string
		Test1Strategy::getOutFaceUri(const string originStr)
		{
			vector<string> splitVec;
			split(splitVec, originStr, is_any_of(":"), token_compress_on);
			// cout << splitVec.back() << endl;
			return splitVec.back();
		} 

	}
}