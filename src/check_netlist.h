/*
 * check_netlist.h - checker definitions for the Qucs netlist
 *
 * Copyright (C) 2003, 2004 Stefan Jahn <stefan@lkcc.org>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 *
 * $Id: check_netlist.h,v 1.16 2004/10/29 18:01:29 ela Exp $
 *
 */

#ifndef __CHECK_NETLIST_H__
#define __CHECK_NETLIST_H__

#include "netdefs.h"

__BEGIN_DECLS

/* Externalize variables used by the scanner and parser. */
extern struct definition_t * definition_root;
extern struct node_t * node_root;
extern struct pair_t * pair_root;
extern int netlist_lineno;
extern FILE * netlist_in;

/* Available functions of the checker. */
void netlist_status (void);
void netlist_list (void);
void netlist_destroy (void);
int  netlist_checker (void);
int  netlist_parse (void);
int  netlist_error (char *);
int  netlist_lex (void);
int  netlist_checker_variables (void);

__END_DECLS

#endif /* __CHECK_NETLIST_H__ */
