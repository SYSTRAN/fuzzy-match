#pragma once

#include <algorithm>
#include <queue>
#include <fuzzy/bm25.hh>
#include <fuzzy/costs.hh>
#include <fuzzy/filter_matches.hh>
#include <Eigen/Sparse>

namespace fuzzy
{
  struct ComparePairs {
    bool operator()(const std::pair<float, unsigned>& p1, const std::pair<float, unsigned>& p2) {
        return p1.first > p2.first;
    }
  };

  class BM25Matches : public FilterMatches
  {
  public:
    using FilterMatches::FilterMatches;
    BM25Matches(float fuzzy,
                unsigned p_length,
                unsigned min_seq_len,
                const BM25&,
                const unsigned buffer=10,
                const float cutoff_threshold=0.);
    // Registers a pattern to compute its bm25 score
    void register_pattern(
      const std::vector<unsigned>& pattern_wids,
      const EditCosts& edit_costs=EditCosts());

    using FilterMatches::theoretical_rejection;
    using FilterMatches::theoretical_rejection_cover;

    std::vector<std::pair<unsigned, int>> get_best_matches() const override;
    std::vector<float> cover(const std::vector<unsigned>& unique_pattern_wids, const std::vector<unsigned>& count, unsigned s_id) const;

  private:
    // Num of sentences to place in the buffer
    const unsigned _buffer;
    const float _cutoff_threshold;
    std::vector<std::pair<unsigned, int>> _best_matches;
  };
}
