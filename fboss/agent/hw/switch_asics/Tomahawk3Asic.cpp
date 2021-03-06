// Copyright 2004-present Facebook. All Rights Reserved.

#include "fboss/agent/hw/switch_asics/Tomahawk3Asic.h"

namespace facebook::fboss {

bool Tomahawk3Asic::isSupported(Feature feature) const {
  switch (feature) {
    case HwAsic::Feature::ACLv4:
    case HwAsic::Feature::ACLv6:
    case HwAsic::Feature::SPAN:
    case HwAsic::Feature::ERSPANv4:
    case HwAsic::Feature::SFLOWv4:
    case HwAsic::Feature::MPLS:
    case HwAsic::Feature::MPLS_ECMP:
    case HwAsic::Feature::TRUNCATE_MIRROR_PACKET:
    case HwAsic::Feature::ERSPANv6:
    case HwAsic::Feature::SFLOWv6:
    case HwAsic::Feature::HOT_SWAP:
    case HwAsic::Feature::HASH_FIELDS_CUSTOMIZATION:
    case HwAsic::Feature::QUEUE:
    case HwAsic::Feature::ECN:
    case HwAsic::Feature::L3_QOS:
    case HwAsic::Feature::WARM_BOOT:
    case HwAsic::Feature::ROUTE_METADATA:
    case HwAsic::Feature::SCHEDULER_PPS:
    case HwAsic::Feature::NEXTHOP_TTL_DECREMENT_DISABLE:
      return true;

    case HwAsic::Feature::HOSTTABLE_FOR_HOSTROUTES:
    case HwAsic::Feature::TX_VLAN_STRIPPING_ON_PORT:
    case HwAsic::Feature::QOS_MAP_GLOBAL:
    case HwAsic::Feature::QCM:
    case HwAsic::Feature::SMAC_EQUALS_DMAC_CHECK_ENABLED:
    case HwAsic::Feature::PORT_TTL_DECREMENT_DISABLE:
    case HwAsic::Feature::PORT_INTERFACE_TYPE:
      return false;
  }
  return false;
}

} // namespace facebook::fboss
