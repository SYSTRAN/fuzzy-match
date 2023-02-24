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
    std::cerr << "registering..." << std::endl;
    for (unsigned s_id = 0; s_id < bm25.num_sentences(); s_id++)
    {
      auto s_length = bm25.get_sentence_length(s_id);
      if (theoretical_rejection(_p_length, s_length, edit_costs))
        continue;
      double bm25_score = bm25.bm25_score_pattern(s_id, pattern_wids);
      std::cerr << "(" << s_id << ", " << bm25_score << ") " << std::endl;
      if (bm25_score <= _cutoff_threshold)
        continue;
      _best_matches.try_emplace(s_id, bm25_score);
      // std::cerr << "#";
    }
  }

  std::vector<std::pair<unsigned, unsigned>>
  BM25Matches::get_longest_matches() const
  {
    // (s_id, longest_match=1)
    std::cerr << "get_longest...";
    const BM25& bm25 = static_cast<const BM25&>(_filter);
    std::vector<std::pair<unsigned, unsigned>> sorted_matches(_best_matches.begin(),
                                                              _best_matches.end());
    std::sort(sorted_matches.begin(), sorted_matches.end(),
              [](const std::pair<unsigned, float>& a, const std::pair<unsigned, float>& b) {
                return a.second > b.second || (a.second == b.second && a.first < b.first);
              });
    
    // Maximum number of potential matches is _buffer
    std::cerr << "done"  << std::endl;
    return std::vector<std::pair<unsigned, unsigned>>(
      sorted_matches.begin(),
      sorted_matches.begin() + std::min(_buffer, (unsigned)sorted_matches.size()));
  }
}
