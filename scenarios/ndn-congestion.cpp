
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {

int
main(int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName("topologies/topo-congestion-1.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "10000");
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Getting containers for the consumer/producer
  // Zou: Ptr<Node>  are used to refer to the nodes, then NodeContainers hold the multiple Ptr<Node>
  Ptr<Node> consumer1 = Names::Find<Node>("consumer1");
  Ptr<Node> consumer2 = Names::Find<Node>("consumer2");
  Ptr<Node> consumer3 = Names::Find<Node>("consumer3");

  Ptr<Node> producer1 = Names::Find<Node>("producer1");
  Ptr<Node> producer2 = Names::Find<Node>("producer2");
  Ptr<Node> producer3 = Names::Find<Node>("producer3");

  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetAttribute("Frequency", StringValue("100")); // 100 interests a second

  // on the first consumer node install a Consumer application
  // that will express interests in /dst1 namespace
  consumerHelper.SetPrefix("/dst1");
  consumerHelper.Install(consumer1);

  // on the second consumer node install a Consumer application
  // that will express interests in /dst2 namespace
  consumerHelper.SetPrefix("/dst2");
  consumerHelper.Install(consumer2);

  consumerHelper.SetPrefix("/dst3");
  consumerHelper.Install(consumer3);

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));

  // Register /dst1 prefix with global routing controller and
  // install producer that will satisfy Interests in /dst1 namespace
  ndnGlobalRoutingHelper.AddOrigins("/dst1", producer1);
  producerHelper.SetPrefix("/dst1");
  producerHelper.Install(producer1);

  // Register /dst2 prefix with global routing controller and
  // install producer that will satisfy Interests in /dst2 namespace
  ndnGlobalRoutingHelper.AddOrigins("/dst2", producer2);
  producerHelper.SetPrefix("/dst2");
  producerHelper.Install(producer2);

  ndnGlobalRoutingHelper.AddOrigins("/dst3", producer3);
  producerHelper.SetPrefix("/dst3");
  producerHelper.Install(producer3);

  ndnGlobalRoutingHelper.AddOrigins("/dst3", consumer1);
  producerHelper.SetPrefix("/dst3");
  producerHelper.Install(consumer1);

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(20.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}