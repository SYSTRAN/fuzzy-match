#include <fuzzy/fuzzy_match.hh>

#include <queue>
#include <vector>
#include <cmath>
#include <set>

#include <fuzzy/ngram_matches.hh>
#include <fuzzy/edit_distance.hh>

#include <onmt/unicode/Unicode.h>
#include <boost/make_unique.hpp>

namespace fuzzy
{
  const char FuzzyMatch::version = '1';

  class CompareMatch
  {
  public:
    bool operator()(const FuzzyMatch::Match &x, const FuzzyMatch::Match &y)
    {
      return x.score < y.score || 
             (x.score == y.score && x.s_id > y.s_id);
    }
  };


  FuzzyMatch::FuzzyMatch(int pt, size_t max_tokens_in_pattern)
  : _pt(pt)
  , _suffixArrayIndex(boost::make_unique<SuffixArrayIndex>(max_tokens_in_pattern))
  {
    _update_tokenizer();
  }

  void FuzzyMatch::_update_tokenizer() {
    int flags_tokenizer = onmt::Tokenizer::Flags::SegmentAlphabetChange
                          | onmt::Tokenizer::Flags::NoSubstitution
                          | onmt::Tokenizer::Flags::SupportPriorJoiners;
    if (_pt & pt_cas)
      flags_tokenizer |= onmt::Tokenizer::Flags::CaseFeature;
    if (_pt & pt_jnr)
      flags_tokenizer |= onmt::Tokenizer::Flags::JoinerNew
                         | onmt::Tokenizer::Flags::JoinerAnnotate;
    else if (_pt & pt_sep)
      flags_tokenizer |= onmt::Tokenizer::Flags::SpacerNew
                         | onmt::Tokenizer::Flags::SpacerAnnotate;

    _ptokenizer = boost::make_unique<decltype(_ptokenizer)::element_type>(onmt::Tokenizer::Mode::Aggressive, flags_tokenizer, "");
    /* segment on following alphabets */
    _ptokenizer->add_alphabet_to_segment("Han");
    _ptokenizer->add_alphabet_to_segment("Kanbun");
    _ptokenizer->add_alphabet_to_segment("Katakana");
    _ptokenizer->add_alphabet_to_segment("Hiragana");
    UErrorCode status = U_ZERO_ERROR;
    UParseError error;

    _ptrans = std::unique_ptr<decltype(_ptrans)::element_type>(icu::Transliterator::createFromRules("NFC", "::NFC;", UTRANS_FORWARD, error, status));
  }

  void
  FuzzyMatch::_tokenize_and_normalize(const std::string &sentence,
                                      Sentence& real,
                                      Tokens& pattern) const {
    std::vector<std::string> tokens;
    std::vector<std::vector<std::string> > features;
    std::vector<unsigned> map_tokens;
    _tokenize_and_normalize(sentence,
                            real,
                            pattern,
                            map_tokens,
                            tokens,
                            features);
  }

