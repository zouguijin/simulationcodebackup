
// test1-strategy.cpp

#include "test1-strategy.hpp"
#include "core/logger.hpp"
#include "core/common.hpp"

#include <ctime>
#include <boost/algorithm/string.hpp>

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

			// init mapFaceLinkData and mapFaceTimePoint
			this->init();

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
				cout << " NEW Interest: " << pitEntry->getName() << endl;
					// << " To Face: " << outFace.getId() 
					// << " Face Uri: " << outFace.getLocalUri() << endl
					// << " Scheme: " << outFace.getLocalUri().getScheme() << endl
					// << " Host: " << outFace.getLocalUri().getHost() << endl
					// << " Port: " << outFace.getLocalUri().getPort() << endl
					// << " Path: " << outFace.getLocalUri().getPath() << endl
					// << " toString() " << outFace.getLocalUri().toString()
					// << endl 
					// << " InData: " << outFace.getCounters().nInData << endl;
				
				
				string outFaceUri = getOutFaceUri(outFace.getLocalUri().getHost());
				// cout << "outFaceUri: " << outFaceUri << endl;

				this->setMmtEntry(outFace, pitEntry);
				// string mostContentPrefix = getMostContentPrefix(outFace);
				// cout << "MOST_CONTENT_PREFIX: " << mostContentPrefix << endl;

				this->sendInterest(pitEntry, outFace, interest);
			}
		}

		void
		Test1Strategy::beforeSatisfyInterest(const shared_ptr<pit::Entry>& pitEntry,
                                      const Face& inFace, const Data& data)
		{
			// ------global parameter--------
			// 4 parameters to store link capacity according to Face Uri:
			// 07LinkCapacity 0aLinkCapacity 0cLinkCapacity 0eLinkCapacity
			// 4 parameters to store InData's amount according to Face Uri: (InData - xxInData == diff)
			// 07InDataCounter 0aInDataCounter 0cInDataCounter 0eInDataCounter
			// 4 Time Point according to Face Uri: (now() - timePoint == duration)
			// 07TimePoint 0aTimePoint 0cTimePoint 0eTimePoint
			// WHEN now() - timePoint == duration, THEN flowRate = diff*Data_Size / duration (bit/s)
			cout << "---Test1Strategy::beforeSatisfyInterest()---" << endl;
			if(this->isCongested(inFace, data)) 
			{
				cout << "Congested!" << endl;
				string mostContentPrefix = this->getMostContentPrefix(inFace);
				cout << "CS Policy should firstly store: " << mostContentPrefix << endl;
			}
			else
			{
				cout << "Not Congested!" << endl;
			}
			
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
			
			this->getMeasurements().get(mmtEntryName);
			// measurements::Entry* mmtEntry = this->getMeasurements().get(mmtEntryName); // insert into mmt
			// this->getMeasurements().extendLifetime(*mmtEntry, MEASUREMENTS_LIFETIME);
			
		}

		string 
		Test1Strategy::getMostContentPrefix(const Face& outFace) 
		{
			// const string& prefixA("/bupt/zou/A");
			// const string& prefixB("/bupt/zou/B");
			// const string& prefixC("/bupt/zou/C");

			// vector<string> prefixVec;
			// prefixVec.push_back(prefixA);
			// prefixVec.push_back(prefixB);
			// prefixVec.push_back(prefixC);

			unsigned int MOST_CONTENT_COUNTER = 0;
			string MOST_CONTENT_PREFIX = "";

			const string OUTFACE_HOST = outFace.getLocalUri().getHost();
			const string OUTFACE_URI = getOutFaceUri(OUTFACE_HOST);

			for(int i = 0; i < prefixVec.size(); i++)
			{
				string uriCurrent = OUTFACE_URI + prefixVec.at(i);
				string uriTestCurrent = "/" + uriCurrent;

				// cout << "uriCurrent: " << uriCurrent << endl;
				// cout << "uriTestCurrent: " << uriTestCurrent << endl;
				// cout << "mmt size: " << this->getMeasurements().m_measurements.size() << endl;

				Name name(uriCurrent);
				name_tree::NameTree& nt = this->getMeasurements().m_measurements.m_nameTree;
								
				auto&& enumerable = nt.partialEnumerate(name);
				unsigned int tmpCounter = 0;
				for (const name_tree::Entry& nte : enumerable) 
				{
					// cout << "Loop: " << endl;
					if(name.isPrefixOf(nte.getName()) && uriTestCurrent.compare(nte.getName().toUri()) != 0) 
					{

						cout << "---bupt/zou/--- child: " << nte.getName().toUri() << endl;
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


		std::chrono::system_clock::time_point
		Test1Strategy::getCurrentTimePoint()
		{
			std::chrono::system_clock::time_point 
				currentTimePoint = std::chrono::system_clock::now();
			return currentTimePoint;
		}

		bool
		Test1Strategy::isCongested(const Face& outFace, const Data& data)
		{
			if(this->isFaceCounterTimeOut(outFace))
			{
				string faceHost = outFace.getLocalUri().getHost();
				string faceUri = getOutFaceUri(faceHost);
				int curInData = outFace.getCounters().nInData;

				std::map<string, vector<int>>::iterator it = mapFaceLinkData.find(faceUri);
				int linkCapaciy = it->second.at(0);
				int beforeInData = it->second.at(1);
				
				int flowRate 
					= (curInData - beforeInData) * DATA_SIZE / (FACECOUNTER_DURATION / 1000);
				
				// reset beforeInData
				it->second.at(1) = curInData;

				cout << "Face Counter TimeOut" << endl
					 << "Face Uri: " << faceUri << endl
					 << "Link Capacity: " << linkCapaciy
					 << " BeforeInData: " << beforeInData
					 << " curInData: " << curInData << endl
					 << "Flow Rate: " << flowRate << endl;


				if(flowRate > 0.9 * linkCapaciy)
				{
					// congesting || congested
					// string mostContentPrefix = this->getMostContentPrefix(outFace);
					// cout << "CS Policy should firstly store: " << mostContentPrefix << endl;
					return true;
				}
				else 
				{
					return false;
				}
			}
			else 
			{
				cout << "Not TimeOut - ";
				return false;
			}
		}

		bool
		Test1Strategy::isFaceCounterTimeOut(const Face& outFace) 
		{
			// Face operation and get face uri
			// Use uri to get related timepoint before: beforeTP
			// duration = curTP - beforeTP  >  DURATION ?
			// if yes, return true
			std::chrono::system_clock::time_point curTP = getCurrentTimePoint();
			string faceHost = outFace.getLocalUri().getHost();
			string faceUri = getOutFaceUri(faceHost);

			std::map<string, std::chrono::system_clock::time_point>::iterator it;
			it = mapFaceTimePoint.find(faceUri);
			if(it == mapFaceTimePoint.end()) 
			{
				return false;
			}

			std::chrono::system_clock::time_point beforeTP = it->second;
			int duration = std::chrono::duration_cast<std::chrono::milliseconds>(curTP - beforeTP).count();
			if(duration >= FACECOUNTER_DURATION)
			{
				it->second = curTP;
				return true;
			}
			else 
			{
				return false;
			}
		}

		void
		Test1Strategy::init()
		{
			// Face Uri: "07" "0a" "0c" "0e"
			// mapFaceTimePoint
			std::chrono::system_clock::time_point 
				currentTimePoint = std::chrono::system_clock::now();
			mapFaceTimePoint.insert({"0b", currentTimePoint});
			mapFaceTimePoint.insert({"0e", currentTimePoint});
			mapFaceTimePoint.insert({"10", currentTimePoint});
			mapFaceTimePoint.insert({"12", currentTimePoint});

			// mapFaceLinkData
			// 07LinkCapacity: 10000000 bps 0aLinkCapacity: 3000000 bps 
			// 0cLinkCapacity: 1000000  bps 0eLinkCapacity: 5000000 bps
			int face07[2] = {10000000, 0};
			vector<int> face07Vec(face07, face07 + 2);
			int face0a[2] = {1000000, 0};
			vector<int> face0aVec(face0a, face0a + 2);
			int face0c[2] = {3000000, 0};
			vector<int> face0cVec(face0c, face0c + 2);
			int face0e[2] = {5000000, 0};
			vector<int> face0eVec(face0e, face0e + 2);
			mapFaceLinkData.insert({"0b", face07Vec});
			mapFaceLinkData.insert({"0e", face0aVec});
			mapFaceLinkData.insert({"10", face0cVec});
			mapFaceLinkData.insert({"12", face0eVec});

			const string& prefixA("/bupt/zou/A");
			const string& prefixB("/bupt/zou/B");
			const string& prefixC("/bupt/zou/C");
			prefixVec.push_back(prefixA);
			prefixVec.push_back(prefixB);
			prefixVec.push_back(prefixC);

			FACECOUNTER_DURATION = 2000;
			DATA_SIZE = 10240;
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