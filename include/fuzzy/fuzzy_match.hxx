#include <boost/serialization/version.hpp>
#include <boost/make_unique.hpp>

namespace fuzzy
{
  inline size_t FuzzyMatch::max_tokens_in_pattern() const
  {
    return _filterIndex->max_tokens_in_pattern();
  }

  template<class Archive>
  void FuzzyMatch::save(Archive& archive, unsigned int) const
  {
    // Older versions of operator& (e.g. in Boost 1.58) do not accept a rvalue pointer,
    // so we first store the pointer value in a local variable.
    const FilterIndex* filterIndex = _filterIndex.get();
    archive
    & _pt
    & filterIndex;
  }

  template<class Archive>
  void FuzzyMatch::load(Archive& archive, unsigned int)
  {
    // IndexType type = _filterIndex->getType();
    // FilterIndex* filterIndex = new FilterIndex(type);
    // if (_filterIndex->getType() == IndexType::BM25)
    // {
    //   std::cerr << "previous is BM25..." << std::endl;

    //   // filterIndex = new BM25();
    // }
    // // else if (_filterIndex->_type == IndexType::SUFFIX)
    // // {
    // //   // filterIndex = new SuffixArray();
    // // }
    // // FilterIndex* filterIndex = nullptr;

    // if (filterIndex->getType() == IndexType::BM25)
    //   std::cerr << "should load BM25..." << std::endl;

    // std::cerr << "loading fuzzy_match..." << std::endl;

    FilterIndex* filterIndex = nullptr;
    archive &
    _pt &
    filterIndex;

    _filterIndex = std::unique_ptr<FilterIndex>(filterIndex);
  }
}

BOOST_CLASS_VERSION(fuzzy::FuzzyMatch, 1)
