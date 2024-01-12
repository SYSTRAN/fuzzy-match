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
        std::cerr << "avant " << sentence_length << std::endl;
        std::vector<NGram> all_ngrams = get_sorted_ngrams(N, sentence, sentence_length);
        std::cerr << "apres" << std::endl;
        // all_ngrams.reserve(N * sentence_length - N * (N - 1) / 2);
        // for (unsigned n = 1; n <= N; n++)
        //     for (unsigned i = 0; i < sentence_length - n + 1; i++)
        //         all_ngrams.push_back(NGram(sentence + i, n));

        // std::sort(all_ngrams.begin(), all_ngrams.end());

        for (const NGram& ngram : all_ngrams)
            ngram.print();
        std::cerr << std::endl;

        get_score(sorted_pattern_terms, all_ngrams, count_terms, score, cover, idf_penalty);
        std::cerr << "xxx" << std::endl;
    }

    // void get_bow_score(
    //     std::vector<unsigned>& sorted_pattern_terms,
    //     std::vector<unsigned>& count_terms,
    //     const unsigned* sentence,
    //     const unsigned sentence_length,
    //     float& score,
    //     std::vector<float>& cover)
    // {
    //     std::vector<unsigned> sorted_sentence_terms(sentence, sentence + sentence_length);
    //     std::sort(sorted_sentence_terms.begin(), sorted_sentence_terms.end());
    //     cover = std::vector<float>(sorted_pattern_terms.size(), 0.f);
    //     score = 0.f;
    //     for (unsigned i, j = 0; (i < sorted_pattern_terms.size()) && (j < sorted_sentence_terms.size()); j++)
    //     {
    //         while (
    //             (i < sorted_pattern_terms.size()) && 
    //             (sorted_pattern_terms[i] < sorted_sentence_terms[j]))
    //             i++;
    //         if (sorted_pattern_terms[i] == sorted_sentence_terms[j])
    //             if ((float)count_terms[i] > cover[i] + 1e-6f)
    //             {
    //                 cover[i] += 1.f;
    //                 score += 1.f;
    //             }
    //     }
    // }

    void get_all_ngrams(
        const unsigned* sequence,
        const unsigned length,
        const unsigned N,
        std::vector<NGram>& ngrams,
        std::vector<unsigned>& counts)
    {
        std::vector<NGram> all_ngrams = get_sorted_ngrams(N, sequence, length);
        // std::vector<NGram> all_ngrams;
        // all_ngrams.reserve(N * length - N * (N - 1) / 2);
        // for (unsigned n = 1; n <= N; n++)
        //     for (unsigned i = 0; i < length - n + 1; i++)
        //         all_ngrams.push_back(NGram(sequence + i, n));
        // // std::cerr << std::endl;
        // std::sort(all_ngrams.begin(), all_ngrams.end());
        for (const NGram& ngram : all_ngrams)
            ngram.print();
        std::cerr << std::endl;

        get_unique_with_count(all_ngrams, ngrams, counts);
        // std::cerr << "ALL" << std::endl;
        // for (unsigned i = 0; i < ngrams.size(); i++)
        // {
        //     ngrams[i].print();
        //     std::cerr << counts[i] << std::endl;
        // }
        // std::cerr << std::endl;
        // std::cerr << "ONLY > 1" << std::endl;
        // for (unsigned i = 0; i < ngrams.size(); i++)
        //     if (counts[i] > 1)
        //     {
        //         ngrams[i].print();
        //         std::cerr << counts[i] << std::endl;
        //     }
        // std::cerr << std::endl;
        
    }
}