  void
  FuzzyMatch::_tokenize_and_normalize(const std::string &sentence,
                                      Sentence& real,
                                      Tokens& pattern,
                                      std::vector<unsigned> &map_tokens,
                                      std::vector<std::string> &tokens,
                                      std::vector<std::vector<std::string>> &features) const {
    /* ICU basic normalization */
    icu::UnicodeString u_sentence = icu::UnicodeString::fromUTF8(sentence);
    std::string sentence_norm;
    _ptrans->transliterate(u_sentence);
    u_sentence.toUTF8String(sentence_norm);
    _ptokenizer->tokenize(sentence_norm, tokens, features);

    real.reserve(tokens.size());
    pattern.reserve(tokens.size());

    map_tokens.push_back(0);

    for(size_t real_i = 0, i = 0; i < tokens.size(); i++)
    {
      const std::string &token = tokens[i];
      if (token == onmt::Tokenizer::spacer_marker || token == onmt::Tokenizer::joiner_marker) {
        real.set_itok(real_i, " ");
        continue;
      }
      /* for word tokens - keep only the case feature */
      if ((_pt & pt_cas) && features[0][i] != "N") {
        pattern.emplace_back(token);
        real.push_back(features[0][i]);
        real_i++;
        map_tokens.push_back(i+1);
      }
      else {
        if (onmt::Tokenizer::is_placeholder(token)) {
          size_t ph_begin = token.find(onmt::Tokenizer::ph_marker_open);
          static const std::string ph_id_sep("＃");
          static const std::string ph_value_sep("：");
          size_t ph_end = token.find(ph_id_sep, ph_begin);
          if (ph_end == std::string::npos) {
            ph_end = token.find(ph_value_sep, ph_begin);
            if (ph_end == std::string::npos) {
              ph_end = token.find(onmt::Tokenizer::ph_marker_close, ph_begin);
              if (ph_end == std::string::npos)
                ph_end = token.length();
            }
          }
          std::string ent = token.substr(ph_begin+onmt::Tokenizer::ph_marker_open.size(),
                                         ph_end-ph_begin-onmt::Tokenizer::ph_marker_open.size());
          if (ent.length()>=2 && ent.substr(0,2)=="it")
            ent = "it";
          if (ent == "it" && (_pt & pt_tag))
            real.set_itok(real_i, "T");
          else {
            pattern.emplace_back(onmt::Tokenizer::ph_marker_open+ent+onmt::Tokenizer::ph_marker_close);
            real.push_back(token);
            real_i++;
            map_tokens.push_back(i+1);
          }
        } else {
          unsigned int l;
          auto cp = onmt::unicode::utf8_to_cp((const unsigned char*)token.c_str(), l);
          if (onmt::unicode::is_number(cp)) {
            if (_pt & pt_nbr) {
              pattern.emplace_back(onmt::Tokenizer::ph_marker_open+"num"+onmt::Tokenizer::ph_marker_close);
              real.push_back(token);
              real_i++;
              map_tokens.push_back(i+1);
            } else {
              pattern.emplace_back(token);
              real.push_back(token);
              real_i++;
              map_tokens.push_back(i+1);
            }
          }
          else {
            if (!onmt::unicode::is_letter(cp) && _pt & pt_pct) {
              real.set_itok(real_i, token);
            }
            else {
              pattern.emplace_back(token);
              real.push_back(token);
              real_i++;
              map_tokens.push_back(i+1);
            }
          }
        }
      }
    }
  }

  /* backward compatibility */
  bool
  FuzzyMatch::add_tm(const std::string& id, const Tokens& norm, bool sort)
  {
    const Sentence real(norm);
    _suffixArrayIndex->add_tm(id, real, norm, sort);

    return true;
  }

  bool
  FuzzyMatch::add_tm(const std::string& id, const Sentence& source, const Tokens& norm, bool sort)
  {
    _suffixArrayIndex->add_tm(id, source, norm, sort);

    return true;
  }

  bool FuzzyMatch::add_tm(const std::string &id, const std::string &sentence, bool sort)
  {
    Sentence real;
    Tokens norm;
    _tokenize_and_normalize(sentence, real, norm);
    if (norm.size()==0) {
      std::cerr<<"WARNING: cannot index empty segment: "<<sentence<<" ("<<id<<")"<<std::endl;
      return false;
    }
    add_tm(id, real, norm, sort);
    return true;
  }

#ifndef NDEBUG
  std::ostream& FuzzyMatch::dump(std::ostream& os) const {
    return _suffixArrayIndex->dump(os);
  }
#endif

  void
  FuzzyMatch::sort()
  {
    _suffixArrayIndex->sort();
  }

  struct Subseq {
    float weight;
    size_t position;
    size_t length;

    bool operator<(const Subseq& other) const
    {
      return weight < other.weight || (weight == other.weight && position > other.position);
    }
  };

  class PatternMatch
  {
  public:
    PatternMatch(size_t pattern_length)
      : _pattern_length(pattern_length)
    {
    }

    void register_pattern_word(unsigned word_id, size_t position)
    {
      _words_positions[word_id].push_back(position);
    }

    // Counts the number of words in the pattern that are also in the sentence.
    size_t count_matched_words(const unsigned* sentence, size_t length) const
    {
      std::vector<bool> matched_words(_pattern_length, false);
      size_t num_matched_words = 0;

      for (size_t i = 0; i < length; ++i)
      {
        const auto it = _words_positions.find(sentence[i]);
        if (it == _words_positions.end())  // Sentence word is not in the pattern.
          continue;

        for (const auto position : it->second)
        {
          if (!matched_words[position])
          {
            matched_words[position] = true;
            num_matched_words++;
          }
        }
      }

      return num_matched_words;
    }

