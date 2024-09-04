#ifndef METRICS_H
#define METRICS_H
#include "stdio.h"
struct Metrics {
    int num_stores;
    int num_loads;
    int bytes_stored;
    int bytes_loaded;
    int num_iops;
    int num_flops;
    int num_transops;
};

extern Metrics sharedVariable;
extern int sharedLoads;
void increment_bandwidth_ops(int loads, int stores, int bytes_loaded, int bytes_stored);
void increment_compute_ops(int iops, int flops, int transops);

#endif