/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "fboss/agent/platforms/common/fake_test/FakeTestPlatformMapping.h"

#include <folly/Format.h>

namespace {
using namespace facebook::fboss;
static const std::unordered_map<int, std::vector<cfg::PortProfileID>>
    kPortProfilesInGroup = {
        {0,
         {
             cfg::PortProfileID::PROFILE_100G_4_NRZ_CL91_OPTICAL,
             cfg::PortProfileID::PROFILE_50G_2_NRZ_CL74_COPPER,
             cfg::PortProfileID::PROFILE_40G_4_NRZ_NOFEC_OPTICAL,
             cfg::PortProfileID::PROFILE_25G_1_NRZ_NOFEC_OPTICAL,
             cfg::PortProfileID::PROFILE_10G_1_NRZ_NOFEC_OPTICAL,
         }},
        {1,
         {
             cfg::PortProfileID::PROFILE_25G_1_NRZ_NOFEC_OPTICAL,
             cfg::PortProfileID::PROFILE_10G_1_NRZ_NOFEC_OPTICAL,
         }},
        {2,
         {
             cfg::PortProfileID::PROFILE_50G_2_NRZ_CL74_COPPER,
             cfg::PortProfileID::PROFILE_25G_1_NRZ_NOFEC_OPTICAL,
             cfg::PortProfileID::PROFILE_10G_1_NRZ_NOFEC_OPTICAL,
         }},
        {3,
         {
             cfg::PortProfileID::PROFILE_25G_1_NRZ_NOFEC_OPTICAL,
             cfg::PortProfileID::PROFILE_10G_1_NRZ_NOFEC_OPTICAL,
         }},
};

static const std::unordered_map<
    cfg::PortProfileID,
    std::tuple<
        cfg::PortSpeed,
        int,
        phy::FecMode,
        TransmitterTechnology,
        phy::InterfaceMode>>
    kProfiles = {
        {cfg::PortProfileID::PROFILE_100G_4_NRZ_CL91_OPTICAL,
         std::make_tuple(
             cfg::PortSpeed::HUNDREDG,
             4,
             phy::FecMode::CL91,
             TransmitterTechnology::OPTICAL,
             phy::InterfaceMode::CAUI)},
        {cfg::PortProfileID::PROFILE_50G_2_NRZ_CL74_COPPER,
         std::make_tuple(
             cfg::PortSpeed::FIFTYG,
             2,
             phy::FecMode::CL74,
             TransmitterTechnology::COPPER,
             phy::InterfaceMode::CR2)},
        {cfg::PortProfileID::PROFILE_40G_4_NRZ_NOFEC_OPTICAL,
         std::make_tuple(
             cfg::PortSpeed::FORTYG,
             4,
             phy::FecMode::NONE,
             TransmitterTechnology::OPTICAL,
             phy::InterfaceMode::XLAUI)},
        {cfg::PortProfileID::PROFILE_25G_1_NRZ_NOFEC_OPTICAL,
         std::make_tuple(
             cfg::PortSpeed::TWENTYFIVEG,
             1,
             phy::FecMode::NONE,
             TransmitterTechnology::OPTICAL,
             phy::InterfaceMode::CAUI)},
        {cfg::PortProfileID::PROFILE_10G_1_NRZ_NOFEC_OPTICAL,
         std::make_tuple(
             cfg::PortSpeed::XG,
             1,
             phy::FecMode::NONE,
             TransmitterTechnology::OPTICAL,
             phy::InterfaceMode::SFI)},
};
} // namespace

