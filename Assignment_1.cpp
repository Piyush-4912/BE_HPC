#include <iostream>
#include <omp.h>
using namespace std;

#define MAX 100

int graph[MAX][MAX];
int sizeList[MAX];
int visited[MAX];

// ─────────────────────────────────────────────
//  PARALLEL BFS
// ─────────────────────────────────────────────
void parallel_bfs(int start, int n) {
    int queue[MAX];
    int new_queue[MAX];
    int rear     = 0;
    int new_rear = 0;

    for (int i = 0; i < n; i++) visited[i] = 0;

    visited[start]  = 1;
    queue[rear++]   = start;

    cout << "BFS Traversal:\n";

    while (rear > 0) {
        for (int i = 0; i < rear; i++)
            cout << queue[i] << " ";

        new_rear = 0;

        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < rear; i++) {
            int node = queue[i];

            for (int j = 0; j < sizeList[node]; j++) {
                int nb = graph[node][j];

                if (!visited[nb]) {
                    #pragma omp critical
                    {
                        if (!visited[nb]) {
                            visited[nb]           = 1;
                            new_queue[new_rear++] = nb;
                        }
                    }
                }
            }
        }

        for (int i = 0; i < new_rear; i++)
            queue[i] = new_queue[i];
        rear = new_rear;
    }
}

// ─────────────────────────────────────────────
//  RECURSIVE DFS UTILITY
// ─────────────────────────────────────────────
void dfs_util(int node) {
    // Mark and print BEFORE spawning threads
    visited[node] = 1;
    cout << node << " ";

    // Try each neighbor: go deep on first unvisited
    for (int i = 0; i < sizeList[node]; i++) {
        int nb = graph[node][i];

        if (!visited[nb]) {
            dfs_util(nb);   // recurse immediately — preserves DFS order
        }
    }
}

// ─────────────────────────────────────────────
//  PARALLEL DFS
// ─────────────────────────────────────────────
void parallel_dfs(int start, int n) {
    for (int i = 0; i < n; i++) visited[i] = 0;

    cout << "\nDFS Traversal:\n";

    // Outer parallel region — each thread picks an unvisited
    // start point (handles disconnected graphs too)
    #pragma omp parallel for schedule(dynamic) shared(visited)
    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            #pragma omp critical
            {
                // Double-check after acquiring lock
                if (!visited[i]) {
                    dfs_util(i);
                }
            }
        }
    }
}

// ─────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────
int main() {
    int n = 6;

    sizeList[0] = 2; graph[0][0] = 1; graph[0][1] = 2;
    sizeList[1] = 3; graph[1][0] = 0; graph[1][1] = 3; graph[1][2] = 4;
    sizeList[2] = 2; graph[2][0] = 0; graph[2][1] = 5;
    sizeList[3] = 1; graph[3][0] = 1;
    sizeList[4] = 2; graph[4][0] = 1; graph[4][1] = 5;
    sizeList[5] = 2; graph[5][0] = 2; graph[5][1] = 4;

    parallel_bfs(0, n);
    parallel_dfs(0, n);
    return 0;
}
