#include <chrono>
#include <iostream>
#include <pthread.h>
#include <atomic>
#include <unistd.h>
#include <random>

using namespace std;

typedef struct
{
    int id;
    int n;
    int k;
    int t1;
    int t2;
} thread_args;

void* testCS(void* args);
string getSysTime();

std::atomic_flag lock = ATOMIC_FLAG_INIT;

int main(void)
{
    int n, k, l1, l2; // read n from file
    FILE *fp = fopen("input.txt", "r");
    fscanf(fp, "%d %d %d %d", &n, &k, &l1, &l2);
    fclose(fp);

    pthread_t threads[n];
    thread_args args[n];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<double> exp1(l1);
    std::exponential_distribution<double> exp2(l2);

    for (int i = 0; i < n; i++)
    {
        args[i].id = i;
        args[i].k = k;
        args[i].t1 = exp1(gen);
        args[i].t2 = exp2(gen);// need to generate t1 and t2 from 
        // exponentially distributed with an average of λ1, λ2 seconds.
    }

    for (int i = 0; i < n; i++)
    {
        pthread_create(&threads[i], NULL, testCS, (void*)&args[i]);
    }

    // join
    for (int i = 0; i < n; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

void* testCS(void* args)
{
    thread_args* targs = (thread_args*)args;
    int id = targs->id;
    int n = targs->n;
    int k = targs->k;
    int t1 = targs->t1;
    int t2 = targs->t2;

    for (int i = 0; i < k; i++)// replacement for while(true)
    {
        auto requested = std::chrono::system_clock::now();
        auto reqEnterTime = std::chrono::system_clock::to_time_t(requested);

        while(atomic_flag_test_and_set(&lock)); // do nothing
        
        cout << "Thread " << id << " requested CS at " << getSysTime() << endl;

        auto actual = std::chrono::system_clock::now();
        auto actEnterTime = std::chrono::system_clock::to_time_t(actual);
        cout << "Thread " << id << " entered CS at " << getSysTime() << endl;

        sleep(t1); // critical section
        // will calculate the worst time and average time here ?

        auto exit = std::chrono::system_clock::now();
        auto exitTime = std::chrono::system_clock::to_time_t(exit);
        cout << "Thread " << id << " exited CS at " << getSysTime() << endl;

        lock.clear();

        sleep(t2); // remainder section
    }

    return NULL;
}

string getSysTime()
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    string time = to_string(ltm->tm_min) + ":" + to_string(ltm->tm_sec);
    return time;
}