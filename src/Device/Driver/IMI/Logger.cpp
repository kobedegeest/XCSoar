// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Internal.hpp"
#include "Protocol/Protocol.hpp"
#include "Device/Port/Port.hpp"
#include "Operation/Operation.hpp"
#include "system/Path.hpp"
#include "Device/Descriptor.hpp"

bool
IMIDevice::ReadFlightList(RecordedFlightList &flight_list,
                          OperationEnvironment &env)
{
  port.StopRxThread();

  bool success = Connect(env);
  success = success && IMI::ReadFlightList(port, flight_list, env);

  return success;
}

bool
IMIDevice::DownloadFlight(DeviceDescriptor &device, const RecordedFlightInfo &flight, Path path,
                          OperationEnvironment &env)
{
  port.StopRxThread();

  bool success = Connect(env);
  success = success && IMI::FlightDownload(port, flight, path, env);

  return success;
}
