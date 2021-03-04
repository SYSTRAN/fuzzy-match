# Using the FuzzyMatch-cli

`FuzzyMatch-cli` is a commandline utility allowing to compile FuzzyMatch indexes and use them to lookup fuzzy matches.

## Compiling a Fuzzy match index

Simplest command is the following:
```
FuzzyMatch-cli -c CORPUS [--penalty_tokens (none|tag,sep/jnr,pct)]
```

* `CORPUS` can be a single file - in which case, the index of each segment is simply the sentence id - or you can provide a target file using `-c CORPUSSOURCE,CORPUSTARGET` and add option `--add-target` to include in the index the actual target sentence (format ID=target). This is useful for having the index fully containing the translation memory. Not useful, if the translation memory is saved in side database.
* `--penalty_tokens` (default `tag,cas,nbr`) is either `none` or comma-separated list of `tag`, `sep`, `jnr`, `pct`, `nbr`, `cas` modifying normalization (for `cas` performing case normalization, and `nbr` triggering number normalization), removing some tokens from index (`tag` for tags, and `pct` for punctuations), or generates spacer/joiner (`sep`/`jnr`). In each case, a penalty tokens is added.

This option used in index forces the same logic in matching.

The above command generates a file `CORPUS.fmi`.

## Fuzzy Lookup

```
FuzzyMatch-cli -i CORPUS.fmi -a match -f FUZZY -N NTHREAD -n NMATCH [--ml ML] [--mr MR] --idf-penalty IDFPENALTYRATIO < INPUTFILE > MATCHES 
```

* `CORPUS.fmi` path to the complete generated index file
* `FUZZY`, the fuzzy threshold in [0,1]. Not really relevant < 0.5.
* `NTHREAD` number of thread to use - default 4. Scales well with the number of threads.
* `NMATCH` number of match to return
* `ML` minimal length of the longest subsequence (in tokens) - defaut 3. If the pattern size is strictly less than `ML`, then this parameter is ignored.
* `MR` minimal ratio of the longest subsequence (in tokens) - default 0. Interesting to use for lowest fuzzy - for instance a value of 0.5, used with fuzzy threshold 0.5, will guarantee the presence of at least 50% of the sentence length
* `IDFPENALTYRATIO` if not null, gives extra penalty to word missing weighted on IDF: a value of 1 is equivalent to give a penalty of one additional missing word for a word appearing only once in all the translation memory.

Add `--no-perfect` (`-P`) to discard perfect matches. For instance the following command returns one fuzzy match higher than 0.7 but not perfect:

```
FuzzyMatch-cli -l en -i CORPUS.fmi -a match -f 0.7 -N 4 -n 1 -P < INPUTFILE
```

*To prevent FuzzyMatch from freezing there is a limit on how long the pattern can be, the limit is 300 tokens now.*
*This is necessary because the edit distance computation runs in O(T^2) where T is the number of tokens in the pattern.*

# The Algorithm
## Tokenization and Vocabulary Indexing

All the sentences stored in the Fuzzy Match index are considered as sequences of index - where the index is coming from a vocabulary list as defined in `VocabIndexer`.

It is therefore important to tokenize and normalize the sentences stored. Default integrated tokenization is OpenNMT tokenization:

```
onmt::Tokenizer tokenizer(onmt::Tokenizer::Mode::Aggressive,
                          onmt::Tokenizer::Flags::CaseFeature
                          | onmt::Tokenizer::Flags::SegmentAlphabetChange
                          | onmt::Tokenizer::Flags::NoSubstitution
                          | onmt::Tokenizer::Flags::SupportPriorJoiners,
                          "",
                          "");
tokenizer.add_alphabet_to_segment("Han");
tokenizer.add_alphabet_to_segment("Kanbun");
tokenizer.add_alphabet_to_segment("Katakana");
tokenizer.add_alphabet_to_segment("Hiragana");
```

Also - the tokens are normalized:
* NFC encoding
* lowercase (if `cas` penalty-token is activated)
* ID and values of placeholders are removed, and all `it_...` placeholders normalized by `｟it｠`. These placeholders are removed from index if `tag` penalty-token is activated
* numbers replaced by `｟ent_num｠` (if `nbr` penalty-token is activated)
* punctuations are removed from index if `pct` penalty-token is activated

