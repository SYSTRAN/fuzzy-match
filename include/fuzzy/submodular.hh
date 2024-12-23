#pragma once

#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>


namespace fuzzy
{
    class NGram
    {
        public:
            NGram(const unsigned* start, unsigned N);
            ~NGram() {}
            NGram& operator=(const NGram& other);
            bool operator==(NGram& other);
            bool operator<(NGram& other);
            void print() const;
            const unsigned* _start;
            unsigned _N;
    };

    inline
    std::vector<NGram> get_sorted_ngrams(
        const unsigned N,
        const unsigned* sentence,
        const unsigned sentence_length);

    template <typename T>
    void get_unique_with_count(
        std::vector<T>& sorted_salient,
        std::vector<T>& unique,
        std::vector<unsigned>& count);

    template <typename T>
    void get_score(
        std::vector<T>& sorted_pattern_terms,
        std::vector<T>& sorted_sentence_terms,
        std::vector<unsigned>& count_terms,
        float& score,
        std::vector<float>& cover,
        std::vector<float>& idf_penalty);

    void get_bow_score(
        std::vector<unsigned>& sorted_pattern_terms,
        std::vector<unsigned>& count_terms,
        const unsigned* sentence,
        const unsigned sentence_length,
        float& score,
        std::vector<float>& cover,
        std::vector<float>& idf_penalty);

    void get_ngram_score(
        std::vector<NGram>& sorted_pattern_terms,
        const unsigned N,
        std::vector<unsigned>& count_terms,
        const unsigned* sentence,
        const unsigned sentence_length,
        float& score,
        std::vector<float>& cover,
        std::vector<float>& idf_penalty);

    // void get_bow_score_idf(
    //     std::vector<unsigned>& sorted_pattern_terms,
    //     std::vector<unsigned>& count_terms,
    //     const unsigned* sentence,
    //     const unsigned sentence_length,
    //     std::vector<float>& idf_penalty,
    //     float& score,
    //     std::vector<float>& cover);

    void get_all_ngrams(
        const unsigned* sequence,
        const unsigned length,
        const unsigned N,
        std::vector<NGram>& ngrams,
        std::vector<unsigned>& counts);
}

#include "submodular.hxx"
