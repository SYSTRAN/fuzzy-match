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
  class Filter
  {
  public:
    virtual unsigned add_sentence(const std::vector<unsigned>& sentence);
    virtual void sort(size_t vocab_size) = 0;

    std::ostream& dump(std::ostream&) const;

    size_t num_sentences() const;

    const unsigned* get_sentence(size_t sentence_id, size_t* length = nullptr) const;
    unsigned short get_sentence_length(size_t suffix_id) const;

  protected:
    virtual void compute_sentence_length() = 0;

    bool _sorted = false;

    // the concatenated sentences, as 0-terminated sequences of vocab
    std::vector<unsigned>         _sentence_buffer;
    // sentence id > position in sentence buffer
    std::vector<unsigned>         _sentence_pos;
    /* index first word in _sentences */
    std::vector<unsigned>         _quickVocabAccess;
    // cache friendly access to the sentence length associated with the prefix (used to speed up NGramMatches::register_ranges)
    std::vector<unsigned short> _sentence_length;

    friend class boost::serialization::access;

    // template<class Archive>
    // void save(Archive&, unsigned int version) const;

    // template<class Archive>
    // void load(Archive&, unsigned int version);

    // BOOST_SERIALIZATION_SPLIT_MEMBER()
  };
}

BOOST_CLASS_VERSION(fuzzy::Filter, 1)

#include "fuzzy/filter.hxx"