It is possible to pre-tokenize and pre-normalize with other algorithms for possible special processing.

The actual tokenization is not important but needs to be consistent for indexation and lookup.

Also, the more generous is the "normalization", the better the fuzzy matcher will be able to match variants of the same sentence.

Fuzzy matching is performed on normalized forms, but final score is calculated on real forms with potential penalties.


## Suffix Arrays and Fuzzy Matching

The base structure used for fuzzy matching index is a suffix array. This structure is implemented in `suffix-array.hh`and works as following: each sentence can be seen as a sequence of suffix. For instance the sentence `A B C A D` is containing suffixes `A B C A D`, `B C A D`, `C A D`, `A D` and `D`. In a suffix array the suffix are sorted by lexicographical order and suffixes are simply indicated as their position in the sentence.

The suffix array corresponding to the sentences 0: `A B C A D` and 1: `D A B A` is:

| Suffix ID | SentenceID,Position | Equivalent Suffix |
| --------- | ------------------- | ----------------- |
| 0 | 1,3 | `A` |
| 1 | 1,1 | `A B A` |
| 2 | 0,0 | `A B C A D` |
| 3 | 0,3 | `A D` |
| 4 | 1,2 | `B A` |
| 5 | 0,1 | `B C A D` |
| 6 | 0,2 | `C A D` |
| 7 | 0,4 | `D` |
| 8 | 1,0 | `D A B A` |

What needs to be saved is only pair of (sentence id, position), and the actual sentences. This is implemented in `SuffixArray` class through:

```cpp
struct SuffixView
{
  unsigned sentence_id;
  unsigned short subsentence_pos;
};

...

// ordered sequence of sentence id, pos in sentence
std::vector<SuffixView>       _suffixes;
// the concatenated sentences, as 0-terminated sequences of vocab
std::vector<unsigned>         _sentence_buffer;
// sentence id > position in sentence buffer
std::vector<unsigned>         _sentence_pos;
/* index first word in _sentences */
std::vector<unsigned>         _quickVocabAccess;
```

Suffix arrays have the nice property of instantly locating sub-matches from a pattern. For instance when searching common ngrams with the pattern `A B A D` - we can simply iterate at each position of the pattern and perform *narrowing* lookup in the suffix array:

1st position:
* `A` finds the matching range [0, 3] in the suffix array
* `A B` we can narrow previous range, and find new range [1, 2]
* `A B A` - 0 result

2nd position:
* `B` => range [4, 5]
* `B A` => range [4, 4]
* `B A D` - 0 result

3nd position:
* `A` => range [0, 3]
* `A D` => range [3, 3]

4th position:
* `D` => range [7, 8]

The algorithm implemented in `fuzzy-match.cc` is disregarding the unigram to avoid matching all single sentences - but then build NGramMatch for each sentence by checking longest match - in the previous case, we are keeping:

Sentence 1: `A B`, `A D`
Sentence 2: `A B A`

These occurrences might overlap, so a second step in the process is building a `pattern_map` matching all tokens covered by the ngrams while completing with unigrams.

During the match process, we can restrict the candidate sentences by looking at their length. Indeed a 70% fuzzy on a 100 tokens match can not match sentence shorter than 70 tokens or longer than 130 tokens.

Last phase of the fuzzy match is to actually perform a standard edit distance between the *unnormalized* tokens to obtain actual fuzzy match.

Following rules apply to calculate the actual fuzzy match when looking for a specific pattern:
* penalty token as defined when building index `｟it｠`, punctuations are not taken into account in the real pattern length: hence `aa ｟it｠ bb cc dd` length is 4 tokens. Separators are never taken into account.
* modification, insertion, and substitution costs in edit distance are 100/real_pattern_length.
* cost of _penalty tokens_ is `cost_penalty=1`
* cost of case difference is `cost_diff_case=1`
* cost of any other difference in real forms is `cost_diff_real=2` (for instance number)

### Note on Penalties

Fuzzy-Matching score is not formally defined - and the above-mentionned rules are a reasonnable choice that can be however different than for another implementation:

