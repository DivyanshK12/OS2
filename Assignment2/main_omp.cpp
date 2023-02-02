#include <iostream>
#include <omp.h>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>
#include <unordered_map>
#include <chrono>

using namespace std;

enum Mode
{
    ROW,
    COL,
    BOX
};

class Board
{

public:
    int n;
    int k;
    vector<vector<int>> data;

    Board()
    {
        k = 0;
        n = 0;
        data = vector<vector<int>>(n, vector<int>(n));
    }

    Board(string filename) // default constructor
    {
        ifstream file(filename);
        file >> k >> n;
        data = vector<vector<int>>(n, vector<int>(n));
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                file >> data[i][j];
            }
        }
    }

    Board(const Board &b) // copy constructor
    {
        k = b.k;
        n = b.n;
        data = vector<vector<int>>(n, vector<int>(n));
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                data[i][j] = b.data[i][j];
            }
        }
    }

    void print()
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                cout << data[i][j] << " ";
            }
            cout << endl;
        }
    }

    void repr()
    {
        cout << "k = " << k << ", n = " << n << endl;
        cout << &data << endl;
    }
};

typedef struct threadData
{
    int threadId;
    Board board;
    int start_point;
    int check_count;
    bool result;
} threadData;

void *checkRow(void *arg);
void *checkColumn(void *arg);
void *checkBox(void *arg);
int createThreads(Mode mode, int remainingThreadCount, int remTasks, int iter,
                  threadData *dataArr, pthread_t *threadArr, Board board);

int main()
{
    Board board("data/random1.txt");
    int K = board.k;
    int N = board.n;
    threadData dataArr[N];

    for (int iter = 0; iter < N; iter++)
    {
        dataArr[iter].start_point = (iter == 0) ? 0 : dataArr[iter - 1].start_point + dataArr[iter - 1].check_count;
        dataArr[iter].board = Board(board); // create copies of board for each thread
        dataArr[iter].check_count = 1;
        dataArr[iter].result = false;
    }

    omp_set_dynamic(false);
    omp_set_num_threads(K);
    auto start = chrono::high_resolution_clock::now();
#pragma omp parallel for shared(dataArr)
    for (int i = 0; i < N; i++)
    {
        int tid = omp_get_thread_num(); // is now working
        dataArr[i].threadId = tid;
        checkBox((void *)&dataArr[i]);
        checkColumn((void *)&dataArr[i]);
        checkRow((void *)&dataArr[i]);
    }

#pragma omp barrier
    auto end = chrono::high_resolution_clock::now();
    cout << "Time taken: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " microseconds" << endl;
    for (int i = 0; i < board.n; i++)
    {
        if (!dataArr[i].result)
        {
            cout << "Board is not valid" << endl;
            return 0;
        }
        cout << "Result of i" << i << ": " << dataArr[i].result << endl;
    }
    cout << "Board is valid" << endl;
    return 0;
}

void *checkRow(void *arg)
{
    threadData *data = (threadData *)arg;
    Board board = data->board;
    int start_point = data->start_point;
    int threadId = data->threadId;
    int check_count = data->check_count;
    unordered_map<int, bool> nums_seen; // key = number, value = seen

    for (int i = 0; i < check_count; i++)
    {
        for (int j = 0; j < board.n; j++)
        {
            int num = board.data[start_point + i][j];
            if (nums_seen.find(num) != nums_seen.end())
            {
                data->result = false;
                cout << "Thread " << threadId << " checking row " << start_point << endl;
                return NULL; // can call pthread_exit() here ?
            }
            else
            {
                nums_seen[num] = true;
            }
        }
        nums_seen.clear();
    }
    data->result = true;
    return NULL;
}

void *checkColumn(void *arg)
{
    threadData *data = (threadData *)arg;
    Board board = data->board;
    int start_point = data->start_point;
    int threadId = data->threadId;
    int check_count = data->check_count;
    unordered_map<int, bool> nums_seen; // key = number, value = seen
    for (int i = 0; i < check_count; i++)
    {
        for (int j = 0; j < board.n; j++)
        {
            int num = board.data[j][start_point + i];
            if (nums_seen.find(num) != nums_seen.end())
            {
                data->result = false;
                cout << "Thread " << threadId << " checking column " << start_point << endl;
                return NULL; // can call pthread_exit() here ?
            }
            else
            {
                nums_seen[num] = true;
            }
        }
        nums_seen.clear();
    }
    data->result = true;
    return NULL;
}

void *checkBox(void *arg)
{
    threadData *data = (threadData *)arg;
    Board board = data->board;
    int start_point = data->start_point;
    int threadId = data->threadId;
    int check_count = data->check_count;
    unordered_map<int, bool> nums_seen; // key = number, value = seen
    int sqrtn = sqrt(board.n);

    for (int i = 0; i < check_count; i++)
    {
        int startRow = ((start_point + i) / sqrtn) * sqrtn;
        int startCol = ((start_point + i) % sqrtn) * sqrtn;
        for (int j = 0; j < sqrtn; j++)
        {
            for (int k = 0; k < sqrtn; k++)
            {
                int num = board.data[startRow + j][startCol + k];
                if (nums_seen.find(num) != nums_seen.end())
                {
                    data->result = false;
                    cout << "Thread " << threadId << " checking box " << start_point << endl;
                    return NULL; // can call pthread_exit() here ?
                }
                else
                {
                    nums_seen[num] = true;
                }
            }
        }
        nums_seen.clear();
    }
    data->result = true;
    return NULL;
}