//	Astrophysics Science Division,
//	NASA/ Goddard Space Flight Center
//	HEASARC
//	http://heasarc.gsfc.nasa.gov
//	e-mail: ccfits@legacy.gsfc.nasa.gov
//
//	Original author: Ben Dorman

// KeyData
#include "KeyData.h"
#include <complex>
#include <iomanip>

namespace CCfits
{
   KeyNull::KeyNull(const KeyNull& right)
      :Keyword(right),
      m_numValue(right.m_numValue),
      m_stringValue(right.m_stringValue),
      m_state(right.m_state),
      m_writeAs(right.m_writeAs)
   {
   }
   
   KeyNull::KeyNull(const String &keyname,
                 HDU* p,
                 const String &comment)
        :Keyword(keyname, Tnull, p, comment, false),
         m_numValue(0),
         m_stringValue(""),
         m_state(ValState::Empty),
         m_writeAs(Tdouble)
   {
   }
   
   KeyNull* KeyNull::clone() const
   {
      return new KeyNull(*this);
   }
   
   KeyNull::~KeyNull()
   {
   }
   
   void KeyNull::write ()
   {
      int status=0;
      Keyword::write();
      switch (m_state) {
         case ValState::Empty:
            fits_update_key_null(fitsPointer(),
                    const_cast<char *>(name().c_str()),
                    const_cast<char *>(comment().c_str()),&status);
            break;
         case ValState::NumData:
            if (m_writeAs == Tint)
            {
               int outval = static_cast<int>(m_numValue);
               fits_update_key(fitsPointer(), Tint, 
	               const_cast<char *>(name().c_str()),
	               &outval, const_cast<char *>(comment().c_str()), 
		       &status);         
            }
            else if (m_writeAs == Tfloat)
            {
               float outval = static_cast<float>(m_numValue);
               fits_update_key(fitsPointer(), Tfloat, 
	               const_cast<char *>(name().c_str()),
	               &outval, const_cast<char *>(comment().c_str()), 
		       &status);         
            }
            else if (m_writeAs == Tdouble)
            {
               fits_update_key(fitsPointer(), Tdouble, 
	               const_cast<char *>(name().c_str()),
	               &m_numValue, const_cast<char *>(comment().c_str()), 
		       &status);         
            }
            break;
         case ValState::StrData:
            fits_update_key(fitsPointer(), Tstring, 
	            const_cast<char *>(name().c_str()),
	            const_cast<char*>(m_stringValue.c_str()),
	            const_cast<char *>(comment().c_str()), 
			           &status);
            break;
      }
      if (status)
         throw FitsError(status);
   }
   
   void KeyNull::copy (const Keyword& right)
   {
      Keyword::copy(right);
      const KeyNull& that = static_cast<const KeyNull&>(right);
      m_numValue = that.m_numValue;
      m_stringValue = that.m_stringValue;
      m_state = that.m_state;
   }
   
   bool KeyNull::compare (const Keyword &right) const
   {
      if (!Keyword::compare(right))
         return false;
      const KeyNull& that = static_cast<const KeyNull&>(right);
      if (m_numValue != that.m_numValue ||
          m_stringValue != that.m_stringValue ||
          m_state != that.m_state)
          return false;
      return true;
   }
   
   std::ostream& KeyNull::put(std::ostream &s) const
   {
     s << "Keyword Name: " << name();
     switch (m_state) {
        case ValState::Empty:
           s << "\t NO VALUE " << "\t Type: Tnull";
           break;
        case ValState::NumData:
           s << "\t Value: " << m_numValue << "\t Type: Tnull to number";
           break;
        case ValState::StrData:
           s << "\t Value: " << m_stringValue << "\t Type: Tnull to string";
           break;
     }
     s <<"\t Comment: " << comment();
     return s;
   }
      
#ifndef SPEC_TEMPLATE_IMP_DEFECT
#ifndef SPEC_TEMPLATE_DECL_DEFECT
        template<>
        void KeyData<bool>::write() 
        {
           Keyword::write();
           int status = 0;
           int value(0);
           if (m_keyval) value = 1; 
           if (fits_update_key(fitsPointer(), Tlogical, 
			           const_cast<char *>(name().c_str()),
			           &value,
			           const_cast<char *>(comment().c_str()), 
			           &status)) throw FitsError(status);

        }

