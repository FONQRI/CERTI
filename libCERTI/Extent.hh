// ----------------------------------------------------------------------------
// CERTI - HLA RunTime Infrastructure
// Copyright (C) 2003  ONERA
//
// This file is part of CERTI-libCERTI
//
// CERTI-libCERTI is free software ; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation ; either version 2 of
// the License, or (at your option) any later version.
//
// CERTI-libCERTI is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY ; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program ; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
//
// $Id: Extent.hh,v 3.3 2003/07/01 13:32:58 breholee Exp $
// ----------------------------------------------------------------------------

#ifndef _CERTI_EXTENT_HH
#define _CERTI_EXTENT_HH

#include "Range.hh"

#include <vector>

namespace certi {

class Extent
{
public:
    Extent(int);
    ~Extent();

    ULong getRangeLowerBound(DimensionHandle)
        throw (ArrayIndexOutOfBounds);

    ULong getRangeUpperBound(DimensionHandle)
        throw (ArrayIndexOutOfBounds);

    void setRangeUpperBound(DimensionHandle, ULong)
        throw (ArrayIndexOutOfBounds);

    void setRangeLowerBound(DimensionHandle, ULong)
        throw (ArrayIndexOutOfBounds);

    long getNumberOfRanges();
    
private:
    std::vector<Range*> ranges ;
};

} // namespace certi

#endif // _CERTI_EXTENT_HH

// $Id: Extent.hh,v 3.3 2003/07/01 13:32:58 breholee Exp $
