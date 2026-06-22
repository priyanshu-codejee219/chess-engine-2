#include "UCI.hpp"

extern void run_perft_benchmark(int depth);

int main() {
    //run_perft_benchmark(5);
    
    uci_loop();
    return 0;
}