  private:
    std::map<unsigned, std::vector<size_t>> _words_positions;
    size_t _pattern_length;
  };

  /* subsequence operator */
  bool FuzzyMatch::subsequence(const std::string &sentence,
                         unsigned number_of_matches,
                         bool no_perfect,
                         std::vector<Match>& matches,
                         int min_subseq_length,
                         float min_subseq_ratio,
                         bool idf_weighting) const {

    Sentence real;
    Tokens pattern;
    std::vector<std::string> tokens;
    std::vector<std::vector<std::string>> features;
    std::vector<unsigned> map_tokens;

    _tokenize_and_normalize(sentence, real, pattern, map_tokens, tokens, features);

    size_t p_length = pattern.size();

    if ((int)(min_subseq_ratio*p_length) > min_subseq_length)
      min_subseq_length = min_subseq_ratio*p_length;

    if ((int)p_length < min_subseq_length)
      return false;

    SuffixArrayIndex& SAI = *_suffixArrayIndex;

    /* get vocab id once for all */
    std::vector<unsigned> pidx = SAI.get_VocabIndexer().getIndex(pattern);

    std::vector<float> idf_penalty;
    const std::vector<unsigned> &sfreq = SAI.get_VocabIndexer().getSFreq();
    unsigned num_sentences = SAI.get_SuffixArray().num_sentences();
    idf_penalty.reserve(pidx.size());
    for(auto idx: pidx) {
      if (idx != fuzzy::VocabIndexer::VOCAB_UNK)
        idf_penalty.push_back(std::log(num_sentences*1.0/sfreq[idx]));
      else
        /* unknown word - we cannot find a subsequence with it */
        idf_penalty.push_back(-1);
    }

    /* sort the subsequences by idf weight */
    std::priority_queue<Subseq> subseq_queue;
    for(size_t it=0; it < p_length; it++) {
      float idf_weight = 0;
      for(size_t jt=it; jt < p_length; jt++) {
        float weight = idf_penalty[jt];
        if (weight == -1) break;
        idf_weight += idf_weighting?weight:1;
        if (int(jt-it+1) >= min_subseq_length)
          subseq_queue.emplace(Subseq{idf_weight, it, jt-it+1});
      }
    }

    int max_distance = 10000;
    Match best_match;

    std::set<unsigned> candidates;
    std::set<unsigned> perfect;

    std::vector<const char*> st(p_length+1);
    std::vector<int> sn(p_length+1);
    Tokens realtok = (Tokens)real;
    real.get_itoks(st, sn);

    while(!subseq_queue.empty() &&
          max_distance == 10000) {
      auto &subseq = subseq_queue.top();

      size_t current_min_suffixid = 0;
      size_t current_max_suffixid = 0;
      std::pair<size_t, size_t> range_suffixid = SAI.get_SuffixArray().equal_range(pidx.data() + subseq.position, subseq.length, current_min_suffixid, current_max_suffixid);

      for(auto suffixIt=range_suffixid.first; suffixIt < range_suffixid.second &&
                                            candidates.size()<number_of_matches; suffixIt++) {
        size_t s_id = SAI.get_SuffixArray().get_suffix_view(suffixIt).sentence_id;
        if (candidates.find(s_id) == candidates.end() &&
            perfect.find(s_id) == perfect.end()) {
          size_t s_length = 0;
          const auto* thes = SAI.get_SuffixArray().get_sentence(s_id, &s_length);

          Costs costs;
          costs.diff_word = 100. / std::max(s_length, p_length);

          /* let us calculate edit_distance  */
          float cost = _edit_distance(thes, SAI.real_tokens(s_id), s_length,
                                      pidx.data(), realtok, p_length,
                                      st, sn,
                                      idf_penalty, 0,
                                      costs, max_distance);
          if (cost==0 && no_perfect) {
            perfect.insert(s_id);
            continue;
          }
          if (cost < max_distance) {
            Match m;
            best_match.score = int(10000-cost*100)/10000.0;
            best_match.max_subseq = subseq.length;
            best_match.s_id = s_id;
            best_match.id = SAI.id(s_id);
            unsigned org_it = map_tokens[subseq.position];
            unsigned org_jt = map_tokens[subseq.position + subseq.length];
            std::vector<std::string> tokens_subseq(tokens.begin()+org_it, tokens.begin()+org_jt);
            std::vector<std::vector<std::string> > features_subseq;
            if (features.size()) {
              features_subseq.push_back(std::vector<std::string>(features[0].begin()+org_it, features[0].begin()+org_jt));
            }
            best_match.id += "\t" + _ptokenizer->detokenize(tokens_subseq, features_subseq);

            max_distance = cost;
            if (cost == 0) break;
          }
          candidates.insert(s_id);
        }
      }

      subseq_queue.pop();
    }

    if (max_distance != 10000) {
      matches.push_back(best_match);
      return true;
    }
    return false;
  }

