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

atomic_bool lock = false;

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

    bool expected = false;
    for (int i = 0; i < k; i++)// replacement for while(true)
    {
        auto requested = getSysTime();
        
        while(!lock.compare_exchange_strong(expected, true))
            expected = false; // do nothing)
        
        cout << "Thread " << id << " requested CS at " << requested << endl;

        auto actual = getSysTime();
        cout << "Thread " << id << " entered CS at " << actual << endl;

        sleep(t1); // critical section

        auto exit = getSysTime();
        cout << "Thread " << id << " exited CS at " << exit << endl;

        lock = false;

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