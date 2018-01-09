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

#include "ndn-consumer-fuzzy.hpp"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"

#include "distance_lib.h"

#include "ns3/ndnSIM/utils/mem-usage.hpp"

NS_LOG_COMPONENT_DEFINE("ndn.ConsumerFuzzy");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ConsumerFuzzy);

TypeId
ConsumerFuzzy::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ConsumerFuzzy")
      .SetGroupName("Ndn")
      .SetParent<Consumer>()
      .AddConstructor<ConsumerFuzzy>()

      .AddAttribute("Frequency", "Frequency of interest packets", StringValue("1.0"),
                    MakeDoubleAccessor(&ConsumerFuzzy::m_frequency), MakeDoubleChecker<double>())

      .AddAttribute("Randomize",
                    "Type of send time randomization: none (default), uniform, exponential",
                    StringValue("none"),
                    MakeStringAccessor(&ConsumerFuzzy::SetRandomize, &ConsumerFuzzy::GetRandomize),
                    MakeStringChecker())

      .AddAttribute("MaxSeq", "Maximum sequence number to request",
                    IntegerValue(std::numeric_limits<uint32_t>::max()),
                    MakeIntegerAccessor(&ConsumerFuzzy::m_seqMax), MakeIntegerChecker<uint32_t>())

      .AddAttribute("Filename", "Name of the file containing the prefixes to express", StringValue("/"),
                    MakeStringAccessor(&ConsumerFuzzy::m_filename), MakeStringChecker())

      .AddAttribute("WarmUpApp", "Is this app instance for cache warm-up?", BooleanValue(true),
                    MakeBooleanAccessor(&ConsumerFuzzy::m_warmUpApp), MakeBooleanChecker())

      .AddAttribute("MemoryLogs", "Is this app instance for cache warm-up?", BooleanValue(false),
                    MakeBooleanAccessor(&ConsumerFuzzy::m_memoryLogs), MakeBooleanChecker())

      .AddAttribute("PrintStats", "Print out stats at the end?", BooleanValue(true),
                    MakeBooleanAccessor(&ConsumerFuzzy::m_printStats), MakeBooleanChecker())

      .AddAttribute("AppLifeTime", "Application LifeTime", StringValue("5s"),
                    MakeTimeAccessor(&ConsumerFuzzy::m_appLifeTime), MakeTimeChecker())
    ;

  return tid;
}

void
ConsumerFuzzy::SetPrefixFileName(std::string fileName)
{
  m_filename = fileName;
}

ConsumerFuzzy::ConsumerFuzzy()
  : m_frequency(1.0)
  , m_firstTime(true)
{
  NS_LOG_FUNCTION_NOARGS();
  m_seqMax = std::numeric_limits<uint32_t>::max();

  m_nameIndex = 0;

  m_interestsSent = 0;
  m_dataReceived = 0;
}

ConsumerFuzzy::~ConsumerFuzzy()
{
}

// Application Methods
void
ConsumerFuzzy::StartApplication() // Called at time specified by Start
{
  // std::cerr << m_filename << std::endl;
  std::ifstream ifs(m_filename);
  if(ifs.fail()) {
        std::cerr << "failed to open input file " << m_filename << std::endl;
  }

  int i = 0;
  while(ifs >> m_random_words_names[i]) {
    // std::cerr << "word number " << i << " is " << m_random_words_names[i]  << std::endl;
    i++;
  }

  Consumer::StartApplication();
}

void
ConsumerFuzzy::StopApplication() // Called at time specified by Stop
{
  if (!m_warmUpApp && m_printStats){
    NS_LOG_UNCOND ("Interests sent: " << m_interestsSent);
    NS_LOG_UNCOND ("Data received: " << m_dataReceived);
  }
  Consumer::StopApplication();
}

void
ConsumerFuzzy::ScheduleNextPacket()
{
  // double mean = 8.0 * m_payloadSize / m_desiredRate.GetBitRate ();
  // std::cout << "next: " << Simulator::Now().ToDouble(Time::S) + mean << "s\n";
  //m_random = 0;
  // std::cerr << "Simulator Now: " << Simulator::Now().GetSeconds() << std::endl;
  // std::cerr << "Lifetime: "  << m_appLifeTime.GetSeconds() << std::endl;
  // if (Simulator::Now().GetSeconds() >= m_appLifeTime.GetSeconds() - 0.1)
  //     return;

  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &Consumer::SendPacket, this,
                                      make_shared<Name>(m_interestName.toUri() + "/" + m_random_words_names[m_nameIndex]));
    m_firstTime = false;
  }
  else if (!m_sendEvent.IsRunning())
    m_sendEvent = Simulator::Schedule(Seconds(1.0 / m_frequency),
                                      &Consumer::SendPacket, this,
                                      make_shared<Name>(m_interestName.toUri() + "/" + m_random_words_names[m_nameIndex]));
  m_nameIndex++;
  m_interestsSent++;
}

void
ConsumerFuzzy::SetRandomize(const std::string& value)
{
  if (value == "uniform") {
    m_random = CreateObject<UniformRandomVariable>();
    m_random->SetAttribute("Min", DoubleValue(0.0));
    m_random->SetAttribute("Max", DoubleValue(2 * 1.0 / m_frequency));
  }
  else if (value == "exponential") {
    m_random = CreateObject<ExponentialRandomVariable>();
    m_random->SetAttribute("Mean", DoubleValue(1.0 / m_frequency));
    m_random->SetAttribute("Bound", DoubleValue(50 * 1.0 / m_frequency));
  }
  else
    m_random = 0;

  m_randomType = value;
}

std::string
ConsumerFuzzy::GetRandomize() const
{
  return m_randomType;
}

void
ConsumerFuzzy::OnData(shared_ptr<const Data> data)
{
  NS_LOG_INFO("> Data for " << data->getName());
  double overhead = MemUsage::Get() / 1024.0 / 1024.0;
  if (m_memoryLogs)
    std::cerr << "Memory overhead: " << overhead << " MB" << std::endl;
  m_dataReceived++;
  Consumer::OnData(data);
}

} // namespace ndn
} // namespace ns3
