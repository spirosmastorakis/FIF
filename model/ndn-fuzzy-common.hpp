#ifndef NDNSIM_NDN_FUZZY_COMMON_H
#define NDNSIM_NDN_FUZZY_COMMON_H

#define NUM_OF_RESULTS 5
#define COMP_INDEX_FUZZY 1
#define THRESHOLD 0.6
#define CS_LOOKUPS 100
#define TOTAL_BUDGET 300
#define FIB_LOOKUPS 100

struct result {
  char resultValue[100];
};

struct resultFormat {
  int nResultsToReturn;
  result resultsArray[NUM_OF_RESULTS];
  float similarity[NUM_OF_RESULTS];
};

struct initStruct {
  long long size;
  char* vocab;
  float *M;
  long long words;
  char *bestw[50];
};

#endif // NDN_FUZZY_COMMON_H
