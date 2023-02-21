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

    Board() // default constructor
    {
        k = 0;
        n = 0;
        data = vector<vector<int>>(n, vector<int>(n));
    }

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

    void print() // print the board
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
        cout << &data << endl; // print the address of the data vector
    }
};

typedef struct threadData
{
    int threadId;
    Board board;
    int start_point;
    int check_count;
    vector<bool> results;
} threadData; // struct to store data for each thread

void *checkRow(void *arg);
void *checkColumn(void *arg);
void *checkBox(void *arg);
int createThreads(Mode mode, int remainingThreadCount, int remTasks, int iter,
                  threadData *dataArr, pthread_t *threadArr, Board board);

string valid_output_helper(bool result)
{
    return result ? "is valid" : "is invalid";
} // helper function to print valid/invalid

int main()
{
    Board board("input.txt");
    int K = board.k;
    int N = board.n;
    threadData dataArr[N];

    for (int iter = 0; iter < N; iter++)
    {
        dataArr[iter].start_point = (iter == 0) ? 0 : dataArr[iter - 1].start_point + dataArr[iter - 1].check_count;
        dataArr[iter].board = Board(board); // create copies of board for each thread
        dataArr[iter].check_count = 1;
        dataArr[iter].results = vector<bool>(3, false);
    }

    omp_set_dynamic(false); // disable dynamic adjustment of threads to use all K threads
    omp_set_num_threads(K); // set number of threads to K

    auto start = chrono::high_resolution_clock::now(); // start timer
#pragma omp parallel for shared(dataArr)
    for (int i = 0; i < N; i++) // create threads with N task units, each of 1 row check, 1 col check and 1 box check
    {
        int tid = omp_get_thread_num();
        dataArr[i].threadId = tid;
        checkBox((void *)&dataArr[i]);
        checkColumn((void *)&dataArr[i]);
        checkRow((void *)&dataArr[i]);
    }

    auto end = chrono::high_resolution_clock::now();                          // end timer
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start); // calculate time taken

    ofstream file;
    file.open("output.txt");
    for (int i = 0; i < N; i++)
    {
        // output data to file
        file << "Thread " << dataArr[i].threadId + 1;
        file << " checks row " << dataArr[i].start_point << " and ";
        file << valid_output_helper(dataArr[i].results[0]) << endl;

        file << "Thread " << dataArr[i].threadId + 1;
        file << " checks column " << dataArr[i].start_point << " and ";
        file << valid_output_helper(dataArr[i].results[1]) << endl;

        file << "Thread " << dataArr[i].threadId + 1;
        file << " checks box " << dataArr[i].start_point << " and ";
        file << valid_output_helper(dataArr[i].results[2]) << endl;
    }

    file << "Time taken : " << duration.count() << " microseconds" << endl;
    for (int i = 0; i < board.n; i++)
    {
        for (int j = 0; j < 3; j++)     // for any of row, column or box check for each thread
            if (!dataArr[i].results[j]) // if any of the checks is invalid, board is invalid
            {
                file << "Board is not valid" << endl;
                return 0;
            }
    }
    file << "Board is valid" << endl; // if all checks are valid, board is valid
    file.close();
    return 0;
}

void *checkRow(void *arg)
{
    threadData *data = (threadData *)arg;
    Board board = data->board;
    int start_point = data->start_point;
    int threadId = data->threadId;
    int check_count = data->check_count; // read data from struct
    unordered_map<int, bool> nums_seen;  // key = number, value = seen
    bool result = true;

    for (int i = 0; i < check_count; i++)
    {
        for (int j = 0; j < board.n; j++) // for all elements in row
        {
            int num = board.data[start_point + i][j];   // get number
            if (nums_seen.find(num) != nums_seen.end()) // if number is already seen
            {
                result = false;
                // return NULL; // can call pthread_exit() here ?
            }
            else
            {
                nums_seen[num] = true;
            }
        }
        nums_seen.clear(); // clear map for next row
    }
    data->results[0] = result;
    return NULL;
}

void *checkColumn(void *arg)
{
    threadData *data = (threadData *)arg;
    Board board = data->board;
    int start_point = data->start_point;
    int threadId = data->threadId;
    int check_count = data->check_count; // read data from struct
    unordered_map<int, bool> nums_seen;  // key = number, value = seen
    bool result = true;

    for (int i = 0; i < check_count; i++)
    {
        for (int j = 0; j < board.n; j++) // for all elements in column
        {
            int num = board.data[j][start_point + i];   // get number
            if (nums_seen.find(num) != nums_seen.end()) // if number is already seen
            {
                result = false;
                // return NULL; // can call pthread_exit() here ?
            }
            else
            {
                nums_seen[num] = true;
            }
        }
        nums_seen.clear(); // clear map for next column
    }
    data->results[1] = result;
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
    bool result = true;

    for (int i = 0; i < check_count; i++)
    {
        int startRow = ((start_point + i) / sqrtn) * sqrtn; // get starting row and column of box
        int startCol = ((start_point + i) % sqrtn) * sqrtn; // from start_point
        for (int j = 0; j < sqrtn; j++)
        {
            for (int k = 0; k < sqrtn; k++)
            {
                int num = board.data[startRow + j][startCol + k]; // get number
                if (nums_seen.find(num) != nums_seen.end())       // if number is already seen
                {
                    result = false;
                    // return NULL; // can call pthread_exit() here ?
                }
                else
                {
                    nums_seen[num] = true;
                }
            }
        }
        nums_seen.clear(); // clear map for next box
    }
    data->results[2] = result;
    return NULL;
}