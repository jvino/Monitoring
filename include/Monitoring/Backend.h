///
/// \file Backend.h
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#ifndef ALICEO2_MONITORING_CORE_BACKEND_H
#define ALICEO2_MONITORING_CORE_BACKEND_H

#include <chrono>
#include <string>
#include "Monitoring/Metric.h"

namespace o2
{
/// ALICE O2 Monitoring system
namespace monitoring 
{

/// \brief Backend pure virtual interface
///
/// Interface that allows to send a metric to remote backend.
/// In addition, default tagset (for all handled metrics) can be created.
class Backend
{
  public:
    /// Default constructor
    Backend() = default;

    /// Default destructor
    virtual ~Backend() = default;
	
    /// Sends metric via backend
    virtual void send(const Metric& metric) = 0;

    /// Sends multiple metrics not related to each other
    virtual void send(std::vector<Metric>&& metrics) = 0;

    /// Sends multiple related to each other metrics under a common name
    /// If not supported by backend, falls back into sending single metrics
    virtual void sendMultiple(std::string measurement, std::vector<Metric>&& metrics) = 0;

    /// Sets a tag
    virtual void addGlobalTag(std::string name, std::string value) = 0;
};

} // namespace monitoring
} // namespace o2

#endif // ALICEO2_MONITORING_CORE_BACKEND_H
