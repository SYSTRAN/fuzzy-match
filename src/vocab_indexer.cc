#include <fuzzy/vocab_indexer.hh>

#include <unordered_set>

using namespace std;

namespace fuzzy
{
  const VocabIndexer::index_t VocabIndexer::SENTENCE_SEPARATOR = 0;
  const VocabIndexer::index_t VocabIndexer::VOCAB_UNK = 1;

  VocabIndexer::VocabIndexer()
  {
    init();
  }

  VocabIndexer::VocabIndexer(const VocabIndexer& other)
  {
    *this = other;
  }

  VocabIndexer& VocabIndexer::operator=(const  VocabIndexer& other)
  {
    if (&other == this)
      return *this;

    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    boost::lock_guard<boost::recursive_mutex> guardother(other._mutex);
    forms = other.forms;
    form2index = other.form2index;
    return *this;
  }

  void VocabIndexer::init()
  {
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    forms.clear();
    form2index.clear();
    /* index 0 should never been attributed since it is used for sentence separator */
    addWord("\0");
    addWord(_unknownword);
  }

#ifndef NDEBUG
  std::ostream&  VocabIndexer::dump(std::ostream& os, size_t nsentences) const
  {
    for (size_t i = 1; i < forms.size(); i++)
      os << i << "\t" << forms[i] << "\t" << sfreq[i] << "\t"<<log(nsentences*1.0/sfreq[i])<<endl;

    return os;
  }
#endif

  void VocabIndexer::clear()
  {
    init();
  }

  unsigned VocabIndexer::size() const
  {
    return forms.size();
  }

  VocabIndexer::index_t VocabIndexer::addWord(const std::string& word)
  {
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    const auto it = form2index.find(word);

    if (it != form2index.end())
      return it->second;
    else
    {
      form2index[word] = forms.size();
      forms.push_back(word);
      sfreq.push_back(0);
      return ((index_t)forms.size() - 1);
    }
  }

  VocabIndexer::index_t VocabIndexer::getIndex(const std::string& word) const
  {
    boost::lock_guard<boost::recursive_mutex> guard(_mutex);
    const auto it = form2index.find(word);

    if (it != form2index.end())
      return it->second;
    else
      return VOCAB_UNK;
  }

  std::vector<VocabIndexer::index_t> VocabIndexer::getIndex(const std::vector<std::string>& ngram) const
  {
    std::vector<VocabIndexer::index_t> res;
    res.reserve(ngram.size());

    for (size_t i = 0; i < ngram.size(); i++)
      res.push_back(getIndex(ngram[i]));

    return res;
  }

  std::vector<VocabIndexer::index_t> VocabIndexer::getIndexCreate(const std::vector<std::string>& ngram)
  {
    std::unordered_set<VocabIndexer::index_t> vocab_set;
    std::vector<VocabIndexer::index_t> res;
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

  std::string VocabIndexer::getWord(VocabIndexer::index_t ind) const
  {
    if (ind >= (index_t)forms.size())
      return _unknownword;

    return forms[ind];
  }

  void VocabIndexer::getWord(VocabIndexer::index_t ind, std::string& res) const
  {
    if (ind >= (index_t)forms.size())
      res = _unknownword;

    res = forms[ind];
  }

  const std::vector<unsigned> &VocabIndexer::getSFreq() const {
    return sfreq;
  }


  const std::string VocabIndexer::_unknownword = "｟unk｠";
}
