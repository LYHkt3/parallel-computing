#include <stdio.h>
#include <algorithm>
#include "mpi.h"
using namespace std;

int NUM = 4;

int main(int argc, char *argv[])
{

    int my_rank, size;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int num[(NUM * 4)];
    int recv[NUM];
    int send[NUM];
    int partner = -1;
    int process[(NUM * 2)];

    if (my_rank == 0)
    {
        for (int i = 0; i < NUM * size; i++)
        {
            scanf("%d", &num[i]);
        }
    }

    MPI_Scatter(num, NUM, MPI_INT, send, NUM, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    
    sort(send, send + NUM); // 内部排序

    // 循环

    int global_tag = 1;

    do{

        int local_tag = 0;

        // 偶数阶段

        if (my_rank % 2 == 0) partner = my_rank + 1;
        else partner = my_rank - 1;
        
        if (partner >= 0 && partner < size){
            
            MPI_Sendrecv(send, NUM, MPI_INT, partner, 0, recv, NUM, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            int old[NUM];
            for (int i = 0; i < NUM; i++)
                old[i] = send[i];

            for (int i = 0; i < NUM ; i++)
            {
                process[i] = send[i];
                process[i + NUM] = recv[i];
                
            }

            sort(process, process + (2 * NUM ));

            if (my_rank % 2 == 0)
            {
                for (int i = 0; i < NUM; i++)
                {
                    send[i] = process[i];
                }
            }
            else
            {
                for (int i = 0; i < NUM; i++)
                {
                    send[i] = process[i + NUM];
                }
            }

            for (int i = 0; i < NUM; i++)
            {
                if (old[i] != send[i])
                {
                    local_tag = 1;
                    break;
                }
            }
        }


        // 奇数阶段

        if (my_rank % 2 == 1) partner = my_rank + 1;
        else partner = my_rank - 1;

        if (partner >= 0 && partner < size){

            MPI_Sendrecv(send, NUM, MPI_INT, partner, 0, recv, NUM, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            int old[NUM];
            for (int i = 0; i < NUM; i++)
                old[i] = send[i];

            for (int i = 0; i < NUM * 2; i++)
            {
                process[i] = send[i];
                process[i+NUM] = recv[i];
            }

            sort(process, process + NUM * 2);

            if (my_rank % 2 == 1)
            {
                for (int i = 0; i < NUM; i++)
                {
                    send[i] = process[i];
                }
            }
            else
            {
                for (int i = 0; i < NUM; i++)
                {
                    send[i] = process[i + NUM];
                }
            }

            for (int i = 0; i < NUM; i++)
            {
                if (old[i] != send[i])
                {
                    local_tag = 1;
                    break;
                }
            }
        }

        // printf("my_rank is %d\n",my_rank);

        // for(int i=0;i<NUM;i++){
        //     printf("%d ",send[i]);
        // }
        // printf("\n");


        MPI_Allreduce(&local_tag,&global_tag,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);

    } while (global_tag > 0);

    // 输出

    printf("the rank %d\nthe num:", my_rank);

    for (int i = 0; i < NUM; i++)
    {
        printf(" %d ", send[i]);
    }
    printf("\n");

    MPI_Finalize();

    return 0;
}

// 数据
//  6 8 7 5    9 4 1 7    8 2 1 7   8 7 3 9