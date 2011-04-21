#ifndef ___UTAU_H_
#define ___UTAU_H_
void utau_init();
Sample* utau_get_sample(int *count);
void utau_set_text(char* text);
char* utau_get_text();
void utau_prescan_on(int t);
void utau_prescan_off(int t);
void utau_prescan_lyr(char* t);
void utau_mix(Sample* samp,int i);
#endif
