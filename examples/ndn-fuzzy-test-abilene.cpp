/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// ndn-simple.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

#include "distance_lib.h"

#include <fstream>

#include "ns3/ndnSIM/utils/mem-usage.hpp"

namespace ns3 {

/**
 * This scenario simulates a very simple network topology:
 *
 *
 *      +----------+     1Mbps      +--------+     1Mbps      +----------+
 *      | consumer | <------------> | router | <------------> | producer |
 *      +----------+         10ms   +--------+          10ms  +----------+
 *
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-simple
 */

int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  int fibSize = 0;
  int simTime = 0;
  int memLogs = 0;
  cmd.AddValue("fibSize", "Number of random FIB entries", fibSize);
  cmd.AddValue("simTime", "Simulation Time", simTime);
  cmd.AddValue("memLogs", "Memory Logs On", memLogs);
  cmd.Parse(argc, argv);

    NS_LOG_UNCOND ("Abilene Fuzzy");

  // char filename[100];
  // strcpy(filename, "/Users/spyros/Downloads/word2vec/trunk/vectors.bin");
  // int fibSize = 1000;
  // char random_words_routing[fibSize][100];
  // // char random_words_names[10000][100];
  // get_random_words(filename, fibSize, random_words_routing);
  // //get_random_words(file_name, 1000, random_words_names);

  char filename[100];
  strcpy(filename, "routing.txt");
  std::ifstream ifs(filename);
  if(ifs.fail()) {
        std::cerr << "failed to open input file" << std::endl;
        return -1;
  }

  char random_words_routing[fibSize][100];

  int i = 0;
  while(ifs >> random_words_routing[i]) {
    // std::cerr << "word number " << i << " is " << random_words_routing[i]  << endl;
    i++;
  }

  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/topo-abilene.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  // ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/fuzzy");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // Installing applications

  // First Consumer
  ndn::AppHelper consumerHelper1("ns3::ndn::ConsumerFuzzy");
  consumerHelper1.SetPrefix("/prefix/dog");
  consumerHelper1.SetAttribute("Filename", StringValue("names1.txt")); // file name for prefixes
  consumerHelper1.SetAttribute("Frequency", StringValue("10")); // 10 interests a second
  consumerHelper1.SetAttribute("WarmUpApp", BooleanValue(false));
  if (memLogs == 1)
    consumerHelper1.SetAttribute("MemoryLogs", BooleanValue(true));
  ApplicationContainer consumer1 = consumerHelper1.Install(Names::Find<Node>("Seattle"));
  consumer1.Start(Seconds(10));
  consumer1.Stop(Seconds(simTime - 0.01));

  // Second Consumer
  ndn::AppHelper consumerHelper2("ns3::ndn::ConsumerFuzzy");
  consumerHelper2.SetPrefix("/prefix/dog");
  consumerHelper2.SetAttribute("Frequency", StringValue("10")); // 10 interests a second
  consumerHelper2.SetAttribute("Filename", StringValue("names2.txt")); // file name for prefixes
  consumerHelper2.SetAttribute("WarmUpApp", BooleanValue(false));
  if (memLogs == 1)
    consumerHelper2.SetAttribute("MemoryLogs", BooleanValue(true));
  ApplicationContainer consumer2 = consumerHelper2.Install(Names::Find<Node>("Houston"));
  consumer2.Start(Seconds(10));
  consumer2.Stop(Seconds(simTime - 0.01));

  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  // Producer will reply to all requests starting with /prefix
  producerHelper.SetPrefix("/");
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(Names::Find<Node>("New-York")); // last node

  // Warming up caches along the data retrieval path
  ndn::AppHelper consumerHelperWarmup1("ns3::ndn::ConsumerFuzzy");
  consumerHelperWarmup1.SetPrefix("/prefix/dog");
  consumerHelperWarmup1.SetAttribute("Frequency", StringValue("30")); // 30 interests a second
  consumerHelperWarmup1.SetAttribute("Filename", StringValue("names-warmup1.txt")); // file name for prefixes
  ApplicationContainer consumerWarmup1 = consumerHelperWarmup1.Install(Names::Find<Node>("Seattle"));
  consumerWarmup1.Stop(Seconds(10)); // stop consumers at 10s

  // Warming up caches along the data retrieval path
  ndn::AppHelper consumerHelperWarmup2("ns3::ndn::ConsumerFuzzy");
  consumerHelperWarmup2.SetPrefix("/prefix/dog");
  consumerHelperWarmup2.SetAttribute("Frequency", StringValue("30")); // 30 interests a second
  consumerHelperWarmup2.SetAttribute("Filename", StringValue("names-warmup2.txt")); // file name for prefixes
  ApplicationContainer consumerWarmup2 = consumerHelperWarmup2.Install(Names::Find<Node>("Houston"));
  consumerWarmup2.Stop(Seconds(10)); // stop consumers at 10s

  // Warming up caches along the data retrieval path
  ndn::AppHelper consumerHelperWarmup3("ns3::ndn::ConsumerFuzzy");
  consumerHelperWarmup3.SetPrefix("/prefix/dog");
  consumerHelperWarmup3.SetAttribute("Frequency", StringValue("30")); // 30 interests a second
  consumerHelperWarmup3.SetAttribute("Filename", StringValue("names-warmup3.txt")); // file name for prefixes
  ApplicationContainer consumerWarmup3 = consumerHelperWarmup3.Install(Names::Find<Node>("Sunnyvale"));
  consumerWarmup3.Stop(Seconds(10)); // stop consumers at 10s

  // Warming up caches along the data retrieval path
  ndn::AppHelper consumerHelperWarmup4("ns3::ndn::ConsumerFuzzy");
  consumerHelperWarmup4.SetPrefix("/prefix/dog");
  consumerHelperWarmup4.SetAttribute("Frequency", StringValue("30")); // 30 interests a second
  consumerHelperWarmup4.SetAttribute("Filename", StringValue("names-warmup4.txt")); // file name for prefixes
  ApplicationContainer consumerWarmup4 = consumerHelperWarmup3.Install(Names::Find<Node>("Atlanta"));
  consumerWarmup4.Stop(Seconds(10)); // stop consumers at 10s


  // Add /prefix origins to ndn::GlobalRouter
  for (int i = 0; i < fibSize; i++)
    ndnGlobalRoutingHelper.AddOrigins(string("/prefix/") + random_words_routing[i], Names::Find<Node>("New-York"));
  // ndnGlobalRoutingHelper.AddOrigins("/prefix/", nodes.Get(2));

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(simTime));

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
