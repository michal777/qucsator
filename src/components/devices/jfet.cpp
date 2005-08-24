/*
 * jfet.cpp - jfet class implementation
 *
 * Copyright (C) 2004, 2005 Stefan Jahn <stefan@lkcc.org>
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
 * $Id: jfet.cpp,v 1.27 2005/08/24 07:10:46 raimi Exp $
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "complex.h"
#include "matrix.h"
#include "object.h"
#include "node.h"
#include "circuit.h"
#include "net.h"
#include "component_id.h"
#include "constants.h"
#include "device.h"
#include "jfet.h"

#define NODE_G 0 /* gate node   */
#define NODE_D 1 /* drain node  */
#define NODE_S 2 /* source node */

jfet::jfet () : circuit (3) {
  rs = rd = NULL;
  type = CIR_JFET;
}

void jfet::calcSP (nr_double_t frequency) {
  setMatrixS (ytos (calcMatrixY (frequency)));
}

matrix jfet::calcMatrixY (nr_double_t frequency) {

  // fetch computed operating points
  nr_double_t Cgd = getOperatingPoint ("Cgd");
  nr_double_t Cgs = getOperatingPoint ("Cgs");
  nr_double_t ggs = getOperatingPoint ("ggs");
  nr_double_t ggd = getOperatingPoint ("ggd");
  nr_double_t gds = getOperatingPoint ("gds");
  nr_double_t gm  = getOperatingPoint ("gm");

  // compute the models admittances
  complex Ygd = rect (ggd, 2.0 * M_PI * frequency * Cgd);
  complex Ygs = rect (ggs, 2.0 * M_PI * frequency * Cgs);
  complex Yds = gds;

  // build admittance matrix and convert it to S-parameter matrix
  matrix y (3);
  y.set (NODE_G, NODE_G, Ygd + Ygs);
  y.set (NODE_G, NODE_D, -Ygd);
  y.set (NODE_G, NODE_S, -Ygs);
  y.set (NODE_D, NODE_G, gm - Ygd);
  y.set (NODE_D, NODE_D, Ygd + Yds);
  y.set (NODE_D, NODE_S, -Yds - gm);
  y.set (NODE_S, NODE_G, -Ygs - gm);
  y.set (NODE_S, NODE_D, -Yds);
  y.set (NODE_S, NODE_S, Ygs + Yds + gm);
  return y;
}

void jfet::calcNoiseSP (nr_double_t frequency) {
  setMatrixN (cytocs (calcMatrixCy (frequency) * z0, getMatrixS ()));
}

matrix jfet::calcMatrixCy (nr_double_t frequency) {
  /* get operating points and noise properties */
  nr_double_t Kf  = getPropertyDouble ("Kf");
  nr_double_t Af  = getPropertyDouble ("Af");
  nr_double_t Ffe = getPropertyDouble ("Ffe");
  nr_double_t gm  = fabs (getOperatingPoint ("gm"));
  nr_double_t Ids = fabs (getOperatingPoint ("Id"));
  nr_double_t T   = getPropertyDouble ("Temp");

  /* compute channel noise and flicker noise generated by the DC
     transconductance and current flow from drain to source */
  nr_double_t i = 8 * kelvin (T) / T0 * gm / 3 +
    Kf * pow (Ids, Af) / pow (frequency, Ffe) / kB / T0;

  /* build noise current correlation matrix and convert it to
     noise-wave correlation matrix */
  matrix cy = matrix (3);
  cy.set (NODE_D, NODE_D, +i);
  cy.set (NODE_S, NODE_S, +i);
  cy.set (NODE_D, NODE_S, -i);
  cy.set (NODE_S, NODE_D, -i);
  return cy;
}

void jfet::initDC (void) {

  // allocate MNA matrices
  allocMatrixMNA ();

  // initialize starting values
  UgdPrev = real (getV (NODE_G) - getV (NODE_D));
  UgsPrev = real (getV (NODE_G) - getV (NODE_S));

  // apply polarity of JFET
  char * type = getPropertyString ("Type");
  pol = !strcmp (type, "pfet") ? -1 : 1;

  // get device temperature
  nr_double_t T = getPropertyDouble ("Temp");

  // possibly insert series resistance at source
  nr_double_t Rs = getPropertyDouble ("Rs");
  if (Rs != 0.0) {
    // create additional circuit if necessary and reassign nodes
    rs = splitResistance (this, rs, this->getNet (), "Rs", "source", NODE_S);
    rs->setProperty ("Temp", T);
    rs->setProperty ("R", Rs);
    rs->initDC ();
  }
  // no series resistance at source
  else {
    disableResistance (this, rs, getNet (), NODE_S);
  }

  // possibly insert series resistance at drain
  nr_double_t Rd = getPropertyDouble ("Rd");
  if (Rd != 0.0) {
    // create additional circuit if necessary and reassign nodes
    rd = splitResistance (this, rd, getNet (), "Rd", "drain", NODE_D);
    rd->setProperty ("Temp", T);
    rd->setProperty ("R", Rd);
    rd->initDC ();
  }
  // no series resistance at drain
  else {
    disableResistance (this, rd, getNet (), NODE_D);
  }
}

