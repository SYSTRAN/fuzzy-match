namespace fuzzy
{
  inline const SuffixArray&
  SuffixArrayIndex::get_SuffixArray() const
  {
    return _suffixArray;
  }

  inline const VocabIndexer&
  SuffixArrayIndex::get_VocabIndexer() const
  {
    return _vocabIndexer;
  }

  inline void
  SuffixArrayIndex::sort()
  {
    _suffixArray.sort(_vocabIndexer.size());
  }

  inline size_t
  SuffixArrayIndex::size() const
  {
    return _ids.size();
  }

  inline size_t
  SuffixArrayIndex::max_tokens_in_pattern() const
  {
    return _max_tokens_in_pattern;
  }

  template<class Archive>
  void
  SuffixArrayIndex::save(Archive& ar, unsigned int) const
  {
    ar
      & _vocabIndexer
      & _suffixArray
      & _ids
      & _real_tokens
      & _max_tokens_in_pattern;
  }

  template<class Archive>
  void
  SuffixArrayIndex::load(Archive& ar, unsigned int version)
  {
    ar
      & _vocabIndexer
      & _suffixArray
      & _ids
      & _real_tokens;

    if (version >= 1)
      ar & _max_tokens_in_pattern;
  }

}

BOOST_CLASS_VERSION(fuzzy::SuffixArrayIndex, 1)
