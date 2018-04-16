// this code assum that the array is divisible by number of processers
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <mpi.h>

#define BLOCK 1024*1024
double *data, *finaldata;

int double_cmp(const void *a,const void *b){
    return *(double *)a > *(double *)b ? 1 : -1;
}

void merge(double *a, int len, double *b, double *c, int isBig){
    int i,j,k;
    
    if(isBig){
        i=j=k=len-1;
        for(k;k>=0;k--){
            if(a[i]>=b[j]){
                c[k]=a[i--];
            }
            else{
                c[k]=b[j--];
            }
        
        }
    }
    else{
        i=j=k=0;
        for(k;k<len;k++){
            if(a[i]<=b[j]){
                c[k]=a[i++];
            }
            else{
                c[k]=b[j++];
            }
        
        }
    }
    memcpy(a,c,sizeof(double)*len);
}

int merge1(double *ina, int lena, double *inb, int lenb, double *out) {
    int i,j;
    int outcount=0;

    for (i=0,j=0; i<lena; i++) {
        while ((inb[j] < ina[i]) && j < lenb) {
            out[outcount++] = inb[j++];
        }
        out[outcount++] = ina[i];
    }
    while (j<lenb)
        out[outcount++] = inb[j++];

    return 0;
}

int Compute_partner(int phase,int my_rank,int np){
    int partner;
    if(!(phase&1)){ // even phase
        if(my_rank&1)
            partner=my_rank-1;
        else
            partner=my_rank+1;
    }
    else{ // odd phase
        if(my_rank&1)
            partner=my_rank+1;
        else
            partner=my_rank-1;
    }

    if(partner==-1 || partner==np){
        partner=MPI_PROC_NULL;
    }
    return partner;
}

void mpi_oddevensort(int argc, char* argv[], int len, int root, MPI_Comm comm){
    // MPI_Init(&argc, &argv);
    int my_rank, np;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    double* local_data = (double *)malloc(sizeof(double)*len/np);
    double* recv_data = (double *)malloc(sizeof(double)*len/np);
    double* merged_data = (double *)malloc(sizeof(double)*len/np);
    // double* merged_data = (double *)malloc(sizeof(double)*len/np*2);

    MPI_Scatter(data, len/np , MPI_DOUBLE, local_data, len/np , MPI_DOUBLE, root, comm);
    free(data);
    
    printf("before:%d: %f, %f\n", my_rank, local_data[0], local_data[len/np-1]);
    MPI_Barrier(MPI_COMM_WORLD);

    qsort(local_data, len/np, sizeof(double), double_cmp); // do this once at first, because local_data will be in order atfer this sort.
    // printf("After:%d: %f\n", my_rank, local_data[0]);
    for(int i = 0; i < np; ++i)
    {
        int partner = Compute_partner(i, my_rank, np);
        // MPI_PROC_NULL 代表空进程， 即不存在的进程
        if (partner != MPI_PROC_NULL){
            //发送自己的数据给 partner , 并接受 partner 的数据
            MPI_Sendrecv(local_data, len/np, MPI_DOUBLE, partner, 0, recv_data, len/np, MPI_DOUBLE, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if(my_rank < partner){
                int isBig = 0;
                merge(local_data, len/np, recv_data, merged_data, isBig);
                // merge1(local_data, len/np, recv_data, len/np, merged_data);
                // memcpy(local_data, merged_data, sizeof(double)*len/np);
                // // local_data = merged_data; // same address, don't free twice, and merged_data will be changed so local_data will be changed too
            }
            else{
                int isBig = 1;
                merge(local_data, len/np, recv_data, merged_data, isBig);
                // merge1(local_data, len/np, recv_data, len/np, merged_data);
                // memcpy(local_data, merged_data + len/np, sizeof(double)*len/np);
                // // local_data = merged_data + len/np; //bug
            }
        }
    }

    MPI_Gather(local_data, len/np , MPI_DOUBLE, finaldata, len/np , MPI_DOUBLE, root, comm);
    free(local_data);
    free(recv_data);
    free(merged_data);
    MPI_Finalize();
    if(my_rank == 0){ // https://stackoverflow.com/questions/30439856/mpi-finalize-does-not-end-any-processes
        printf("data[0]=%f, %f, %f, %f, %f, last data=%f, len/np is %d\n", finaldata[0], finaldata[1], finaldata[len/np], finaldata[1+len/np], finaldata[2+len/np], finaldata[len-1], len/np);
        free(finaldata);
    }
}

int main(int argc,char* argv[]){

    MPI_Init(&argc, &argv);
    int my_rank, np;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    
    if(argc != 3){
        printf("Usage:%s loop_times data_file\n",argv[0]);
    }
    int loop = atoi(argv[1]);

    if (my_rank == 0){
        FILE *fp;
        if((fp = fopen(argv[2],"r")) == NULL){
            printf("Cannot Open file %s\n",argv[2]);
            return -1;
        }

        data = (double *)malloc(sizeof(double)*BLOCK*loop);
        finaldata = (double *)malloc(sizeof(double)*BLOCK*loop);

        fread(data, sizeof(double) ,BLOCK*loop, fp);
        int len = BLOCK*loop;
        printf("data[0]=%f, %f, %f, %f, %f, last data=%f, len/np is %d\n", data[0], data[1], data[len/np], data[1+len/np], data[2+len/np], data[len-1], len/np);
        fclose(fp);  
    }
    MPI_Barrier(MPI_COMM_WORLD);
    mpi_oddevensort(argc, argv, BLOCK*loop, 0, MPI_COMM_WORLD); //BLOCK*loop or BLOCK*loop*sizeof(double)?
}
