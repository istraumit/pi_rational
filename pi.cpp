#include <iostream>
#include <thread>
#include <list>
#include <mutex>

#include <mpfr.h>
// Multiple Precision Floating-Point Reliable Library
// To compile: g++ pi.cpp -o pi -lmpfr

static std::mutex mtx;

void do_stuff(int th_id, int n_th){
    mpfr_rnd_t ROUND = MPFR_RNDN;
    long unsigned int i = th_id;

    mpfr_set_default_prec(512);
    mpfr_t pi,q,div,resid,best_resid;
    mpfr_init(pi);
    mpfr_const_pi(pi, ROUND);
    mpfr_init(q);
    mpfr_init(div);
    mpfr_init(resid);
    mpfr_init(best_resid);
    mpfr_set_d(best_resid, 2.0, ROUND);

    while(1){
        i += n_th;
        mpfr_ui_div(q, i, pi, ROUND);
        mpfr_round(div, q);
        mpfr_ui_div(resid, i, div, ROUND);
        mpfr_sub(resid, resid, pi, ROUND);
        mpfr_abs(resid, resid, ROUND);
        if(mpfr_cmp(best_resid, resid)>0){
            mpfr_set(best_resid, resid, ROUND);
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "|Pi - " << i << "/" << mpfr_get_ui(div, ROUND)<<"| = ";
            mpfr_printf("%.100RNf\n", resid);
        }
    }

}

int main(int argc, char* argv[]){
    if(argc < 2){
        std::cout << "This code searches for rational approximations of Pi" << std::endl;
        std::cout << "Use: " << argv[0] << " <N_threads>" << std::endl;
        return 0;
    }

    int n_threads = std::stoi(argv[1]);
    int cores = std::thread::hardware_concurrency();
    if(n_threads > cores){
        std::string resp;
        std::cout << "The number of threads you requested seems to exceed the number" << std::endl;
        std::cout << "of cores in the processor ("<< cores <<"). Are you sure you want that? [y/n]:";
        std::cin >> resp;
        if(resp[0]!='Y' and resp[0]!='y') return 0;
    }

    std::list<std::thread> threads;

    for(int i=0; i<n_threads; i++)
        threads.push_front(std::thread(do_stuff, i+1, n_threads));

    for(std::thread& th:threads) th.join();

    return 0;
}
