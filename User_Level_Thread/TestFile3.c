// Project 2: User level thread library
// Test file
// DO NOT MODIFY

#include "functions.c"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Usage: %s [FCFS] | [RR] | [vLOT] | [mLOT]\n", argv[0]);
        return 1;
    } else if (strcmp(argv[1], "FCFS") == 0) {
        setup(FCFS); // First Come, First Served
    } else if (strcmp(argv[1], "RR") == 0) {
        setup(RR); // Round Robin
    } else if (strcmp(argv[1], "vLOT") == 0) {
        setup(vLOT); // Vanilla Lottery
    } else if (strcmp(argv[1], "mLOT") == 0) {
        setup(mLOT); // Modified Lottery
    } else {
        printf("Usage: %s [FCFS] | [RR] | [vLOT] | [mLOT]\n", argv[0]);
        return 1;
    }

    CreateThread(clean_up_thread, 10); // 0
    CreateThread(clean_up_thread, 1); // 1
    CreateThread(suspend_test, 20); // 2
    CreateThread(clean_up_thread, 5); // 3
    CreateThread(clean_up_thread, 10); // 4

    Go();

    return 0;
}
