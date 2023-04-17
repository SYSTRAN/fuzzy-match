#pragma once

#include <fuzzy/suffix_array.hh>
#include <fuzzy/costs.hh>
#include <fuzzy/tsl/hopscotch_map.h>

namespace fuzzy
{
  struct IntHash {
    unsigned int operator()(unsigned int x) const {
      // Credit: https://stackoverflow.com/a/12996028
      x = ((x >> 16) ^ x) * 0x45d9f3b;
      x = ((x >> 16) ^ x) * 0x45d9f3b;
      x = (x >> 16) ^ x;
      return x;
    }
  };

  // Sentence ID -> longest N-gram match
  using LongestMatches = tsl::hopscotch_map<unsigned, unsigned, IntHash>;

  class NGramMatches
  {
  public:
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
    bool theoretical_rejection(size_t p_length, size_t s_length, const EditCosts& edit_costs) const;
    bool theoretical_rejection_cover(size_t p_length, size_t s_length, size_t cover, const EditCosts& edit_costs) const;

    std::vector<std::pair<unsigned, unsigned>> get_longest_matches() const;

    float fuzzy_threshold;
    unsigned min_exact_match; // Any suffix without an subsequence of at least this with the pattern won't be accepted later

  private:
    unsigned _p_length;
    unsigned _min_seq_len;
    const SuffixArray& _suffixArray;
    LongestMatches _longest_matches;
  };
}
