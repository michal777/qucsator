/*
 * attenuator.cpp - attenuator class implementation
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
 * $Id: attenuator.cpp,v 1.13 2004/11/24 19:15:47 raimi Exp $
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "complex.h"
#include "object.h"
#include "node.h"
#include "circuit.h"
#include "matrix.h"
#include "constants.h"
#include "component_id.h"
#include "attenuator.h"

attenuator::attenuator () : circuit (2) {
  type = CIR_ATTENUATOR;
}

void attenuator::initSP (void) {
  allocMatrixS ();
  nr_double_t a = getPropertyDouble ("L");
  nr_double_t z = getPropertyDouble ("Zref");
  nr_double_t r = (z - z0) / (z + z0);
  nr_double_t s11 = r * (1 - a) / (a - r * r);
  nr_double_t s21 = sqrt (a) * (1 - r * r) / (a - r * r);
  setS (1, 1, s11);
  setS (2, 2, s11);
  setS (1, 2, s21);
  setS (2, 1, s21);
}

void attenuator::calcNoise (nr_double_t) {
  nr_double_t T = getPropertyDouble ("Temp");
  nr_double_t l = getPropertyDouble ("L");
  nr_double_t z = getPropertyDouble ("Zref");
  nr_double_t r = (z - z0) / (z + z0);
  nr_double_t f = (l - 1) * (r * r - 1) / sqr (l - r * r) * kelvin (T) / T0;
  setN (1, 1, -f * (r * r + l));
  setN (2, 2, -f * (r * r + l));
  setN (1, 2, +f * 2 * r * sqrt (l));
  setN (2, 1, +f * 2 * r * sqrt (l));
}

void attenuator::initDC (void) {
  nr_double_t a = getPropertyDouble ("L");
  if (a == 1.0) { // no attenuation
    setVoltageSources (1);
    allocMatrixMNA ();
    voltageSource (1, 1, 2);
  }
  else { // compute Z-parameters
    setVoltageSources (2);
    allocMatrixMNA ();
    nr_double_t zref = getPropertyDouble ("Zref");
    nr_double_t z11 = zref * (a + 1) / (a - 1);
    nr_double_t z21 = zref * (sqrt (a) * 2) / (a - 1);
    setB (1, 1, +1.0); setB (1, 2, +0.0); setB (2, 1, +0.0); setB (2, 2, +1.0);
    setC (1, 1, -1.0); setC (1, 2, +0.0); setC (2, 1, +0.0); setC (2, 2, -1.0);
    setD (1, 1, +z11); setD (2, 2, +z11); setD (1, 2, +z21); setD (2, 1, +z21);
    setE (1, +0.0); setE (2, +0.0);
  }
  clearY ();
}

void attenuator::initAC (void) {
  nr_double_t a = getPropertyDouble ("L");

  if (a == 1.0) { // no attenuation
    setVoltageSources (1);
    allocMatrixMNA ();
    clearY ();
    voltageSource (1, 1, 2);
  }
  else { // compute Z-parameters
    setVoltageSources (0);
    allocMatrixMNA ();
    nr_double_t zref = getPropertyDouble ("Zref");
    nr_double_t z11 = zref * (a + 1) / (a - 1);
    nr_double_t z21 = zref * (sqrt (a) * 2) / (a - 1);
  
    // build Z-parameter matrix and convert it to Y-parameters
    matrix z (2);
    z.set (1, 1, z11); z.set (2, 2, z11);
    z.set (1, 2, z21); z.set (2, 1, z21);
    setMatrixY (ztoy (z));
  }
}

void attenuator::initTR (void) {
  initDC ();
}
