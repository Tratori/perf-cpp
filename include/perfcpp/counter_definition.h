#pragma once

#include "counter.h"
#include "metric.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace perf {
class CounterDefinition
{
public:
  explicit CounterDefinition(const std::string& config_file);
  explicit CounterDefinition(std::string&& config_file)
    : CounterDefinition(config_file)
  {
  }
  CounterDefinition();

  CounterDefinition(CounterDefinition&&) noexcept = default;
  CounterDefinition& operator=(CounterDefinition&&) noexcept = default;

  ~CounterDefinition() = default;

  void add(std::string&& name, const std::uint32_t type, const std::uint64_t event_id)
  {
    add(std::move(name), CounterConfig{ type, event_id });
  }

  void add(std::string&& name, CounterConfig config)
  {
    _counter_configs.insert(std::make_pair(std::move(name), config));
  }

  void add(std::string&& name, std::unique_ptr<Metric>&& metric)
  {
    _metrics.insert(std::make_pair(std::move(name), std::move(metric)));
  }

  void add(std::unique_ptr<Metric>&& metric) { _metrics.insert(std::make_pair(metric->name(), std::move(metric))); }

  void perf::CounterDefinition::clear_counter_configuration()
  {
    this->_counter_configs.clear();
    this->_metrics.clear();
  }

  [[nodiscard]] std::optional<std::pair<std::string_view, CounterConfig>> counter(std::string&& name) const noexcept
  {
    return counter(name);
  }
  [[nodiscard]] std::optional<std::pair<std::string_view, CounterConfig>> counter(
    const std::string& name) const noexcept;
  [[nodiscard]] std::optional<std::pair<std::string_view, CounterConfig>> counter(
    const std::string_view name) const noexcept
  {
    return counter(std::string{ name });
  }
  [[nodiscard]] bool is_metric(const std::string& name) const noexcept { return _metrics.find(name) != _metrics.end(); }
  [[nodiscard]] bool is_metric(std::string_view name) const noexcept { return is_metric(std::string{ name }); }
  [[nodiscard]] std::optional<std::pair<std::string_view, Metric&>> metric(const std::string& name) const noexcept;
  [[nodiscard]] std::optional<std::pair<std::string_view, Metric&>> metric(std::string&& name) const noexcept { return metric(name); }
  [[nodiscard]] std::optional<std::pair<std::string_view, Metric&>> metric(const std::string_view name) const noexcept { return metric(std::string{name.data(), name.size()}); }

  [[nodiscard]] std::vector<std::string> names() const
  {
    auto names = std::vector<std::string>{};
    std::transform(_counter_configs.begin(), _counter_configs.end(), std::back_inserter(names), [](const auto& config) {
      return config.first;
    });
    return names;
  }

  void read_counter_configuration(const std::string& config_file);

private:
  std::unordered_map<std::string, CounterConfig> _counter_configs;
  std::unordered_map<std::string, std::unique_ptr<Metric>> _metrics;

  void initialized_default_counters();
};
}