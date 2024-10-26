#include "detail/builder.h"

#include "counter.h"
#include "gauge.h"
#include "histogram.h"
#include "info.h"
#include "registry.h"
#include "summary.h"

namespace prometheus
{

  namespace detail
  {

    template <typename T>
    Builder<T> &Builder<T>::Labels(const ::prometheus::Labels &labels)
    {
      labels_ = labels;
      return *this;
    }

    template <typename T>
    Builder<T> &Builder<T>::Name(const std::string &name)
    {
      name_ = name;
      return *this;
    }

    template <typename T>
    Builder<T> &Builder<T>::Help(const std::string &help)
    {
      help_ = help;
      return *this;
    }

    template <typename T>
    Family<T> &Builder<T>::Register(Registry &registry)
    {
      return registry.Add<T>(name_, help_, labels_);
    }

    template class PROMETHEUS_EXPORT Builder<Counter>;
    template class PROMETHEUS_EXPORT Builder<Gauge>;
    template class PROMETHEUS_EXPORT Builder<Histogram>;
    template class PROMETHEUS_EXPORT Builder<Info>;
    template class PROMETHEUS_EXPORT Builder<Summary>;

  } // namespace detail

  detail::Builder<Counter> BuildCounter() { return {}; }
  detail::Builder<Gauge> BuildGauge() { return {}; }
  detail::Builder<Histogram> BuildHistogram() { return {}; }
  detail::Builder<Info> BuildInfo() { return {}; }
  detail::Builder<Summary> BuildSummary() { return {}; }

} // namespace prometheus
