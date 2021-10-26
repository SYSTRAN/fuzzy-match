#include <fuzzy/vocab_indexer.hh>

#include <cmath>
#include <unordered_set>

using namespace std;

namespace fuzzy
{
  const VocabIndexer::index_t VocabIndexer::SENTENCE_SEPARATOR = 0;
  const VocabIndexer::index_t VocabIndexer::VOCAB_UNK = 1;
  static const std::string sentence_separator_word = "\0";
  static const std::string vocab_unk_word = "｟unk｠";

  VocabIndexer::VocabIndexer()
  {
    /* index 0 should never been attributed since it is used for sentence separator */
    addWord(sentence_separator_word);
    addWord(vocab_unk_word);
  }

#ifndef NDEBUG
  std::ostream&  VocabIndexer::dump(std::ostream& os, size_t nsentences) const
  {
    for (size_t i = 1; i < forms.size(); i++)
      os << i << "\t" << forms[i] << "\t" << sfreq[i] << "\t"<<std::log(nsentences*1.0/sfreq[i])<<endl;

    return os;
  }
#endif

  size_t VocabIndexer::size() const
  {
    return forms.size();
  }

  VocabIndexer::index_t VocabIndexer::addWord(const std::string& word)
  {
    const auto it = form2index.find(word);

    if (it != form2index.end())
      return it->second;
    else
    {
      form2index.emplace(word, forms.size());
      forms.push_back(word);
      sfreq.push_back(0);
      return ((index_t)forms.size() - 1);
    }
  }

  VocabIndexer::index_t VocabIndexer::getIndex(const std::string& word) const
  {
    const auto it = form2index.find(word);

    if (it != form2index.end())
      return it->second;
    else
      return VOCAB_UNK;
  }

  std::vector<VocabIndexer::index_t> VocabIndexer::getIndex(const std::vector<std::string>& ngram) const
  {
    std::vector<index_t> res;
    res.reserve(ngram.size());

    for (const auto& gram : ngram)
      res.push_back(getIndex(gram));

    return res;
  }

  std::vector<VocabIndexer::index_t> VocabIndexer::addWords(const std::vector<std::string>& ngram)
  {
    std::unordered_set<index_t> vocab_set;
    std::vector<index_t> res;
    res.reserve(ngram.size());

    for (const auto& gram : ngram) {
      auto idx = addWord(gram);
      vocab_set.insert(idx);
      res.push_back(idx);
    }

    for(auto idx: vocab_set) {
      sfreq[idx]++;
    }

    return res;
  }

  const std::string& VocabIndexer::getWord(index_t ind) const
  {
    if (ind >= (index_t)forms.size())
      return vocab_unk_word;

    return forms[ind];
  }

  const std::vector<unsigned> &VocabIndexer::getSFreq() const {
    return sfreq;
  }
}
