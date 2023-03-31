#include <fuzzy/bm25_matches.hh>

#include <cmath>

namespace fuzzy
{
  BM25Matches::BM25Matches(float fuzzy,
                           unsigned p_length,
                           unsigned min_seq_len,
                           const BM25& bm25,
                           const unsigned buffer,
                           const float cutoff_threshold)
      : FilterMatches(fuzzy, p_length, min_seq_len, bm25),
        _buffer(buffer),
        _cutoff_threshold(cutoff_threshold)
  {}

  void 
  BM25Matches::register_pattern(
    const std::vector<unsigned>& pattern_wids,
    const EditCosts& edit_costs)
  {
    const BM25& bm25 = static_cast<const BM25&>(_filter);

    std::unordered_set<int> candidates = bm25.get_candidates(pattern_wids);

    std::priority_queue<std::pair<float, unsigned>, std::vector<std::pair<float, unsigned>>, ComparePairs> k_best;
    // for (unsigned s_id = 0; s_id < bm25.num_sentences(); s_id++)
    // std::cerr << candidates.size();
    for (const unsigned &s_id : candidates)
    {
      auto s_length = bm25.get_sentence_length(s_id);
      if (theoretical_rejection(_p_length, s_length, edit_costs))
        continue;
      float bm25_score = bm25.bm25_score_pattern(s_id, pattern_wids);
      // float bm25_score = 0;
      if (bm25_score <= _cutoff_threshold)
        continue;
      
      k_best.emplace(bm25_score, s_id);
      if (k_best.size() > _buffer)
        k_best.pop();
    }
    _best_matches.reserve(k_best.size());
    while (!k_best.empty())
    {
      _best_matches.push_back({k_best.top().second, 0});
      k_best.pop();
    }
    std::reverse(_best_matches.begin(), _best_matches.end()); 
  }

  std::vector<std::pair<unsigned, unsigned>>
  BM25Matches::get_best_matches() const
  {
    // std::cerr << _best_matches.size();
    return _best_matches;
  }
}
