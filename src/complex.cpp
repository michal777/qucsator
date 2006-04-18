/*
 * complex.cpp - complex number class implementation
 *
 * Copyright (C) 2003, 2004, 2005, 2006 Stefan Jahn <stefan@lkcc.org>
 * Copyright (C) 2006 Gunther Kraut <gn.kraut@t-online.de>
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
 * $Id: complex.cpp,v 1.30 2006-04-18 08:03:11 raimi Exp $
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <math.h>

#include "complex.h"
#include "consts.h"
#include "fspecial.h"

using namespace fspecial;

complex::complex (nr_double_t real, nr_double_t imag) {
  r = real;
  i = imag;
}

complex::complex () {
  r = 0.0;
  i = 0.0;
}

complex::complex (const complex& z) {
  r = z.r;
  i = z.i;
}

nr_double_t complex::abs (void) {
  return xhypot (r, i);
}

nr_double_t abs (const complex z) {
  return xhypot (z.r, z.i);
}

nr_double_t abs (const nr_double_t r) {
  return fabs (r);
}

nr_double_t complex::norm (void) {
  return r * r + i * i;
}

nr_double_t norm (const complex z) {
  return z.r * z.r + z.i * z.i;
}

nr_double_t norm (const nr_double_t r) {
  return r * r;
}

nr_double_t arg (const complex z) {
  return atan2 (z.i, z.r);
}

nr_double_t complex::real (void) {
  return r;
}

nr_double_t real (const complex z) {
  return z.r;
}

nr_double_t real (const nr_double_t r) {
  return r;
}

nr_double_t complex::imag (void) {
  return i;
}

nr_double_t imag (const complex z) {
  return z.i;
}

nr_double_t imag (const nr_double_t) {
  return 0.0;
}

complex complex::conj (void) {
  return complex (r, -i);
}

complex conj (const complex z) {
  return complex (z.r, -z.i);
}

nr_double_t conj (const nr_double_t r) {
  return r;
}

nr_double_t dB (const complex z) {
  return 10.0 * log10 (norm (z));
}

// returns the first result of square root z
complex sqrt (const complex z) {
  nr_double_t phi = arg (z);
  return polar (sqrt (abs (z)), phi / 2.0);
}

complex exp (const complex z) {
  nr_double_t mag = exp (real (z));
  return complex (mag * cos (imag (z)), mag * sin (imag (z)));
}

// returns the first result of natural logarithm z
complex ln (const complex z) {
  nr_double_t phi = arg (z);
  return complex (log (abs (z)), phi);
}

// returns the first result of decimal logarithm z
complex log10 (const complex z) {
  nr_double_t phi = arg (z);
  return complex (log10 (abs (z)), phi * M_LOG10E);
}

// returns the first result of binary logarithm z
complex log2 (const complex z) {
  nr_double_t phi = arg (z);
  return complex (log (abs (z)) * M_LOG2E, phi * M_LOG2E);
}

complex pow (const complex z, const nr_double_t d) {
  return polar (pow (abs (z), d), arg (z) * d);
}

complex pow (const nr_double_t d, const complex z) {
  return exp (z * log (d));
}

complex pow (const complex z1, const complex z2) {
  return exp (z2 * ln (z1));
}

complex sin (const complex z) {
  nr_double_t r = real (z);
  nr_double_t i = imag (z);
  return (polar (exp (-i), r - M_PI_2) -
          polar (exp (i), -r - M_PI_2)) / 2.0;
}

complex arcsin (const complex z) {
  nr_double_t r = real (z);
  nr_double_t i = imag (z);
  return complex (0.0, -1.0) * ln (rect (-i, r) + sqrt (1.0 - z * z));
}

complex cos (const complex z) {
  nr_double_t r = real (z);
  nr_double_t i = imag (z);
  return (polar (exp (-i), r) + polar (exp (i), -r)) / 2.0;
}

complex arccos (const complex z) {
#if 1
  return rect (0.0, -2.0) * ln (M_SQRT1_2 * (sqrt (z + 1.0) + sqrt (z - 1.0)));
#else
  complex y = sqrt (z * z - 1.0);
  if (z.r * z.i < 0.0) y = -y;
  return rect (0, -1.0) * ln (z + y);
#endif
}

complex tan (const complex z) {
  nr_double_t r = 2.0 * real (z);
  nr_double_t i = 2.0 * imag (z);
  return rect (0.0, -1.0) + rect (0.0, 2.0) / (polar (exp (-i), r) + 1.0);
}

complex arctan (const complex z) {
  return rect (0.0, -0.5) * ln (rect (0.0, 2.0) / (z + rect (0.0, 1.0)) - 1.0);
}

complex arctan2 (const complex y, const complex x) {
  complex a = arctan (y / x);
  return real (x) > 0.0 ? a : -a;
}

complex cot (const complex z) {
  nr_double_t r = 2.0 * real (z);
  nr_double_t i = 2.0 * imag (z);
  return rect (0.0, 1.0) + rect (0.0, 2.0) / (polar (exp (-i), r) - 1.0);
}

complex arccot (const complex z) {
  return rect (0.0, -0.5) * ln (rect (0.0, 2.0) / (z - rect (0.0, 1.0)) + 1.0);
}

complex sinh (const complex z) {
  nr_double_t r = real (z);
  nr_double_t i = imag (z);
  return (polar (exp (r), i) - polar (exp (-r), -i)) / 2.0;
}

complex arsinh (const complex z) {
  return ln (z + sqrt (z * z + 1));
}

complex cosh (const complex z) {
  nr_double_t r = real (z);
  nr_double_t i = imag (z);
  return (polar (exp (r), i) + polar (exp (-r), -i)) / 2.0;
}

complex arcosh (const complex z) {
  return ln (z + sqrt (z * z - 1));
}

complex tanh (const complex z) {
  nr_double_t r = 2.0 * real (z);
  nr_double_t i = 2.0 * imag (z);
  return 1.0 - 2.0 / (polar (exp (r), i) + 1.0);
}

complex artanh (const complex z) {
  return 0.5 * ln ( 2.0 / (1.0 - z) - 1.0);
}

complex coth (const complex z) {
  nr_double_t r = 2.0 * real (z);
  nr_double_t i = 2.0 * imag (z);
  return 1.0 + 2.0 / (polar (exp (r), i) - 1.0);
}

complex arcoth (const complex z) {
  return 0.5 * ln (2.0 / (z - 1.0) + 1.0);
}

// converts impedance to reflexion coefficient
complex ztor (const complex z, complex zref) {
  return (z - zref) / (z + zref);
}

// converts admittance to reflexion coefficient
complex ytor (const complex y, complex zref) {
  return (1 - y * zref) / (1 + y * zref);
}

// converts reflexion coefficient to impedance
complex rtoz (const complex r, complex zref) {
  return zref * (1 + r) / (1 - r);
}

// converts reflexion coefficient to admittance
complex rtoy (const complex r, complex zref) {
  return (1 - r) / (1 + r) / zref;
}

complex floor (const complex z) {
  return rect (floor (real (z)), floor (imag (z)));
}

complex sign (const complex z) {
  if (z == 0) return 0;
  return z / abs (z);
}

nr_double_t xhypot (const nr_double_t a, const nr_double_t b) {
  nr_double_t c = fabs (a);
  nr_double_t d = fabs (b);
  if (c > d) {
    nr_double_t e = d / c;
    return c * sqrt (1 + e * e);
  }
  else if (d == 0)
    return 0;
  else {
    nr_double_t e = c / d;
    return d * sqrt (1 + e * e);
  }
}

nr_double_t xhypot (const complex a, const complex b) {
  nr_double_t c = norm (a);
  nr_double_t d = norm (b);
  if (c > d)
    return abs (a) * sqrt (1 + d / c);
  else if (d == 0)
    return 0;
  else
    return abs (b) * sqrt (1 + c / d);
}

nr_double_t sign (const nr_double_t d) {
  if (d == 0) return 0;
  return d < 0 ? -1 : 1;
}

complex sinc (const complex z) {
  if (z == 0) return 1;
  return sin (z) / z;
}

nr_double_t sinc (const nr_double_t d) {
  if (d == 0) return 1;
  return sin (d) / d;
}

complex polar (const nr_double_t mag, const nr_double_t ang) {
  return rect (mag * cos (ang), mag * sin (ang));
}

complex rect (const nr_double_t x, const nr_double_t y) {
  return complex (x, y);
}

complex polar (const complex a, const complex p) {
  return a * exp (rect (p.i, -p.r));
}

complex ceil (const complex z) {
  return rect (ceil (z.r), ceil (z.i));
}

nr_double_t fix (const nr_double_t d) {
  return (d > 0) ? floor (d) : ceil (d);
}

complex fix (const complex z) {
  nr_double_t x = z.r;
  nr_double_t y = z.i;
  x = (x > 0) ? floor (x) : ceil (x);
  y = (y > 0) ? floor (y) : ceil (y);
  return rect (x, y);
}

complex round (const complex z) {
  return rect (round (z.r), round (z.i));
}

complex sqr (const complex z) {
  return rect (z.r * z.r - z.i * z.i, 2 * z.r * z.i);
}

nr_double_t step (const nr_double_t d) {
  nr_double_t x = d;
  if (x < 0.0)
    x = 0.0;
  else if (x > 0.0)
    x = 1.0;
  else
    x = 0.5;
  return x;
}

complex step (const complex z) {
  nr_double_t x = z.r;
  nr_double_t y = z.i;
  if (x < 0.0)
    x = 0.0;
  else if (x > 0.0)
    x = 1.0;
  else
    x = 0.5;
  if (y < 0.0)
    y = 0.0;
  else if (y > 0.0)
    y = 1.0;
  else
    y = 0.5;
  return rect (x, y);
}

complex jn (const int n, const complex z) {
  return rect (jn (n, z.r), 0);
}

complex yn (const int n, const complex z) {
  return rect (yn (n, z.r), 0);
}

complex i0 (const complex z) {
  return rect (i0 (z.r), 0);
}

complex erf (const complex z) {
  return rect (erf (z.r), 0);
}

complex erfc (const complex z) {
  return rect (erfc (z.r), 0);
}

complex erfinv (const complex z) {
  return rect (erfinv (z.r), 0);
}

complex erfcinv (const complex z) {
  return rect (erfcinv (z.r), 0);
}

complex complex::operator+() {
  return complex (r, i);
}

complex complex::operator-() {
  return complex (-r, -i);
}

complex& complex::operator+=(const complex z2) {
  r += z2.r;
  i += z2.i;
  return *this;
}

complex& complex::operator+=(const nr_double_t r2) {
  r += r2;
  return *this;
}

complex& complex::operator-=(const complex z2) {
  r -= z2.r;
  i -= z2.i;
  return *this;
}

complex& complex::operator-=(const nr_double_t r2) {
  r -= r2;
  return *this;
}

complex& complex::operator*=(const nr_double_t r2) {
  r *= r2;
  i *= r2;
  return *this;
}

complex& complex::operator/=(const nr_double_t r2) {
  r /= r2;
  i /= r2;
  return *this;
}

complex operator+(const complex z1, const complex z2) {
  return complex (z1.r + z2.r, z1.i + z2.i);
}

complex operator+(const nr_double_t r1, const complex z2) {
  return complex (r1 + z2.r, z2.i);
}

complex operator+(const complex z1, const nr_double_t r2) {
  return complex (z1.r + r2, z1.i);
}

complex operator-(const complex z1, const complex z2) {
  return complex (z1.r - z2.r, z1.i - z2.i);
}

complex operator-(const nr_double_t r1, const complex z2) {
  return complex (r1 - z2.r, -z2.i);
}

complex operator-(const complex z1, const nr_double_t r2) {
  return complex (z1.r - r2, z1.i);
}

complex operator*(const complex z1, const nr_double_t r2) {
  return complex (z1.r * r2, z1.i * r2);
}

complex operator*(const nr_double_t r1, const complex z2) {
  return complex (z2.r * r1, z2.i * r1);
}

complex operator*(const complex z1, const complex z2) {
  return complex (z1.r * z2.r - z1.i * z2.i, z1.i * z2.r + z1.r * z2.i);
}

bool operator==(const complex z1, const complex z2) {
  return (z1.r == z2.r) && (z1.i == z2.i);
}

bool operator!=(const complex z1, const complex z2) {
  return (z1.r != z2.r) || (z1.i != z2.i);
}

bool operator>=(const complex z1, const complex z2) {
  return norm (z1) >= norm (z2);
}

bool operator<=(const complex z1, const complex z2) {
  return norm (z1) <= norm (z2);
}

bool operator>(const complex z1, const complex z2) {
  return norm (z1) > norm (z2);
}

bool operator<(const complex z1, const complex z2) {
  return norm (z1) < norm (z2);
}

complex operator/(const complex z1, const nr_double_t r2) {
  return complex (z1.r / r2, z1.i / r2);
}

complex operator/(const complex z1, const complex z2) {
#if 0
  nr_double_t n = norm (z2);
  return complex ((z1.r * z2.r + z1.i * z2.i) / n,
		  (z1.i * z2.r - z1.r * z2.i) / n);
#else /* avoid numerical overflow and underrun */
  nr_double_t r, i, n, d;
  if (fabs (z2.r) > fabs (z2.i)) {
    n = z2.i / z2.r;
    d = z2.r + z2.i * n;
    r = (z1.r + z1.i * n) / d;
    i = (z1.i - z1.r * n) / d;
  }
  else {
    n = z2.r / z2.i;
    d = z2.r * n + z2.i;
    r = (z1.r * n + z1.i) / d;
    i = (z1.i * n - z1.r) / d;
  }
  return complex (r, i);
