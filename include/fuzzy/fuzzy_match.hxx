#include <boost/serialization/version.hpp>

namespace fuzzy
{
  template<class Archive>
  void
  FuzzyMatch::serialize(Archive& ar, const unsigned int version)
  {
    ar &
    _pt &
    _suffixArrayIndex;
  }
}

BOOST_CLASS_VERSION(fuzzy::FuzzyMatch, 1)
