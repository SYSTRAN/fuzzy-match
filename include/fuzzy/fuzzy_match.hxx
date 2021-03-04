#include <boost/serialization/version.hpp>
#include <boost/make_unique.hpp>

namespace fuzzy
{
  template<class Archive>
  void FuzzyMatch::save(Archive& archive, unsigned int) const
  {
    archive
    & _pt
    & _suffixArrayIndex.get();
  }

  template<class Archive>
  void FuzzyMatch::load(Archive& archive, unsigned int)
  {
    SuffixArrayIndex* suffixArrayIndex = nullptr;

    archive &
    _pt &
    suffixArrayIndex;

    _suffixArrayIndex = std::unique_ptr<SuffixArrayIndex>(suffixArrayIndex);
  }
}

BOOST_CLASS_VERSION(fuzzy::FuzzyMatch, 1)
