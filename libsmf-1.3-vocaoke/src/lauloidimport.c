#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef __MINGW32__
#define EX_OK 0
#define EX_USAGE 64
#else /* ! __MINGW32__ */
#include <sysexits.h>
#endif /* ! __MINGW32__ */
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "smf.h"
#include <math.h>

void
write_lyric (smf_track_t * track, int pulses)
{
  static int i=0;
  i++;	
  char lyr[10];
  sprintf(lyr,"%i",i);	
  smf_event_t *event = smf_event_new_textual (SMF_TEXT_TYPE_LYRIC, lyr);
  smf_track_add_event_pulses (track, event, pulses);
}

void
write_event (smf_track_t * track, void *data, int pulses)
{
  smf_event_t *event = smf_event_new_from_pointer (data, 3);
  smf_track_add_event_pulses (track, event, pulses);
}

void
write_tempo (smf_track_t * track, smf_tempo_t * tempo,int pulses)
{
  static int i;
  i++;
  g_message
    ("Tempo #%d: Starts at %d pulses, %f seconds, setting %d microseconds per quarter note, %.2f BPM.",
     i, tempo->time_pulses, tempo->time_seconds,
     tempo->microseconds_per_quarter_note,
     60000000.0 / (double) tempo->microseconds_per_quarter_note);
  g_message
    ("Time signature: %d/%d, %d clocks per click, %d 32nd notes per quarter note.",
     tempo->numerator, tempo->denominator, tempo->clocks_per_click,
     tempo->notes_per_note);

  char data[7];
  //FF 58 04 nn dd cc bb
  data[0] = 0xFF;
  data[1] = 0x58;
  data[2] = 0x04;
  data[3] = tempo->numerator;			
  data[4] = (int)log2(tempo->denominator);			
  data[5] = tempo->clocks_per_click;
  data[6] = tempo->notes_per_note;
  
  
  smf_event_t *timesig = smf_event_new_from_pointer (&data, 7);

  assert (smf_event_is_valid (timesig));
  int musec = tempo->microseconds_per_quarter_note;	

  //FF 51 03 tttttt
  data[1] = 0x51;
  data[2] = 0x03;
  data[3] = (musec >> 16 & 0xFF);
  data[4] = (musec >> 8 & 0xFF);
  data[5] = (musec & 0xFF);
  smf_event_t *tempo_evt = smf_event_new_from_pointer (&data, 6);
  assert (smf_event_is_valid (tempo_evt));

  printf ("%s\n", smf_event_decode (timesig));
  printf ("%s\n", smf_event_decode (tempo_evt));
  smf_track_add_event_pulses (track, timesig, pulses);
  smf_track_add_event_pulses (track, tempo_evt, pulses);
}


int
main (int argc, char **argv)
{

  smf_t *smf_in = smf_load (argv[1]);
  smf_t *smf_out = smf_new ();

  smf_track_t *track_in = smf_get_track_by_number (smf_in, atoi (argv[2]));
  smf_track_t *track_out = smf_track_new ();
  smf_add_track (smf_out, track_out);

  smf_event_t *event;
  smf_tempo_t *oldtempo = 0;

  int i = 0;
  smf_set_ppqn(smf_out,smf_in->ppqn);




  while ((event = smf_track_get_next_event (track_in)) != NULL)
    {
      if (!smf_event_is_metadata (event))
	{
	  if (event->midi_buffer_length == 3)
	    {

	      if (event->midi_buffer[0] == 0x90)
		write_lyric (track_out, event->time_pulses);

	      write_event (track_out, event->midi_buffer, event->time_pulses);
	      smf_tempo_t *newtempo =
		smf_get_tempo_by_pulses (smf_in, event->time_pulses);

	      if (newtempo != oldtempo)
		{
		  oldtempo = newtempo;
		  write_tempo(track_out,oldtempo,event->time_pulses);




		}
	    }
	}
    }
  //copy_tempo(smf_in,smf_out);
  smf_save (smf_out, argv[3]);

}
