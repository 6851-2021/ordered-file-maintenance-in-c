#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "OrderedFileMaintenance.h"

inline double get_density(list_t* list, int index, int len) {

	int full = 0;
	for (int i = index; i < index+len; i++) {
		full += (! list->items[i]->empty());
	}
	double full_d = (double) full;
	return full_d/len;
}

inline int ofm_max(list_t* list){
	int size = list->N;
	int max_elem = INT32_MIN;
	for(int i=0; i<size; i++){
		int temp_max = list->items[i]->get_max();
		if(temp_max > max_elem){
			max_elem = temp_max;
		}
	}
	return max_elem;
}

// find index of first 1-bit (least significant bit)
static inline int bsf_word(int word) {
  int result;
  __asm__ volatile("bsf %1, %0" : "=r"(result) : "r"(word));
  return result;
}

static inline int bsr_word(int word) {
  int result;
  __asm__ volatile("bsr %1, %0" : "=r"(result) : "r"(word));
  return result;
}

inline __attribute__((always_inline)) int get_depth(list_t* list, int len) {
	return bsr_word(list->N/len);
}


inline __attribute__((always_inline)) pair_int get_parent(list_t* list, int index, int len) {
	int parent_len = len*2;
	int depth = get_depth(list, len);
	pair_int pair;
	pair.x = parent_len;
	pair.y = depth;
	return pair;
}

inline __attribute__((always_inline)) pair_double density_bound(list_t* list, int depth) {
	pair_double pair;

	// between 1/4 and 1/2
	// pair.x = 1.0/2.0 - (( .25*depth)/list->H);
	// between 1/8 and 1/4
	pair.x = 1.0/4.0 - (( .125*depth)/list->H);
	pair.y = 3.0/4.0 + ((.25*depth)/list->H);
	return pair;
}


// Evenly redistribute elements in the ofm, given a range to look into
// index: starting position in ofm structure
// len: area to redistribute
void redistribute(list_t* list, int index, int len) {
	list->min_index = (list->min_index < index) ? list->min_index : index;
	list->max_index = (list->max_index > index + len - 1) ? list->max_index : index + len - 1;

	indirection_group **space = (indirection_group**)malloc(len*sizeof(*(list->items)));
	int j = 0;

	// move all items in ofm in the range into
	// a temp array
	for (int i = index; i< index+len; i++) {
		if (! list->items[i]->empty()) {
			space[j++] = list->items[i];
			list->items[i] = NULL;
			continue;
		}
		// space[j] = list->items[i];
		// j+=(! list->items[i].empty());
		delete list->items[i];
		list->items[i] = NULL;
	}

	// evenly redistribute for a uniform density
	double index_d = index;
	double step = ((double) len)/j;
	for (int i = 0; i < j; i++) {
	  	int in = index_d;
	  	list->items[in] = space[i];
	  	index_d+=step;
	}
	for (int j = index; j < index + len; j++) {
		if (list->items[j] != NULL) continue;
  	  	list->items[j] = new indirection_group(list->logn);
    }

	free(space);
}

inline __attribute__((always_inline)) int isPowerOfTwo (int x) {
  return ((x != 0) && !(x & (x - 1)));
}

void double_list(list_t* list) {
	list->n *= 2;
	list->logn = (1 << bsr_word(bsr_word(list->n)+1));
	int old_N = list->N;
	list->N = list->n / list->logn;
	list->logN = (1 << bsr_word(bsr_word(list->N)+1));
	list->H = bsr_word(list->N/list->logN);
	indirection_group **new_items = (indirection_group**)realloc(list->items, list->N*sizeof(*(list->items)));
	if (! new_items) {
		printf("OH FUCKKKK.\n\n");
		exit(-1);
	}
	list->items = new_items;
	for (int i = 0; i < old_N; i++) {
		list->items[i]->reset_capacity(list->logn);
	}
	for (int i = old_N; i < list->N; i++) {
		list->items[i] = new indirection_group(list->logn);
	}
	redistribute(list, 0, list->N);
}

void half_list(list_t* list) {
	list->n /= 2;
	list->logn = (1 << bsr_word(bsr_word(list->n)+1));
	int old_N = list->N;
	list->N = list->N = list->n / list->logn;
	list->logN = (1 << bsr_word(bsr_word(list->N)+1));
	list->H = bsr_word(list->N/list->logN);
	indirection_group **new_array = (indirection_group**)malloc(list->N*sizeof(*(list->items)));
	int j = 0;
	for (int i = 0; i < old_N; i++) {
		if (! list->items[i]->empty()) {
			new_array[j++] = list->items[i];
			continue;
		}
		delete list->items[i];
	}
	for (int i = j; j < list->N; j++) {
		new_array[i] = new indirection_group(list->logn);
	}
	free(list->items);
	list->items = new_array;
	redistribute(list, 0, list->N);
}

