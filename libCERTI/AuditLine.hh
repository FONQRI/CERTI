// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*- 
// ---------------------------------------------------------------------------
// CERTI - HLA RunTime Infrastructure
// Copyright (C) 2002, 2003  ONERA
//
// This file is part of CERTI-libCERTI
//
// CERTI-libCERTI is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// CERTI-libCERTI is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
//
// $Id: AuditLine.hh,v 3.3 2003/02/17 09:17:03 breholee Exp $
// ---------------------------------------------------------------------------

#ifndef _CERTI_AUDIT_LINE_HH
#define _CERTI_AUDIT_LINE_HH

// Project
#include <config.h>
#include "RTItypes.hh"

// Standard libraries
#include <fstream>
#include <string>
#include <time.h>

using std::ofstream;
using std::endl;
using std::string;

namespace certi {

class AuditLine {

public:
    AuditLine(void);
    ~AuditLine(void);

    void write(ofstream &); //!< Write data to file
    void addComment(const char *); //!< Add str at the end of comment.

    FederationHandle federation;
    FederateHandle federate;
    unsigned short type;
    unsigned short level;
    unsigned short status;

private:
    time_t date ;   //!< date, automatically set at construction time.
    string comment; //!< comment internally managed.
};

}

#endif // _CERTI_AUDIT_LINE_HH

// $Id: AuditLine.hh,v 3.3 2003/02/17 09:17:03 breholee Exp $
