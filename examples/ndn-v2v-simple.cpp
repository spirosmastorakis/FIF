#include "ns3/core-module.h"

#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/energy-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ndnSIM/apps/ndn-producer.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"
#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "ns3/ndnSIM/helper/ndn-app-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include <ns3/ndnSIM/helper/ndn-global-routing-helper.hpp>
#include "ns3/animation-interface.h"

// #include "tensorflow/c/c_api.h"
// #include "tensorflow/core/lib/strings/str_util.h"
// #include "tensorflow/core/lib/strings/strcat.h"
// #include "tensorflow/cc/saved_model/tag_constants.h"
// #include "tensorflow/core/lib/io/path.h"
// //#include "tensorflow/core/platform/test.h"
// #include "tensorflow/cc/saved_model/signature_constants.h"
// #include "tensorflow/core/platform/env.h"
// #include "tensorflow/core/public/session.h"

#include "distance_lib.h"

#include <algorithm>
#include <vector>

namespace ns3{

/**
 * This scenario simulates a scenario with 6 cars movind and communicating
 * in an ad-hoc way.
 *
 * 5 consumers request data from producer with frequency 1 interest per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-v2v-simple
 *
 * To modify the mobility model, see function installMobility.
 * To modify the wifi model, see function installWifi.
 * To modify the NDN settings, see function installNDN and for consumer and
 * producer settings, see functions installConsumer and installProducer
 * respectively.
 */

NS_LOG_COMPONENT_DEFINE ("V2VSimple");


static const uint32_t numNodes = 6;

void printPosition(Ptr<const MobilityModel> mobility) //DEBUG purpose
{
  Simulator::Schedule(Seconds(1), &printPosition, mobility);
  NS_LOG_INFO("Car "<<  mobility->GetObject<Node>()->GetId() << " is at: " <<mobility->GetPosition());
}


void installMobility(NodeContainer &c, int simulationEnd)
{
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::WaypointMobilityModel");
  mobility.Install(c);

  bool test = true;
  if(test){
    Ptr<WaypointMobilityModel> wayMobility[numNodes];
    for (uint32_t i = 0; i < numNodes; i++) {
      wayMobility[i] = c.Get(i)->GetObject<WaypointMobilityModel>();
      Waypoint waypointStart(Seconds(0), Vector3D(i*10, 0, 0));
      Waypoint waypointEnd(Seconds(simulationEnd), Vector3D(i*10, 0, 0));

      wayMobility[i]->AddWaypoint(waypointStart);
      wayMobility[i]->AddWaypoint(waypointEnd);
      NS_LOG_INFO("Node " << i << " positions " << waypointStart << " " << waypointEnd);
    }


    return;
  }
  Ptr<WaypointMobilityModel> wayMobility[numNodes];
  for (uint32_t i = 0; i < numNodes; i++) {
    wayMobility[i] = c.Get(i)->GetObject<WaypointMobilityModel>();
    Waypoint waypointStart(Seconds(0), Vector3D(i*10, 0, 0));
    Waypoint waypointMiddle(Seconds(simulationEnd/2), Vector3D(i*20+1000, 0, 0));
    Waypoint waypointEnd(Seconds(simulationEnd+1), Vector3D(i*20+1000, 0, 0));

    wayMobility[i]->AddWaypoint(waypointStart);
    wayMobility[i]->AddWaypoint(waypointMiddle);
    wayMobility[i]->AddWaypoint(waypointEnd);
    NS_LOG_INFO("Node " << i << " positions " << waypointStart << " " << waypointMiddle << " " << waypointEnd);
  }



}

void
RemainingEnergy(Ptr<BasicEnergySource> basicSourcePtr, uint32_t nodeId)
{
  NS_LOG_UNCOND (Simulator::Now().GetSeconds()
                << "s Current remaining energy for node "
                << nodeId
                << " = " << basicSourcePtr->GetRemainingEnergy() << "J");
  Simulator::Schedule(Seconds(1), &RemainingEnergy, basicSourcePtr, nodeId);
}

void
TotalEnergy (double oldValue, double totalEnergy)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds ()
                << "s Total energy consumed by radio = " << totalEnergy << "J");
}

void installWifi(NodeContainer &c, NetDeviceContainer &devices)
{
  // Modulation and wifi channel bit rate
  std::string phyMode("OfdmRate24Mbps");

  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211a);

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
  wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                 "MaxRange", DoubleValue(19.0));
  wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel",
                                 "m0", DoubleValue(1.0),
                                 "m1", DoubleValue(1.0),
                                 "m2", DoubleValue(1.0));
  wifiPhy.SetChannel(wifiChannel.Create());

  // Add a non-QoS upper mac
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();
  // Set it to adhoc mode
  wifiMac.SetType("ns3::AdhocWifiMac");

  // Disable rate control
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode", StringValue(phyMode),
                               "ControlMode", StringValue(phyMode));

  devices = wifi.Install(wifiPhy, wifiMac, c);
}

