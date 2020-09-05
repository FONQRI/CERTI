// ----------------------------------------------------------------------------
// CERTI - HLA RunTime Infrastructure
// Copyright (C) 2002-2014  ONERA
//
// This file is part of CERTI-libRTI
//
// CERTI-libRTI is free software ; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation ; either version 2 of
// the License, or (at your option) any later version.
//
// CERTI-libRTI is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY ; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program ; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
//
// ----------------------------------------------------------------------------
#ifndef RTI1516FEDTIME_H
#define RTI1516FEDTIME_H

#include <RTI/certiLogicalTime.h>
#include <RTI/certiLogicalTimeInterval.h>
#include <RTI/certiLogicalTimeFactory.h>

namespace rti1516 {
	class VariableLengthData;
}

class FEDTIME_EXPORT RTI1516fedTime : public rti1516::LogicalTime
{
public:
	RTI1516fedTime(double timeVal);
	RTI1516fedTime(const RTI1516fedTime &other);

    virtual ~RTI1516fedTime() noexcept;

	virtual
		void
		setInitial();

	virtual
		bool
		isInitial() const;

	virtual
		void
		setFinal();

	virtual
		bool
		isFinal() const;

	virtual
		rti1516::LogicalTime &
        operator=(rti1516::LogicalTime const & value);

	virtual
		RTI1516fedTime &
		operator=(RTI1516fedTime const & other)
		throw ();

	virtual
		rti1516::LogicalTime &
        operator+=(rti1516::LogicalTimeInterval const & value);

	virtual
		rti1516::LogicalTime &
        operator-=(rti1516::LogicalTimeInterval const & value);

	virtual
		bool
        operator>(rti1516::LogicalTime const & value) const;

	virtual
		bool
        operator<(rti1516::LogicalTime const & value) const;

	virtual
		bool
        operator==(rti1516::LogicalTime const & value) const;

	virtual
		bool
        operator>=(rti1516::LogicalTime const & value) const;

	virtual
		bool
        operator<=(rti1516::LogicalTime const & value) const;

	// Generates an encoded value that can be used to send
	// LogicalTimes to other federates in updates or interactions
	// Not implemented.
	virtual
		rti1516::VariableLengthData 
		encode() const;

	// Alternate encode for directly filling a buffer
	virtual 
		unsigned long 
		encodedLength() const;

	virtual 
		unsigned long 
        encode(void* buffer, unsigned long bufferSize) const;

	// Decode encodedLogicalTime into self
	// Not implemented.
	virtual 
		void 
        decode(rti1516::VariableLengthData const & encodedLogicalTime);

	// Alternate decode that reads directly from a buffer
	virtual 
		void 
        decode(void* buffer, unsigned long bufferSize);

	virtual 
		std::wstring 
		toString() const;

	// Returns the name of the implementation, as needed by
	// createFederationExecution.
	virtual 
		std::wstring 
		implementationName() const;

	double getFedTime() const
	{ return _fedTime; }

	bool isInfinity() const
	{ return _fedTime == _positiveInfinity; }


private:
	double _fedTime;
	double _zero;
	double _epsilon;
	double _positiveInfinity;

}; // class RTI_EXPORT_FEDTIME RTI1516fedTime

class FEDTIME_EXPORT RTI1516fedTimeInterval : public rti1516::LogicalTimeInterval
{
public:
	RTI1516fedTimeInterval(double timeVal);
	RTI1516fedTimeInterval(const RTI1516fedTimeInterval &other);

	virtual
        ~RTI1516fedTimeInterval()
            noexcept;

	virtual
		void
		setZero();

	virtual
		bool
		isZero() const;

	virtual
		void
		setEpsilon();

	virtual
		bool
		isEpsilon() const;

	virtual
		rti1516::LogicalTimeInterval &
        operator=(rti1516::LogicalTimeInterval const & value);

	virtual
		RTI1516fedTimeInterval &
		operator=(RTI1516fedTimeInterval const & other)
        noexcept;

