#pragma once

#include <fuzzy/bm25.hh>
#include <fuzzy/costs.hh>
#include <fuzzy/tsl/hopscotch_map.h>
#include <fuzzy/filter_matches.hh>

namespace fuzzy
{
  // Sentence ID -> longest N-gram match
  using LongestMatches = tsl::hopscotch_map<unsigned, unsigned, IntHash>;

  class BM25Matches : public FilterMatches
  {
  public:
    using FilterMatches::FilterMatches;
    BM25Matches(float fuzzy,
                 unsigned p_length,
                 unsigned min_seq_len,
                 const BM25&);

    using FilterMatches::theoretical_rejection;
    using FilterMatches::theoretical_rejection_cover;

    std::vector<std::pair<unsigned, unsigned>> get_longest_matches() const override;

  private:
  };
}
