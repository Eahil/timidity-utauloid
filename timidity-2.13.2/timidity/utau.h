#ifndef ___UTAU_H_
#define ___UTAU_H_
void utau_init();
SpecialPatch* utau_special_patch();
//for debugging
utau_hack_sample(Sample*);
void karaoke_write_wave(short* samples,int length,int sample_rate,char* filename);
#endif
