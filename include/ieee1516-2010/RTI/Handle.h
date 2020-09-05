/***********************************************************************
   The IEEE hereby grants a general, royalty-free license to copy, distribute,
   display and make derivative works from this material, for all purposes,
   provided that any use of the material contains the following
   attribution: "Reprinted with permission from IEEE 1516.1(TM)-2010".
   Should you require additional information, contact the Manager, Standards
   Intellectual Property, IEEE Standards Association (stds-ipr@ieee.org).
***********************************************************************/
/***********************************************************************
   IEEE 1516.1 High Level Architecture Interface Specification C++ API
   File: RTI/Handle.h
***********************************************************************/

#ifndef RTI_Handle_h
#define RTI_Handle_h

#include <RTI/SpecificConfig.h>
#include <RTI/Exception.h>
#include <RTI/VariableLengthData.h>
#include <string>

// The following macro is used to define each of the Handle classes
// that are used by the RTI's API, e.g. AttributeHandle, ParameterHandle, etc.
// Each kind of handle contains the same set of operators and functions, but
// each is a separate class for type safety.  The encode method can be used to
// generate an encoded value for a handle, that can be sent to other federates
// as an attribute or parameter.  (Use RTIambassador functions to reconstruct a
// handle from an encoded value).  RTI implementations contain definitions
// for each kind of the HandleKindImplementation classes (e.g.
// AttributeHandleImplementation), but these classes are not needed by
// federate code.

#define DEFINE_HANDLE_CLASS(HandleKind)                           \
                                                                  \
/* Forward declaration for the RTI-internal class            */   \
/* used to implement a specific kind of handle               */   \
class HandleKind##Implementation;                                 \
                                                                  \
/* Each handle class generated by this macro provides the    */   \
/* following interface                                       */   \
class RTI_EXPORT HandleKind                                       \
{                                                                 \
   public:                                                        \
                                                                  \
   /* Constructs an invalid handle                           */   \
   HandleKind ();                                                 \
                                                                  \
   ~HandleKind ()                                                 \
      noexcept;                                                    \
                                                                  \
   HandleKind (                                                   \
      HandleKind const & rhs);                                    \
                                                                  \
   HandleKind & operator= (                                       \
      HandleKind const & rhs);                                    \
                                                                  \
   /* Indicates whether this handle is valid                 */   \
   bool isValid () const;                                         \
                                                                  \
   /* All invalid handles are equivalent                     */   \
   bool operator== (                                              \
      HandleKind const & rhs) const;                              \
                                                                  \
   bool operator!= (                                              \
      HandleKind const & rhs) const;                              \
                                                                  \
   bool operator< (                                               \
      HandleKind const & rhs) const;                              \
                                                                  \
   /* Generate a hash value for use in storing handles in a  */   \
   /* in a hash table.                                       */   \
   /* Note: The hash value may not be unique across two      */   \
   /* separate handle values but it will be equal given two  */   \
   /* separate instances of the same handle value.           */   \
   /* H1 == H2 implies H1.hash() == H2.hash()                */   \
   /* H1 != H2 does not imply H1.hash() != H2.hash()         */   \
   long hash () const;                                            \
                                                                  \
   /* Generate an encoded value that can be used to send     */   \
   /* handles to other federates in updates or interactions. */   \
   VariableLengthData encode () const;                            \
                                                                  \
   /* Encode into an existing VariableLengthData             */   \
   void encode (VariableLengthData& buffer) const;                \
                                                                  \
   /* Alternate encode for directly filling a buffer         */   \
   size_t encode (                                                \
      void* buffer,                                               \
      size_t bufferSize) const;                                   \
                                                                  \
   size_t encodedLength () const;                                 \
                                                                  \
   std::wstring toString () const;                                \
                                                                  \
   protected:                                                     \
                                                                  \
   /* Friend declaration for an RTI-internal class that      */   \
   /* can access the implementation of a handle.             */   \
   friend class HandleKind##Friend;                               \
                                                                  \
   const HandleKind##Implementation* getImplementation () const;  \
                                                                  \
   HandleKind##Implementation* getImplementation ();              \
                                                                  \
   explicit HandleKind (                                          \
      HandleKind##Implementation* impl);                          \
                                                                  \
   explicit HandleKind (                                          \
      VariableLengthData const & encodedValue);                   \
                                                                  \
   HandleKind##Implementation* _impl;                             \
                                                                  \
};                                                                \
                                                                  \
/* Output operator for Handles                               */   \
std::wostream RTI_EXPORT &                                        \
operator<< (                                                      \
   std::wostream &,                                               \
   HandleKind const &);


namespace rti1516e
{

// All of the RTI API's Handle classes are defined
// by invoking the macro above.
DEFINE_HANDLE_CLASS(FederateHandle)
DEFINE_HANDLE_CLASS(ObjectClassHandle)
DEFINE_HANDLE_CLASS(InteractionClassHandle)
DEFINE_HANDLE_CLASS(ObjectInstanceHandle)
DEFINE_HANDLE_CLASS(AttributeHandle)
DEFINE_HANDLE_CLASS(ParameterHandle)
DEFINE_HANDLE_CLASS(DimensionHandle)
DEFINE_HANDLE_CLASS(MessageRetractionHandle)
DEFINE_HANDLE_CLASS(RegionHandle)

}

#endif // RTI_Handle_h

