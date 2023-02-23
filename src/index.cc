#include <fuzzy/index.hh>

namespace fuzzy
{
  FilterIndex::FilterIndex(size_t max_tokens_in_pattern, IndexType type)
    : _max_tokens_in_pattern(max_tokens_in_pattern),
      _type(type)
  {
   if (_type == IndexType::SUFFIX)
    _filter = createSuffixArray();
  }

  int
  FilterIndex::add_tm(const std::string& id,
                      const Sentence& real_tokens,
                      const Tokens& norm_tokens,
                      bool sort)
  {
    if (!real_tokens.empty() && norm_tokens.size() <= _max_tokens_in_pattern) // patterns greater than this size would be ignored in match
    {
      std::vector<unsigned> tokens_idx = _vocabIndexer.addWords(norm_tokens);
      _filter->add_sentence(tokens_idx);

      _ids.push_back(id);

      _real_tokens.push_back(real_tokens);
    }
    // this.get_Filter();
    // std::cerr << "+";
    return _ids.size();
  }

  std::string
  FilterIndex::sentence(size_t sindex) const
  {
    std::string sent =">";
    size_t slength = 0;
    const auto* sentence = _filter->get_sentence(sindex, &slength);

    for (size_t j = 0; j < slength; j++)
    {
      const std::string& form = _vocabIndexer.getWord(sentence[j]);
      if (!sent.empty())
        sent += " ";
      sent += form;
    }

    return sent;
  }

#ifndef NDEBUG
  std::ostream& FilterIndex::dump(std::ostream& os) const {
    os << "=== Vocabulary ==="<<std::endl;
    _vocabIndexer.dump(os, _filter->num_sentences()) << std::endl;
    os << "=== Suffix Array ==="<<std::endl;
    _filter->dump(os) << std::endl;
    return os;
  }
#endif


  const Sentence &FilterIndex::real_tokens(size_t s_id) const
  {
    return _real_tokens[s_id];
  }

  const std::string&
  FilterIndex::id(unsigned int index)
  { 
    return _ids[index];
  }

}
