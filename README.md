# FIF-ndnSIM

This is a modified version of ndnSIM for the simulation of Fuzzy Interest
Forwarding (FIF).

To build this version of ndnSIM:

```
mkdir ndnSIM
cd ndnSIM
git clone https://github.com/spirosmastorakis/ns3-dev-FIF ns-3
git clone https://github.com/named-data-ndnSIM/pybindgen.git pybindgen
git clone https://github.com/spirosmastorakis/word2vec
git clone --recursive https://github.com/spirosmastorakis/FIF ns-3/src/ndnSIM
```

To run the simulations presented in the paper:

1) Compile the `extract_words.cpp` and `extract_words_overlap.cpp` source code  
located under the ns-3 directory:

```
g++ -ldistance extract_words.cpp -o extract_words
g++ -ldistance extract_words_overlap.cpp -o extract_words_overlap
```

2) Run the `extract_words_overlap.sh` (for the evaluation of the
Forward or Wait Challenge) and `extract_words.sh` (for all the other experiments)
scripts located under the ns-3 directory with the appropriate parameters.

Take a look at `extract_words.cpp` and `extract_words_overlap.cpp` for usage
information.
