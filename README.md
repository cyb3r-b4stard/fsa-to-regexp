# fsa-to-regexp
Translator from FSA to Regular Expression
# Input file format
states=[s1,s2,...]	// s1 , s2, ... ∈ latin letters, words and numbers
alpha=[a1,a2,...]	// a1 , a2, ... ∈ latin letters, words, numbers and character '_’(underscore)
initial=[s]	// s ∈ states
accepting=[s1,s2,...]	// s1, s2 ∈ states
trans=[s1>a>s2,...]	// s1,s2,...∈ states; a ∈ alpha
