#pragma once

#include <fuzzy/bm25.hh>
#include <fuzzy/costs.hh>
#include <fuzzy/tsl/hopscotch_map.h>
#include <fuzzy/filter_matches.hh>

namespace fuzzy
{
  // Sentence ID -> BM25score
  using BestMatches = tsl::hopscotch_map<unsigned, float, IntHash>;

  class BM25Matches : public FilterMatches
  {
  public:
    using FilterMatches::FilterMatches;
    BM25Matches(float fuzzy,
                unsigned p_length,
                unsigned min_seq_len,
                const BM25&,
                const unsigned buffer=10);
    // Registers a match for this range of suffixes.
    void register_pattern(
      std::vector<unsigned>& pattern_wids,
      const EditCosts& edit_costs=EditCosts());

    using FilterMatches::theoretical_rejection;
    using FilterMatches::theoretical_rejection_cover;

    std::vector<std::pair<unsigned, unsigned>> get_longest_matches() const override;

  private:
    // Num of sentences to place in the buffer
    const unsigned _buffer;
    BestMatches _best_matches;
  };
}
