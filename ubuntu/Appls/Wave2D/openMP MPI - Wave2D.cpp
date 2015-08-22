/*
 * prog2.cpp
 *
 *  Created on: 2014Äê4ÔÂ28ÈÕ
 *      Author: Hongbin
 */

#include <iostream>  
#include <fstream>  
#include <string.h>  
#include <stdlib.h>  
#include <omp.h>
#include "mpi.h"
#include "Timer.h"

using namespace std;

const int defaultN = 100; // the default system size

const double c = 1.0; // wave speed
const double dt = 0.1; // time quantum
const double dd = 2.0; // change in system

int main(int argc, char* argv[]) {

  int N = 0;
  int maxTime = 0; // maximum simulation time
  int time; // the current simulation time
  int nThreads = 1;
  int my_rank = 0;
  int mpi_size = 1;

  // verify the arguments
  if (argc == 4) {
    N = atoi(argv[1]);
    maxTime = atoi(argv[2]);
    nThreads = atoi(argv[3]);
  } else {
    cout << "usage: parameter error" << endl;
    return -1; // wrong arguments
  }

  // start a timer
  Timer timer;
  timer.start( );

  MPI_Init( &argc, &argv ); // start MPI
  MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );
  MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
  int i=0;
  int j=0;
  int i_end = 0;
  int i_init = 0;
  double *z0 = new double[N*N];
  double *z1 = new double[N*N];
  double *z2 = new double[N*N];
  for(i=0; i<N; i++){
	for(j=0; j<N; j++){
	  *(z0+i*N+j) = 0;
	  *(z1+i*N+j) = 0;
	  *(z2+i*N+j) = 0;
	}
  }
  MPI_Bcast( &N, 1, MPI_INT, 0, MPI_COMM_WORLD );

  int weight = N / defaultN;
  int stripe = N / mpi_size;
  MPI_Status r_status;
  
  omp_set_num_threads( nThreads );



  // initialize the simulation space at time = 0
  // calculate z[0][][]
  if(my_rank == 0){
#pragma omp parallel for default(none) firstprivate(N, weight) private(i,j) shared(z0)
  for (i=0; i < N; i++) {
    for (j = 0; j < N; j++) {
      if (i >= 4 * weight && i <= 9 * weight
	  && j >= 4 * weight && j <= 9 * weight) {
        *(z0+i*N+j) = 20.0;
		//cout << *(z0+i*N+j)<<" "<<i<<" "<<j;
      } else {
        *(z0+i*N+j) = 0.0;
      }
    }
	}
  }
  
  
  if(my_rank == 0){
    /*master send data to slaves, send two more lines(before and after) to slaves
      for computation, master sends two more rows, one before each stripe and one after a stripe
      master (stripe+1)*N, slaves (stripe+2)*N
    */
    for(int k=1; k<mpi_size; k++){
      int sendsize = 0;
      if(k == mpi_size-1)
		sendsize = (stripe+1)*N;
      else
		sendsize = (stripe+2)*N;
      MPI_Send(z0 + stripe*N*k-1*N, sendsize, MPI_DOUBLE, k, 0, MPI_COMM_WORLD);
      MPI_Send(z1 + stripe*N*k-1*N, sendsize, MPI_DOUBLE, k, 0, MPI_COMM_WORLD);
      MPI_Send(z2 + stripe*N*k-1*N, sendsize, MPI_DOUBLE, k, 0, MPI_COMM_WORLD);
    }
  }else{
    //slaves recv data from master
    int recesize = 0;
    if(my_rank < mpi_size-1)
      recesize = (stripe+2)*N;
    else
      recesize = (stripe+1)*N;
    MPI_Recv(z0, recesize, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &r_status);
    MPI_Recv(z1, recesize, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &r_status);
    MPI_Recv(z2, recesize, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &r_status);
  }
  
  /*simulation at time = 1, calculate z1[][],  cells not on edge*/
  if(mpi_size > 1)
	i_end = my_rank == (mpi_size-1) ? stripe : stripe+1;
  else if(mpi_size == 1)
	i_end = stripe-1;
  #pragma omp parallel for default(none) firstprivate(N, i_end, weight) private(i,j) shared(z0, z1)
  for (i = 1; i < i_end; i++) {
    for (j = 1; j < N - 1; j++) {
      z1[i*N + j] = z0[i*N + j] 
	  + c * c / 2.0 * dt * dt / (dd*dd)
	  * ( z0[(i+1)*N + j] + z0[(i-1)*N + j] 
	    + z0[i*N + j+1] + z0[i*N + j-1] - 4.0 * z0[i*N + j]);
    }
  }

  // cells on edge
  i_init = my_rank == 0 ? 0 : 1;
  i_end = my_rank == 0 ? stripe : stripe+1;
