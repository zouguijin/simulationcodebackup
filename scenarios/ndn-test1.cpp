
// ndn-test1.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "../extensions/test1-strategy.hpp"


using namespace ns3;
using ns3::AnnotatedTopologyReader;
using ns3::ndn::StackHelper;
using ns3::ndn::GlobalRoutingHelper;
using ns3::ndn::StrategyChoiceHelper;
using ns3::ndn::AppHelper;


int
main(int argc, char* argv[])
{
	CommandLine cmd;
	cmd.Parse(argc, argv);

  // ----------Parameter---------------------------
  const std::string INTERESTS_PER_SECOND = "1";
  const std::string DATA_PAYLOADSIZE = "1000";
  const int CS_SIZE = 5;

  //-----------------------------------------------

	AnnotatedTopologyReader topologyReader("", 20); // 20 means the scale between nodes and lines
	topologyReader.SetFileName("topologies/topo-congestion-1.txt");
	topologyReader.Read();

	// Install NDN stack on all nodes
	StackHelper ndnHelper;
	// cs policy: LRU, cs max size: 10000
	// ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "10000");
  ndnHelper.setPolicy("nfd::cs::lru");
  ndnHelper.setCsSize(CS_SIZE);
	ndnHelper.InstallAll();

	// Installing global routing interface on all nodes
  	GlobalRoutingHelper ndnGlobalRoutingHelper;
  	ndnGlobalRoutingHelper.InstallAll();

  	// Getting containers for the producer / consumer
  	// Method 1 --- shiyongyu fenbie peizhi meiyige jiedian
  	Ptr<Node> p1 = Names::Find<Node>("producer1");
  	Ptr<Node> p2 = Names::Find<Node>("producer2");
  	Ptr<Node> p3 = Names::Find<Node>("producer3");

  	Ptr<Node> c1 = Names::Find<Node>("consumer1");
  	Ptr<Node> c2 = Names::Find<Node>("consumer2");
  	Ptr<Node> c3 = Names::Find<Node>("consumer3");
    Ptr<Node> c4 = Names::Find<Node>("consumer4");
    Ptr<Node> c5 = Names::Find<Node>("consumer5");
  	// Method 2 --- shiyongyu jiti peizhi yiqun jiedian
  	NodeContainer routeNodes;
  	routeNodes.Add(Names::Find<Node>("router1"));
  	routeNodes.Add(Names::Find<Node>("router2"));

  	// Define name prefix
  	std::string prefix = "/bupt/zou";
  	// Forwarding Strategy
  	StrategyChoiceHelper::Install<nfd::fw::Test1Strategy>(routeNodes, prefix);
  	// StrategyChoiceHelper::Install<nfd::fw::RandomLoadBalancerStrategy>(routeNodes, prefix);
  	// StrategyChoiceHelper::InstallAll(prefix, "/localhost/nfd/strategy/best-route");
  	// StrategyChoiceHelper::InstallAll(prefix, "/localhost/nfd/strategy/ncc");

  	// Install NDN Applications: Consumer & Producer
  	std::string prefixA = "bupt/zou/A";
  	std::string prefixB = "bupt/zou/B";
  	std::string prefixC = "bupt/zou/C";

  	// Consumer APP
  	AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  	// consumerHelper.SetAttribute("Frequency", StringValue("100")); // 100 interests per second
    consumerHelper.SetAttribute("Frequency", StringValue(INTERESTS_PER_SECOND));

  	consumerHelper.SetPrefix(prefixA);
  	ApplicationContainer c1Container = consumerHelper.Install(c1);

  	consumerHelper.SetPrefix(prefixB);
  	ApplicationContainer c2Container = consumerHelper.Install(c2);

  	consumerHelper.SetPrefix(prefixC);
  	ApplicationContainer c3Container = consumerHelper.Install(c3);

    consumerHelper.SetPrefix(prefixA);
    ApplicationContainer c4Container = consumerHelper.Install(c4);

    consumerHelper.SetPrefix(prefixA);
    ApplicationContainer c5Container = consumerHelper.Install(c5);

    c1Container.Start(Seconds(0));
    c2Container.Start(Seconds(5));
    c3Container.Start(Seconds(10));
    c4Container.Start(Seconds(15));
    c5Container.Start(Seconds(20));

  	// Producer APP
  	AppHelper producerHelper("ns3::ndn::Producer");
  	producerHelper.SetAttribute("PayloadSize", StringValue(DATA_PAYLOADSIZE));

  	producerHelper.SetPrefix(prefixA);
  	producerHelper.Install(p1);

  	producerHelper.SetPrefix(prefixB);
  	producerHelper.Install(p2);

  	producerHelper.SetPrefix(prefixC);
  	producerHelper.Install(p3);

  	ndnGlobalRoutingHelper.AddOrigins(prefixA, p1);
  	ndnGlobalRoutingHelper.AddOrigins(prefixB, p2);
  	ndnGlobalRoutingHelper.AddOrigins(prefixC, p3);

  	// Calculate and install FIBs
  	GlobalRoutingHelper::CalculateRoutes();

  	Simulator::Stop(Seconds(20.0));

  	Simulator::Run();
  	Simulator::Destroy();

  	return 0;
}