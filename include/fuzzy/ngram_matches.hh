#pragma once

#include <fuzzy/suffix_array.hh>
#include <fuzzy/costs.hh>
#include <fuzzy/tsl/hopscotch_map.h>
#include <fuzzy/filter_matches.hh>

namespace fuzzy
{
  // Sentence ID -> longest N-gram match
  using LongestMatches = tsl::hopscotch_map<unsigned, unsigned, IntHash>;

  class NGramMatches : public FilterMatches
  {
  public:
    using FilterMatches::FilterMatches;
    NGramMatches(float fuzzy,
                 unsigned p_length,
                 unsigned min_seq_len,
                 const SuffixArray&);

    // Registers a match for this range of suffixes.
    void register_suffix_range_match(
      size_t begin,
      size_t end,
      unsigned match_length,
      const EditCosts& edit_costs=EditCosts()
    );
    using FilterMatches::theoretical_rejection;
    using FilterMatches::theoretical_rejection_cover;

    std::vector<std::pair<unsigned, unsigned>> get_longest_matches() const override;

  private:
    LongestMatches _longest_matches;
  };
}
