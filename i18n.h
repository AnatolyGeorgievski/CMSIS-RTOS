#ifndef I18N_H
#define I18N_H
#include <stdint.h>
/*
	\see https://www.gnu.org/software/gettext/manual/

The initialization code for a program was

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

For a library it is reduced to

  bindtextdomain (PACKAGE, LOCALEDIR);


The usual declaration of the ‘_’ macro in each source file was

#include <libintl.h>
#define _(String) gettext (String)

for a program. For a library, which has its own translation domain, it reads like this:

#include <libintl.h>
#define _(String) dgettext (PACKAGE, String)

 */
//#include <libintl.h>
static inline const char* gettext(const char* s) {
	return s;
}
#define _(s) gettext(s)
// (const char* __attribute__ ((section (".gettext"))))(s) 

extern char* g_utf8_next_char(const char* str);
extern char* g_utf8_prev_char(const char* str);
extern char* g_utf8_offset_to_pointer  (const char *str, uint32_t offset);
extern uint_least32_t g_utf8_get_char (const char* str);
extern int32_t g_unichar_to_utf8(uint_least32_t , char* str);
#define UTF8_LENGTH(Char)              \
  ((Char) < 0x80 ? 1 :                 \
   ((Char) < 0x800 ? 2 :               \
    ((Char) < 0x10000 ? 3 :            \
     ((Char) < 0x200000 ? 4 :          \
      ((Char) < 0x4000000 ? 5 : 6)))))

#endif// I18N_H
