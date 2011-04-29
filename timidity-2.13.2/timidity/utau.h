#ifndef ___UTAU_H_
#define ___UTAU_H_
void utau_init();//loads voicebank
Sample* utau_get_sample(int *count);//returns sample from voicebank
void utau_set_text(char* text);
char* utau_get_text();

void utau_prescan_on(MidiEvent* e);	//prescan << oto.ini
void utau_prescan_off(MidiEvent* e);
void utau_prescan_lyr(char* t);

#endif
