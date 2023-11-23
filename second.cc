/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 3;
  bool tracing = true;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  nCsma = nCsma == 0 ? 1 : nCsma;

  NodeContainer hosts;
  hosts.Create (1);

  NodeContainer routers;
  routers.Create (4);

  NodeContainer bus;
  bus.Create (nCsma);

  InternetStackHelper stack;
  stack.Install (hosts);
  stack.Install (routers);
  stack.Install (bus);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("3ms"));

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("275Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (500)));
  
  Ipv4AddressHelper address;


  NodeContainer subnet1;
  subnet1.Add(hosts.Get(0));
  subnet1.Add(routers.Get(0));
  NetDeviceContainer sub1Device = p2p.Install(subnet1);
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer sub1Interface = address.Assign (sub1Device);


  NodeContainer subnet2;
  subnet2.Add(routers.Get(0));
  subnet2.Add(routers.Get(1));
  NetDeviceContainer sub2Device = p2p.Install(subnet2);
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer sub2Interface = address.Assign (sub2Device);

  NodeContainer subnet3;
  subnet3.Add(routers.Get(1));
  subnet3.Add(routers.Get(2));
  NetDeviceContainer sub3Device = p2p.Install(subnet3);
  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer sub3Interface = address.Assign (sub3Device);

  NodeContainer subnet4;
  subnet4.Add(routers.Get(1));
  subnet4.Add(routers.Get(3)); 
  NetDeviceContainer sub4Device = p2p.Install(subnet4);
  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer sub4Interface = address.Assign (sub4Device);


  NodeContainer subnet5;
  subnet5.Add(routers.Get(2));
  subnet5.Add(bus);
  NetDeviceContainer sub5Device = csma.Install (subnet5);
  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces = address.Assign (sub5Device);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (bus.Get (0)); //csma(bus) node 3i, ganan karanne 0,1,2 // o=server
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress ((1)), 9); //address gannakota patn ganne router ekath akka gann karanne (0,1,2,3)
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));		   // 0 = router   // 1,2,3 = bus //So, 1=server
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (2048));

  ApplicationContainer clientApps1 = echoClient.Install (subnet1.Get (0)); //1st workstation
  clientApps1.Start (Seconds (2.0));
  clientApps1.Stop (Seconds (10.0));

  //ApplicationContainer clientApps2 = echoClient.Install (bus.Get (1)); //bus node 3i, gann karanne 0,1,2, So 1= workstation
  //clientApps2.Start (Seconds (2.0));
  //clientApps2.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  AnimationInterface anim("bus.xml");
  anim.SetConstantPosition(subnet1.Get(0),10.0,10.0);
  anim.SetConstantPosition(subnet1.Get(1),20.0,20.0);

  anim.SetConstantPosition(subnet2.Get(0),30.0,30.0);
  anim.SetConstantPosition(subnet2.Get(1),40.0,40.0);

  anim.SetConstantPosition(subnet3.Get(0),50.0,50.0);
  anim.SetConstantPosition(subnet3.Get(1),60.0,60.0);

  anim.SetConstantPosition(subnet4.Get(0),70.0,70.0);
  anim.SetConstantPosition(subnet4.Get(1),80.0,80.0);

  anim.SetConstantPosition(subnet5.Get(0),90.0,90.0); //router 3
  anim.SetConstantPosition(subnet5.Get(1),100.0,100.0); //bus 1 = server
  anim.SetConstantPosition(subnet5.Get(2),110.0,110.0); //bus 2 = workstation 2 
  anim.SetConstantPosition(subnet5.Get(3),120.0,120.0); //bus 3 = printer

  
  
  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("Point2Point.tr"));
  csma.EnableAsciiAll (ascii.CreateFileStream ("Csma.tr"));


  if (tracing == true)
    {
  		p2p.EnablePcap ("subnet2", sub3Device.Get (1));
	}

  

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
