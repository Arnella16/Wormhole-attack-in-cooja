<img src="https://github.com/contiki-ng/contiki-ng.github.io/blob/master/images/logo/Contiki_logo_2RGB.png" alt="Logo" width="256">

# Contiki-NG: The OS for Next Generation IoT Devices

[![Github Actions](https://github.com/contiki-ng/contiki-ng/workflows/CI/badge.svg?branch=develop)](https://github.com/contiki-ng/contiki-ng/actions)
[![Documentation Status](https://readthedocs.org/projects/contiki-ng/badge/?version=master)](https://contiki-ng.readthedocs.io/en/master/?badge=master)
[![license](https://img.shields.io/badge/license-3--clause%20bsd-brightgreen.svg)](https://github.com/contiki-ng/contiki-ng/blob/master/LICENSE.md)
[![Latest release](https://img.shields.io/github/release/contiki-ng/contiki-ng.svg)](https://github.com/contiki-ng/contiki-ng/releases/latest)
[![GitHub Release Date](https://img.shields.io/github/release-date/contiki-ng/contiki-ng.svg)](https://github.com/contiki-ng/contiki-ng/releases/latest)
[![Last commit](https://img.shields.io/github/last-commit/contiki-ng/contiki-ng.svg)](https://github.com/contiki-ng/contiki-ng/commit/HEAD)

[![Stack Overflow Tag](https://img.shields.io/badge/Stack%20Overflow%20tag-Contiki--NG-blue?logo=stackoverflow)](https://stackoverflow.com/questions/tagged/contiki-ng)
[![Gitter](https://img.shields.io/badge/Gitter-Contiki--NG-blue?logo=gitter)](https://gitter.im/contiki-ng)
[![Twitter](https://img.shields.io/badge/Twitter-%40contiki__ng-blue?logo=twitter)](https://twitter.com/contiki_ng)

Contiki-NG is an open-source, cross-platform operating system for Next-Generation IoT devices. It focuses on dependable (secure and reliable) low-power communication and standard protocols, such as IPv6/6LoWPAN, 6TiSCH, RPL, and CoAP. Contiki-NG comes with extensive documentation, tutorials, a roadmap, release cycle, and well-defined development flow for smooth integration of community contributions.

Unless explicitly stated otherwise, Contiki-NG sources are distributed under
the terms of the [3-clause BSD license](LICENSE.md). This license gives
everyone the right to use and distribute the code, either in binary or
source code format, as long as the copyright license is retained in
the source code.

Contiki-NG started as a fork of the Contiki OS and retains some of its original features.

### Abstract

With the increasing prevalence of IoT devices, low-power and lossy networks (LLNs) have become integral to many applications. One commonly used routing protocol in these networks is the RPL (Routing Protocol for Low-Power and Lossy Networks). However, RPL is vulnerable to various security threats, particularly wormhole attacks, where two or more malicious nodes collude to create a tunnel, forwarding packets between distant network points. This attack disrupts normal routing, increases delays, and may even lead to network partitioning.

In this project, we developed a simulation of a wormhole attack within an IoT network using the Contiki-NG operating system and the Cooja simulator. The goal was to demonstrate the impact of a wormhole attack on RPL-based networks and to propose a method to detect such attacks without the need for additional hardware. Our detection approach involves monitoring the network for abnormal packet forwarding patterns and measuring discrepancies in the expected round-trip times of packets. Identifying these anomalies can help us locate the wormhole nodes and mitigate the impact of the attack.

The results of our implementation show that it is feasible to detect wormhole attacks in resource-constrained IoT environments using software-based techniques. This is significant as it enhances the security of IoT networks without requiring additional infrastructure, thus providing a cost-effective solution to protect low-power, lossy networks from malicious interference.

### Literature review 

The paper "A Novel Detection and Localization Scheme for Wormhole Attack in Internet of Things" proposes a novel detection and localization scheme specifically designed to identify and locate wormhole attacks in Internet of Things (IoT) networks. The main contribution is the development of a lightweight, energy-efficient, and accurate mechanism that addresses the challenges of detecting wormhole attacks in resource-constrained IoT environments.

The authors introduced a detection scheme that is energy-efficient and designed to operate within the limitations of typical IoT devices. This is especially important because IoT devices are often battery-powered and have limited processing capabilities, which can hinder the implementation of complex security mechanisms. Thus, a solution that minimizes computational overhead while effectively detecting attacks is crucial for the practical deployment of IoT networks.

The detection scheme is based on analyzing the timing and distance between packets sent within the network. By observing the Time-to-Live (TTL) field in the packet headers, the scheme can identify unusual behaviors or discrepancies caused by the wormhole tunnel. The TTL value is an effective measure because wormhole attacks often result in abnormal packet forwarding behavior, causing discrepancies in the TTL values as packets travel through the malicious tunnel, bypassing the normal routing path.

### Implementation of the Attack

The wormhole attack was implemented using the Contiki-NG operating system and the Cooja network simulator, which provides a visual and interactive environment to emulate IoT networks. 

To download and install cooja simulator follow [Step-by-Step: Setup Contiki-NG Cooja IoT Network Simulator on Linux System](https://www.youtube.com/watch?v=a4radETt04U)

The implementation proceeded in two phases: first, we set up a baseline (normal) topology consisting entirely of legitimate nodes, and second, we simulated the wormhole attack by reconfiguring two specific nodes (nodes 7 and 8) as malicious.

The topologies are as follows 

<p>
    <img src="examples/rpl-udp/Normal too.drawio.png" hspace="5" width="450">
    <img src="examples/rpl-udp/Wormhole attack.drawio (3).png" hspace="5" width="450">
</p>

- **Node 6**: Acts as the *Border Router (root node)*. It connects the internal IoT network to the external Internet (represented by a cloud symbol).

- **Nodes 1, 2, 3, 4 and 5**: These are all *normal UDP client nodes*, responsible for sensing and forwarding data within the network.

- **Nodes 7 and 8**: These are *normal UDP client nodes* in the normal simulation, and act as wormhole nodes which intercept and modify the packets and forward the packets to each other in the wormhole simulation.


