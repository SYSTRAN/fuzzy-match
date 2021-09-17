#include <fuzzy/suffix_array.hh>

#include <fuzzy/ngram_matches.hh>
#include <cassert>

namespace fuzzy
{
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

  void
  SuffixArray::sort(size_t vocab_size)
  {
    if (_sorted)
      return;

    // word id => prefixes
    std::vector<std::vector<SuffixView> > prefixes_by_word_id(vocab_size);

    // sort suffixes according to their first word id
    for (const auto& suffix : _suffixes)
    {
      const auto* suffix_wids = get_suffix(suffix);
      const auto wid = suffix_wids[0];
      assert((size_t)wid < vocab_size);

      prefixes_by_word_id[wid].push_back(suffix);
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
        std::sort(prefixes_by_word_id[wid].begin(), prefixes_by_word_id[wid].end(),
                  [this](const SuffixView& a, const SuffixView& b) {
                    return comp(a, b) < 0;
                  });
        std::copy(prefixes_by_word_id[wid].begin(), prefixes_by_word_id[wid].end(),
                  back_inserter(_suffixes));
      }
    }

    _quickVocabAccess[vocab_size] = _suffixes.size();
    _sorted = true;

    compute_sentence_length();
  }

  /**range of suffixe starting with ngram**/
  std::pair<size_t, size_t>
  SuffixArray::equal_range(const unsigned* ngram, size_t length, size_t min, size_t max) const
  {
    assert(_suffixes.empty() || _sorted);

    if (length == 0)
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

      if (length == 1)
        return std::pair<size_t, size_t>(min, max);
    }

    size_t      cur = (min + max) / 2;
    std::pair<size_t, size_t> res;
    int           r;
    assert(min <= max);

    while (max > min)
    {
      r = start_by(_suffixes[cur], ngram, length);

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
      r = start_by(_suffixes[cur], ngram, length);

      if (r == 0)
        min = cur;
      else //r>0 : ngram< _suffixes[cur]
        max = cur;

      cur = (min + max) / 2;
    }

    res.second = max; //max is the highest index with  ngram< _suffixes[max]
    //compute lower bound
    min = savemin;

    if (start_by(_suffixes[min], ngram, length) == 0) //can happen if min=0, doesnt hurt to check in all cases
      max = min; //the loop won't be executed
    else
      max = savecur;

    cur = (min + max) / 2; // max-min>1  ==> cur!=min

    while (cur > min)
    {
      //loop invariant : start_by(_suffixes[min], ngram)<0 ,  start_by(_suffixes[max], ngram)==0
      r = start_by(_suffixes[cur], ngram, length);

      if (r == 0)
        max = cur;
      else //r<0 : ngram> _suffixes[cur]
        min = cur;

      cur = (min + max) / 2;
    }

    res.first = max; //max is the lowest index with start_by(_suffixes[max], ngram)==0
    //postcondition on range:
    assert(res.second > res.first); //non empty range
    assert(start_by(_suffixes[res.first], ngram, length) == 0);
    assert(res.first == 0 || (start_by(_suffixes[res.first - 1], ngram, length) < 0));
    assert(res.second == _suffixes.size() || start_by(_suffixes[res.second], ngram, length) > 0);
    assert(res.second > 0 && (start_by(_suffixes[res.second - 1], ngram, length) == 0));
    return res;
  }

  static int
  compare_ngrams(const unsigned* v1, size_t v1_length,
                 const unsigned* v2, size_t v2_length,
                 bool equal_if_startby = false)
  {
    size_t i = 0;
    size_t j = 0;

    for (; i < v1_length && j < v2_length; i++, j++)
    {
      if (v1[i] < v2[j])
        return -1;
      else if (v1[i] > v2[j])
        return 1;
    }

    if (i == v1_length && j != v2_length)
      return -1;

    if (i != v1_length && j == v2_length && !equal_if_startby)
      return 1;

    return 0;
  }

  int
  SuffixArray::comp(const SuffixView& a, const SuffixView& b) const
  {
    size_t length_a;
    size_t length_b;
    const auto* suffix_a = get_suffix(a, &length_a);
    const auto* suffix_b = get_suffix(b, &length_b);
    const auto c = compare_ngrams(suffix_a, length_a, suffix_b, length_b);

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
  SuffixArray::start_by(const SuffixView& p, const unsigned* ngram, size_t length) const
  {
    size_t suffix_length;
    const auto* suffix = get_suffix(p, &suffix_length);
    return compare_ngrams(suffix,
                          suffix_length,
                          ngram,
                          length,
                          /*equal_if_startby=*/true);
  }

}
