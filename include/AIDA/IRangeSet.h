// -*- C++ -*-
// AID-GENERATED
// =========================================================================
// This class was generated by AID - Abstract Interface Definition          
// DO NOT MODIFY, but use the org.freehep.aid.Aid utility to regenerate it. 
// =========================================================================
#ifndef AIDA_IRANGESET_H
#define AIDA_IRANGESET_H 1

//  This file is part of the AIDA library
//  Copyright (C) 2002 by the AIDA team.  All rights reserved.
//  This library is free software and under the terms of the
//  GNU Library General Public License described in the LGPL.txt 

#include <vector>

namespace AIDA {

/**
 * User level interface to RangeSet.
 *
 *
 * Proposed rules for ranges:
 *
 * - By default (no arguments) RangeSet object is created with 
 *   one valid range: (-infinity, +infinity).
 *
 * - size=0 means no valid ranges (isInRange(double point) will
 *   return false for any point). It also can be used to check if
 *   any range is set - RangeSet include valid interval only
 *   if size!=0.
 *
 * - RangeSet can be smart, e.g merge overlapping valid ranges.
 *   So size() does not always equal to the number of times
 *   user call include method. 
 *
 *
 * @author The AIDA team (http://aida.freehep.org/)
 *
 */

class IRangeSet {

public: 
    /// Destructor.
    virtual ~IRangeSet() { /* nop */; }

    /** 
     * Return array of lower Bounds for the current set of ranges
     * @return Array of lower bounds for all valid ranges
     */
    virtual const std::vector<double>  & lowerBounds() = 0;

    /** 
     * Return array of upper Bounds for the current set of ranges
     * @return Array of upper bounds for all valid ranges
     */
    virtual const std::vector<double>  & upperBounds() = 0;

    /**
     * Add [xMin, xMax] interval to existing set of valid ranges
     * @param xMin - lower bound of a new valid range
     * @param xMax - upper bound of a new valid range
     */  
    virtual void include(double xMin, double xMax) = 0;

    /**
     * Exclude [xMin, xMax] interval from the existing set of valid ranges
     * @param xMin - lower bound of range to be excluded 
     * @param xMax - upper bound of range to be excluded
     */  
    virtual void exclude(double xMin, double xMax) = 0;

    /**
     * Set full range (from MINUS_INF to PLUS_INF ).
     */
    virtual void includeAll() = 0;

    /**
     * Set empty range.
     */
    virtual void excludeAll() = 0;

    /*
     * @return Return true if the point is in range 
     */
    virtual bool isInRange(double point) = 0;

    /**
     * Return current number of disjoint ranges (non-overlapping intervals).
     * Note: it is not always equal to the number of times user set the range
     * @return Number of disjoint ranges
     */
    virtual int size() = 0;

    /*
     * Define +infinity according to specific implementation
     * @return Numeric definition for +infinity
     */
    virtual double PLUS_INF() = 0;

    /*
     * Define -infinity according to specific implementation
     * @return Numeric definition for -infinity
     */
    virtual double MINUS_INF() = 0;
}; // class
} // namespace AIDA
#endif /* ifndef AIDA_IRANGESET_H */
