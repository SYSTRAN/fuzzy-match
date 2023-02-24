#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include <boost/serialization/vector.hpp>

#include "fuzzy/filter.hh"
#include "fuzzy/suffix_array.hh"
#include "fuzzy/bm25.hh"
#include "fuzzy/vocab_indexer.hh"
#include "fuzzy/sentence.hh"

namespace fuzzy
{
  constexpr size_t DEFAULT_MAX_TOKENS_IN_PATTERN = 300; // if you change this value, update README.md
  enum class IndexType { SUFFIX, BM25 };
  class FilterIndex
  {
  public:
    FilterIndex(
      size_t max_tokens_in_pattern = DEFAULT_MAX_TOKENS_IN_PATTERN,
      IndexType type = IndexType::SUFFIX
    );

    const Filter &get_Filter() const;
    const VocabIndexer& get_VocabIndexer() const;

    int                add_tm(const std::string& id,
                              const Sentence& real_tokens,
                              const Tokens& norm_tokens,
                              bool sort = true);
    void               sort();
    const std::string& id(unsigned int index);
    size_t             size() const;
    const Sentence    &real_tokens(size_t s_id) const;
    std::string        sentence(size_t s_id) const;
    std::ostream&      dump(std::ostream& os) const;

    size_t max_tokens_in_pattern() const;
    IndexType getType() const;

  private:
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive&, unsigned int version) const;

    template<class Archive>
    void load(Archive&, unsigned int version);

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    VocabIndexer _vocabIndexer;
    std::shared_ptr<Filter> _filter;
    inline std::shared_ptr<Filter> createSuffixArray() { return std::make_shared<SuffixArray>(); }
    inline std::shared_ptr<Filter> createBM25() { return std::make_shared<BM25>(); }
    std::vector<std::string> _ids;
    std::vector<Sentence>    _real_tokens;
    size_t _max_tokens_in_pattern;
    IndexType _type;
  };
}

#include <fuzzy/index.hxx>
