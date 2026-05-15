#include<stdio.h>

#define year 30
#define  n  92
#define window 11

double Day[year*n];

void Input();
void Process();
double Get_num(int ,int );



int main(){

    Input();

    int data[n];

    Process();
    
    return 0;
}

void Process(){

    int W = window / 2 ;
    

    for(int i=W;i<n-W;i++){

        double num;

        num = Get_num(W,i);  //获取330天数据的平均值
        
    }
}

    
double Get_num(int W,int day){

    int Aver = 0;

    for(int j=0;j<year;j++){
        for(int t=day-W;t<day+W;t++){

        Aver+=Day[t+j*n];

        }
    }

    return Aver/(double)(year*W);
}