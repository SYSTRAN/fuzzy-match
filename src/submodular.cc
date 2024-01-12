#include <fuzzy/submodular.hh>
#include <cmath>
#include <iostream>

namespace fuzzy
{
    NGram::NGram(const unsigned* start, unsigned N) : 
        _start(start),
        _N(N) {}
    NGram& NGram::operator=(const NGram& other) {
        if (this != &other)
        {
            _start = other._start;
            _N = other._N;
        }
        return *this;
    }
    bool NGram::operator==(NGram& other)
    {
        bool out = _N == other._N;
        for (unsigned i = 0; out && (i < _N); i++)
            out = (other._start[i] == _start[i]);
        return out;
    }
    bool NGram::operator<(NGram& other)
    {
        if (_N != other._N)
            return _N < other._N;
        return std::lexicographical_compare(
            _start, _start + _N, 
            other._start, other._start + other._N);
    }
    void NGram::print() const
    {
        for (unsigned i = 0; i < _N; i++)
            std::cerr << _start[i] << ",";
        std::cerr << '\t';
    }

    void get_bow_score(
        std::vector<unsigned>& sorted_pattern_terms,
        std::vector<unsigned>& count_terms,
        const unsigned* sentence,
        const unsigned sentence_length,
        float& score,
        std::vector<float>& cover,
        std::vector<float>& idf_penalty)
    {
        std::vector<unsigned> sorted_sentence_terms(sentence, sentence + sentence_length);
        std::sort(sorted_sentence_terms.begin(), sorted_sentence_terms.end());
        get_score(sorted_pattern_terms, sorted_sentence_terms, count_terms, score, cover, idf_penalty);
    }

    void get_ngram_score(
        std::vector<NGram>& sorted_pattern_terms,
        const unsigned N,
        std::vector<unsigned>& count_terms,
        const unsigned* sentence,
        const unsigned sentence_length,
        float& score,
        std::vector<float>& cover,
        std::vector<float>& idf_penalty)
    {
        std::vector<NGram> all_ngrams = get_sorted_ngrams(N, sentence, sentence_length);
        get_score(sorted_pattern_terms, all_ngrams, count_terms, score, cover, idf_penalty);
    }

    void get_all_ngrams(
        const unsigned* sequence,
        const unsigned length,
        const unsigned N,
        std::vector<NGram>& ngrams,
        std::vector<unsigned>& counts)
    {
        std::vector<NGram> all_ngrams = get_sorted_ngrams(N, sequence, length);
        get_unique_with_count(all_ngrams, ngrams, counts);
    }
}