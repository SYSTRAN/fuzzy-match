#include <boost/serialization/version.hpp>
#include <boost/make_unique.hpp>

namespace fuzzy
{
  inline size_t FuzzyMatch::max_tokens_in_pattern() const
  {
    return _filterIndex->max_tokens_in_pattern();
  }

  template<class Archive>
  void FuzzyMatch::save(Archive& archive, unsigned int version) const
  {
    // Older versions of operator& (e.g. in Boost 1.58) do not accept a rvalue pointer,
    // so we first store the pointer value in a local variable.
    const FilterIndex* filterIndex = _filterIndex.get();
    archive
    & _pt
    & filterIndex;
  }

  template<class Archive>
  void FuzzyMatch::load(Archive& archive, unsigned int version)
  {
    FilterIndex* filterIndex;

    archive & _pt & filterIndex;

    _filterIndex = std::unique_ptr<FilterIndex>(filterIndex);
  }
}

BOOST_CLASS_VERSION(fuzzy::FuzzyMatch, 2)
