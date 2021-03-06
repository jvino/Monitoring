///
/// \file InfluxDB.cxx
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#include "InfluxDB.h"
#include <boost/lexical_cast.hpp>
#include <string>
#include "../Transports/UDP.h"
#include "../Transports/HTTP.h"
#include "../Exceptions/MonitoringInternalException.h"

namespace o2
{
/// ALICE O2 Monitoring system
namespace monitoring
{
/// Monitoring backends
namespace backends
{

InfluxDB::InfluxDB(const std::string& host, unsigned int port)
{
  transport = std::make_unique<transports::UDP>(host, port);
  MonLogger::Get() << "InfluxDB/UDP backend initialized"
                   << " ("<< host << ":" << port << ")" << MonLogger::End();
}

InfluxDB::InfluxDB(const std::string& host, unsigned int port, const std::string& path)
{
  transport = std::make_unique<transports::HTTP>(
    "http://" + host + ":" + std::to_string(port) + "/?" + path
  );
  MonLogger::Get() << "InfluxDB/HTTP backend initialized" << " (" << "http://" << host
                   << ":" <<  std::to_string(port) << "/?" << path << ")" << MonLogger::End();
}

inline unsigned long InfluxDB::convertTimestamp(const std::chrono::time_point<std::chrono::system_clock>& timestamp)
{
  return std::chrono::duration_cast <std::chrono::nanoseconds>(
    timestamp.time_since_epoch()
  ).count();
}

void InfluxDB::escape(std::string& escaped)
{
  boost::replace_all(escaped, ",", "\\,");
  boost::replace_all(escaped, "=", "\\=");
  boost::replace_all(escaped, " ", "\\ ");
}

void InfluxDB::sendMultiple(std::string measurement, std::vector<Metric>&& metrics)
{
  escape(measurement);
  std::stringstream convert;
  convert << measurement << "," << tagSet << " ";

  for (const auto& metric : metrics) {
    std::string value = boost::lexical_cast<std::string>(metric.getValue());
    prepareValue(value, metric.getType());
    convert << metric.getName() << "=" << value << ",";
  }
  convert.seekp(-1, std::ios_base::end);
  convert << " " <<  convertTimestamp(metrics.back().getTimestamp());

  try {
    transport->send(convert.str());
  } catch (MonitoringInternalException&) {
  }
}

void InfluxDB::send(std::vector<Metric>&& metrics) {
  std::string influxMetrics = "";
  for (const auto& metric : metrics) {
    influxMetrics += toInfluxLineProtocol(metric);
    influxMetrics += "\n";
  }

  try {
    transport->send(std::move(influxMetrics));
  } catch (MonitoringInternalException&) {
  }

}

void InfluxDB::send(const Metric& metric)
{
  try {
    transport->send(toInfluxLineProtocol(metric));
  } catch (MonitoringInternalException&) {
  }
}

std::string InfluxDB::toInfluxLineProtocol(const Metric& metric) {
  std::string metricTags{};
  for (const auto& tag : metric.getTags()) {
    metricTags += "," + tag.name + "=" + tag.value;
  }

  std::string value = boost::lexical_cast<std::string>(metric.getValue());
  prepareValue(value, metric.getType());
  std::string name = metric.getName();
  escape(name);

  std::stringstream convert;
  convert << name << "," << tagSet << metricTags << " value=" << value << " " << convertTimestamp(metric.getTimestamp());
  return convert.str();
}

void InfluxDB::prepareValue(std::string& value, int type)
{
  if (type == MetricType::STRING) {
    escape(value);
    value.insert(value.begin(), '"');
    value.insert(value.end(), '"');
  }

  if (type == MetricType::INT) {
    value.insert(value.end(), 'i');
  }
}

void InfluxDB::addGlobalTag(std::string name, std::string value)
{
  escape(name); escape(value);
  if (!tagSet.empty()) tagSet += ",";
  tagSet += name + "=" + value;
}

} // namespace backends
} // namespace monitoring
} // namespace o2
