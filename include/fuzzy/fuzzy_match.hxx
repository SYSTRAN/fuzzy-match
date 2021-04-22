#include <boost/serialization/version.hpp>
#include <boost/make_unique.hpp>

namespace fuzzy
{
  template<class Archive>
  void FuzzyMatch::save(Archive& archive, unsigned int) const
  {
    // Older versions of operator& (e.g. in Boost 1.58) do not accept a rvalue pointer,
    // so we first store the pointer value in a local variable.
    const SuffixArrayIndex* suffixArrayIndex = _suffixArrayIndex.get();
    archive
    & _pt
    & suffixArrayIndex;
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
