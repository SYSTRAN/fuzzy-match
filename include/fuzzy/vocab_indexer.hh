#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <ostream>

#include <boost/serialization/serialization.hpp>

namespace fuzzy
{
  // stores a unique association (word => index) where the indexes are contiguous and start at >= 2
  class VocabIndexer
  {
  public:
    typedef unsigned index_t;

    const static index_t SENTENCE_SEPARATOR; // 0
    const static index_t VOCAB_UNK; // 1

    VocabIndexer();

    size_t                                size() const;

    VocabIndexer::index_t                 addWord(const std::string& word);
    std::vector<VocabIndexer::index_t>    addWords(const std::vector<std::string>& ngram);

    VocabIndexer::index_t                 getIndex(const std::string& word) const;
    std::vector<VocabIndexer::index_t>    getIndex(const std::vector<std::string>& ngram) const;

    const std::string&                    getWord(VocabIndexer::index_t) const;

    std::ostream&                         dump(std::ostream& os, size_t) const;
    const std::vector<unsigned> &         getSFreq() const;

  private:
    /* reverse index - only for debugging */
    std::vector<std::string>  forms;
    /* use for idf count, number of sentences where each word appear */
    std::vector<unsigned>     sfreq;
    std::unordered_map<std::string, index_t> form2index;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
      ar &
      forms &
      sfreq &
      form2index;
    }
  };
}