  float FuzzyMatch::compute_max_idf_penalty() const {
    const unsigned num_sentences = _suffixArrayIndex->get_SuffixArray().num_sentences();
    return std::log(num_sentences);
  }

  std::vector<float> FuzzyMatch::compute_idf_penalty(const std::vector<unsigned>& pattern_wids) const {
    std::vector<float> idf_penalty;
    idf_penalty.reserve(pattern_wids.size());

    const unsigned num_sentences = _suffixArrayIndex->get_SuffixArray().num_sentences();

    const std::vector<unsigned>& word_frequency_in_sentences = _suffixArrayIndex->get_VocabIndexer().getSFreq();

    for (const auto wid : pattern_wids) {
      // https://en.wikipedia.org/wiki/TF-IDF
      if (wid != fuzzy::VocabIndexer::VOCAB_UNK)
        idf_penalty.push_back(std::log((float)num_sentences/(float)word_frequency_in_sentences[wid]));
      else
        idf_penalty.push_back(0);
    }

    return idf_penalty;
  }

  /* interface with integrated tokenization */
  bool FuzzyMatch::match(const std::string &sentence,
                         float fuzzy,
                         unsigned number_of_matches,
                         bool no_perfect,
                         std::vector<Match>& matches,
                         int min_subseq_length,
                         float min_subseq_ratio,
                         float vocab_idf_penalty) const {

    Sentence real;
    Tokens norm;
    _tokenize_and_normalize(sentence, real, norm);
    return match(real, norm, fuzzy, number_of_matches, no_perfect, matches,
                 min_subseq_length, min_subseq_ratio, vocab_idf_penalty);
  }

  /* backward compatibility */
  bool
  FuzzyMatch::match(const Tokens& pattern,
                    float fuzzy,
                    unsigned number_of_matches,
                    std::vector<Match>& matches,
                    int min_subseq_length,
                    float min_subseq_ratio,
                    float vocab_idf_penalty) const
  {
    const Sentence real(pattern);
    return match(real, pattern, fuzzy, number_of_matches, false, matches,
                 min_subseq_length, min_subseq_ratio, vocab_idf_penalty);
  }

