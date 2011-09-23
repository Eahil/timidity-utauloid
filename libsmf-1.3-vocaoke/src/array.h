typedef struct
{
	void** data;
	int len;
	int size;
	
}
array;

#define g_critical printf

typedef int bool;

#define TRUE 1
#define FALSE 0

array* array_new();
void array_add(array*,void*);
void* array_index(array*,int);
void array_free(array*,bool);
void array_remove(array*,void*);
void array_sort(array*,int (*)(void*,void*));
