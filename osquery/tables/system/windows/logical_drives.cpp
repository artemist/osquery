/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed in accordance with the terms specified in
 *  the LICENSE file found in the root directory of this source tree.
 */

#include <unordered_set>

#include "osquery/core/windows/wmi.h"
#include <osquery/tables.h>

namespace osquery {
namespace tables {

QueryData genLogicalDrives(QueryContext& context) {
  QueryData results;

  const WmiRequest wmiLogicalDiskReq(
      "select DeviceID, Description, FreeSpace, Size, FileSystem from "
      "Win32_LogicalDisk");
  auto const& logicalDisks = wmiLogicalDiskReq.results();

  const WmiRequest wmiBootConfigurationReq(
      "select BootDirectory from Win32_BootConfiguration");
  auto const& bootConfigurations = wmiBootConfigurationReq.results();
  std::unordered_set<char> bootDeviceIds;

  for (const auto& bootConfiguration : bootConfigurations) {
    std::string bootDirectory;
    bootConfiguration.GetString("BootDirectory", bootDirectory);
    bootDeviceIds.insert(bootDirectory.at(0));
  }

  for (const auto& logicalDisk : logicalDisks) {
    Row r;
    std::string deviceId;
    logicalDisk.GetString("DeviceID", deviceId);
    logicalDisk.GetString("Description", r["description"]);
    logicalDisk.GetString("FreeSpace", r["free_space"]);
    logicalDisk.GetString("Size", r["size"]);
    logicalDisk.GetString("FileSystem", r["file_system"]);

    if (r["free_space"].empty()) {
      r["free_space"] = "-1";
    }

    if (r["size"].empty()) {
      r["size"] = "-1";
    }

    // NOTE(ww): Previous versions of this table used the type
    // column to provide a non-canonical description of the drive.
    // However, a bug in WMI marshalling caused the type to always
    // return "Unknown". That behavior is preserved here.
    r["type"] = "Unknown";
    r["device_id"] = deviceId;
    r["boot_partition"] = INTEGER(bootDeviceIds.count(deviceId.at(0)));

    results.push_back(std::move(r));
  }
  return results;
}
} // namespace tables
} // namespace osquery
