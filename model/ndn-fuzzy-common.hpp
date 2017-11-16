#ifndef NDNSIM_NDN_FUZZY_COMMON_H
#define NDNSIM_NDN_FUZZY_COMMON_H

#define NUM_OF_RESULTS 10
#define COMP_INDEX_FUZZY 1
#define THRESHOLD 0.4

struct result {
  char resultValue[100];
};

struct resultFormat {
  int nResultsToReturn;
  result resultsArray[NUM_OF_RESULTS];
};

struct initStruct {
  long long size;
  char* vocab;
  float *M;
  long long words;
  char *bestw[50];
};

#endif // NDN_FUZZY_COMMON_H
