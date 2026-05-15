#include<stdio.h>

#define year 30
#define  n  92
#define window 11

double Day[year*n];

void Input();
int Sort(double *);
void Process(double *,double *);
void Get_num(int ,int ,double *,double *);
void average(double * ,double *);


int main(){

    Input();

    double data[n];

    Process(&Day[0],&data[0]);
    
    return 0;
}

void Process(double *day ,double *data){

    int W = window / 2 ;
    
    double data_90[n];

    for(int i=W;i<n-W;i++){

        double Num[year*window];

        Get_num(W,i,&Num[0],day);  //获取330天数据
        
        data_90[i] = Sort(&Num[0]);   //提取90%分位数

    }

    for(int i=W;i<n-W;i++){

        average(&data_90[0],data);  //31天滑动平均

    }

}

    

void Get_num(int W,int d,double *num_to,double *day){

    for(int j=0;j<year;j++){
        for(int t=d-W;t<d+W;t++){

            *num_to++ = day[t+j*n];

        }
    }
}