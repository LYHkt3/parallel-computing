#include<stdio.h>
#include<omp.h>

int N=4,M=5,K=3;

//    A = [1 0 2 0 0
//         0 3 0 0 4
//         5 0 6 0 0
//         0 0 0 7 8]

int row_ptr[20] = {0, 2, 4, 6, 8};
int col_idx[20] = {0, 2, 1, 4, 0, 2, 3, 4};
int val[20] = {1, 2, 3, 4, 5, 6, 7, 8};

int B[5][3] = {  1, 2, 3,
                 4, 5, 6,
                 7, 8, 9,
                10,11,12,
                13,14,15};

int c[4][3];

int main(){

    #pragma omp  parallel for schedule(dynamic , 4)
    for(int i=0;i<N;i++){

        int start = row_ptr[i];
        int end = row_ptr[i+1];

        for(int j=start;j<end;j++){

            int value = val[j];
            int row = col_idx[j];

            #pragma omp simd
            for(int t=0;t<K;t++){
                c[i][t]+=B[row][t]*value;
            }

        }
    }

    for(int i=0;i<N;i++){
        for(int t=0;t<K;t++){
            printf("%d ",c[i][t]);
        }
        printf("\n");
    }

    return 0;
}