namespace facebook {
namespace fboss {
FakeTestPlatformMapping::FakeTestPlatformMapping(
    std::vector<int> controllingPortIds)
    : PlatformMapping(), controllingPortIds_(std::move(controllingPortIds)) {
  for (auto itProfile : kProfiles) {
    phy::PortProfileConfig profile;
    *profile.speed_ref() = std::get<0>(itProfile.second);
    *profile.iphy_ref()->numLanes_ref() = std::get<1>(itProfile.second);
    *profile.iphy_ref()->fec_ref() = std::get<2>(itProfile.second);
    setSupportedProfile(itProfile.first, profile);
  }

  for (int groupID = 0; groupID < controllingPortIds_.size(); groupID++) {
    auto portsInGroup = getPlatformPortEntriesByGroup(groupID);
    for (auto port : portsInGroup) {
      setPlatformPort(port.mapping.id, port);
    }

    phy::DataPlanePhyChip iphy;
    iphy.name = folly::sformat("core{}", groupID);
    iphy.type = phy::DataPlanePhyChipType::IPHY;
    iphy.physicalID = groupID;
    setChip(iphy.name, iphy);

    phy::DataPlanePhyChip tcvr;
    tcvr.name = folly::sformat("eth1/{}", groupID + 1);
    tcvr.type = phy::DataPlanePhyChipType::TRANSCEIVER;
    tcvr.physicalID = groupID;
    setChip(tcvr.name, tcvr);
  }

  CHECK(
      getPlatformPorts().size() ==
      controllingPortIds_.size() * kPortProfilesInGroup.size());
}

cfg::PlatformPortConfig FakeTestPlatformMapping::getPlatformPortConfig(
    int portID,
    int startLane,
    int groupID,
    cfg::PortProfileID profileID) {
  cfg::PlatformPortConfig platformPortConfig;
  auto& profileTuple = kProfiles.find(profileID)->second;
  auto lanes = std::get<1>(profileTuple);
  platformPortConfig.subsumedPorts_ref() = {};
  for (auto i = 1; i < lanes; i++) {
    platformPortConfig.subsumedPorts_ref()->push_back(portID + i);
  }

  // right now, we can just use iphy<->tcvr mode in fake platform
  platformPortConfig.pins.transceiver_ref() = {};
  for (auto i = 0; i < lanes; i++) {
    phy::PinConfig iphy;
    iphy.id.chip = folly::sformat("core{}", groupID);
    iphy.id.lane = (startLane + i);
    platformPortConfig.pins.iphy_ref()->push_back(iphy);

    phy::PinConfig tcvr;
    tcvr.id.chip = folly::sformat("eth1/{}", groupID + 1);
    tcvr.id.lane = (startLane + i);
    platformPortConfig.pins.transceiver_ref()->push_back(tcvr);
  }

  return platformPortConfig;
}

std::vector<cfg::PlatformPortEntry>
FakeTestPlatformMapping::getPlatformPortEntriesByGroup(int groupID) {
  std::vector<cfg::PlatformPortEntry> platformPortEntries;
  for (auto& portProfiles : kPortProfilesInGroup) {
    int portID = controllingPortIds_.at(groupID) + portProfiles.first;
    cfg::PlatformPortEntry port;
    port.mapping.id = PortID(portID);
    port.mapping.name =
        folly::sformat("eth1/{}/{}", groupID + 1, portProfiles.first + 1);
    port.mapping.controllingPort = controllingPortIds_.at(groupID);

    phy::PinConnection pinConnection;
    pinConnection.a.chip = folly::sformat("core{}", groupID);
    pinConnection.a.lane = portProfiles.first;
    phy::PinID pinEnd;
    pinEnd.chip = folly::sformat("eth1/{}", groupID + 1);
    pinEnd.lane = portProfiles.first;
    phy::Pin zPin;
    zPin.set_end(pinEnd);
    pinConnection.z_ref() = zPin;
    port.mapping.pins.push_back(pinConnection);

    for (auto profileID : portProfiles.second) {
      port.supportedProfiles_ref()->emplace(
          profileID,
          getPlatformPortConfig(
              portID, portProfiles.first, groupID, profileID));
    }

    platformPortEntries.push_back(port);
  }
  return platformPortEntries;
}
} // namespace fboss
} // namespace facebook
