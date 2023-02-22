#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <ostream>

#include <fuzzy/filter.hh>

#include <boost/multi_array.hpp>
#include <boost/format.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>

namespace fuzzy
{
  class BM25 : public Filter
  {
  public:
    unsigned add_sentence(const std::vector<unsigned>& sentence) override;

    using Filter::dump;
    using Filter::num_sentences;
    using Filter::get_sentence;
    using Filter::get_sentence_length;

    void sort(size_t vocab_size);

    std::ostream& dump(std::ostream&) const;

    size_t num_sentences() const;

    const unsigned* get_sentence(size_t sentence_id, size_t* length = nullptr) const;
    unsigned short get_sentence_length(size_t suffix_id) const;

    double bm25_score(unsigned term, unsigned s_id, double avg_doc_length);
    double bm25_score(std::vector<unsigned> query, unsigned s_id);
    void compute_bm25_cache();

  private:
    void compute_sentence_length() override;
    
    bool _sorted = false;

    unsigned vocab_size;
    // BM25 cache
    boost::multi_array<unsigned, 2> _bm25;
    // Term-Document frequency
    boost::multi_array<unsigned, 2> _tf;
    // IDF
    std::vector<unsigned>& _idf;
    // Total number of tokens
    unsigned _num_tokens;
    // BM25 usual parameters
    float _k1;
    float _b;
    

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
