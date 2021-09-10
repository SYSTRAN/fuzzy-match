#pragma once

#include <fstream>
#include <string>
#include <vector>

#include <boost/serialization/vector.hpp>

#include "fuzzy/suffix_array.hh"

#include "fuzzy/sentence.hh"

namespace fuzzy
{
  class SuffixArrayIndex
  {
  public:
    static const size_t DEFAULT_MAX_TOKENS_IN_PATTERN;

    SuffixArrayIndex(size_t max_tokens_in_pattern = DEFAULT_MAX_TOKENS_IN_PATTERN);

    const SuffixArray &get_SuffixArray() const;
    VocabIndexer      &get_VocabIndexer();

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

  private:
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive&, unsigned int version) const;

    template<class Archive>
    void load(Archive&, unsigned int version);

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    VocabIndexer _vocabIndexer;
    SuffixArray  _suffixArray;
    std::vector<std::string> _ids;
    std::vector<Sentence>    _real_tokens;
    size_t _max_tokens_in_pattern;
  };
}

#include <fuzzy/suffix_array_index.hxx>