#endif
}

complex& complex::operator/=(const complex z) {
#if 0
  nr_double_t n1, n2;
  n1 = norm (z);
  n2 = (r * z.r + i * z.i) / n1;
  i  = (i * z.r - r * z.i) / n1;
  r  = n2;
#else /* avoid numerical overflow and underrun */
  nr_double_t n, d, t;
  if (fabs (z.r) > fabs (z.i)) {
    n = z.i / z.r;
    d = z.r + z.i * n;
    t = (r + i * n) / d;
    i = (i - r * n) / d;
    r = t;
  }
  else {
    n = z.r / z.i;
    d = z.r * n + z.i;
    t = (r * n + i) / d;
    i = (i * n - r) / d;
    r = t;
  }
#endif
  return *this;
}

complex operator/(const nr_double_t r1, const complex z2) {
#if 0
  nr_double_t n = norm (z2);
  return complex (r1 * z2.r / n, -r1 * z2.i / n);
#else /* avoid numerical overflow and underrun */
  nr_double_t r, i, n, d;
  if (fabs (z2.r) > fabs (z2.i)) {
    n = z2.i / z2.r;
    d = z2.r + z2.i * n;
    r = r1 / d;
    i = -n * r1 / d;
  }
  else {
    n = z2.r / z2.i;
    d = z2.r * n + z2.i;
    r = r1 * n / d;
    i = -r1 / d;
  }
  return complex (r, i);
#endif
}

complex operator%(const complex z1, const complex z2) {
  return z1 - z2 * floor (z1 / z2);
}

complex operator%(const complex z1, const nr_double_t r2) {
  return z1 - r2 * floor (z1 / r2);
}

complex operator%(const nr_double_t r1, const complex z2) {
  return r1 - z2 * floor (r1 / z2);
}

complex& complex::operator%=(const complex z) {
  *this = *this % z;
  return *this;
}

complex& complex::operator%=(const nr_double_t r) {
  *this = *this % r;
  return *this;
}

complex& complex::operator=(const complex z) {
  r = z.r;
  i = z.i;
  return *this;
}

complex& complex::operator=(const nr_double_t x) {
  r = x;
  i = 0.0;
  return *this;
}

complex& complex::operator*=(const complex z) {
  nr_double_t n;
  n = r * z.r - i * z.i;
  i = i * z.r + r * z.i;
  r = n;
  return *this;
}

#ifdef DEBUG
// Debug function: Prints the complex number.
#include <stdio.h>
void complex::print (void) {
  fprintf (stderr, "%+.2e,%+.2e ", (double) r, (double) i);
}
#endif /* DEBUG */