void scan(list_t* list, int index, int len) {
	for (int i = index; i < index+len; i++) {
		if (! list->items[i]->empty()) {
			printf("%d ", list->items[i]->get_max());
		}
	}
}


inline void slide_right(list_t* list, int index) {	
	indirection_group* el = list->items[index];
	while (! list->items[++index]->empty()) {
		indirection_group* temp = list->items[index];
		list->items[index] = el;
		el = temp;
	}
	list->items[index] = el;
	list->max_index = (list->max_index > index) ? list->max_index : index;
}

inline void slide_left(list_t* list, int index) {	
	indirection_group* el = list->items[index];
	while (! list->items[--index]->empty()) {
		indirection_group* temp = list->items[index];
		list->items[index] = el;
		el = temp;
	}
	list->items[index] = el;
	list->min_index = (list->min_index < index) ? list->min_index : index;
}

// given index, return the starting index of the leaf it is in
inline __attribute__((always_inline)) int find_leaf(list_t* list, int index) {
	return (index/list->logN)*list->logN;
}

// same as find_leaf, but does it for any level in the tree
// index: index in array
// len: length of sub-level. 
inline __attribute__((always_inline)) int find_node(int index, int len) {
	return (index/len)*len;
}

inline indirection_group** find_elem_pointer(list_t* list, int index, int elem){

	int item = list->items[index]->get_max();
	while(item!=elem){
		item = list->items[++index]->get_max();
	}
	return &(list->items[index]);

}

inline bool canSlideRight(list_t* list, int index) {
	while(index < list->N) {
		if(list->items[index]->empty()) {
			return true;
		}
		index += 1;
	}
	return false;
}

indirection_group** insert( list_t* list, int index, int elem) {
	list->min_index = index;
	list->max_index = index;

	int node_index = find_leaf(list, index);
	int level = list->H;
	int len = list->logN;

	// always deposit on the left
	if (list->items[index]->insert(elem)) {
		indirection_group* new_group = list->items[index]->split();
		int next_index = index + 1;
		list->max_index += list->items[next_index]->empty();
		while (next_index < list->N && list->items[next_index++]->merge_and_split(new_group)) {
			if (next_index < list->N && list->items[next_index]->empty()) list->max_index = next_index;
			// list->max_index = next_index++;
		}
		if (next_index == list->N) {
			double_list(list);
			if (list->items[list->N - 1]->empty()) {
				list->items[list->N - 1]->merge_and_split(new_group);
				list->max_index = list->N - 1;
			} else {
				printf("oops! we expected the last element to be empty..\n");
				exit(-1);
			}
		}
	}

	double density = get_density(list, node_index, len);

	// spill over into next level up, node is completely full.
	if (density == 1) {
	  node_index = find_node(node_index, len*2);
	  redistribute(list, node_index, len*2);
	} else {
	  redistribute(list, node_index, len);
	}

	pair_double density_b = density_bound(list, level);
	density = get_density(list, node_index, len);
	//printf("density %f, upperbound %f, len = %d, N = %d, logN = %d\n", density, density_b.y, len, list->N, list->logN);
	while (density >= density_b.y) {
		len*=2;
		if (len <= list->N){
			level--;
			node_index = find_node(node_index, len);
			density_b = density_bound(list, level);
			density = get_density(list, node_index, len);
		} else {
			double_list(list);
			return find_elem_pointer(list, 0, elem);
		}
		//printf("density %f, upperbound %f, len = %d, N = %d, logN = %d\n", density, density_b.y, len, list->N, list->logN);
	}
	redistribute(list, node_index, len); 
	return find_elem_pointer(list, node_index, elem);
}

int binary_search(list_t *list, int elem) {
	int start = 0;
	int end = list->N-1;
	while (start+1 < end) {

		int mid = (start + end)/2;
		
		int item = list->items[mid]->get_max();
		int change = 1;
		int check = mid;
		while (item == INT32_MIN && check >= start) {
			check = mid+change;
			if (check <= end) {
				item = list->items[check]->get_max();
				if (item != INT32_MIN) {
					break;
				}
			}
			check = mid-change;
			if (check >= start) {
				item = list->items[check]->get_max();
			}
			change++;

		}
		if (item == INT32_MIN) {
			return mid;
		}
		if (elem < item) {
			end = mid;
		} else {
			start = mid;
		}
	}
	if (elem < list->items[start]->get_max() && ! list->items[start]->empty()) {
		return start;
	}
	return start+1;
}

inline __attribute__((always_inline)) indirection_group** insert_sorted(list_t *list, int elem) {
	return insert(list, binary_search(list, elem), elem);
}

