#include <fuzzy/no_filter.hh>

#include <fuzzy/ngram_matches.hh>
#include <fuzzy/vocab_indexer.hh>
#include <cassert>

namespace fuzzy
{
  NoFilter::NoFilter(const FilterIndexParams& params)
  {}
  NoFilter::~NoFilter() {}
  // unsigned
  // NoFilter::add_sentence(const std::vector<unsigned>& sentence)
  // {
  //   size_t sidx = _sentence_pos.size();
  //   _sentence_pos.push_back(_sentence_buffer.size());

  //   /* first token in sentence buffer is the sentence size */
  //   _sentence_buffer.push_back(sentence.size());

  //   for (size_t i = 0; i < sentence.size(); i++)
  //   {
  //     _sentence_buffer.push_back(sentence[i]);
  //   }
  //   _sentence_buffer.push_back(fuzzy::VocabIndexer::SENTENCE_SEPARATOR);
  //   return sidx;
  // }

  void NoFilter::prepare(size_t vocab_size) {}
}
