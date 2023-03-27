#pragma once

#include <fuzzy/filter.hh>
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
  unsigned compute_min_exact_match(float fuzzy, unsigned p_length);

  // Sentence ID -> longest N-gram match
  // using LongestMatches = tsl::hopscotch_map<unsigned, unsigned, IntHash>;

  class FilterMatches
  {
  public:
    FilterMatches(float fuzzy,
                 unsigned p_length,
                 unsigned min_seq_len,
                 const Filter&);

    const bool theoretical_rejection(size_t p_length, size_t s_length, const EditCosts& edit_costs) const;
    const bool theoretical_rejection_cover(size_t p_length, size_t s_length, size_t cover, const EditCosts& edit_costs) const;

    virtual std::vector<std::pair<unsigned, unsigned>> get_best_matches() const = 0;

    float fuzzy_threshold;
    // unsigned max_differences_with_pattern;
    unsigned min_exact_match; // Any suffix without an subsequence of at least this with the pattern won't be accepted later

  protected:
    unsigned _min_seq_len;
    unsigned _p_length;
    const Filter& _filter;
  };
}