        template<>
        void KeyData<String>::write() 
        {
          Keyword::write();
          int status = 0;
          // if the user specified that this is a long string, use the
          // Long String Keyword convention that is described in the
          // 'Local FITS Conventions'
          // https://heasarc.gsfc.nasa.gov/fitsio/c/c_user/node116.html
          if (isLongStr()) {
            if (fits_write_key_longstr(fitsPointer(),
                const_cast<char *>(name().c_str()),
                const_cast<char*>(m_keyval.c_str()),
                const_cast<char *>(comment().c_str()),
                &status)) throw FitsError(status);
            // also write the LONGSTRN keyword
            if (fits_write_key_longwarn(fitsPointer(), &status))
                throw FitsError(status);
          } else {
            // the user did not specify the long string convention.
            // use the normal fits key writing routine, which truncates
            // to 68 characters
            if (fits_update_key(fitsPointer(), Tstring,
                const_cast<char *>(name().c_str()),
                const_cast<char*>(m_keyval.c_str()),
                const_cast<char *>(comment().c_str()),
                &status)) throw FitsError(status);
          }
            // +++ 20180808 KLR TODO do we also want to make sure m_keyval is
            //     truncated, so the data member reflects what cfitsio is doing?
        }

        template<>
        void KeyData<std::complex<float> >::write() 
        {
             Keyword::write();
             int status = 0;
             FITSUtil::auto_array_ptr<float> keyVal( new float[2]);
             keyVal[0] = m_keyval.real(); 
             keyVal[1] = m_keyval.imag(); 
             if (fits_update_key(fitsPointer(), Tcomplex, 
        			   const_cast<char *>(name().c_str()),
        			   keyVal.get(),
         			   const_cast<char *>(comment().c_str()), 
        			   &status)) throw FitsError(status);

        }

        template<>
        void KeyData<std::complex<double> >::write() 
        {
             Keyword::write();
             int status = 0;
             FITSUtil::auto_array_ptr<double> keyVal(new double[2]);
             keyVal[0] = m_keyval.real(); 
             keyVal[1] = m_keyval.imag(); 
             if (fits_update_key(fitsPointer(), Tdblcomplex, 
        			   const_cast<char *>(name().c_str()),
        			   keyVal.get(),
         			   const_cast<char *>(comment().c_str()), 
        			   &status)) throw FitsError(status);

        }


        template <>
        std::ostream & KeyData<String>::put (std::ostream &s) const
        {
   		using std::setw;
   		s << "Keyword Name: " << setw(10) << name() << "  Value: " << setw(14) 
                  << keyval() << " Type: " << setw(20) << " string "  << " Comment: " << comment();
          return s;
        }
#endif
#endif

        template  <>
        void KeyData<String>::keyval(const String& newVal)
        {
                m_keyval = newVal;

        }

#ifndef SPEC_TEMPLATE_IMP_DEFECT
#ifndef SPEC_TEMPLATE_DECL_DEFECT
        template <>
        std::ostream & KeyData<std::complex<float> >::put (std::ostream &s) const
        {
   		using std::setw;
           	s << "Keyword Name: " << name() << " Value: " << m_keyval.real() << " +   i " 
             	  << m_keyval.imag() <<   " Type: " <<  setw(20) << " complex<float> " 
             	  << " Comment: " << comment()   << std::endl;
          return s;
        }

        template <>
        std::ostream & KeyData<std::complex<double> >::put (std::ostream &s) const
        {
   		using std::setw;
           	s << "Keyword Name: " << name() << " Value: " << m_keyval.real() << " +   i " 
             	  << m_keyval.imag() <<   " Type: " <<  setw(20) << " complex<double> " 
             	  << " Comment: " << comment()   << std::endl;

	          return s;
        }

        template <>
        std::ostream & KeyData<bool>::put (std::ostream &s) const
        {
   		using std::setw;
   		s << "Keyword Name: " << setw(10) << name() 
                  << "  Value: " << std::boolalpha  << setw(8) << keyval() 
                  << "  Type: " << setw(20) << " logical "  << " Comment: " << comment();
                return s;
        }


#endif
#endif

#ifndef SPEC_TEMPLATE_DECL_DEFECT
        template  <>
        const String& KeyData<String>::keyval() const
        {
                return m_keyval;

        }
#endif

#ifndef SPEC_TEMPLATE_DECL_DEFECT
        template  <>
        void KeyData<std::complex<float> >::keyval(const std::complex<float>&  newVal)
        {
                m_keyval = newVal;

        }
        template  <>
        void KeyData<std::complex<double> >::keyval(const std::complex<double>&  newVal)
        {
                m_keyval = newVal;

        }

        template  <>
        const std::complex<float>& KeyData<std::complex<float> >::keyval() const
        {
                return m_keyval;

        }

        template  <>
        const std::complex<double>& KeyData<std::complex<double> >::keyval() const
        {
                return m_keyval;

        }

#endif


} // namespace CCfits
