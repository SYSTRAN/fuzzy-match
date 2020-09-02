* code clean-up and documentation
* implement IDF ngram selection in subseq
* fix min_seq_length parameter in subseq not respected
* add IDF-weight penalty in score
* more efficient vocab
* simplify code - remove vocab from SuffixArray
* FuzzyMatch-cli displays number of matches / number of input
* add `cas` penalty token (default) - allowing to enable/disable case normalization
* add `nbr` penalty token (default) - allowing to enable/disable number normalization
* fix empty "perfect" and "no-perfect" subseq matches for 1-length string 
* better implementation of subseq matching
* ignore fuzzy thresholds for subseq matching
* introduce max subseq matching
* remove remaining dependencies
* simpler serialization format for fast uncompressed index reading
* code simplification
* do not require extra dependency on API file
* implement support of joiner as an alternative penalty token
* fix performance lost with adding penalty token logic
* skip empty segments during indexing
* add min subsequence length and ratio (`--ml`, `--mr`) options for lookup
* generalize penalty for tags, separators and punctuations optionally selected with `--penalty_tokens` option
* totally remove tags from actual index, only consider them for penalty
* implement special fuzzy match penalties for tag differences, entity differences, and case differences
* make fuzzy match score precision 0.01% to avoid odd fuzzy rates
* sort equivalent fuzzy matches by id
* simplify code