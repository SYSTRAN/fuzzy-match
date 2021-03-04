#include <fuzzy/fuzzy_match.hh>

#include <queue>
#include <vector>
#include <cmath>
#include <set>

#include <fuzzy/ngram_matches.hh>
#include <fuzzy/edit_distance.hh>

#include <onmt/unicode/Unicode.h>
#include <onmt/Tokenizer.h>

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


  FuzzyMatch::FuzzyMatch(int pt)
    : _pt(pt), _ptokenizer(0), _ptrans(0), _suffixArrayIndex(new SuffixArrayIndex())
  {
    _update_tokenizer();
  }

  FuzzyMatch::~FuzzyMatch()
  {
    delete _ptokenizer;
    delete _ptrans;
    delete _suffixArrayIndex;
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

    delete _ptokenizer;
    _ptokenizer = new onmt::Tokenizer(onmt::Tokenizer::Mode::Aggressive,
                                      flags_tokenizer, "");
    /* segment on following alphabets */
    _ptokenizer->add_alphabet_to_segment("Han");
    _ptokenizer->add_alphabet_to_segment("Kanbun");
    _ptokenizer->add_alphabet_to_segment("Katakana");
    _ptokenizer->add_alphabet_to_segment("Hiragana");
    UErrorCode status = U_ZERO_ERROR;
    UParseError error;
    delete _ptrans;
    _ptrans = icu::Transliterator::createFromRules(
                "NFC",
                "::NFC;",
                UTRANS_FORWARD, error, status);
  }

  void
  FuzzyMatch::_tokenize_and_normalize(const std::string &sentence,
                                      Sentence& real,
                                      Tokens& pattern) {
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
                                      std::vector<std::vector<std::string>> &features) {
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

  class Subseq {
  public:
    Subseq(float w, unsigned p, unsigned l):_w(w),_p(p),_l(l) {}
    float _w;
    unsigned _p;
    unsigned _l;
  };

  class CompareSubseq
  {
  public:
    bool operator()(const Subseq &x, const Subseq &y)
    {
      return x._w < y._w || 
             (x._w == y._w && x._p > y._p);
    }
  };

  /* subsequence operator */
  bool FuzzyMatch::subsequence(const std::string &sentence,
                         unsigned number_of_matches,
                         bool no_perfect,
                         std::vector<Match>& matches,
                         int min_subseq_length,
                         float min_subseq_ratio,
                         bool idf_weighting) {

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
    unsigned nsentences = SAI.get_SuffixArray().nsentences();
    idf_penalty.reserve(pidx.size());
    for(auto idx: pidx) {
      if (idx != fuzzy::VocabIndexer::VOCAB_UNK)
        idf_penalty.push_back(std::log(nsentences*1.0/sfreq[idx]));
      else
        /* unknown word - we cannot find a subsequence with it */
        idf_penalty.push_back(-1);
    }

    /* sort the subsequences by idf weight */
    std::priority_queue<Subseq, std::vector<Subseq>, CompareSubseq> subseq_queue;
    for(size_t it=0; it < p_length; it++) {
      float idf_weight = 0;
      for(size_t jt=it; jt < p_length; jt++) {
        float weight = idf_penalty[jt];
        if (weight == -1) break;
        idf_weight += idf_weighting?weight:1;
        if (int(jt-it+1) >= min_subseq_length)
          subseq_queue.push(Subseq(idf_weight, it, jt-it+1));
      }
    }

    int max_distance = 10000;
    Match best_match;

    std::set<unsigned> candidates;
    std::set<unsigned> perfect;
    std::vector<unsigned> p_i;

    std::vector<const char*> st(p_length+1);
    std::vector<int> sn(p_length+1);
    Tokens realtok = (Tokens)real;
    real.get_itoks(st, sn);

    while(!subseq_queue.empty() &&
          max_distance == 10000) {
      auto &subseq = subseq_queue.top();

      unsigned it = subseq._p;
      unsigned subseq_length = subseq._l;
      p_i.resize(subseq_length);
      size_t jt = it+subseq._l;
      std::copy(pidx.begin()+it, pidx.begin()+jt, p_i.begin());
      size_t current_min_suffixid = 0;
      size_t current_max_suffixid = 0;
      std::pair<size_t, size_t> range_suffixid = SAI.get_SuffixArray().equal_range(p_i, current_min_suffixid, current_max_suffixid);

      for(auto suffixIt=range_suffixid.first; suffixIt < range_suffixid.second &&
                                            candidates.size()<number_of_matches; suffixIt++) {
        size_t s_id = SAI.get_SuffixArray().suffixid2sentenceid()[suffixIt].sentence_id;
        if (candidates.find(s_id) == candidates.end() &&
            perfect.find(s_id) == perfect.end()) {
          unsigned idx = (SAI.get_SuffixArray())[s_id];
          size_t s_length = SAI.get_SuffixArray().sentence_buffer().begin()[idx];
          std::vector<unsigned> thes(SAI.get_SuffixArray().sentence_buffer().begin()+idx+1,
                                     SAI.get_SuffixArray().sentence_buffer().begin()+idx+s_length+1);

          Costs costs;
          costs.diff_word = 100. / std::max(s_length, p_length);

          /* let us calculate edit_distance  */
          float cost = _edit_distance(thes, SAI.real_tokens(s_id),
                                      pidx, realtok,
                                      p_length, st, sn,
                                      idf_penalty, 0,
                                      costs, max_distance);
          if (cost==0 && no_perfect) {
            perfect.insert(s_id);
            continue;
          }
          if (cost < max_distance) {
            Match m;
            best_match.score = int(10000-cost*100)/10000.0;
            best_match.max_subseq = subseq_length;
            best_match.s_id = s_id;
            best_match.id = SAI.id(s_id);
            unsigned org_it = map_tokens[it];
            unsigned org_jt = map_tokens[jt];
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
    const unsigned nr_sentences = _suffixArrayIndex->get_SuffixArray().nsentences();
    return std::log(nr_sentences);
  }

  std::vector<float> FuzzyMatch::compute_idf_penalty(const std::vector<unsigned>& pattern_wids) const {
    std::vector<float> idf_penalty;
    idf_penalty.reserve(pattern_wids.size());

    const unsigned nr_sentences = _suffixArrayIndex->get_SuffixArray().nsentences();

    const std::vector<unsigned>& word_frequency_in_sentences = _suffixArrayIndex->get_VocabIndexer().getSFreq();

    for (const auto wid : pattern_wids) {
      // https://en.wikipedia.org/wiki/TF-IDF
      if (wid != fuzzy::VocabIndexer::VOCAB_UNK)
        idf_penalty.push_back(std::log((float)nr_sentences/(float)word_frequency_in_sentences[wid]));
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
                         float vocab_idf_penalty) {

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
                    float vocab_idf_penalty)
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
                    float vocab_idf_penalty)
  {
    size_t p_length = pattern.size();

    // performance guard
    if (p_length >= SuffixArrayIndex::MAX_TOKENS_IN_PATTERN)
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
    int p_pos = 0;

    NGramMatches nGramMatches(_suffixArrayIndex->get_SuffixArray().nsentences(), fuzzy, p_length, _suffixArrayIndex->get_SuffixArray());

    /* index position of unigrams in the pattern */
    std::map<int, std::list<size_t> > p_unigrams;

    if (p_length == 1)
    {
      unsigned idx = pattern_wids[0];
      if (idx)
      {
        std::vector<unsigned> p_i;
        p_i.push_back(idx);
        std::pair<size_t, size_t> range_suffixid = _suffixArrayIndex->get_SuffixArray().equal_range(p_i, 0, 0);

        if (range_suffixid.first != range_suffixid.second)
          nGramMatches.register_ranges(true, fuzzy::Range(0, range_suffixid.first, range_suffixid.second, 1));
      }
    }

    std::vector<unsigned> p_i;
    p_i.reserve(pattern.size());
    for (size_t it=0; it < p_length; it++)
    {
      /* unigram indexing */
      p_unigrams[pattern_wids[it]].push_back(p_pos);
      p_pos++;
      p_i.clear();

      size_t current_min_suffixid = 0;
      size_t current_max_suffixid = 0;
      std::pair<size_t, size_t> previous_range_suffixid;

      for (size_t jt = it; jt < p_length; jt++)
      {
        int idx = pattern_wids[jt];
        p_i.push_back(idx);
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
        std::pair<size_t, size_t> range_suffixid = _suffixArrayIndex->get_SuffixArray().equal_range(p_i, current_min_suffixid, current_max_suffixid);

        if (range_suffixid.first != range_suffixid.second)
        {
          /* do not register unigrams - yet */
          if (p_i.size() > 2)
          {
            /* register (n-1) grams */
            nGramMatches.register_ranges(fuzzy::Range(p_pos - 1, previous_range_suffixid.first, range_suffixid.first, p_i.size() - 1), min_subseq_length);
            nGramMatches.register_ranges(fuzzy::Range(p_pos - 1, range_suffixid.second, previous_range_suffixid.second, p_i.size() - 1), min_subseq_length);
          }

          previous_range_suffixid = range_suffixid;
          current_min_suffixid = range_suffixid.first;
          current_max_suffixid = range_suffixid.second;
        }
        else
        {
          p_i.pop_back();
          break;
        }
      }
      if (p_i.size() >= 2)
        nGramMatches.register_ranges(fuzzy::Range(p_pos - 1, previous_range_suffixid.first, previous_range_suffixid.second, p_i.size()), min_subseq_length);
    }
    nGramMatches.process_backlogs();

    /* Consolidation of the results */

    /* now explore for the best segments */

    std::vector<const char*> st(p_length+1);
    std::vector<int> sn(p_length+1);
    Tokens pattern_realtok = (Tokens)real;

    real.get_itoks(st, sn);

    for (auto agendaItemIt = nGramMatches.get_psentences().begin(); agendaItemIt != nGramMatches.get_psentences().end(); ++agendaItemIt)
    {
      auto& agendaItem = agendaItemIt.value();
      int s_id = agendaItem.s_id;
      const auto suffix_wids = nGramMatches.sentence(s_id); //TODO we may have to update this

      /* time to add unigram now */
      /* we just need to add matches when matching free slot in the sentence */
      for (const auto wid : suffix_wids)
      {
        // If this suffix word appears at least once in the pattern
        const auto it = p_unigrams.find(wid);
        if (it != p_unigrams.end())
        {
          // Add coverage of unigrams
          const auto& unigram_list = it->second;
          for (const auto i : unigram_list)
          {
            if (!agendaItem.map_pattern[i]) {
              agendaItem.map_pattern[i] = true;
              agendaItem.coverage++;
            }
          }
        }
      }

      /* do not care checking sentences that do not have enough ngram matches for the fuzzy threshold */
      if (p_length <= agendaItem.coverage + nGramMatches.max_differences_with_pattern)
      {
        Costs costs;
        costs.diff_word = 100. / std::max(suffix_wids.size(), p_length);

        /* let us check the candidates */
        const auto suffix_realtok = _suffixArrayIndex->real_tokens(s_id);
        float cost = _edit_distance(suffix_wids, suffix_realtok,
                                    pattern_wids, pattern_realtok,
                                    p_length, st, sn,
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
          m.max_subseq = agendaItem.maxmatch;
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

