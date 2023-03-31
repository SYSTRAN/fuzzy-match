#include <fuzzy/filter.hh>

#include <fuzzy/ngram_matches.hh>
#include <fuzzy/vocab_indexer.hh>
#include <cassert>

namespace fuzzy
{
  unsigned
  Filter::add_sentence(const std::vector<unsigned>& sentence)
  {
    size_t sidx = _sentence_pos.size();
    _sentence_pos.push_back(_sentence_buffer.size());

    /* first token in sentence buffer is the sentence size */
    _sentence_buffer.push_back(sentence.size());

    for (size_t i = 0; i < sentence.size(); i++)
    {
      _sentence_buffer.push_back(sentence[i]);
      // _suffixes.push_back(SuffixView{static_cast<unsigned int>(sidx), static_cast<unsigned short>(i+1)});
    }
    _sentence_buffer.push_back(fuzzy::VocabIndexer::SENTENCE_SEPARATOR);
    _prepared = false;

    return sidx;
  }

#ifndef NDEBUG
  std::ostream&
  Filter::dump(std::ostream& os)const
  {
    os << "   ===text===" << std::endl;

    for (size_t i = 0; i < _sentence_pos.size(); i++)
    {
      size_t idx = _sentence_pos[i];
      for (size_t j = 0; _sentence_buffer[j+idx]; j++)
        os << _sentence_buffer[j+idx] << " ";
      os << std::endl;
    }
    return os;
  }
#endif
}
