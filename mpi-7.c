#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>

int main(int argc, char* argv[]){
  int i;
  int myrank, size;
  int l, a, b, N, x, y, r;
  unsigned int seed;
  unsigned int* seeds;
  MPI_Status Status;
  MPI_File file;
  char arr[10];
  arr[0]='0';
  arr[1]='0';
  arr[2]='0';
  arr[3]='\n';
  if (argc != 5){
    printf("Wrong number of arguments\n");
    return 1;
  }
  l = atoi(argv[1]);
  a = atoi(argv[2]);
  b = atoi(argv[3]);
  N = atoi(argv[4]);
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_File_open(MPI_COMM_WORLD, "data.bin", MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &file);

  int* num_of_points = (int*)calloc(a*b*l*l,sizeof(int));

  if (myrank == 0){
    seeds = (unsigned int*)malloc(size * sizeof(unsigned int));
    srand(time(NULL));
    for (i = 0; i < size; ++i) {
      seeds[i] = rand();
    }
  }
  MPI_Scatter(seeds, 1, MPI_UNSIGNED, &seed, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

  for(i = 0; i < N; ++i){
    x = rand_r(&seed) % l;
    y = rand_r(&seed) % l;
    r = rand_r(&seed) % (a*b);
    num_of_points[(r/a * l + y)*a*l + r%a * l + x] += 1;
  }
  if (myrank == 0){
      for (i = 0; i < a*b*l*l; ++i){
        printf("%d ", num_of_points[i]);
      }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (myrank == 1){
      for (i = 0; i < a*b*l*l; ++i){
        printf("%d ", num_of_points[i]);
      }
  }
  MPI_Datatype view;
  MPI_Type_vector( a*b*l*l ,1, size, MPI_INT, &view);
  MPI_Type_commit(&view);

  MPI_File_set_view(file, myrank*4, MPI_INT, view, "native", MPI_INFO_NULL);

  MPI_File_write(file, num_of_points, a*b*l*l, MPI_INT, MPI_STATUS_IGNORE);

  MPI_Type_free(&view);

  MPI_File_close(&file);
  MPI_Finalize();
  return 0;
}
