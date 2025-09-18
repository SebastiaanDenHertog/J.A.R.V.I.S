#include "utils.h"
#include "hash.h"
#include <cstddef>
#include <map>
#include <utility>

  namespace prometheus::detail
  {
    std::size_t LabelHasher::operator()(const Labels &labels) const
    {
      std::size_t seed = 0;
      for (auto &label : labels)
      {
        hash_combine(&seed, label.first, label.second);
      }
      return seed;
    }
  } // namespace prometheus::detail