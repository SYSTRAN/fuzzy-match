#pragma once

#include <fuzzy/suffix_array.hh>

#include <fuzzy/tsl/hopscotch_map.h>

namespace fuzzy
{
  // This class tracks the words in the input pattern that are also found in a sentence.
  class PatternMatch {
  public:
    PatternMatch(size_t pattern_length)
      : _matched_words(pattern_length, false)
      , _num_matches(0)
      , _longest_match(0)
    {
    }

    // Mark a range of words as matched in a sentence.
    void set_match(size_t index, size_t length = 1) {
      for (size_t i = index; i < index + length; ++i) {
        if (!_matched_words[index]) {
          _matched_words[index] = true;
          _num_matches++;
        }
      }

      _longest_match = std::max(_longest_match, length);
    }

    size_t num_non_matched_words() const {
      return _matched_words.size() - _num_matches;
    }

    size_t num_matched_words() const {
      return _num_matches;
    }

    // Longest consecutive match.
    size_t longest_match() const {
      return _longest_match;
    }

  private:
    std::vector<bool> _matched_words;
    size_t _num_matches;
    size_t _longest_match;
  };

  // Sentence ID -> PatternMatch
  using PatternMatches = tsl::hopscotch_map<unsigned, PatternMatch>;

  class NGramMatches
  {
  public:
    NGramMatches(size_t size_tm,
                 float fuzzy, unsigned p_length,
                 unsigned min_seq_len,
                 const SuffixArray&);

    // Registers that the pattern words at [match_offset, match_offset+match_length-1] are matching
    // this range of suffixes.
    void register_suffix_range_match(size_t begin,
                                     size_t end,
                                     size_t match_offset,
                                     size_t match_length);

    PatternMatches& get_pattern_matches();

    unsigned max_differences_with_pattern;
    unsigned min_exact_match; // Any suffix without an subsequence of at least this with the pattern won't be accepted later

  private:
    unsigned _p_length;
    unsigned _min_seq_len;
    const SuffixArray& _suffixArray;
    PatternMatches _pattern_matches;
  };
}
