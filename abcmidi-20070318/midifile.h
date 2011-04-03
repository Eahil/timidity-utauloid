/* definitions for MIDI file parsing code */
#include <stdio.h>
extern int (*Mf_getc)();
extern void (*Mf_header)();
extern void (*Mf_trackstart)();
extern void (*Mf_trackend)();
extern void (*Mf_noteon)();
extern void (*Mf_noteoff)();
extern void (*Mf_pressure)();
extern void (*Mf_parameter)();
extern void (*Mf_pitchbend)();
extern void (*Mf_program)();
extern void (*Mf_chanpressure)();
extern void (*Mf_sysex)();
extern void (*Mf_metamisc)();
extern void (*Mf_seqspecific)();
extern void (*Mf_seqnum)();
extern void (*Mf_text)();
extern void (*Mf_eot)();
extern void (*Mf_timesig)();
extern void (*Mf_smpte)();
extern void (*Mf_tempo)();
extern void (*Mf_keysig)();
extern void (*Mf_arbitrary)();
extern void (*Mf_error)();
extern long Mf_currtime;
extern int Mf_nomerge;

/* definitions for MIDI file writing code */
extern int (*Mf_putc)();
extern long (*Mf_writetrack)();
extern int (*Mf_writetempotrack)();
float mf_ticks2sec();
long mf_sec2ticks();
void mfwrite();
void mfread();
int mf_write_meta_event();
int mf_write_midi_event();
void mf_write_tempo();
void mferror();

/* MIDI status commands most significant bit is 1 */
#define note_off         	0x80
#define note_on          	0x90
#define poly_aftertouch  	0xa0
#define control_change    	0xb0
#define program_chng     	0xc0
#define channel_aftertouch      0xd0
#define pitch_wheel      	0xe0
#define system_exclusive      	0xf0
#define delay_packet	 	(1111)

/* 7 bit controllers */
#define damper_pedal            0x40
#define portamento	        0x41 	
#define sostenuto	        0x42
#define soft_pedal	        0x43
#define general_4               0x44
#define	hold_2		        0x45
#define	general_5	        0x50
#define	general_6	        0x51
#define general_7	        0x52
#define general_8	        0x53
#define tremolo_depth	        0x5c
#define chorus_depth	        0x5d
#define	detune		        0x5e
#define phaser_depth	        0x5f

/* parameter values */
#define data_inc	        0x60
#define data_dec	        0x61

/* parameter selection */
#define non_reg_lsb	        0x62
#define non_reg_msb	        0x63
#define reg_lsb		        0x64
#define reg_msb		        0x65

/* Standard MIDI Files meta event definitions */
#define	meta_event		0xFF
#define	sequence_number 	0x00
#define	text_event		0x01
#define copyright_notice 	0x02
#define sequence_name    	0x03
#define instrument_name 	0x04
#define lyric	        	0x05
#define marker			0x06
#define	cue_point		0x07
#define channel_prefix		0x20
#define	end_of_track		0x2f
#define	set_tempo		0x51
#define	smpte_offset		0x54
#define	time_signature		0x58
#define	key_signature		0x59
#define	sequencer_specific	0x74

/* Manufacturer's ID number */
#define Seq_Circuits (0x01) /* Sequential Circuits Inc. */
#define Big_Briar    (0x02) /* Big Briar Inc.           */
#define Octave       (0x03) /* Octave/Plateau           */
#define Moog         (0x04) /* Moog Music               */
#define Passport     (0x05) /* Passport Designs         */
#define Lexicon      (0x06) /* Lexicon 			*/
#define Tempi        (0x20) /* Bon Tempi                */
#define Siel         (0x21) /* S.I.E.L.                 */
#define Kawai        (0x41) 
#define Roland       (0x42)
#define Korg         (0x42)
#define Yamaha       (0x43)

/* miscellaneous definitions */
#define MThd 0x4d546864
#define MTrk 0x4d54726b
#define lowerbyte(x) ((unsigned char)(x & 0xff))
#define upperbyte(x) ((unsigned char)((x & 0xff00)>>8))

enum midi_event_t
{
	ME_NONE,
	
