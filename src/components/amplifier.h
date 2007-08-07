/*
 * amplifier.h - amplifier class definitions
 *
 * Copyright (C) 2004 Stefan Jahn <stefan@lkcc.org>
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
 * $Id: amplifier.h,v 1.3 2007/08/07 20:43:09 ela Exp $
 *
 */

#ifndef __AMPLIFIER_H__
#define __AMPLIFIER_H__

/*! Ideal and linear amplifier 

    An ideal amplifier increases signal strength from input to output and
    blocks all signals flowing into the output.  
    The ideal amplifier is an isolator with voltage gain \f$G\f$
*/
class amplifier : public circuit
{
 public:
  amplifier ();
  void initSP (void);
  void initDC (void);
  void initAC (void);
  void initTR (void);
};

#endif /* __AMPLIFIER_H__ */
