#pragma once

#include <vector>

namespace fuzzy
{
  struct AgendaItem
  {
  public:
    inline AgendaItem() = default;

    inline AgendaItem(unsigned s_id, unsigned p_length)
            : s_id(s_id), map_pattern(p_length), coverage(0), maxmatch(0)
    {
    }

    unsigned s_id;
    std::vector<bool> map_pattern; /* set of unigrams that are both in the pattern and in the suffix
                                      map_pattern[i]==true means that the word pattern[i] appears in the suffix */
    int coverage; // number of true values in map_pattern
    size_t maxmatch; // maximum length of the longest subsequence between the sentence associated with the AgendaItem and the pattern
  };
}
