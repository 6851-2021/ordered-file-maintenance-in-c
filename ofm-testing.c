#include <time.h>
#include "OrderedFileMaintenance.c"

// inserts N elements sequentially into ofm.
// prints out time it takes for N operations
void testSequentialInsertOFM(int size){
	setup();
	print_array();

	clock_t start, end;
    double cpu_time_used;
	
	start = clock();
	
	for(int i=0; i<size; i++){
		insert(i,i);
	}
	end = clock();

	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	print_array();
	printf("%d sequential insertions, time: %f \n", size, cpu_time_used);
}

// inserts N elements at random locations into OFM. 
// prints out itme it takes for N operations
void testRandomInsertOFM(int size){

	int randomInsert[N] = {0};
	int index;
	for(int i=0; i< size; i++){
		index = rand()%(size-1);
		randomInsert[i] = index;
	}

	setup();
	print_array();

	clock_t start, end;
    double cpu_time_used;
	
	start = clock();

	for(int i=0; i<size; i++)
		{insert(randomInsert[i],i);}
	
	end = clock();

	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	print_array();
	printf("%d random insertions, time: %f \n", size, cpu_time_used);
}

int main(){

	printf("Beginning Testing Suite. N is: %d\n", N);
	//testSequentialInsertOFM(N/4);
	//testRandomInsertOFM(N/4);	

}

