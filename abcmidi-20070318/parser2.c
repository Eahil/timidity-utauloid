/* parser2.c - further parsing */
/* part of abc2midi and yaps */
/* implements event_info, event_instruction, event_gchord and event_slur */
/* also implements event_reserved */
/* parser2 requires parseabc, but parseabc does not need parser2 */

#include <stdio.h>
#include "abc.h"
#include "parseabc.h"
#include "parser2.h"

void event_info(s)
char* s;
/* An I: field has been encountered. This routine scans the following */
/* text to extract items of the form key=value  */
{
  char* key = NULL;
  char* endkey = NULL;
  char* value = NULL;
  char* endvalue =  NULL;
  char* lastendvalue;
  char* newword = NULL;
  char* lastnewword;
  char* ptr;
  int doval;

  ptr = s;
  doval = 0;
  while (*ptr != '\0') {
    if (doval == 0) {
      /* look for key */
      skipspace(&ptr);
      key = ptr;
      while ((*ptr != '\0') && (*ptr != ' ') && (*ptr != '=')) {
        ptr = ptr + 1;
      };
      endkey = ptr;
      skipspace(&ptr);
      if (*ptr == '=') {
        doval = 1;
        ptr = ptr + 1;
        skipspace(&ptr);
        value = ptr;
        newword = ptr;
        endvalue = NULL;
        lastendvalue = NULL;
      } else {
        key = ptr;
      };
    } else {
      /* look for value */
      skipspace(&ptr);
      while ((*ptr != '\0') && (*ptr != ' ') && (*ptr != '=')) {
        ptr = ptr + 1;
      };
      lastendvalue = endvalue;
      endvalue = ptr;
      skipspace(&ptr);
      lastnewword = newword;
      newword = ptr;
      if (*ptr == '\0') {
        *endkey = '\0';
        *endvalue = '\0';
        event_info_key(key, value);
      } else {
        if (*ptr == '=') {
          *endkey = '\0';
          if (lastendvalue == NULL) {
            event_error("missing key or value in I: field");
          } else {
            *lastendvalue = '\0';
            event_info_key(key, value);
          };
          key = lastnewword;
          endkey = endvalue; 
          doval = 1;
          ptr = ptr + 1;
          skipspace(&ptr);
          value = ptr;
          endvalue = NULL;
          lastendvalue = NULL;
        };
      };
    };
  };
}


static void splitstring(s, sep, handler)
/* breaks up string into fields with sep as the field separator */
/* and calls handler() for each sub-string */
char* s;
char sep;
void (*handler)();
{
  char* out;
  char* p;
  int fieldcoming;

  p = s;
  fieldcoming = 1;
  while (fieldcoming) {
    out = p;
    while ((*p != '\0') && (*p != sep)) p = p + 1;
    if (*p == sep) {
      *p = '\0';
      p = p + 1;
    } else {
      fieldcoming = 0;
    };
    (*handler)(out);
  };
}

void event_gchord(s)
/* handles guitar chords " ... " */
char* s;
{
  splitstring(s, ';', event_handle_gchord);
}

void event_instruction(s)
/* handles a ! ... ! event in the abc */
char* s;
{
  splitstring(s, ';', event_handle_instruction);
}

void event_slur(t)
int t;
/* handles old 's' notation for slur on/slur off */
{
  if (t) {
    event_sluron(1);
  } else {
    event_sluroff(0);
  };
}

void event_reserved(char s)
/* handles H - Z encountered in abc */
{
  char *expansion;

  expansion = lookup_abbreviation(s);
  if (expansion != NULL) {
    event_handle_instruction(expansion);
  } else {
    event_x_reserved(s);
  };
}
