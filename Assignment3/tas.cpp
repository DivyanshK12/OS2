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

static chrono::milliseconds averageTime = chrono::milliseconds(0);
static chrono::milliseconds worstTime = chrono::milliseconds(0);
static int counts = 0;

int main(void)
{
    int n, k, l1, l2;
    FILE *fp = fopen("inp-params.txt", "r");
    fscanf(fp, "%d %d %d %d", &n, &k, &l1, &l2);
    fclose(fp);

    pthread_t threads[n];
    thread_args args[n];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<> exp1((float)1/l1);
    std::exponential_distribution<> exp2((float)1/l2);

    for (int i = 0; i < n; i++)
    {
        args[i].id = i;
        args[i].k = k;
        args[i].t1 = exp1(gen);
        args[i].t2 = exp2(gen);
        args[i].n = n;
        // cout << "Thread " << i << " t1: " << args[i].t1 << " t2: " << args[i].t2 << endl;
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

    cout << "Average time: " << averageTime.count() / counts << endl;
    cout << "Worst time: " << worstTime.count() << endl;
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
        auto requested = getSysTime();
        auto start = chrono::high_resolution_clock::now();

        while(atomic_flag_test_and_set(&lock));
        
        //cout << "Thread " << id << " requested CS at " << requested << endl;

        auto actual = getSysTime();
        auto end = chrono::high_resolution_clock::now();
        //cout << "Thread " << id << " entered CS at " << actual << endl;

        usleep(t1*1000); // critical section

        auto exit = getSysTime();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        
        worstTime = max(worstTime, chrono::milliseconds(duration));
        averageTime += chrono::milliseconds(duration);
        counts++;

        lock.clear();

        usleep(t2*1000); // remainder section
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