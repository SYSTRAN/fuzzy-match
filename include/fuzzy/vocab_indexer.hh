#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include <boost/thread/locks.hpp>

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
    VocabIndexer(const VocabIndexer&);

    VocabIndexer& operator=(const VocabIndexer&);

    void                                  clear();
    unsigned                              size() const;

    VocabIndexer::index_t                 addWord(const std::string& word);
    std::vector<VocabIndexer::index_t>    getIndexCreate(const std::vector<std::string>& ngram);

    VocabIndexer::index_t                 getIndex(const std::string& word) const;
    std::vector<VocabIndexer::index_t>    getIndex(const std::vector<std::string>& ngram) const;

    std::string                           getWord(VocabIndexer::index_t) const;
    void                                  getWord(VocabIndexer::index_t, std::string& res) const;

    boost::iostreams::filtering_ostream&  Serialize(boost::iostreams::filtering_ostream&) const;
    std::ostream&                         dump(std::ostream& os, size_t) const;
    const std::vector<unsigned> &         getSFreq() const;

    const static std::string _unknownword;

  private:
    void init();
    /* reverse index - only for debugging */
    std::vector<std::string>  forms;
    /* use for idf count, number of sentences where each word appear */
    std::vector<unsigned>     sfreq;
    std::unordered_map<std::string, index_t> form2index;

    mutable boost::recursive_mutex  _mutex;//may be locked several time by the same thread

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
      ar &
      forms &
      sfreq &
      form2index;
    }
  };
}
