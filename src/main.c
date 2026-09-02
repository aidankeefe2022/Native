#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include <native/native.h>

static void
usage (const char *prog)
{
  printf ("Usage: %s [OPTION]...\n"
          "\n"
          "  -h, --help     display this help and exit\n"
          "  -V, --version  output version information and exit\n",
          prog);
}

int
main (int argc, char **argv)
{
  for (int i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "-h") == 0 || strcmp (argv[i], "--help") == 0)
        {
          usage (argv[0]);
          return 0;
        }
      if (strcmp (argv[i], "-V") == 0 || strcmp (argv[i], "--version") == 0)
        {
          printf ("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);
          return 0;
        }
      fprintf (stderr, "%s: unexpected argument '%s'\n", argv[0], argv[i]);
      usage (argv[0]);
      return 1;
    }

  return native_run ();
}