//#pragma omp parallel for default(none) firstprivate(N, i_end, i_init, my_rank, mpi_size, stripe) private(i) shared(z1)
  for (i=i_init; i < i_end; i++) {
    z1[i*N+0] = 0.0;
    z1[i*N+N-1] = 0.0;
  }
  
  if(my_rank == 0){
    for(i=0; i<N; i++)
      z1[i] = 0.0;
  }
      
  if(my_rank == mpi_size - 1){
	int tmp = mpi_size == 1 ? stripe-1 : stripe;
    for(i=0; i<N; i++){
      z1[tmp*N + i] = 0.0;
	}
  }

  // simulation from time = 2 through to maxTime - 1
  // calculate z[2][][] from z[0][][] and z[1][][]
  for (time = 2; time < maxTime; time++) {
    // cells not on edge
    /*only z1's change need update with each other
      send actual first row(row 1) to pre-rank
      send actual last row(row stripe) to next-rank
      recv one row from pre-rank, put it in row 0
      recv one row from next-rank, put it in row stripe+1
     */
    if(my_rank % 2 == 0){
      //send first, and then receive 
      if(my_rank == 0){
	    if(mpi_size > 1){
	      MPI_Send(z1 + (stripe-1)*N, N, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
	      MPI_Recv(z1 + stripe*N, N, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, &r_status);
	    }
      }else{
	    MPI_Send(z1+N, N, MPI_DOUBLE, my_rank-1, 0, MPI_COMM_WORLD);
	    if(my_rank != mpi_size-1)
	      MPI_Send(z1+stripe*N, N, MPI_DOUBLE, my_rank+1, 0, MPI_COMM_WORLD);

	    MPI_Recv(z1, N, MPI_DOUBLE, my_rank-1, 0, MPI_COMM_WORLD, &r_status);
	    if(my_rank != mpi_size-1)
	      MPI_Recv(z1+(stripe+1)*N, N, MPI_DOUBLE, my_rank+1, 0, MPI_COMM_WORLD, &r_status);
      }
    }else{
      //receive first and then send
      MPI_Recv(z1, N, MPI_DOUBLE, my_rank-1, 0, MPI_COMM_WORLD, &r_status);
      if(my_rank !=  mpi_size-1)
	    MPI_Recv(z1+(stripe+1)*N, N, MPI_DOUBLE, my_rank+1, 0, MPI_COMM_WORLD, &r_status);
	
      MPI_Send(z1+N, N, MPI_DOUBLE, my_rank-1, 0, MPI_COMM_WORLD);
      if(my_rank != mpi_size-1)
	    MPI_Send(z1+stripe*N, N, MPI_DOUBLE, my_rank+1, 0, MPI_COMM_WORLD);
    }
    
	//calculate the end index for each data chunk
    i_end = my_rank == 0 ? stripe : stripe+1;
	if(mpi_size == 1)
	  i_end = stripe-1;
	else{
	  if(my_rank == 0 || my_rank == mpi_size-1)
	    i_end = stripe;
	  else
	    i_end = stripe+1;
	}
    #pragma omp parallel for default(none) firstprivate(N, i_end) private(i,j) shared(z1,z2,z0)
    for (i = 1; i < i_end; i++) {
      for (j = 1; j < N - 1; j++) {
	    z2[i*N + j] = 2.0 * z1[i*N + j]
	     - z0[i*N+j] + c * c * dt * dt / (dd*dd)
	     * (z1[(i+1)*N + j] + z1[(i-1)*N + j]
	     + z1[i*N+j+1] + z1[i*N+j-1] - 4.0 * z1[i*N+j]);		 
      }
    }
    

    // cells on edge
  i_init = my_rank == 0 ? 0 : 1;
  i_end = my_rank == 0 ? stripe : stripe+1;
//#pragma omp parallel for default(none) firstprivate(N, i_end, i_init, my_rank, mpi_size, stripe) private(i) shared(z2)
  for (i=i_init; i < i_end; i++) {
    z2[i*N+0] = 0.0;
    z2[i*N+N-1] = 0.0;
  }
  
  if(my_rank == 0){
    for(i=0; i<N; i++)
      z2[i] = 0.0;
  }
  if(my_rank == mpi_size - 1){
	int tmp = mpi_size == 1 ? stripe-1 : stripe;
    for(i=0; i<N; i++){
      z2[tmp*N + i] = 0.0;
	}
  }
    
    
    // shift z[1][][] to z[0][][], shift[2][][] to z[1][][] for
    // the next simulation step
    i_init = my_rank == 0 ? 0 : 1;
    i_end = my_rank == 0 ? stripe : stripe+1; 
#pragma omp parallel for default(none) firstprivate(N, mpi_size,i_init, i_end) private(i,j) shared(z0, z1, z2)
    for (i = i_init; i < i_end; i++) {
      for (j = 0; j < N; j++) {
	*(z0+ i*N + j) = *(z1 + i*N + j);
	*(z1 + i*N + j) = *(z2 + i*N +j);
      }
    }
  }


  //master receive data from slaves
  if(my_rank == 0){
    for(i=1; i<mpi_size; i++){
      MPI_Recv(z0+stripe*N*i, stripe*N, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &r_status);
      MPI_Recv(z1+stripe*N*i, stripe*N, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &r_status);
      MPI_Recv(z2+stripe*N*i, stripe*N, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &r_status);
    }
  }else{
    //send data(from row 1) to master 
    MPI_Send(z0+N, stripe*N, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    MPI_Send(z1+N, stripe*N, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    MPI_Send(z2+N, stripe*N, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
  }

  MPI_Finalize( ); // shut down MPI

  // stop a timer
  if(my_rank == 0)
	cout <<"elapsed time = " << timer.lap( ) << endl;
	//cout << "rank "<<my_rank<<" done! "<<"elapsed time = " << timer.lap( ) << endl;
  if(my_rank == 0){
    /*cout<<"print array z0:"<<endl;
  for(i=0; i<N; i++){
    for(j=0; j<N; j++)
      cout << *(z0+i*N+j)<<" ";
    cout <<endl;
  }
  cout<<"print array z1:"<<endl;
  for(i=0; i<N; i++){
    for(j=0; j<N; j++)
      cout << *(z1+i*N+j)<<" ";
    cout <<endl;
    }*/
  cout<<"print array z2:"<<endl;
  for(i=0; i<N; i++){
    for(j=0; j<N; j++)
      cout << *(z2+i*N+j)<<" ";
    cout <<endl;
  }  
  }
  return 0;

}
