# fsa-to-regexp
Translator from FSA to Regular Expression
# Input file format
states=[s1,s2,...]	// s1 , s2, ... ∈ latin letters, words and numbers<br/>
alpha=[a1,a2,...]	// a1 , a2, ... ∈ latin letters, words, numbers and character '_’(underscore)<br/>
initial=[s]	// s ∈ states<br/>
accepting=[s1,s2,...]	// s1, s2 ∈ states<br/>
trans=[s1>a>s2,...]	// s1,s2,...∈ states; a ∈ alpha<br/>
