#include <mpi.h>
#include <stdio.h>
#include <math.h>

#define M_REQUEST 1
#define M_MARKER 2
#define proc_num 64

void add_to_queue(int* queue, int length, int elem) {
    for (int i = 0; i < length; i++) {
        if (queue[i] == -1) {
            queue[i] = elem;
            return;
        }
    }
} 

int remove_from_queue(int* queue, int length) {
	int first = queue[0];
    for (int i = 0; i < length - 1; i++) {
        queue[i] = queue[i+1];
        if (queue[i] == -1) return first;
    }
    queue[length - 1] = -1;
    return first;
}

int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int up, left, right, to_marker;
    int queue[proc_num];
    for (int i = 0; i < proc_num; i++) {
        queue[i] = -1;
    }
    MPI_Status status;
    up = (rank) ? (floor((rank - 1) / 2)) : -1;
    left = (2 * rank + 1 < proc_num) ? 2 * rank + 1 : -1;
    right = (2 * rank + 2 < proc_num) ? 2 * rank + 2 : -1;
    to_marker = (rank) ? up : rank;
    if (to_marker != rank) {
        printf("--%d-- sending request to %d\n", rank, to_marker);
        MPI_Send(0, 0, MPI_CHAR, to_marker, M_REQUEST, MPI_COMM_WORLD);
        add_to_queue(queue, proc_num, rank);
        while (1) {
            MPI_Recv(0, 0, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);   
            if (status.MPI_TAG == M_REQUEST) {
                if (to_marker == rank) {
                    printf("--%d-- sending marker to %d\n", rank, status.MPI_SOURCE);
                    to_marker = status.MPI_SOURCE;
                    MPI_Send(0, 0, MPI_CHAR, status.MPI_SOURCE, M_MARKER, MPI_COMM_WORLD);
                } else {
                    add_to_queue(queue, proc_num, status.MPI_SOURCE);
                    printf("--%d-- redirecting request from %d to %d\n", rank, status.MPI_SOURCE, to_marker);
                    MPI_Send(0, 0, MPI_CHAR, to_marker, M_REQUEST, MPI_COMM_WORLD);
                }
            } else if (status.MPI_TAG == M_MARKER) {
                to_marker = remove_from_queue(queue, proc_num);
                if (to_marker != rank) {
                    printf("--%d-- redirecting marker from %d to %d\n", rank, status.MPI_SOURCE, to_marker);
                    MPI_Send(0, 0, MPI_CHAR, to_marker, M_MARKER, MPI_COMM_WORLD);
                } else {
                    printf("--%d-- recieved marker from %d\n", rank, status.MPI_SOURCE);
                    break;            
                }
            }
        }    
    }

    FILE *f;
    printf("--%d-- entring critical section\n", rank);
    f = fopen("critical.txt", "r");
    if (f) {
        printf("--%d-- critical.txt exists\n", rank);
    } else {
        f = fopen("critical.txt", "w");
        int sec = 1; //1 + rand()%10;
        // printf("--%d-- sleeping for %d seconds\n", rank, sec);
        double t = MPI_Wtime();
        while (MPI_Wtime() - t < sec);
        remove("critical.txt");
    }
    printf("--%d-- leaving critical section\n", rank);
    
    while (1) {
        MPI_Recv(0, 0, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);  
        if (status.MPI_TAG == M_REQUEST) {
            if (to_marker == rank) {
                printf("--%d-- sending marker to %d\n", rank, status.MPI_SOURCE);
                to_marker = status.MPI_SOURCE;
                MPI_Send(0, 0, MPI_CHAR, status.MPI_SOURCE, M_MARKER, MPI_COMM_WORLD);
            } else {
                add_to_queue(queue, proc_num, status.MPI_SOURCE);
                printf("--%d-- redirecting request from %d to %d\n", rank, status.MPI_SOURCE, to_marker);
                MPI_Send(0, 0, MPI_CHAR, to_marker, M_REQUEST, MPI_COMM_WORLD);
            }
        } else if (status.MPI_TAG == M_MARKER) {
            to_marker = remove_from_queue(queue, proc_num);
            if (to_marker != rank) {
                printf("--%d-- redirecting marker from %d to %d\n", rank, status.MPI_SOURCE, to_marker);
                MPI_Send(0, 0, MPI_CHAR, to_marker, M_MARKER, MPI_COMM_WORLD);
            } else {
                printf("--%d-- recieved marker from %d\n", rank, status.MPI_SOURCE);
                break;            
            }
        }
    }    

    MPI_Finalize();
    return 0;
}