	// Set self to the difference between two LogicalTimes
	virtual
		void
		setToDifference(rti1516::LogicalTime const & minuend,
        rti1516::LogicalTime const& subtrahend);

	virtual
		rti1516::LogicalTimeInterval &
        operator+=(rti1516::LogicalTimeInterval const & value);

	virtual
		rti1516::LogicalTimeInterval &
        operator-=(rti1516::LogicalTimeInterval const & value);

	virtual
		bool
        operator>(rti1516::LogicalTimeInterval const & value) const;

	virtual
		bool
        operator<(rti1516::LogicalTimeInterval const & value) const;

	virtual
		bool
        operator==(rti1516::LogicalTimeInterval const & value) const;

	virtual
		bool
        operator>=(rti1516::LogicalTimeInterval const & value) const;

	virtual
		bool
        operator<=(rti1516::LogicalTimeInterval const & value) const;

	// Generates an encoded value that can be used to send
	// LogicalTimeIntervals to other federates in updates or interactions
	// Not implemented.
	virtual 
		rti1516::VariableLengthData 
		encode() const;

	// Alternate encode for directly filling a buffer
	virtual 
		unsigned long 
		encodedLength() const;

	virtual 
		unsigned long 
        encode(void* buffer, unsigned long bufferSize) const;

	// Decode encodedValue into self
	// Not implemented.
	virtual 
		void 
        decode(rti1516::VariableLengthData const & encodedValue);

	// Alternate decode that reads directly from a buffer
	virtual 
		void 
        decode(void* buffer, unsigned long bufferSize);

	virtual 
		std::wstring 
		toString() const;

	// Returns the name of the implementation, as needed by
	// createFederationExecution.
	virtual 
		std::wstring 
		implementationName() const;

	double getInterval() const
	{ return _fedInterval; }

	double getEpsilon() const
	{ return _epsilon; }

private:
	double _fedInterval;
	double _zero;
	double _epsilon;
	double _positiveInfinity;

}; // class FEDTIME_EXPORT RTI1516fedTimeInterval

class FEDTIME_EXPORT RTI1516fedTimeFactory : public rti1516::LogicalTimeFactory
{
public:
	virtual
		~RTI1516fedTimeFactory()
		throw ();

	// Returns a LogicalTime with a value of "initial"
	virtual
		std::auto_ptr< rti1516::LogicalTime >
        makeLogicalTime();

	virtual
		std::auto_ptr< rti1516::LogicalTime >
        makeLogicalTime(double timeVal);

	// Returns a LogicalTimeInterval with a value of "zero"
	virtual 
		std::auto_ptr< rti1516::LogicalTimeInterval >
        makeLogicalTimeInterval();

	virtual 
		std::auto_ptr< rti1516::LogicalTimeInterval >
        makeLogicalTimeInterval(double timeInterval);

private:
	friend class rti1516::LogicalTimeFactoryFactory;

	// Private constructor - Only for LogicalTimeFactoryFactory to access.
	RTI1516fedTimeFactory()
        noexcept;

}; // class FEDTIME_EXPORT RTI1516fedTimeFactory



// The LogicalTimeFactoryFactory must also be implemented by the 
// federate. The definition is copied here (though commented out) 
// for reference.

//namespace rti1516
//{  
//  class FEDTIME_EXPORT LogicalTimeFactoryFactory
//  {
//  public:
//
//    // The name is used to choose among several LogicalTimeFactories that might
//    // be present in the fedtime library.  Each federation chooses its
//    // implementation by passing the appropriate name to createFederationExecution.
//    // If the supplied name is the empty string, a default LogicalTimeFactory is
//    // returned.  If the supplied implementation name does not match any name 
//    // supported by the library, then a NULL pointer is returned. 
//    static std::auto_ptr< LogicalTimeFactory > 
//       makeLogicalTimeFactory(std::wstring const & implementationName);
//  };
//}




# endif // RTI1516_FED_TIME_H
