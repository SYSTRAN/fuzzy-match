#include <fuzzy/submodular.hh>
#include <algorithm>
#include <cmath>

namespace fuzzy
{
    void get_bow_score(
        std::vector<unsigned>& sorted_pattern_terms,
        std::vector<unsigned>& count_terms,
        const unsigned* sentence,
        const unsigned sentence_length,
        float& score,
        std::vector<float>& cover)
    {
        std::vector<unsigned> sorted_sentence_terms(sentence, sentence + sentence_length);
        std::sort(sorted_sentence_terms.begin(), sorted_sentence_terms.end());
        cover = std::vector<float>(sorted_pattern_terms.size(), 0.f);
        score = 0.f;
        for (unsigned i, j = 0; (i < sorted_pattern_terms.size()) && (j < sorted_sentence_terms.size()); j++)
        {
            while (
                (i < sorted_pattern_terms.size()) && 
                (sorted_pattern_terms[i] < sorted_sentence_terms[j]))
                i++;
            // std::cerr << sorted_pattern_terms[i] << " ?= " << sorted_sentence_terms[j] << "  (" << i << ", " << j << ")" << std::endl;
            if (sorted_pattern_terms[i] == sorted_sentence_terms[j])
                if ((float)count_terms[i] > cover[i] + 1e-6f)
                {
                    cover[i] += 1.f;
                    score += 1.f;
                }
        }
    }
    void get_bow_score_idf(
        std::vector<unsigned>& sorted_pattern_terms,
        std::vector<unsigned>& count_terms,
        const unsigned* sentence,
        const unsigned sentence_length,
        std::vector<float>& idf_penalty,
        float& score,
        std::vector<float>& cover)
    {
        std::vector<unsigned> sorted_sentence_terms(sentence, sentence + sentence_length);
        std::sort(sorted_sentence_terms.begin(), sorted_sentence_terms.end());
        cover = std::vector<float>(sorted_pattern_terms.size(), 0.f);
        score = 0.f;
        for (unsigned i, j, k = 0; (i < sorted_pattern_terms.size()) && (j < sorted_sentence_terms.size()); j++)
        {
            while (
                (i < sorted_pattern_terms.size()) && 
                (sorted_pattern_terms[i] < sorted_sentence_terms[j]))
                i++;
            // std::cerr << sorted_pattern_terms[i] << " ?= " << sorted_sentence_terms[j] << "  (" << i << ", " << j << ")" << std::endl;
            if (sorted_pattern_terms[i] == sorted_sentence_terms[j])
                if ((float)count_terms[i] > cover[i] / idf_penalty[i] + 1e-6f)
                {
                    cover[i] += idf_penalty[i];
                    score += idf_penalty[i];
                }
        }
    }
}