#include "enclave.h"

struct thread_info {
    int tgid; // Thread Group ID
    int pid;  // Process ID
    int is_secure; // Flag to indicate if the thread is secure
};


/****************************
 * Secure Thread Detector
 ****************************/

static int is_secure_thread(enclave_id eid, int tgid, int pid) {
    for (int i = 0; i < MAX_THREADS; i++) {
        if (enclaves[eid].thread_info[i].tgid == tgid && enclaves[eid].thread_info[i].pid == pid) {
            return enclaves[eid].thread_info[i].is_secure;
        }
    }
    return 0; // Default to non-secure if thread not found
}

static void configure_pmp_for_thread(enclave_id eid, int tgid, int pid) {
    if (is_secure_thread(eid, tgid, pid)) {
        for (int memid = 0; memid < MAX_REGIONS; memid++) {
            if (enclaves[eid].regions[memid].type != REGION_INVALID) {
                pmp_set(enclaves[eid].regions[memid].pmp_rid);
            }
        }
    } else {
        for (int memid = 0; memid < MAX_REGIONS; memid++) {
            if (enclaves[eid].regions[memid].type != REGION_INVALID) {
                pmp_set(enclaves[eid].regions[memid].pmp_rid);
            }
        }
    }
}


