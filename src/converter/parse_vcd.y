/* -*-c++-*- */

%{
/*
 * parse_vcd.y - parser for a VCD data file
 *
 * Copyright (C) 2005 Raimund Jacob <raimi@lkcc.org>
 * Copyright (C) 2006 Stefan Jahn <stefan@lkcc.org>
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
 * $Id: parse_vcd.y,v 1.3 2006-01-09 09:11:07 raimi Exp $
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define YYERROR_VERBOSE 42
#define YYDEBUG 1
#define YYMAXDEPTH 1000000

#include "check_vcd.h"

%}

%name-prefix="vcd_"

%token t_END
%token t_COMMENT
%token t_DATE
%token t_ENDDEFINITIONS
%token t_SCOPE
%token t_TIMESCALE
%token t_UPSCOPE
%token t_VAR
%token t_VERSION
%token t_DUMPALL
%token t_DUMPOFF
%token t_DUMPON
%token t_DUMPVARS

%token s_MODULE s_TASK s_FUNCTION s_FORK s_BEGIN
%token ONE B Z ZERO HASHMARK X R TEN HUNDRET

%token PICO MICRO NANO FEMTO SECOND MILLI

%token EVENT INTEGER PARAMETER REAL REG SUPPLY0 SUPPLY1 TIME TRI TRIAND
%token TRIOR TRIREG TRI0 TRI1 WAND WIRE WOR

%token Real
%token Binary
%token PositiveInteger
%token Identifier     /* proper identifiers */
%token IdentifierCode /* shortcuts used in the dump */
%token Reference
%token InvalidCharacter

%union {
  char * ident;
  char * value;
  int integer;
  double real;
  enum vcd_vartypes vtype;
  enum vcd_scopes stype;
  struct vcd_vardef * vardef;
  struct vcd_change * change;
  struct vcd_scope * scope;
  struct vcd_changeset * changeset;
  struct vcd_range * range;
}

%type <ident> Identifier IdentifierCode Reference
%type <value> Value ZERO ONE Z X Binary Real
%type <integer> Size PositiveInteger TimeScale SimulationTime
%type <real> TimeUnit
%type <change> ScalarValueChange VectorValueChange ValueChangeList ValueChange
%type <changeset> ValueChangeset
%type <scope> ScopeDeclaration
%type <range> BitSelect
%type <vardef> VarDeclaration
%type <vtype> VarType
%type <stype> ScopeType

%%

ValueChangeDumpDefinitions:
     DeclarationList SimulationCommandList
;

DeclarationList: /* empty */
   | Declaration DeclarationList
;

Declaration:
     t_COMMENT t_END
   | t_DATE t_END
   | t_ENDDEFINITIONS t_END
   | t_SCOPE ScopeDeclaration t_END {
     $2->next = vcd->scopes;
     vcd->scopes = $2;
   }
   | t_TIMESCALE TimeScaleDeclaration t_END
   | t_UPSCOPE t_END
   | t_VERSION t_END
   | t_VAR VarDeclaration t_END {
     $2->next = vcd->scopes->vardefs;
     $2->scope = vcd->scopes;
     vcd->scopes->vardefs = $2;
   }
;

ScopeDeclaration:
   ScopeType Identifier {
     $$ = (struct vcd_scope *) calloc (1, sizeof (struct vcd_scope));
     $$->type = $1;
     $$->ident = $2;
   }
;

ScopeType:
     s_MODULE   { $$ = SCOPE_MODULE;   }
   | s_TASK     { $$ = SCOPE_TASK;     }
   | s_FUNCTION { $$ = SCOPE_FUNCTION; }
   | s_BEGIN    { $$ = SCOPE_BEGIN;    }
   | s_FORK     { $$ = SCOPE_FORK;     }
;

TimeScaleDeclaration:
   TimeScale TimeUnit {
     vcd->t = $1;
     vcd->scale = $2;
   }
;

TimeScale:
     ONE     { $$ = 1;   }
   | TEN     { $$ = 10;  }
   | HUNDRET { $$ = 100; }
;

TimeUnit:
     SECOND { $$ = 1;     }
   | MILLI  { $$ = 1e-3;  }
   | MICRO  { $$ = 1e-6;  }
   | NANO   { $$ = 1e-9;  }
   | PICO   { $$ = 1e-12; }
   | FEMTO  { $$ = 1e-15; }
;

VarDeclaration:
   VarType Size IdentifierCode Reference BitSelect {
     $$ = (struct vcd_vardef *) calloc (1, sizeof (struct vcd_vardef));
     $$->type = $1;
     $$->size = $2;
     $$->code = $3;
     $$->ident = $4;
     $$->range = $5;
   }
;

BitSelect: /* nothing */ { $$ = NULL; }
    | '[' PositiveInteger ']' {
      $$ = (struct vcd_range *) calloc (1, sizeof (struct vcd_range));
      $$->l = -1;
      $$->h = $2;
    }
    | '[' PositiveInteger ':' PositiveInteger ']' {
      $$ = (struct vcd_range *) calloc (1, sizeof (struct vcd_range));
      $$->l = $2;
      $$->h = $4;
    }
;

VarType:
     EVENT     { $$ = VAR_EVENT;     }
   | INTEGER   { $$ = VAR_INTEGER;   }
   | PARAMETER { $$ = VAR_PARAMETER; }
   | REAL      { $$ = VAR_REAL;      }
   | REG       { $$ = VAR_REG;       }
   | SUPPLY0   { $$ = VAR_SUPPLY0;   }
   | SUPPLY1   { $$ = VAR_SUPPLY1;   }
   | TIME      { $$ = VAR_TIME;      }
   | TRI       { $$ = VAR_TRI;       }
   | TRIAND    { $$ = VAR_TRIAND;    }
   | TRIOR     { $$ = VAR_TRIOR;     }
   | TRIREG    { $$ = VAR_TRIREG;    }
   | TRI0      { $$ = VAR_TRI0;      }
   | TRI1      { $$ = VAR_TRI1;      }
   | WAND      { $$ = VAR_WAND;      }
   | WIRE      { $$ = VAR_WIRE;      }
   | WOR       { $$ = VAR_WOR;       }
;

Size:
   PositiveInteger
;

SimulationCommandList: /* empty */
   | SimulationCommand SimulationCommandList
;

SimulationCommand:
    t_DUMPALL  ValueChangeList t_END /* probably unsupported */
  | t_DUMPOFF  ValueChangeList t_END /* probably unsupported */
  | t_DUMPON   ValueChangeList t_END /* probably unsupported */
  | t_DUMPVARS ValueChangeList t_END {
      vcd->changesets->changes = $2;
  }
  | ValueChangeset {
      $1->next = vcd->changesets;
      vcd->changesets = $1;
  }
;

ValueChangeset:
    SimulationTime ValueChangeList {
      $$ = (struct vcd_changeset *) calloc (1, sizeof (struct vcd_changeset));
      $$->t = $1;
      $$->changes = $2;
    }
;

SimulationTime:
    HASHMARK PositiveInteger {
      $$ = $2;
    }
;

ValueChangeList: /* nothing */ { $$ = NULL; }
    | ValueChange ValueChangeList {
      $1->next = $2;
    }
;

ValueChange:
    ScalarValueChange
  | VectorValueChange
;

ScalarValueChange:
    Value IdentifierCode {
      $$ = (struct vcd_change *) calloc (1, sizeof (struct vcd_change));
      $$->value = $1;
      $$->code = $2;
    }
;

Value:
    ZERO
  | ONE
  | X
  | Z
;

VectorValueChange:
    'B' Binary IdentifierCode {
      $$ = (struct vcd_change *) calloc (1, sizeof (struct vcd_change));
      $$->value = $2;
      $$->code = $3;
    }
    | 'R' Real IdentifierCode {
      $$ = (struct vcd_change *) calloc (1, sizeof (struct vcd_change));
      $$->value = $2;
      $$->code = $3;
      $$->isreal = 1;
    }
;


%%

int vcd_error (char * error) {
  fprintf (stderr, "line %d: %s\n", vcd_lineno, error);
  return 0;
}
