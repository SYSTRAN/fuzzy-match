#pragma once

#include <fuzzy/suffix_array.hh>
#include <fuzzy/agenda_item.hh>

#include <fuzzy/tsl/hopscotch_map.h>

namespace fuzzy
{
  class NGramMatches
  {
  public:
    NGramMatches(size_t size_tm,
                 float fuzzy, unsigned p_length,
                 unsigned min_seq_len,
                 const SuffixArray&);

    void register_suffix_range(size_t begin, size_t end, size_t match_length);
    int get_sentence_count() const;
    tsl::hopscotch_map<unsigned, AgendaItem>& get_psentences();

    unsigned max_differences_with_pattern;
    unsigned min_exact_match; // Any suffix without an subsequence of at least this with the pattern won't be accepted later

  private:
    unsigned _p_length;
    unsigned _min_seq_len;
    const SuffixArray& _suffixArray;
    tsl::hopscotch_map<unsigned, AgendaItem> _psentences; // association of sentence id => AgendaItem, owns the AgendaItem
  };
}
