#include<bits/stdc++.h>
using namespace std;
#include"mpi.h"

double f(double x){
    return x*x;
}

double trap(double left,double right,int trap_count,double base_len){
    double tot,x;
    tot = (f(left) + f(right)) / 2.0;
    for(int i=1;i<trap_count;i++){
        x=left+i*base_len;
        tot+=f(x);
    }
    tot*=base_len;
    return tot;
}

int main(){

    int my_rank,size,n=1024,local_n;
    double a=0.0,b=10.0,h,local_a,local_b;
    double local_in,total_in;
    int rank_i;

    MPI_Init(NULL,NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    h=(b-a)/n;
    local_n = n/size;
    local_a = a + my_rank*local_n*h;
    local_b = local_a + local_n*h;

    local_in = trap(local_a,local_b,local_n,h);

    if(my_rank != 0){
        MPI_Send(&local_in , 1 , MPI_DOUBLE , 0 , 0 , MPI_COMM_WORLD);
    }
    else{
        total_in = local_in;
        for( rank_i = 1 ; rank_i < size ; rank_i++){
            MPI_Recv(&local_in , 1 , MPI_DOUBLE , rank_i , 0 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
            total_in += local_in;
        }
    }

    if(my_rank == 0){
        cout<<"the result is "<<total_in<<"!"<<endl;
    }
    
    MPI_Finalize();

    return 0;
}