void jfet::calcDC (void) {

  // fetch device model parameters
  nr_double_t Is   = getPropertyDouble ("Is");
  nr_double_t n    = getPropertyDouble ("N");
  nr_double_t Isr  = getPropertyDouble ("Isr");
  nr_double_t nr   = getPropertyDouble ("Nr");
  nr_double_t Vt0  = getPropertyDouble ("Vt0");
  nr_double_t l    = getPropertyDouble ("Lambda");
  nr_double_t beta = getPropertyDouble ("Beta");
  nr_double_t T    = getPropertyDouble ("Temp");

  nr_double_t Ugs, Ugd, Ut, IeqG, IeqD, IeqS, UgsCrit, UgdCrit;
  nr_double_t Uds, Igs, Igd, gtiny;

  T = kelvin (T);
  Ut = T * kB / Q;
  Ugd = real (getV (NODE_G) - getV (NODE_D)) * pol;
  Ugs = real (getV (NODE_G) - getV (NODE_S)) * pol;

  // critical voltage necessary for bad start values
  UgsCrit = pnCriticalVoltage (Is, Ut * n);
  UgdCrit = pnCriticalVoltage (Is, Ut * n);
  UgsPrev = Ugs = pnVoltage (Ugs, UgsPrev, Ut * n, UgsCrit);
  UgdPrev = Ugd = pnVoltage (Ugd, UgdPrev, Ut * n, UgdCrit);

  Uds = Ugs - Ugd;

  // gate-source diode
  gtiny = Ugs < - 10 * Ut * n ? (Is + Isr) : 0;
  ggs = pnConductance (Ugs, Is, Ut * n) +
    pnConductance (Ugs, Isr, Ut * nr) + gtiny;
  Igs = pnCurrent (Ugs, Is, Ut * n) +
    pnCurrent (Ugs, Isr, Ut * nr) + gtiny * Ugs;

  // gate-drain diode
  gtiny = Ugd < - 10 * Ut * n ? (Is + Isr) : 0;
  ggd = pnConductance (Ugd, Is, Ut * n) +
    pnConductance (Ugd, Isr, Ut * nr) + gtiny;
  Igd = pnCurrent (Ugd, Is, Ut * n) +
    pnCurrent (Ugd, Isr, Ut * nr) + gtiny * Ugd;

  // normal (forward) mode of operation
  if (Uds >= 0) {
    nr_double_t Ugst = Ugs - Vt0;
    // normal mode, cutoff region
    if (Ugst <= 0) {
      Ids = 0;
      gm  = 0;
      gds = 0;
    }
    else {
      nr_double_t b = beta * (1 + l * Uds);
      // normal mode, saturation region
      if (Ugst <= Uds) {
	Ids = b * Ugst * Ugst;
	gm  = b * 2 * Ugst;
	gds = l * beta * Ugst * Ugst;
      }
      // normal mode, linear region
      else {
	Ids = b * Uds * (2 * Ugst - Uds);
	gm  = b * 2 * Uds;
	gds = b * 2 * (Ugst - Uds) + l * beta * Uds * (2 * Ugst - Uds);
      }
    }
  }
  // inverse (backward) mode of operation
  else {
    nr_double_t Ugdt = Ugd - Vt0;
    // inverse mode, cutoff region
    if (Ugdt <= 0) {
      Ids = 0;
      gm  = 0;
      gds = 0;
    }
    else {
      nr_double_t b = beta * (1 - l * Uds);
      // inverse mode, saturation region
      if (Ugdt <= -Uds) {
	Ids = - b * Ugdt * Ugdt;
	gm  = - b * 2 * Ugdt;
	gds = beta * l * Ugdt * Ugdt + b * 2 * Ugdt;
      }
      // inverse mode, linear region
      else {
	Ids = b * Uds * (2 * Ugdt + Uds);
	gm  = b * 2 * Uds;
	gds = 2 * b * Ugdt - beta * l * Uds * (2 * Ugdt + Uds);
      }
    }
  }

  // compute autonomic current sources
  IeqG = Igs - ggs * Ugs;
  IeqD = Igd - ggd * Ugd;
  IeqS = Ids - gm * Ugs - gds * Uds;
  setI (NODE_G, (-IeqG - IeqD) * pol);
  setI (NODE_D, (+IeqD - IeqS) * pol);
  setI (NODE_S, (+IeqG + IeqS) * pol);

  // apply admittance matrix elements
  setY (NODE_G, NODE_G, ggs + ggd);
  setY (NODE_G, NODE_D, -ggd);
  setY (NODE_G, NODE_S, -ggs);
  setY (NODE_D, NODE_G, -ggd + gm);
  setY (NODE_D, NODE_D, gds + ggd);
  setY (NODE_D, NODE_S, -gm - gds);
  setY (NODE_S, NODE_G, -ggs - gm);
  setY (NODE_S, NODE_D, -gds);
  setY (NODE_S, NODE_S, ggs + gds + gm);
}

