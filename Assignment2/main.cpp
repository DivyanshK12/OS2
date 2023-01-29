#include <iostream>
#include <pthread.h>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

void* checkRow(void* arg);
void* checkColumn(void* arg);
void* checkBox(void* arg);

enum Mode {
    ROW,
    COL,
    BOX
};

class Board {
public:
    
    int n;
    int k;
    vector<vector<int>> data;

    Board(string filename) {
        ifstream file(filename);
        file >> k >> n;
        data = vector<vector<int>>(n, vector<int>(n, 0));
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                file >> data[i][j];
            }
        }      
    }

    void print() {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                cout << data[i][j] << " ";
            }
            cout << endl;
        }
    }

    void repr() {
        cout << "k = " << k << ", n = " << n << endl;
        cout << &data<< endl;
    }
};

typedef struct threadData {
    int threadId;
    Board* board;
    int start_point;
    int check_count;
    bool result;
} threadData;

int main() {
    Board board("data/solved1.txt");
    int K = board.k;
    int N = board.n;
    pthread_t threadArr[K];
    threadData dataArr[K];

    for (int i = 0; i < K; i++) {
        Mode mode = (Mode)(i % 3);

        dataArr[i].threadId = i;
        dataArr[i].board = &board;
        dataArr[i].check_count = 3*N/K; // This might not work for the last thread
        dataArr[i].result = true;

        if(mode == ROW) {
            dataArr[i].start_point = i * K; // every 1st thread is a row
            pthread_create(&threadArr[i], NULL, checkRow, (void*)&dataArr[i]);
        } 
        else if(mode == COL) {
            dataArr[i].start_point = (i-1) * K; // every 2nd thread is a column
            pthread_create(&threadArr[i], NULL, checkColumn, (void*)&dataArr[i]);
        } 
        else if(mode == BOX) {
            dataArr[i].start_point = (i-2) * K; // every 3rd thread is a box
            pthread_create(&threadArr[i], NULL, checkBox, (void*)&dataArr[i]);
        }
    }
    // Need to find a way to make all threads cancel if one returns false

    for (int i = 0; i < board.k; i++) {
        pthread_join(threadArr[i], NULL);
    }
    
    return 0;
}


void* checkRow(void* arg)
{
    threadData* data = (threadData*)arg;
    Board* board = data->board;
    int start_point = data->start_point;
    int threadId = data->threadId;
    int check_count = data->check_count;

    cout << "Thread " << threadId << " is checking rows " << start_point << " to " << start_point + check_count << endl;

    return NULL;
}

void* checkColumn(void* arg)
{
    threadData* data = (threadData*)arg;
    Board* board = data->board;
    int start_point = data->start_point;
    int threadId = data->threadId;
    int check_count = data->check_count;

    cout << "Thread " << threadId << " is checking columns " << start_point << " to " << start_point + check_count << endl;

    return NULL;
}

void* checkBox(void* arg)
{
    threadData* data = (threadData*)arg;
    Board* board = data->board;
    int start_point = data->start_point;
    int threadId = data->threadId;
    int check_count = data->check_count;

    cout << "Thread " << threadId << " is checking boxes " << start_point << " to " << start_point + check_count << endl;

    return NULL;
}