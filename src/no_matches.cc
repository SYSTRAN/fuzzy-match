#include <fuzzy/no_matches.hh>

#include <cmath>

namespace fuzzy
{
    NoMatches::NoMatches(float fuzzy,
                         unsigned p_length,
                         unsigned min_seq_len,
                         const NoFilter &no_filter)
        /* add a small epsilon to avoid rounding errors counting for an error */
        : FilterMatches(fuzzy, p_length, min_seq_len, no_filter)
    {
    }

    std::vector<std::pair<unsigned, int>>
    NoMatches::get_best_matches() const
    {
        return _all_matches;
    }

    void NoMatches::load_all()
    {
        _all_matches = std::vector<std::pair<unsigned, int>>(_filter.num_sentences());
        size_t *length;

        for (unsigned i = 0; i < _filter.num_sentences(); i++)
            _all_matches[i] = {i, 0};
    }
}