* https://www.tm-town.com/fr/blog/the-fuzziness-of-fuzzy-matches
* https://wordfast.com/WFP2/Managing_projects/Defining_penalties.htm

### Suffix Arrays efficiency

Searching for a n-gram in a collection of N sentences, with average length *m*, is only costing *O(log(Nm))* since the suffix array is sorted. So the approximate number of operations required to find for all of the n-gram in a sentence of sentence *m* is only *O(m.log(Nm))*.

Note that this very nice property of the Suffix Array representation is balanced by an important cost when computing the Suffix Array. Adding a single sentence needs to insert suffixes position inside the array. Today the structure `SuffixArray` is not dynamic: it is necessary to sort completely the suffix array every time a new sentence is added, and it is not possible to remove a sentence from the index.
This could be changed, and we could dynamically insert suffixes where they belong, and also _remove_ a given sentence by searching for all its suffixes. However cost to add or remove a sentence is *O(N)*. 

### Serialization/Deserialization of Fuzzy Match Index

Serialization and Deserialization of Fuzzy Match Index classes in file is performed using functions in `include/fuzzy/fuzzy_matcher_binarization.hh`:

```
  void export_binarized_fuzzy_matcher(const std::string& binarized_tm_filename, const FuzzyMatch& fuzzy_matcher);
  void import_binarized_fuzzy_matcher(const std::string& binarized_tm_filename, FuzzyMatch& fuzzy_matcher);
```

These functions are saving/loading fuzzy_matcher instance into the indicated file - adding a signature to the file corresponding to `FMIv` where `v` is the version of the class:

```
  const char FuzzyMatch::version = '1';
```

Note that the version of the class is necessary for checking version compatibility. The serialization format may also change - and in that case is indicated with `BOOST_CLASS_VERSION` macro in `fuzzy_match.hxx`.

# Development

## Dependencies

+ [OpenNMT Tokenizer](https://github.com/OpenNMT/Tokenizer)
+ [ICU](http://site.icu-project.org/)
+ [Boost](https://www.boost.org/) (we use `serialization` and `archive` to load/save fuzzy match indexes)
+ [Google Test](https://github.com/google/googletest) (is required only to compile tests)
+ [tsl::hopscotch_map](https://github.com/Tessil/hopscotch-map) (provided with the project)

## Compiling
CMake and a compiler that supports the C++11 standard are required to compile the project.

```
mkdir build
cd build
cmake ..
make
```

Custom library locations can be provided with the following cmake variables:
+ `OPENNMT_TOKENIZER_ROOT` for **OpenNMT_Tokenize** path.
+ `CMAKE_PREFIX_PATH` for **ICU** path, _prefer this over `ICU_ROOT` as it will properly use your path over system paths_.
+ `BOOST_ROOT` for **Boost** path.
+ `GTEST_ROOT` for **Google Test** path.

By default you compile the library + CLI (Command Line Interface) + tests.
+ To compile only the CLI (without the tests), use the `-DCLI_ONLY=ON` flag.
+ To compile only the library (without the tests, without the CLI), use the `-DLIB_ONLY=ON` flag.

## Testing

Compile test indexes
```
./cli/src/FuzzyMatch-cli -c ../test/data/tm1,../test/data/tm1 --add-target 1
./cli/src/FuzzyMatch-cli -c ../test/data/tm1,../test/data/tm1 --add-target 1
./cli/src/FuzzyMatch-cli -c ../test/data/tm2.en.gz,../test/data/tm2.fr.gz --add-target
```

Run tests:
```
./test/FuzzyMatch-test ../test/data/
```

Rebuild reference (expected) tm2 test set (!!! be careful, you are changing reference !!!):
```
cat ../test/data/test-tm2.en | ./cli/src/FuzzyMatch-cli -i ../test/data/tm2.en.gz.fmi -f 0.5 -a match --no-perfect -n 2 > test-tm2.matches
paste ../test/data/test-tm2.en test-tm2.matches | perl -pe 's/\t/\t0.5\ttrue\t2\t/;s/^/TEST.($c++)."\t"/e' > ../test/data/test-tm2
```
