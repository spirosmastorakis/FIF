#ifndef NDNSIM_NDN_FUZZY_COMMON_H
#define NDNSIM_NDN_FUZZY_COMMON_H

struct result {
  char resultValue[100];
};

struct resultFormat {
  int nResultsToReturn;
  result resultsArray[5];
};

struct initStruct {
  long long size;
  char* vocab;
  float *M;
  long long words;
  char *bestw[50];
};

#define NUM_OF_RESULTS 5
#define COMP_INDEX_FUZZY 1

#endif // NDN_FUZZY_COMMON_H
