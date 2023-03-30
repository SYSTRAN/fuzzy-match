#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <ostream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <math.h> 
#include <Eigen/Sparse>

#include <fuzzy/utils.hh>
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
  typedef Eigen::SparseMatrix<float> SpMat;
  typedef Eigen::Triplet<float> Triplet;
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

    float bm25_score_pattern(
      unsigned s_id,
      std::vector<unsigned> pattern_wids) const;

    float bm25_score(
      int term,
      int s_id,
      float avg_doc_length,
      float tf,
      std::vector<float>& idf);

  private:
    bool _sorted = false;

    size_t _vocab_size;
    // BM25 (t, d) cache
    std::vector<std::pair<std::pair<int, int>, float>> _key_value_bm25;
    SpMat _reverted_index;
    
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