void installNDN(NodeContainer &c)
{
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);

  ndnHelper.Install(c);
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/broadcast");

  ///todo add v2v face


}

void installConsumer(NodeContainer &c)
{
  ndn::AppHelper helper("ns3::ndn::ConsumerCbr");
  helper.SetAttribute("Frequency", DoubleValue (1.0));
  helper.SetAttribute("Randomize", StringValue("uniform"));
  helper.SetPrefix("/v2v/test");

  helper.Install(c);
}

void installProducer(NodeContainer &c)
{
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix("/v2v");

  producerHelper.Install(c.Get(0));
  NS_LOG_INFO("Producer installed on node " << c.Get(0)->GetId());

}

int main (int argc, char *argv[])
{
  // const char kSavedModel[] = "model";
  // const tensorflow::string saved_model_dir = tensorflow::io::JoinPath(
  //   "/tmp", kSavedModel);
  // // const char kSavedModel[] = "cc/saved_model/testdata/half_plus_two/00000123";
  // // const tensorflow::string saved_model_dir = tensorflow::io::JoinPath(
  // //     "/Users/spyros/tensorflow/tensorflow", kSavedModel);
  // TF_SessionOptions* opt = TF_NewSessionOptions();
  // TF_Buffer* run_options = TF_NewBufferFromString("", 0);
  // TF_Buffer* metagraph = TF_NewBuffer();
  // TF_Status* s = TF_NewStatus();
  // const char* tags[] = {tensorflow::kSavedModelTagServe};
  // TF_Graph* graph = TF_NewGraph();
  // TF_Session* session = TF_LoadSessionFromSavedModel(
  //     opt, run_options, saved_model_dir.c_str(), tags, 1, graph, metagraph, s);

  // Initialize a tensorflow session
  // tensorflow::Session* session2;
  // tensorflow::Status status = tensorflow::NewSession(tensorflow::SessionOptions(), &session2);
  // if (!status.ok()) {
  //   std::cout << status.ToString() << "\n";
  //   return 1;
  // }

  NS_LOG_UNCOND ("V2VTest Simulator");
  //NS_LOG_UNCOND (TF_Version());
  char filename[100];
  char word[100];
  resultFormat results;
  results.nResultsToReturn = 5;
  strcpy(filename, "/Users/spyros/Downloads/word2vec/trunk/vectors.bin");
  strcpy(word, "dog");
  distance(filename, word, (void*)&results);

  NS_LOG_UNCOND("Best match: " << results.resultsArray[1].resultValue);

  uint32_t numProducer = 1;
  int simulationEnd = 1;

  NodeContainer c;
  c.Create(numNodes);

  installMobility(c, simulationEnd);

  NetDeviceContainer netDevices;
  installWifi(c, netDevices);

  /***************************************************************************/
  /* energy source */
  BasicEnergySourceHelper basicSourceHelper;
  // configure energy source
  double initialEnergy = 100.0;
  basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(initialEnergy));
  // install source
  EnergySourceContainer sources = basicSourceHelper.Install(c);
  /* device energy model */
  WifiRadioEnergyModelHelper radioEnergyHelper;
  // configure radio energy model
  radioEnergyHelper.Set("TxCurrentA", DoubleValue(1.0174));
  // install device model
  DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install(netDevices, sources);
  /***************************************************************************/

  installNDN(c);

  //setting application
  Ptr<UniformRandomVariable> randomNum = CreateObject<UniformRandomVariable> ();
  int producerId = randomNum->GetValue(0,numNodes-1);

  NodeContainer producer;
  producer.Add(c.Get(producerId));

  NodeContainer consumers;
  for(int i=0; i<numNodes; i++){
    if(i!=producerId){
      consumers.Add(c.Get(i));
      break;//tmp
    }
  }

  installConsumer(consumers);
  installProducer(producer);

  for(int i=0; i<c.GetN(); i++){
    Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource>(sources.Get(i));
    // basicSourcePtr->TraceConnectWithoutContext("RemainingEnergy", MakeCallback(&RemainingEnergy));
    Simulator::Schedule(Seconds(1), &RemainingEnergy, basicSourcePtr, c.Get(i)->GetId());
  }

  for(int i=0; i<c.GetN(); i++){
    Simulator::Schedule(Seconds(1), &printPosition, c.Get(i)->GetObject<WaypointMobilityModel>());
  }

  Simulator::Stop(Seconds(simulationEnd));

  std::string animFile = "v2v-test.xml";
  AnimationInterface anim(animFile);
  Simulator::Run ();
  return 0;
}
} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
