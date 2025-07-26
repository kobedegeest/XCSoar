// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Device/Descriptor.hpp"

class Path;
class Port;
class RecordedFlightList;
struct RecordedFlightInfo;
class OperationEnvironment;

namespace Nano {
  bool ReadFlightList(Port &port, RecordedFlightList &flight_list,
                      OperationEnvironment &env);

  bool DownloadFlight(DeviceDescriptor &device, Port &port, const RecordedFlightInfo &flight,
                      Path path, OperationEnvironment &env);
}