inline __attribute__((always_inline)) int find_index(list_t* list, indirection_group** elem_pointer){
	indirection_group** array_start = list->items; 
	int index = (elem_pointer- array_start);
	return index;
}

// given an element pointer, find the next element index after it
inline indirection_group** get_next_elem_index(list_t* list, indirection_group** elem_pointer){
	int index = find_index(list, elem_pointer);
	index++;
	while(index<list->N){
		if(! list->items[index]->empty()){
			return list->items+index;
		}
		index++;
	}
	return NULL;
}

// given an element pointer, find previous element index after it
inline indirection_group** get_prev_elem_index(list_t* list, indirection_group** elem_pointer){
	int index = find_index(list, elem_pointer);
	index--;
	while(index>=0){
		if(! list->items[index]->empty()){
			return list->items+index;
		}
		index--;
	}
	return NULL;
}

// given an element value and pointer to an element,
// insert before it.
inline indirection_group** insert_before(list_t* list, int new_elem, indirection_group** elem_pointer){
	int elem_index = find_index(list, elem_pointer);
	if(elem_index!=0){
		return insert(list, elem_index-1, new_elem);
	}
	return NULL;
}

// returns pointer to the element we just inserted!
inline indirection_group** insert_after(list_t* list, int new_elem, indirection_group** elem_pointer){
	int elem_index = find_index(list, elem_pointer);
	if(elem_index<(list->N-1) && elem_index >=0){
		indirection_group** new_elem_pointer =  insert(list, elem_index+1, new_elem);
		return new_elem_pointer;
	}
	else{
		printf("returning null\n");
		return NULL;
	}
}

indirection_group** get_first(list_t* list){
	int index = 0;
	int elem = list->items[index]->get_max();
	while(elem==INT32_MIN){
		elem = list->items[++index]->get_max();
	}
	if(index==list->N){
		// return first index location by default
		return list->items;
	}
	return &(list->items[index]);
}

// insert as the first item into the list.
inline __attribute__((always_inline)) indirection_group** insert_first(list_t*list, int elem){
	list->items[0]->insert(elem);
	return list->items;
}

void delete_ofm(list_t* list, int index){
	int node_index = find_leaf(list, index);
	int level = list->H;
	int len = list->logN;
	pair_double density_b = density_bound(list, level);
	double low_bound = density_b.x;
	if(list->items[index]->empty()){
		printf("Element does not exist at index: %d \n", index);
		return;
	}
	//deletion
	delete list->items[index];
	list->items[index] = new indirection_group(list->logn);

	// redistribute 'recursively' until we are within density bounds.
	double density = get_density(list, node_index, len);
	while(density < density_b.x){
		len*=2;
		if (len <= list->N){
			level--;
			node_index = find_node(node_index, len);
			pair_double density_b = density_bound(list, level);
			density = get_density(list, node_index, len);
		} else {
			half_list(list);
			return;
		}
	}
	redistribute(list, node_index, len);
}

inline __attribute__((always_inline)) void delete_here(list_t* list, indirection_group** elem_pointer){
	int elem_index = find_index(list, elem_pointer);
	if(elem_index>=0 && elem_index<list->N){
		delete_ofm(list, elem_index);
		return;
	}
}


void print_array(list_t* list) {
	for (int i = 0; i < list->N; i++) {
		if (list->items[i]->empty()) {
			printf("/ ");
		} else {
			printf("%d ", list->items[i]->get_max());
		}
	}
	printf("\n\n");
}

void setup(list_t* list){
	list->n = 16;
	list->logn = 4;
	list->N = 4;
	//printf("%d\n", bsf_word(list->N));
	list->logN = (1 << bsr_word(bsr_word(list->N)+1));

	list->H = bsr_word(list->N/list->logN);
	//printf("N = %d, logN = %d, loglogN = %d, H = %d\n", list->N, list->logN, list->loglogN, list->H);
	
	list->items = (indirection_group**)malloc(list->N*sizeof(*(list->items)));
	for (int i = 0; i < list->N; i++) {
		list->items[i] = new indirection_group(list->logn);
	}

	//print_array(list);
}

/*
int main(){
	list_t list;
	setup(&list);
	print_array(&list);
	while (1) {
		printf("loc =");
		int loc;
		scanf("%d", &loc);
		int elem;
		printf("elem =");
		scanf("%d", &elem);
		insert(&list, loc, elem);
		print_array(&list);
	}

	// delete
	// while (1) {
	// 	printf("loc =");
	// 	int loc;
	// 	scanf("%d", &loc);

	// 	delete(loc);
	// 	print_array();
	// }
}
>>>>>>> ab668879e32e3e9d7a895d1fb9b4e6ed3d94ddca

*/
