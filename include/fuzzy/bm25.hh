#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <ostream>
#include <algorithm>
#include <unordered_set>
#include <math.h> 

#include <fuzzy/filter.hh>

#include <boost/multi_array.hpp>
#include <boost/format.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/array.hpp>

namespace fuzzy
{
  // Sentence ID -> BM25-score
  class BM25 : public Filter
  {
  public:
    BM25(float k1=1.5, float b=0.75);
    ~BM25();
    unsigned add_sentence(const std::vector<unsigned>& sentence) override;

    using Filter::dump;
    using Filter::num_sentences;
    using Filter::get_sentence;

    void sort(size_t vocab_size);

    std::ostream& dump(std::ostream&) const;

    unsigned get_sentence_length(size_t s_id) const;

    double bm25_score_pattern(
      unsigned s_id,
      std::vector<unsigned> pattern_wids) const;

    double bm25_score(
      unsigned term,
      unsigned s_id,
      double avg_doc_length,
      boost::multi_array<unsigned, 2>& tf,
      std::vector<float>& idf);

  private:
    bool _sorted = false;

    // BM25 (t, d) cache
    boost::multi_array<float, 2>* _bm25 = nullptr;
    // BM25 usual parameters
    float _k1 = 1.5;
    float _b = 0.75;

    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive&, unsigned int version) const;

    template<class Archive>
    void load(Archive&, unsigned int version);

    BOOST_SERIALIZATION_SPLIT_MEMBER()
  };
}

BOOST_CLASS_VERSION(fuzzy::BM25, 1)

#include "fuzzy/bm25.hxx"
