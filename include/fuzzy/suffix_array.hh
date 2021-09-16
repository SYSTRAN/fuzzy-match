#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <ostream>

#include "fuzzy/vocab_indexer.hh"

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

  class SuffixArray;

  /***compare 2 offset on a suffix array*/
  struct  suffix_array_sorter
  {
    suffix_array_sorter(const SuffixArray& ref)
      : _ref(ref)
    {
    }

    inline bool operator()(const SuffixView& a, const SuffixView& b) const;
    const SuffixArray& _ref;
  };

  class SuffixArray
  {
    friend struct suffix_array_sorter;
    friend std::ostream& operator<<(std::ostream&, const SuffixArray&);
  public:
    SuffixArray(VocabIndexer* VI);
    SuffixArray();
    SuffixArray(const SuffixArray& other);
    ~SuffixArray();
    void clear();

    unsigned add_sentence(const std::vector<unsigned>& sentence);
    void sort(size_t vocab_size);

    unsigned operator[](const SuffixView&) const;
    unsigned operator[](size_t) const;
    const std::vector<unsigned> &sentence_buffer() const;

    std::ostream& dump(std::ostream&) const;

    size_t nsentences() const;

    inline const unsigned* get_sentence(std::size_t sentence_id, std::size_t* length) const;
    inline const unsigned* get_suffix(const SuffixView& p, std::size_t* length) const;
    inline unsigned short sentence_length(std::size_t suffix_id) const;

    /** range of suffixe starting with ngram; return an open range so the number of elemem is just reS.second-res.first **/
    std::pair<size_t, size_t> equal_range(const unsigned* ngram,
                                          size_t length,
                                          size_t min = 0,
                                          size_t max = 0) const;

    /** map suffix id -> sentence position */
    const std::vector<SuffixView>   &suffixid2sentenceid() const;

  private:
    int comp(const SuffixView& a, const SuffixView& b) const;
    void compute_sentence_length();
    int start_by(const SuffixView& p, const unsigned* ngram, size_t length) const;

    bool _sorted;
    mutable VocabIndexer* _vocab; //not owned

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
