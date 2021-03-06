#include <fuzzy/suffix_array.hh>

#include <fuzzy/ngram_matches.hh>
#include <cassert>

namespace fuzzy
{

  SuffixArray::SuffixArray()
    : _sorted(false)
  {
  }

  SuffixArray::SuffixArray(const SuffixArray& other)
    : _sorted(other._sorted)
  {
    _suffixes = other._suffixes;
    _sentence_buffer = other._sentence_buffer;
    _sentence_pos = other._sentence_pos;
  }

  SuffixArray::~SuffixArray()
  {
  }

  inline bool
  suffix_array_sorter::operator()(const SuffixView& a, const SuffixView& b)const
  {
    return _ref.comp(a, b) < 0;
  }

  unsigned
  SuffixArray::add_sentence(const std::vector<unsigned>& sentence)
  {
    size_t sidx = _sentence_pos.size();
    _sentence_pos.push_back(_sentence_buffer.size());

    /* first token in sentence buffer is the sentence size */
    _sentence_buffer.push_back(sentence.size());

    for (size_t i = 0; i < sentence.size(); i++)
    {
      _sentence_buffer.push_back(sentence[i]);
      _suffixes.push_back(SuffixView{static_cast<unsigned int>(sidx), static_cast<unsigned short>(i+1)});
    }
    _sentence_buffer.push_back(fuzzy::VocabIndexer::SENTENCE_SEPARATOR);
    _sorted = false;

    return sidx;
  }

#ifndef NDEBUG
  std::ostream&
  SuffixArray::dump(std::ostream& os)const
  {
    os << "   ===text===" << std::endl;

    for (size_t i = 0; i < _sentence_pos.size(); i++)
    {
      size_t idx = _sentence_pos[i];
      for (size_t j = 0; _sentence_buffer[j+idx]; j++)
        os << _sentence_buffer[j+idx] << " ";
      os << std::endl;
    }

    os << "   ===suffixes===" << std::endl;

    for (size_t i = 0; i < _suffixes.size(); i++)
    {
      os << i << "(" << _suffixes[i].sentence_id << "/" << _suffixes[i].subsentence_pos << "):: ";
      size_t idx = _sentence_pos[_suffixes[i].sentence_id];
      for (size_t j = _suffixes[i].subsentence_pos; _sentence_buffer[idx+j]; j++)
        os << _sentence_buffer[idx+j] << " ";
      os << std::endl;
    }

    return os;
  }
#endif

  size_t
  SuffixArray::nsentences() const
  {
    return _sentence_pos.size();
  }

  void
  SuffixArray::sort(size_t vocab_size)
  {
    if (_sorted)
      return;

    // word id => prefixes
    std::vector<std::vector<SuffixView> > prefixes_by_word_id(vocab_size);

    // sort suffixes according to their first word id
    for (size_t i = 0; i < _suffixes.size(); i++)
    {
      const auto wid = (*this)[_suffixes[i]];
      assert((size_t)wid < vocab_size);

      prefixes_by_word_id[wid].push_back(_suffixes[i]);
    }

    _suffixes.clear();
    _quickVocabAccess.clear();
    _quickVocabAccess.resize(vocab_size + 1);

    // sort each bucket of suffixes
    // we can then append it to the sorted suffix array
    for (size_t wid = 0; wid < vocab_size; wid++)
    {
      _quickVocabAccess[wid] = _suffixes.size();

      // for all suffixes starting by this word id
      if (!prefixes_by_word_id[wid].empty())
      {
          std::sort(prefixes_by_word_id[wid].begin(), prefixes_by_word_id[wid].end(), suffix_array_sorter(*this));
          std::copy(prefixes_by_word_id[wid].begin(), prefixes_by_word_id[wid].end(), back_inserter(_suffixes));
      }
    }

    _quickVocabAccess[vocab_size] = _suffixes.size();
    _sorted = true;

    compute_sentence_length();
  }

  void
  SuffixArray::clear()
  {
    _sentence_buffer.clear();
    _sentence_pos.clear();
    _suffixes.clear();
    _quickVocabAccess.clear();
  }

