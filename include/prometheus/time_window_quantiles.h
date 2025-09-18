#pragma once

#include <chrono>
#include <cstddef>
#include <vector>

#include "ckms_quantiles.h"
#include "core_export.h"

// IWYU pragma: private, include "summary.h"

  namespace prometheus::detail
  {

    class PROMETHEUS_EXPORT TimeWindowQuantiles
    {
      using Clock = std::chrono::steady_clock;

    public:
      using Duration = std::chrono::milliseconds;
      TimeWindowQuantiles(const std::vector<CKMSQuantiles::Quantile> &quantiles,
                          Duration max_age_milliseconds, int age_buckets);

      double get(double q) const;
      void insert(double value);

    private:
      CKMSQuantiles &rotate() const;

      const std::vector<CKMSQuantiles::Quantile> &quantiles_;
      mutable std::vector<CKMSQuantiles> ckms_quantiles_;
      mutable std::size_t current_bucket_;

      mutable Clock::time_point last_rotation_;
      const Clock::duration rotation_interval_;
    };

  } // namespace prometheus::detail

