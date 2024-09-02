#pragma once

#include <string>

#include "detail/core_export.h"
#include "metric_type.h"

namespace prometheus
{

    PROMETHEUS_EXPORT bool CheckMetricName(const std::string &name);
    PROMETHEUS_EXPORT bool CheckLabelName(const std::string &name,
                                          MetricType type);
} // namespace prometheus
