#include "stdio.h"
struct Metrics {
    int num_stores;
    int num_loads;
    int bytes_stored;
    int bytes_loaded;
};
extern int sharedVariable;
void update_metrics() {
    sharedVariable++;
}
// 
// create a struct that has the 