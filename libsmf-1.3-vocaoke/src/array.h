/*
 Copying and distribution of this file, with or without modification,
 are permitted in any medium without royalty provided the copyright
 notice and this notice are preserved.  This file is offered as-is,
 without any warranty.
 */
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
#define array_add_end array_add
void array_add_middle(array* a,void* d,int i);
void* array_index(array*,int);
void array_free(array*,bool);
void array_remove(array*,void*);
void array_sort(array*,int (*)(void*,void*));
