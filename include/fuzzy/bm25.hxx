#include <stdexcept>

namespace fuzzy
{
  inline unsigned
  BM25::get_sentence_length(size_t s_id) const
  {
    if (s_id + 1 == _sentence_pos.size())
      return _sentence_buffer.size() - _sentence_pos[s_id] -  2;
    return _sentence_pos[s_id + 1] - _sentence_pos[s_id] - 2;
  }
  inline float BM25::bm25_score(
    int term,
    int s_id,
    float avg_doc_length,
    float tf,
    std::vector<float>& idf)
  {
    /* BM25 formula */
    return idf[term] * (_k1 + 1) * tf / 
          (tf + _k1 * ((1 - _b) + (_b * (get_sentence_length(s_id) / avg_doc_length))));
  }
  inline float BM25::bm25_score_pattern(
    unsigned s_id,
    std::vector<unsigned> pattern_wids) const
  {
    float score = 0;
    for (unsigned& term : pattern_wids){
      score += _reverted_index.coeff(term, s_id);
    }
    return score;
  }
  template<class Archive>
  void BM25::save(Archive& archive, unsigned int) const
  {
    // std::cerr << "save " << num_sentences() << "  " << _vocab_size << "  " << _key_value_bm25.size() << std::endl;
    archive & num_sentences() & _vocab_size & _key_value_bm25.size();
    for (unsigned i = 0; i < _key_value_bm25.size(); i++)
    {
      archive & _key_value_bm25[i].first.first & _key_value_bm25[i].first.second & _key_value_bm25[i].second;
    }    
    archive
    & _sentence_buffer
    & _sentence_pos
    & _quickVocabAccess;
  }

  template<class Archive>
  void BM25::load(Archive& archive, unsigned int version)
  {
    size_t length, num_sents;
    archive & num_sents & _vocab_size & length;
    _key_value_bm25 = std::vector<std::pair<std::pair<int, int>, float>>(length);
    // _reverted_index.reserve(length / 0.5);

    std::vector<Triplet> triplets;
    int sid;
    int term;
    float bm25_value;
    unsigned max_sid = 0;
    unsigned max_term = 0;
    for (unsigned i = 0; i < length; i++)
    {
      archive & sid & term & bm25_value;
      // _reverted_index.emplace(std::make_pair(sid, term), bm25_value);
      // _key_value_bm25[i] = {{sid, term}, bm25_value};
      if (max_sid < sid)
        max_sid = sid;
      if (max_term < term)
        max_term = term;
      triplets.push_back(Triplet(term, sid, bm25_value));
    }
    // std::cerr << max_sid << " " << max_term << " || " << num_sents << " " << _vocab_size << std::endl;
    _reverted_index = SpMat(_vocab_size, num_sents);
    _reverted_index.setFromTriplets(triplets.begin(), triplets.end());
    archive
    & _sentence_buffer
    & _sentence_pos
    & _quickVocabAccess;
    _sorted = true;
  }
}
