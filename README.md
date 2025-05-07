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

#### Abstract

With the increasing prevalence of IoT devices, low-power and lossy networks (LLNs) have become integral to many applications. One commonly used routing protocol in these networks is the RPL (Routing Protocol for Low-Power and Lossy Networks). However, RPL is vulnerable to various security threats, particularly wormhole attacks, where two or more malicious nodes collude to create a tunnel, forwarding packets between distant network points. This attack disrupts normal routing, increases delays, and may even lead to network partitioning.

In this project, we developed a simulation of a wormhole attack within an IoT network using the Contiki-NG operating system and the Cooja simulator. The goal was to demonstrate the impact of a wormhole attack on RPL-based networks and to propose a method to detect such attacks without the need for additional hardware. Our detection approach involves monitoring the network for abnormal packet forwarding patterns and measuring discrepancies in the expected round-trip times of packets. Identifying these anomalies can help us locate the wormhole nodes and mitigate the impact of the attack.

The results of our implementation show that it is feasible to detect wormhole attacks in resource-constrained IoT environments using software-based techniques. This is significant as it enhances the security of IoT networks without requiring additional infrastructure, thus providing a cost-effective solution to protect low-power, lossy networks from malicious interference.
