#include <iostream>
#include <pthread.h>
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
}; // Enum to use for different modes during thread creation

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
    } // default constructor

    Board(string filename) // constructor to read from file
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

    void print() // print the content of the sudoku board
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

    void repr() // print representation of the sudoku board
    {
        cout << "k = " << k << ", n = " << n << endl;
        cout << &data << endl; // useful to ensure each thread has its own copy of the board data
        // without which there is issue of locking from the vector data structure
    }
};

typedef struct threadData
{
    int threadId;
    Board board;
    int start_point;
    int check_count;
    Mode mode;
    bool result; // can be result of any of the three checks
    vector<bool> isValid;
} threadData; // struct to store data for each thread

void *checkRow(void *arg);    // function to check rows based on information passed from threadData
void *checkColumn(void *arg); // function to check columns based on information passed from threadData
void *checkBox(void *arg);    // function to check boxes based on information passed from threadData
int createThreads(Mode mode, int remainingThreadCount, int remTasks, int iter,
                  threadData *dataArr, pthread_t *threadArr, Board board);

int main()
{
    Board board("input.txt");
    int K = board.k;
    int N = board.n;
    pthread_t threadArr[K]; // array of threads
    threadData dataArr[K];  // array of threadData to store data for each thread

    int rowThreadCount = K / 3;
    int colThreadCount = K / 3;
    int boxThreadCount = K / 3;

    int remainingThreads = K % 3;
    if (remainingThreads == 1) // if there are 1 or 2 threads remaining, add them to the row or column check
    {
        rowThreadCount += 1;
    }
    else if (remainingThreads == 2)
    {
        rowThreadCount += 1;
        colThreadCount += 1;
    }

    int iter = 0;
    auto start = chrono::high_resolution_clock::now();                             // start timer
    iter = createThreads(ROW, rowThreadCount, N, iter, dataArr, threadArr, board); // create threads for row check
    iter = createThreads(COL, colThreadCount, N, iter, dataArr, threadArr, board); // create threads for column check
    iter = createThreads(BOX, boxThreadCount, N, iter, dataArr, threadArr, board); // create threads for box check

    for (int i = 0; i < board.k; i++)
    {
        pthread_join(threadArr[i], NULL); // wait for all threads to finish
    }

    auto stop = chrono::high_resolution_clock::now();                          // stop timer
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start); // calculate time taken

    // file output
    ofstream file("output.txt");
    for (int i = 0; i < board.k; i++)
    {
        for (int j = dataArr[i].start_point; j < dataArr[i].start_point + dataArr[i].check_count; j++)
        {
            file << "Thread " << dataArr[i].threadId + 1 << " checks "; // print thread id
            if (dataArr[i].mode == ROW)
            {
                file << "row ";
            }
            else if (dataArr[i].mode == COL)
            {
                file << "column ";
            }
            else if (dataArr[i].mode == BOX)
            {
                file << "box ";
            }

            file << j + 1 << " and is " << (dataArr[i].isValid[j - dataArr[i].start_point] ? "valid" : "invalid") << endl;
        }
    }

    file << "Time taken : " << duration.count() << " microseconds" << endl; // print time taken
    for (int i = 0; i < board.k; i++)
    {
        if (!dataArr[i].result)
        {
            file << "Board is not valid" << endl; // if any of the threads return false, board is invalid
            return 0;
        }
    }
    file << "Board is valid" << endl; // if all threads return true, board is valid
    file.close();
    return 0;
}

int createThreads(Mode mode, int remainingThreadCount, int remTasks, int iter,
                  threadData *dataArr, pthread_t *threadArr, Board board)
{
    int K = board.k;
    int N = board.n;

    while (remainingThreadCount > 0) // run the loop for each thread allocated to the current MODE
    {
        int currentCount = (remainingThreadCount == 1) ? remTasks : 3 * N / K;
        // calculate number of checks for each thread
        // if there is only one thread remaining, give it all the remaining tasks

        dataArr[iter].start_point = (remTasks == N) ? 0 : dataArr[iter - 1].start_point + dataArr[iter - 1].check_count;
        // calculate the starting point for each thread, based on the number of checks for the previous thread or 0 for the first
        dataArr[iter].board = Board(board); // create copies of board for each thread
        dataArr[iter].threadId = iter;
        dataArr[iter].mode = mode;
        dataArr[iter].check_count = currentCount;
        dataArr[iter].isValid = vector<bool>(currentCount, true);

        // create thread with appropriate function based on the mode
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

        remTasks -= currentCount;  // update remaining tasks
        remainingThreadCount -= 1; // update remaining threads
        iter += 1;                 // update thread id
    }
    return iter;
}
void *checkRow(void *arg)
{
    threadData *data = (threadData *)arg;
    Board board = data->board;
    int start_point = data->start_point;
    int threadId = data->threadId;
    int check_count = data->check_count; // read data from threadData

    unordered_map<int, bool> nums_seen; // key = number, value = seen

    for (int i = 0; i < check_count; i++)
    {
        for (int j = 0; j < board.n; j++) // for all n elements of row
        {
            int num = board.data[start_point + i][j];   // get number
            if (nums_seen.find(num) != nums_seen.end()) // if number is already seen
            {
                data->isValid[i] = false;
                data->result = false;
                // return NULL; // uncomment this to return early
            }
            else
            {
                nums_seen[num] = true;
            }
        }
        nums_seen.clear(); // clear for each row check
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
    int check_count = data->check_count; // read data from threadData
    bool result = true;

    unordered_map<int, bool> nums_seen; // key = number, value = seen
    for (int i = 0; i < check_count; i++)
    {
        for (int j = 0; j < board.n; j++) // for all n elements of column
        {
            int num = board.data[j][start_point + i];   // get number
            if (nums_seen.find(num) != nums_seen.end()) // if number is already seen
            {
                data->isValid[i] = false;
                result = false;
                // return NULL; // uncomment this to return early
            }
            else
            {
                nums_seen[num] = true;
            }
        }
        nums_seen.clear(); // clear for each column check
    }
    data->result = result;
    return NULL;
}

void *checkBox(void *arg)
{
    threadData *data = (threadData *)arg;
    Board board = data->board;
    int start_point = data->start_point;
    int threadId = data->threadId;
    int check_count = data->check_count; // read data from threadData
    bool result = true;

    unordered_map<int, bool> nums_seen; // key = number, value = seen
    int sqrtn = sqrt(board.n);          // sqrt(n)

    for (int i = 0; i < check_count; i++)
    {
        int startRow = ((start_point + i) / sqrtn) * sqrtn; // calculate starting row and column of box
        int startCol = ((start_point + i) % sqrtn) * sqrtn; // based on the current box number
        for (int j = 0; j < sqrtn; j++)
        {
            for (int k = 0; k < sqrtn; k++)
            {
                int num = board.data[startRow + j][startCol + k]; // get number
                if (nums_seen.find(num) != nums_seen.end())       // if number is already seen
                {
                    data->isValid[i] = false;
                    result = false;
                    // return NULL; // uncomment this to return early
                }
                else
                {
                    nums_seen[num] = true;
                }
            }
        }
        nums_seen.clear(); // clear for each box check
    }
    data->result = result;
    return NULL;
}