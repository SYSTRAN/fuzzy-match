namespace fuzzy
{
  inline const Filter&
  FilterIndex::get_Filter() const
  {
    return *_filter;
  }

  inline const VocabIndexer&
  FilterIndex::get_VocabIndexer() const
  {
    return _vocabIndexer;
  }

  inline void
  FilterIndex::prepare()
  {
    _filter->prepare(_vocabIndexer.size());
  }

  inline size_t
  FilterIndex::size() const
  {
    return _ids.size();
  }

  inline size_t
  FilterIndex::max_tokens_in_pattern() const
  {
    return _max_tokens_in_pattern;
  }
  inline IndexType
  FilterIndex::getType() const
  {
    return _type;
  }

  template<class Archive>
  void
  FilterIndex::save(Archive& ar, unsigned int version) const
  {
    if (_type == IndexType::SUFFIX)
    {
      SuffixArray& suffix_array = static_cast<SuffixArray&>(*_filter);
      if (version >= 2)
        ar & _type;
      ar
        & _vocabIndexer
        & suffix_array
        & _ids
        & _real_tokens
        & _max_tokens_in_pattern;
    }
#ifdef USE_EIGEN
    else if (_type == IndexType::BM25)
    {
      BM25& bm25 = static_cast<BM25&>(*_filter);
      ar
        & _type
        & _vocabIndexer
        & bm25
        & _ids
        & _real_tokens
        & _max_tokens_in_pattern;
    }
#endif
  }

  template<class Archive>
  void
  FilterIndex::load(Archive& ar, unsigned int version)
  {
    if (version >= 2)
      ar & _type;
    if (_type == IndexType::SUFFIX)
    {
      _filter = createSuffixArray();
      SuffixArray& suffix_array = static_cast<SuffixArray&>(*_filter);
      ar
        & _vocabIndexer
        & suffix_array
        & _ids
        & _real_tokens;
    }
#ifdef USE_EIGEN
    else if (_type == IndexType::BM25)
    {
      _filter = createBM25();
      BM25& bm25 = static_cast<BM25&>(*_filter);
      ar
        & _vocabIndexer
        & bm25
        & _ids
        & _real_tokens;
    }
#endif
    if (version >= 1)
      ar & _max_tokens_in_pattern;
  }
}

BOOST_CLASS_VERSION(fuzzy::FilterIndex, 2)
