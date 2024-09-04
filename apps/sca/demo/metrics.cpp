#include "metrics.h"

void increment_bandwidth_ops(int loads, int stores, int bytes_loaded, int bytes_stored) {
    sharedVariable.num_loads += loads;
    sharedVariable.num_stores += stores;
    sharedVariable.bytes_loaded += bytes_loaded;
    sharedVariable.bytes_stored += bytes_stored;
}
void increment_compute_ops(int iops, int flops, int transops) {
    sharedVariable.num_iops += iops;
    sharedVariable.num_flops += flops;
    sharedVariable.num_transops += transops;
}
// 
// create a struct that has the 