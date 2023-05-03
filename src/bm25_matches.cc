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

  // void 
  // BM25Matches::register_pattern(
  //   const std::vector<unsigned>& pattern_wids,
  //   const EditCosts& edit_costs)
  // {
  //   const BM25& bm25 = static_cast<const BM25&>(_filter);

  //   std::vector<std::vector<int>> candidates = bm25.get_vec_candidates(pattern_wids);
  //   std::priority_queue<std::pair<float, unsigned>, std::vector<std::pair<float, unsigned>>, ComparePairs> k_best;

  //   bool done = false;
  //   std::vector<int> cursors(candidates.size(), 0);
  //   while (candidates.size() > 0 && !done)
  //   {
  //     int s_id = std::numeric_limits<int>::max();
  //     for (int i = 0; i < candidates.size(); ++i) {
  //       if (cursors[i] < candidates[i].size() && candidates[i][cursors[i]] < s_id) {
  //         s_id = candidates[i][cursors[i]];
  //       }
  //     }

  //     auto s_length = bm25.get_sentence_length(s_id);
  //     if (!theoretical_rejection(_p_length, s_length, edit_costs))
  //     {
  //       float bm25_score = bm25.bm25_score_pattern(s_id, pattern_wids);
  //       if (bm25_score > _cutoff_threshold)
  //       {
  //         k_best.emplace(bm25_score, s_id);
  //         if (k_best.size() > _buffer)
  //           k_best.pop();
  //       }
  //     }

  //     for (int i = 0; i < candidates.size(); i++) {
  //       if (cursors[i] < candidates[i].size() && candidates[i][cursors[i]] == s_id) {
  //         cursors[i]++;
  //       }
  //     }

  //     // Check if we're done
  //     done = true;
  //     for (int i = 0; i < candidates.size(); i++) {
  //       if (cursors[i] < candidates[i].size()) {
  //         done = false;
  //         break;
  //       }
  //     }
  //   }

  //   _best_matches.reserve(k_best.size());
  //   while (!k_best.empty())
  //   {
  //     _best_matches.push_back({k_best.top().second, 0});
  //     k_best.pop();
  //   }
  //   std::reverse(_best_matches.begin(), _best_matches.end()); 
  // }

  void 
  BM25Matches::register_pattern(
    const std::vector<unsigned>& pattern_wids,
    const EditCosts& edit_costs)
  {
    const BM25& bm25 = static_cast<const BM25&>(_filter);

    std::priority_queue<std::pair<float, unsigned>, std::vector<std::pair<float, unsigned>>, ComparePairs> k_best;

    Eigen::SparseVector<float> pattern_sparse_vec(bm25.get_vocab_size());
    for (const unsigned& wid : pattern_wids)
      pattern_sparse_vec.coeffRef(wid) += 1.0;

    Eigen::SparseVector<float> scores = bm25.compute_product(pattern_sparse_vec);

    for (Eigen::SparseVector<float>::InnerIterator it(scores); it; ++it) {
      int s_id = it.index();
      float bm25_score = it.value();
      if (bm25_score > _cutoff_threshold)
      {
        k_best.emplace(bm25_score, s_id);
        if (k_best.size() > _buffer)
          k_best.pop();
      }
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
    return _best_matches;
  }
}
