#include <fuzzy/bm25_matches.hh>

#include <cmath>

namespace fuzzy
{
  BM25Matches::BM25Matches(float fuzzy,
                             unsigned p_length,
                             unsigned min_seq_len,
                             const BM25& bm25)
      /* add a small epsilon to avoid rounding errors counting for an error */
      : FilterMatches(fuzzy, p_length, min_seq_len, bm25)
  {}

  std::vector<std::pair<unsigned, unsigned>>
  BM25Matches::get_longest_matches() const
  {
    std::vector<std::pair<unsigned, unsigned>> sorted_matches;
    // TODO
    return sorted_matches;
  }
}
