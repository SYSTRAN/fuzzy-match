#pragma once

#include <fuzzy/filter_matches.hh>
#include <fuzzy/no_filter.hh>

namespace fuzzy
{
    class NoMatches : public FilterMatches
    {
    public:
        using FilterMatches::FilterMatches;
        NoMatches(float fuzzy,
                  unsigned p_length,
                  unsigned min_seq_len,
                  const NoFilter &);

        // Registers a match for this range of suffixes.
        using FilterMatches::theoretical_rejection;
        using FilterMatches::theoretical_rejection_cover;

        void load_all();
        std::vector<std::pair<unsigned, unsigned>> get_best_matches() const override;

    private:
        std::vector<std::pair<unsigned, unsigned>> _all_matches;
    };
}
