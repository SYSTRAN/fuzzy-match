#include <fuzzy/suffix_array_index.hh>

namespace fuzzy
{
  const size_t SuffixArrayIndex::DEFAULT_MAX_TOKENS_IN_PATTERN = 300; // if you change this value, update README.md

  SuffixArrayIndex::SuffixArrayIndex(size_t max_tokens_in_pattern)
    : _max_tokens_in_pattern(max_tokens_in_pattern)
  {
  }

  int
  SuffixArrayIndex::add_tm(const std::string& id,
                           const Sentence& real_tokens,
                           const Tokens& norm_tokens,
                           bool sort)
  {
    if (!real_tokens.empty() && norm_tokens.size() <= _max_tokens_in_pattern) // patterns greater than this size would be ignored in match
    {
      std::vector<unsigned> tokens_idx = _vocabIndexer.getIndexCreate(norm_tokens);
      _suffixArray.add_sentence(tokens_idx);

      _ids.push_back(id);

      _real_tokens.push_back(real_tokens);
    }

    if (sort)
      _suffixArray.sort(_vocabIndexer.size());

    return _ids.size();
  }

  std::string
  SuffixArrayIndex::sentence(size_t sindex) const
  {
    std::string sent =">";
    size_t idx = _suffixArray[sindex];

    for (size_t j = 1; _suffixArray.sentence_buffer()[idx + j]; j++)
    {
      int ind = _suffixArray.sentence_buffer()[idx + j];
      std::string form = _vocabIndexer.getWord(ind);
      if (!sent.empty())
        sent += " ";
      sent += form;
    }

    return sent;
  }

#ifndef NDEBUG
  std::ostream& SuffixArrayIndex::dump(std::ostream& os) const {
    os << "=== Vocabulary ==="<<std::endl;
    _vocabIndexer.dump(os, _suffixArray.nsentences()) << std::endl;
    os << "=== Suffix Array ==="<<std::endl;
    _suffixArray.dump(os) << std::endl;
    return os;
  }
#endif


  const Sentence &SuffixArrayIndex::real_tokens(size_t s_id) const
  {
    return _real_tokens[s_id];
  }

  const std::string&
  SuffixArrayIndex::id(unsigned int index)
  { 
    return _ids[index];
  }

}