void jfet::saveOperatingPoints (void) {
  nr_double_t Ugs, Ugd;
  Ugd = real (getV (NODE_G) - getV (NODE_D)) * pol;
  Ugs = real (getV (NODE_G) - getV (NODE_S)) * pol;
  setOperatingPoint ("Vgs", Ugs);
  setOperatingPoint ("Vgd", Ugd);
  setOperatingPoint ("Vds", Ugs - Ugd);
}

void jfet::calcOperatingPoints (void) {

  // fetch device model parameters
  nr_double_t z    = getPropertyDouble ("M");
  nr_double_t Cgd0 = getPropertyDouble ("Cgd");
  nr_double_t Cgs0 = getPropertyDouble ("Cgs");
  nr_double_t Pb   = getPropertyDouble ("Pb");
  nr_double_t Fc   = getPropertyDouble ("Fc");
  nr_double_t T    = getPropertyDouble ("Temp");
  
  nr_double_t Ugs, Ugd, Ut, Cgs, Cgd;

  T = kelvin (T);
  Ut = kB * T / Q;
  Ugd = getOperatingPoint ("Vgd");
  Ugs = getOperatingPoint ("Vgs");

  // capacitance of gate-drain diode
  Cgd = pnCapacitance (Ugd, Cgd0, Pb, z, Fc);
  Qgd = pnCharge (Ugd, Cgd0, Pb, z, Fc);

  // capacitance of gate-source diode
  Cgs = pnCapacitance (Ugs, Cgs0, Pb, z, Fc);
  Qgs = pnCharge (Ugs, Cgs0, Pb, z, Fc);

  // save operating points
  setOperatingPoint ("ggs", ggs);
  setOperatingPoint ("ggd", ggd);
  setOperatingPoint ("gds", gds);
  setOperatingPoint ("gm", gm);
  setOperatingPoint ("Id", Ids);
  setOperatingPoint ("Cgd", Cgd);
  setOperatingPoint ("Cgs", Cgs);
}

void jfet::initAC (void) {
  allocMatrixMNA ();
  clearI ();
}

void jfet::calcAC (nr_double_t frequency) {
  setMatrixY (calcMatrixY (frequency));
}

void jfet::calcNoiseAC (nr_double_t frequency) {
  setMatrixN (calcMatrixCy (frequency));
}

#define qgdState 0 // gate-drain charge state
#define cgdState 1 // gate-drain current state
#define qgsState 2 // gate-source charge state
#define cgsState 3 // gate-source current state

void jfet::initTR (void) {
  setStates (4);
  initDC ();
}

void jfet::calcTR (nr_double_t) {
  calcDC ();
  saveOperatingPoints ();
  calcOperatingPoints ();

  nr_double_t Ugs = getOperatingPoint ("Vgs");
  nr_double_t Ugd = getOperatingPoint ("Vgd");
  nr_double_t Cgs = getOperatingPoint ("Cgs");
  nr_double_t Cgd = getOperatingPoint ("Cgd");

  transientCapacitance (qgsState, NODE_G, NODE_S, Cgs, Ugs, Qgs);
  transientCapacitance (qgdState, NODE_G, NODE_D, Cgd, Ugd, Qgd);
}
