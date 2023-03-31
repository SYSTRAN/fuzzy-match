#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <ostream>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>

namespace fuzzy
{
  struct FilterIndexParams 
  {
    float bm25_k1;
    float bm25_b;
    float bm25_ratio_idf;
    FilterIndexParams(
      float ratio_idf=0.5,
      float k1=1.5,
      float b=0.75) :
      bm25_k1(k1),
      bm25_b(b),
      bm25_ratio_idf(ratio_idf) {}
  };
  class Filter
  {
  public:
    virtual ~Filter() = default;
    // Filter(const FilterIndexParams& params);
    virtual unsigned add_sentence(const std::vector<unsigned>& sentence);
    virtual void prepare(size_t vocab_size) = 0;

    std::ostream& dump(std::ostream&) const;

    size_t num_sentences() const;

    const unsigned* get_sentence(size_t sentence_id, size_t* length = nullptr) const;

  protected:
    bool _prepared = false;

    // the concatenated sentences, as 0-terminated sequences of vocab
    std::vector<unsigned>         _sentence_buffer;
    // sentence id > position in sentence buffer
    std::vector<unsigned>         _sentence_pos;
    /* index first word in _sentences */
    std::vector<unsigned>         _quickVocabAccess;

    friend class boost::serialization::access;
  };
}

BOOST_CLASS_VERSION(fuzzy::Filter, 1)

#include "fuzzy/filter.hxx"
