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
    const static size_t MAX_TOKENS_IN_PATTERN = 300; // if you change this value, update README.md

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

  private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version);

    VocabIndexer _vocabIndexer;
    SuffixArray  _suffixArray;
    std::vector<std::string> _ids;
    std::vector<Sentence>    _real_tokens;
  };
}

#include <fuzzy/suffix_array_index.hxx>