  /**range of suffixe starting with ngram**/
  std::pair<size_t, size_t>
  SuffixArray::equal_range(const std::vector<unsigned> &ngram, size_t min, size_t max) const
  {
    assert(_suffixes.empty() || _sorted);

    if (ngram.size() == 0 || !ngram[0])
      return std::pair<size_t, size_t>(0, 0);

    /* if not initialized */
    if (max == 0)
    {
      /* use the quick index */
      if ((unsigned)ngram[0] > _quickVocabAccess.size() - 1)
        return std::pair<size_t, size_t>(0, 0);

      min = _quickVocabAccess[ngram[0]];

      if ((unsigned)(ngram[0] + 1) < _quickVocabAccess.size() - 1)
        max = _quickVocabAccess[ngram[0] + 1];
      else
        max = _suffixes.size();

      if (ngram.size() == 1)
        return
          std::pair<size_t, size_t>(min, max);
    }

    size_t      cur = (min + max) / 2;
    std::pair<size_t, size_t> res;
    int           r;
    assert(min <= max);

    while (max > min)
    {
      r = start_by(_suffixes[cur], ngram);

      if (r == 0)
        break;

      if (r < 0)
      {
        //ngram> _suffixes[cur]
        if (min < cur)
          min = cur;
        else
          max = min; //not found
      }
      else
        max = cur;

      cur = (min + max) / 2; //invariant: min<=cur<max
    }

    if (max - min < 2)
      return std::pair<size_t, size_t>(min, max); //empty range if min=max, or 1 elem range if min=max+1

    // range of size bigger than 1: we must determine the exact boundaries
    //find upperbound
    size_t savecur = cur;
    size_t savemin = min;
    min = cur;
    cur = (min + max) / 2;

    while (cur > min)
    {
      //loop invariant : start_by(_suffixes[max], ngram)>0, start_by(_suffixes[min], ngram)==0
      r = start_by(_suffixes[cur], ngram);

      if (r == 0)
        min = cur;
      else //r>0 : ngram< _suffixes[cur]
        max = cur;

      cur = (min + max) / 2;
    }

    res.second = max; //max is the highest index with  ngram< _suffixes[max]
    //compute lower bound
    min = savemin;

    if (start_by(_suffixes[min], ngram) == 0) //can happen if min=0, doesnt hurt to check in all cases
      max = min; //the loop won't be executed
    else
      max = savecur;

    cur = (min + max) / 2; // max-min>1  ==> cur!=min

    while (cur > min)
    {
      //loop invariant : start_by(_suffixes[min], ngram)<0 ,  start_by(_suffixes[max], ngram)==0
      r = start_by(_suffixes[cur], ngram);

      if (r == 0)
        max = cur;
      else //r<0 : ngram> _suffixes[cur]
        min = cur;

      cur = (min + max) / 2;
    }

    res.first = max; //max is the lowest index with start_by(_suffixes[max], ngram)==0
    //postcondition on range:
    assert(res.second > res.first); //non empty range
    assert(start_by(_suffixes[res.first], ngram) == 0);
    assert(res.first == 0 || (start_by(_suffixes[res.first - 1], ngram) < 0));
    assert(res.second == _suffixes.size() || start_by(_suffixes[res.second], ngram) > 0);
    assert(res.second > 0 && (start_by(_suffixes[res.second - 1], ngram) == 0));
    return res;
  }

  static int
  compare_ngrams(const std::vector<unsigned> &v1, unsigned v1_start,
                 const std::vector<unsigned> &v2, unsigned v2_start, bool equal_if_startby)
  {
    unsigned      i;
    unsigned      j;
    int           cmp;

    for (i = v1_start, j = v2_start; i < v1.size() && v1[i] && j < v2.size() && v2[j]; i++, j++)
    {
      if ((cmp = v1[i] - v2[j]) != 0)
        return cmp;
    }

    if ((i == v1.size() || v1[i] == 0) && !(j == v2.size() || v2[j] == 0))
        return -1;

    if (!(i == v1.size() || v1[i] == 0) && (j == v2.size() || v2[j] == 0))
    {
      if (!equal_if_startby)
        return 1;
    }

    return 0;
  }

  int
  SuffixArray::comp(const SuffixView& a, const SuffixView& b) const
  {

    int c = compare_ngrams(_sentence_buffer, a.subsentence_pos+_sentence_pos[a.sentence_id],
                           _sentence_buffer, b.subsentence_pos+_sentence_pos[b.sentence_id], false);
    if (c != 0 || a.sentence_id == b.sentence_id)
      return c;
    else //same std::vector (c=0),but in different sentences: to have a total order relation we sort n sentence index (if they have the same index they are the same)
      if (a.sentence_id < b.sentence_id) //the fact that the order is total just insure that there won't be platform-specifix difference in sorting due to the sort algo
        return -1;
      else
        return 1;
  }

  void SuffixArray::compute_sentence_length()
  {
    _sentence_length.resize(_suffixes.size());
    for (std::size_t suffix_id=0; suffix_id < _suffixes.size(); suffix_id++)
    {
      const auto sentence_id = _suffixes[suffix_id].sentence_id;
      _sentence_length[suffix_id] = _sentence_buffer[_sentence_pos[sentence_id]];
    }
  }

  int
  SuffixArray::start_by(const SuffixView& p, const std::vector<unsigned> &ngram) const
  {
    return compare_ngrams(_sentence_buffer, p.subsentence_pos+_sentence_pos[p.sentence_id],
                          ngram, 0, true);
  }

}
