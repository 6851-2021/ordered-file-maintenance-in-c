
typedef struct _list {
	int N;
	int H;
	int logN;
	int* items;

	int min_index;
	int max_index;
} list_t;

typedef struct _pair_int {
	int x; // length in array
	int y; // depth 
} pair_int;

typedef struct _pair_double {
	double x;
	double y;
} pair_double;

double get_density(list_t *list, int index, int len);

int ofm_max(list_t *list);

// find index of first 1-bit (least significant bit)
static inline int bsf_word(int word);

static inline int bsr_word(int word);

int get_depth(list_t *list, int len);

pair_int get_parent(list_t *list, int index, int len);

pair_double density_bound(list_t *list, int depth);


// Evenly redistribute elements in the ofm, given a range to look into
// index: starting position in ofm structure
// len: area to redistribute
void redistribute(list_t *list, int index, int len);

int isPowerOfTwo(int x);

void double_list(list_t *list);

void half_list(list_t *list);

void scan(list_t *list, int index, int len);

void slide_right(list_t *list, int index);

// given index, return the starting index of the leaf it is in
int find_leaf(list_t *list, int index);

// same as find_leaf, but does it for any level in the tree
// index: index in array
// len: length of sub-level.
int find_node(int index, int len);

int *find_elem_pointer(list_t *list, int index, int elem);

int *insert(list_t *list, int index, int elem);

int binary_search(list_t *list, int elem);

int *insert_sorted(list_t *list, int elem);

int find_index(list_t *list, int *elem_pointer);

// given an element pointer, find the next element index after it
int *get_next_elem_index(list_t *list, int *elem_pointer);

// given an element pointer, find previous element index after it
int *get_prev_elem_index(list_t *list, int *elem_pointer);

// given an element value and pointer to an element,
// insert before it.
int *insert_before(list_t *list, int new_elem, int *elem_pointer);

// returns pointer to the element we just inserted!
int *insert_after(list_t *list, int new_elem, int *elem_pointer);

int *get_first(list_t *list);

// insert as the first item into the list.
int *insert_first(list_t *list, int elem);

void delete_ofm(list_t *list, int index);

void delete_here(list_t *list, int *elem_pointer);

void print_array(list_t *list);

void setup(list_t *list);