  /* check for the pattern in the suffix-array index SAI */ 
  bool
  FuzzyMatch::match(const Sentence& real,
                    const Tokens& pattern,
                    float fuzzy,
                    unsigned number_of_matches,
                    bool no_perfect,
                    std::vector<Match>& matches,
                    int min_subseq_length,
                    float min_subseq_ratio,
                    float vocab_idf_penalty) const
  {
    size_t p_length = pattern.size();

    // performance guard
    if (p_length > max_tokens_in_pattern())
    {
      return false; // no matches
    }

    if (!p_length)
      return false;

    if ((std::size_t)(min_subseq_length) > pattern.size())
      min_subseq_length = pattern.size();

    if ((int)(min_subseq_ratio*p_length) > min_subseq_length)
      min_subseq_length = min_subseq_ratio*p_length;

    /* get vocab id once for all */
    const auto pattern_wids = _suffixArrayIndex->get_VocabIndexer().getIndex(pattern);

    float idf_max = 0.01;
    std::vector<float> idf_penalty;
    if (vocab_idf_penalty) {
      idf_penalty = compute_idf_penalty(pattern_wids);
      idf_max = compute_max_idf_penalty();
    }

    /* result map - normalized error => sentence */
    std::priority_queue<Match, std::vector<Match>, CompareMatch> result;

    NGramMatches nGramMatches(fuzzy, p_length, min_subseq_length, _suffixArrayIndex->get_SuffixArray());
    PatternMatch pattern_match(p_length);

    if (p_length == 1)
    {
      std::pair<size_t, size_t> range_suffixid = _suffixArrayIndex->get_SuffixArray().equal_range(pattern_wids.data(), p_length);

      if (range_suffixid.first != range_suffixid.second)
        nGramMatches.register_suffix_range_match(range_suffixid.first,
                                                 range_suffixid.second,
                                                 p_length);
    }

    for (size_t it=0; it < p_length; it++)
    {
      /* unigram indexing */
      pattern_match.register_pattern_word(pattern_wids[it], it);

      std::pair<size_t, size_t> previous_range_suffixid(0, 0);
      size_t subseq_length = 0;

      for (size_t jt = it; jt < p_length; jt++)
      {
        ++subseq_length;
        /*
          the set of solution will be a decreasing range
          pos-i     ngram
          pos-i+1   ngram
          pos-i+2   ngram   (n+1)gram
          pos-i+3   ngram   (n+1)gram   (n+2)gram
          pos-i+4   ngram   (n+1)gram
          pos-i+5   ngram

          and we will only keep the matches:
          pos-i     ngram
          pos-i+1   ngram
          pos-i+2   (n+1)gram
          pos-i+3   (n+2)gram
          pos-i+4   (n+1)gram
          pos-i+5   ngram
        */
        std::pair<size_t, size_t> range_suffixid = _suffixArrayIndex->get_SuffixArray().equal_range(pattern_wids.data() + it, subseq_length, previous_range_suffixid.first, previous_range_suffixid.second);

        if (range_suffixid.first != range_suffixid.second)
        {
          /* do not register unigrams - yet */
          if (subseq_length > 2)
          {
            /* register (n-1) grams */
            nGramMatches.register_suffix_range_match(previous_range_suffixid.first,
                                                     range_suffixid.first,
                                                     subseq_length - 1);
            nGramMatches.register_suffix_range_match(range_suffixid.second,
                                                     previous_range_suffixid.second,
                                                     subseq_length - 1);
          }

          previous_range_suffixid = std::move(range_suffixid);
        }
        else
        {
          --subseq_length;
          break;
        }
      }
      if (subseq_length >= 2)
        nGramMatches.register_suffix_range_match(previous_range_suffixid.first,
                                                 previous_range_suffixid.second,
                                                 subseq_length);
    }

    /* Consolidation of the results */

    /* now explore for the best segments */

    std::vector<const char*> st(p_length+1);
    std::vector<int> sn(p_length+1);
    Tokens pattern_realtok = (Tokens)real;

    real.get_itoks(st, sn);

    for (const auto& pair : nGramMatches.get_longest_matches())
    {
      const auto s_id = pair.first;
      const auto longest_match = pair.second;
      size_t s_length = 0;
      const auto* sentence_wids = _suffixArrayIndex->get_SuffixArray().get_sentence(s_id, &s_length);
      const auto num_matched_words = (longest_match < p_length
                                      ? pattern_match.count_matched_words(sentence_wids, s_length)
                                      : p_length);

      /* do not care checking sentences that do not have enough ngram matches for the fuzzy threshold */
      if (p_length - num_matched_words <= nGramMatches.max_differences_with_pattern)
      {
        Costs costs;
        costs.diff_word = 100. / std::max(s_length, p_length);

        /* let us check the candidates */
        const auto sentence_realtok = _suffixArrayIndex->real_tokens(s_id);
        float cost = _edit_distance(sentence_wids, sentence_realtok, s_length,
                                    pattern_wids.data(), pattern_realtok, p_length,
                                    st, sn,
                                    idf_penalty, costs.diff_word*vocab_idf_penalty/idf_max,
                                    costs, 100-fuzzy);
#ifdef DEBUG
        std::cout << "cost=" << cost << "+" << s_idf_penalty << "/" << O.max << std::endl;
#endif

        float score = int(10000-cost*100)/10000.0;
        if (score >= fuzzy &&
            (!no_perfect || score != 1)) {
          Match m;
          m.score = score;
          m.max_subseq = longest_match;
          m.s_id = s_id;
          m.id = _suffixArrayIndex->id(s_id);
          result.push(m);
        }
      }
    }

    while (!result.empty() && (number_of_matches == 0 || matches.size() < number_of_matches))
    {
      auto match = result.top();
#ifdef DEBUG
      std::cout << match.score << "\t" << alignment.second << "\t"
                << match.id << std::endl;
#endif

      matches.push_back(match);

      result.pop();
    }

    return matches.size() > 0;
  }
}

