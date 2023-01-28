#include <iostream>
#include <pthread.h>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

void* checkBasic(void* arg);

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
    int mode; // row, col, box
    int start_point;
} threadData;

int main() {
    Board board("data/solved1.txt");
    board.repr();
    pthread_t threadArr[board.k];
    threadData dataArr[board.k];

    for (int i = 0; i < board.k; i++) {
        dataArr[i].threadId = i;
        dataArr[i].board = &board;
        dataArr[i].mode = i % 3;
        dataArr[i].start_point = i / 3; // this needs to be changed
        pthread_create(&threadArr[i], NULL, checkBasic, (void*)&dataArr[i]);
    }

    for (int i = 0; i < board.k; i++) {
        pthread_join(threadArr[i], NULL);
    }
    
    return 0;
}


void* checkBasic(void* arg)
{
    threadData* data = (threadData*)arg;
    Board* board = data->board;
    int mode = data->mode;
    int start_point = data->start_point;
    int threadId = data->threadId;

    board->repr();
    return NULL;
}