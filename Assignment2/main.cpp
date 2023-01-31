#include <iostream>
#include <pthread.h>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>
#include <unordered_map>
#include <unistd.h>

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
    Board board("data/random2.txt");
    int K = board.k;
    int N = board.n;
    pthread_t threadArr[K];
    threadData dataArr[K];

    int rowThreadCount = K / 3;
    int colThreadCount = K / 3;
    int boxThreadCount = K / 3;

    int remainingThreads = K % 3;
    if (remainingThreads == 1)
    {
        rowThreadCount += 1;
    }
    else if (remainingThreads == 2)
    {
        rowThreadCount += 1;
        colThreadCount += 1;
    }

    int iter = 0;
    iter = createThreads(ROW, rowThreadCount, N, iter, dataArr, threadArr, board);
    iter = createThreads(COL, colThreadCount, N, iter, dataArr, threadArr, board);
    iter = createThreads(BOX, boxThreadCount, N, iter, dataArr, threadArr, board);

    for (int i = 0; i < board.k; i++)
    {
        pthread_join(threadArr[i], NULL);
    }

    for (int i = 0; i < board.k; i++)
    {
        if (!dataArr[i].result)
        {
            cout << "Board is not valid" << endl;
            return 0;
        }
    }
    cout << "Board is valid" << endl;

    return 0;
}

int createThreads(Mode mode, int remainingThreadCount, int remTasks, int iter,
                  threadData *dataArr, pthread_t *threadArr, Board board)
{
    int K = board.k;
    int N = board.n;

    while (remainingThreadCount > 0)
    {
        int currentCount = (remainingThreadCount == 1) ? remTasks : 3 * N / K;

        dataArr[iter].start_point = (remTasks == N) ? 0 : dataArr[iter - 1].start_point + dataArr[iter - 1].check_count;
        dataArr[iter].board = Board(board); // create copies of board for each thread
        dataArr[iter].threadId = iter;
        dataArr[iter].check_count = currentCount;

        if (mode == ROW)
        {
            pthread_create(&threadArr[iter], NULL, checkRow, (void *)&dataArr[iter]);
        }
        else if (mode == COL)
        {
            pthread_create(&threadArr[iter], NULL, checkColumn, (void *)&dataArr[iter]);
        }
        else if (mode == BOX)
        {
            pthread_create(&threadArr[iter], NULL, checkBox, (void *)&dataArr[iter]);
        }

        remTasks -= currentCount;
        remainingThreadCount -= 1;
        iter += 1;
    }
    return iter;
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