	/* MIDI events */
	ME_NOTEOFF,
	ME_NOTEON,
	ME_KEYPRESSURE,
	ME_PROGRAM,
	ME_CHANNEL_PRESSURE,
	ME_PITCHWHEEL,
	
	/* Controls */
	ME_TONE_BANK_MSB,
	ME_TONE_BANK_LSB,
	ME_MODULATION_WHEEL,
	ME_BREATH,
	ME_FOOT,
	ME_MAINVOLUME,
	ME_BALANCE,
	ME_PAN,
	ME_EXPRESSION,
	ME_SUSTAIN,
	ME_PORTAMENTO_TIME_MSB,
	ME_PORTAMENTO_TIME_LSB,
	ME_PORTAMENTO,
	ME_PORTAMENTO_CONTROL,
	ME_DATA_ENTRY_MSB,
	ME_DATA_ENTRY_LSB,
	ME_SOSTENUTO,
	ME_SOFT_PEDAL,
	ME_LEGATO_FOOTSWITCH,
	ME_HOLD2,
	ME_HARMONIC_CONTENT,
	ME_RELEASE_TIME,
	ME_ATTACK_TIME,
	ME_BRIGHTNESS,
	ME_REVERB_EFFECT,
	ME_TREMOLO_EFFECT,
	ME_CHORUS_EFFECT,
	ME_CELESTE_EFFECT,
	ME_PHASER_EFFECT,
	ME_RPN_INC,
	ME_RPN_DEC,
	ME_NRPN_LSB,
	ME_NRPN_MSB,
	ME_RPN_LSB,
	ME_RPN_MSB,
	ME_ALL_SOUNDS_OFF,
	ME_RESET_CONTROLLERS,
	ME_ALL_NOTES_OFF,
	ME_MONO,
	ME_POLY,
	
	/* TiMidity Extensionals */
#if 0
	ME_VOLUME_ONOFF,		/* Not supported */
#endif
	ME_SCALE_TUNING,		/* Scale tuning */
	ME_BULK_TUNING_DUMP,	/* Bulk tuning dump */
	ME_SINGLE_NOTE_TUNING,	/* Single-note tuning */
	ME_RANDOM_PAN,
	ME_SET_PATCH,			/* Install special instrument */
	ME_DRUMPART,
	ME_KEYSHIFT,
	ME_PATCH_OFFS,			/* Change special instrument sample position
							 * Channel, LSB, MSB
							 */
	
	/* Global channel events */
	ME_TEMPO,
	ME_CHORUS_TEXT,
	ME_LYRIC,
	ME_GSLCD,				/* GS L.C.D. Exclusive message event */
	ME_MARKER,
	ME_INSERT_TEXT,			/* for SC */
	ME_TEXT,
	ME_KARAOKE_LYRIC,		/* for KAR format */
	ME_MASTER_VOLUME,
	ME_RESET,				/* Reset and change system mode */
	ME_NOTE_STEP,
	
	ME_TIMESIG,				/* Time signature */
	ME_KEYSIG,				/* Key signature */
	ME_TEMPER_KEYSIG,		/* Temperament key signature */
	ME_TEMPER_TYPE,			/* Temperament type */
	ME_MASTER_TEMPER_TYPE,	/* Master temperament type */
	ME_USER_TEMPER_ENTRY,	/* User-defined temperament entry */
	
	ME_SYSEX_LSB,			/* Universal system exclusive message (LSB) */
	ME_SYSEX_MSB,			/* Universal system exclusive message (MSB) */
	ME_SYSEX_GS_LSB,		/* GS system exclusive message (LSB) */
	ME_SYSEX_GS_MSB,		/* GS system exclusive message (MSB) */
	ME_SYSEX_XG_LSB,		/* XG system exclusive message (LSB) */
	ME_SYSEX_XG_MSB,		/* XG system exclusive message (MSB) */
	
	ME_WRD,					/* for MIMPI WRD tracer */
	ME_SHERRY,				/* for Sherry WRD tracer */
	ME_BARMARKER,
	ME_STEP,				/* for Metronome */
	
	ME_LAST = 254,			/* Last sequence of MIDI list.
							 * This event is reserved for realtime player.
							 */
	ME_EOT = 255			/* End of MIDI.  Finish to play */
};

