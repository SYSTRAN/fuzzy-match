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
  struct SuffixView
  {
    unsigned sentence_id;
    unsigned short subsentence_pos;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& archive, const unsigned int);
  };

  class SuffixArray
  {
  public:
    unsigned add_sentence(const std::vector<unsigned>& sentence);
    void sort(size_t vocab_size);

    std::ostream& dump(std::ostream&) const;

    size_t num_sentences() const;

    const unsigned* get_sentence(size_t sentence_id, size_t* length = nullptr) const;
    const unsigned* get_suffix(const SuffixView& p, size_t* length = nullptr) const;
    const SuffixView& get_suffix_view(size_t suffix_id) const;
    unsigned short get_sentence_length(size_t suffix_id) const;

    /** range of suffixe starting with ngram; return an open range so the number of elemem is just reS.second-res.first **/
    std::pair<size_t, size_t> equal_range(const unsigned* ngram,
                                          size_t length,
                                          size_t min = 0,
                                          size_t max = 0) const;

  private:
    int comp(const SuffixView& a, const SuffixView& b) const;
    void compute_sentence_length();
    int start_by(const SuffixView& p, const unsigned* ngram, size_t length) const;

    bool _sorted = false;

    // ordered sequence of sentence id, pos in sentence
    std::vector<SuffixView>              _suffixes;
    // the concatenated sentences, as 0-terminated sequences of vocab
    std::vector<unsigned>         _sentence_buffer;
    // sentence id > position in sentence buffer
    std::vector<unsigned>         _sentence_pos;
    /* index first word in _sentences */
    std::vector<unsigned>         _quickVocabAccess;
    // cache friendly access to the sentence length associated with the prefix (used to speed up NGramMatches::register_ranges)
    std::vector<unsigned short> _sentence_length;

    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive&, unsigned int version) const;

    template<class Archive>
    void load(Archive&, unsigned int version);

    BOOST_SERIALIZATION_SPLIT_MEMBER()
  };
}

BOOST_CLASS_VERSION(fuzzy::SuffixArray, 1)

#include "fuzzy/suffix_array.hxx"
