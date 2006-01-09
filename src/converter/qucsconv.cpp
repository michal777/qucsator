/*
 * qucsconv.cpp - main converter program implementation
 *
 * Copyright (C) 2004, 2005, 2006 Stefan Jahn <stefan@lkcc.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * $Id: qucsconv.cpp,v 1.13 2006-01-09 09:11:07 raimi Exp $
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "check_spice.h"
#include "check_vcd.h"
#include "qucs_producer.h"

/* structure defining a conversion */
struct actionset_t {
  char * in;  /* -if parameter */
  char * out; /* -of parameter */

  /* callback for the logic, return error code of application */
  int (* execute) (struct actionset_t *, char * infile, char * outfile);
};

/* required forward declarations */
int spice2qucs (struct actionset_t *, char *, char *);
int vcd2qucs (struct actionset_t *, char *, char *);

/* conversion definitions */
struct actionset_t actionset[] = {
  { "spice", "qucs",    spice2qucs },
  { "spice", "qucslib", spice2qucs },
  { "vcd",   "qucsdata",  vcd2qucs },
  { NULL, NULL, NULL}
};

/* opens the given file, fallback to stdin/stdout */
FILE * open_file (char * file, char * flag) {
  FILE * fd = NULL;
  if (file) {
    if ((fd = fopen (file, flag)) == NULL) {
      fprintf (stderr, "cannot open file `%s': %s, using %s instead\n",
	       file, strerror (errno), flag[0] == 'r' ? "stdin" : "stdout");
      fd = flag[0] == 'r' ? stdin : stdout;
    }
  }
  else {
    fd = flag[0] == 'r' ? stdin : stdout;
  }
  return fd;
}

/* main entry point */
int main (int argc, char ** argv) {

  char * infile = NULL, * outfile = NULL, * input = NULL, * output = NULL;

  // check program arguments
  for (int i = 1; i < argc; i++) {
    if (!strcmp (argv[i], "-v") || !strcmp (argv[i], "--version")) {
      fprintf (stdout,
	"QucsConverter " PACKAGE_VERSION "\n"
	"Copyright (C) 2004, 2005, 2006 Stefan Jahn <stefan@lkcc.org>\n"
	"\nThis is free software; see the source for copying "
	"conditions.  There is NO\n"
	"warranty; not even for MERCHANTABILITY or FITNESS FOR A "
	"PARTICULAR PURPOSE.\n");
      return 0;
    }
    if (!strcmp (argv[i], "-h") || !strcmp (argv[i], "--help")) {
      fprintf (stdout,
	"Usage: %s [OPTION]...\n\n"
	"  -h, --help      display this help and exit\n"
	"  -v, --version   display version information and exit\n"
	"  -i  FILENAME    use file as input file (default stdin)\n"
	"  -o  FILENAME    use file as output file (default stdout)\n"
	"  -if FORMAT      input data specification (e.g. spice)\n"
	"  -of FORMAT      output data specification (e.g. qucs)\n"
	"  -a, --noaction  do not include netlist actions in the output\n"
	"  -g  GNDNODE     replace ground node\n"
	"\nReport bugs to <" PACKAGE_BUGREPORT ">.\n", argv[0]);
      return 0;
    }
    else if (!strcmp (argv[i], "-i")) {
      infile = argv[++i];
    }
    else if (!strcmp (argv[i], "-o")) {
      outfile = argv[++i];
    }
    else if (!strcmp (argv[i], "-if")) {
      input = argv[++i];
    }
    else if (!strcmp (argv[i], "-of")) {
      output = argv[++i];
    }
    else if (!strcmp (argv[i], "-a") || !strcmp (argv[i], "--noaction")) {
      qucs_actions = 0;
    }
    else if (!strcmp (argv[i], "-g")) {
      if (argv[++i]) qucs_gnd = argv[i];
    }
  }

  // check input/output formats
  int infound = 0;
  int outfound = 0;
  for (int j = 0; actionset[j].in != NULL; j++) {
    int in = 0, out = 0;
    if (input && !strcmp (input, actionset[j].in)) {
      in = infound = 1;
    }
    if (output && !strcmp (output, actionset[j].out)) {
      out = outfound = 1;
    }
    if (in && out) {
      return actionset[j].execute (&actionset[j], infile, outfile);
    }
  }

  // no appropriate conversion found
  if (!infound) {
    fprintf (stderr, "invalid input data specification `%s'\n",
	     input ? input : "not given");
  }
  if (!outfound) {
    fprintf (stderr, "invalid output data specification `%s'\n",
	     output ? output : "not given");
  }
  fprintf (stderr, "invalid input/output data specification `%s->%s'\n",
	   input ? input : "not given", output ? output : "not given");
  return -1;
}

// SPICE to Qucs conversion.
int spice2qucs (struct actionset_t * action, char * infile, char * outfile) {
  int ret = 0;
  if ((spice_in = open_file (infile, "r")) == NULL) {
    ret = -1;
  } else if (spice_parse () != 0) {
    ret = -1;
  } else if (spice_checker () != 0) {
    ret = -1;
  }
  spice_lex_destroy ();
  fclose (spice_in);
  if (ret) {
    spice_destroy ();
    return -1;
  }

  if ((qucs_out = open_file (outfile, "w")) == NULL)
    return -1;
  if (!strcmp (action->out, "qucs"))
    qucs_producer ();
  else /* "qucslib" */
    qucslib_producer ();
  fclose (qucs_out);
  spice_destroy ();
  return 0;
}

// VCD to Qucs conversion.
int vcd2qucs (struct actionset_t * action, char * infile, char * outfile) {
  int ret = 0;
  vcd_init ();
  if ((vcd_in = open_file (infile, "r")) == NULL) {
    ret = -1;
  } else if (vcd_parse () != 0) {
    ret = -1;
  } else if (vcd_checker () != 0) {
    ret = -1;
  }
  vcd_lex_destroy ();
  fclose (vcd_in);
  if (ret) {
    vcd_destroy ();
    return -1;
  }

  if ((qucs_out = open_file (outfile, "w")) == NULL)
    return -1;
  if (!strcmp (action->out, "qucsdata"))
    qucsdata_producer ();
  fclose (qucs_out);
  vcd_destroy ();
  return 0;
}
