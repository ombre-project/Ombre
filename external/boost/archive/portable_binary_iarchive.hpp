#ifndef PORTABLE_BINARY_IARCHIVE_HPP
#define PORTABLE_BINARY_IARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER)
#pragma once
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// portable_binary_iarchive.hpp

// (C) Copyright 2002-7 Robert Ramey - http://www.rrsd.com .
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <boost/archive/archive_exception.hpp>
#include <boost/archive/basic_binary_iprimitive.hpp>
#include <boost/archive/detail/common_iarchive.hpp>
#include <boost/archive/detail/register_archive.hpp>
#include <boost/serialization/item_version_type.hpp>
#include <boost/serialization/string.hpp>
#include <boost/version.hpp>
#include <istream>

#include <boost/archive/impl/basic_binary_iprimitive.ipp>
#include <boost/archive/portable_binary_archive.hpp>

namespace boost
{
namespace archive
{

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// exception to be thrown if integer read from archive doesn't fit
// variable being loaded
class portable_binary_iarchive_exception : public boost::archive::archive_exception
{
  public:
	enum exception_code
	{
		incompatible_integer_size
	} m_exception_code;
	portable_binary_iarchive_exception(exception_code c = incompatible_integer_size) : boost::archive::archive_exception(boost::archive::archive_exception::other_exception),
																					   m_exception_code(c)
	{
	}
	virtual const char *what() const throw()
	{
		const char *msg = "programmer error";
		switch(m_exception_code)
		{
		case incompatible_integer_size:
			msg = "integer cannot be represented";
			break;
		default:
			msg = boost::archive::archive_exception::what();
			assert(false);
			break;
		}
		return msg;
	}
};

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// "Portable" input binary archive.  It addresses integer size and endienness so
// that binary archives can be passed across systems. Note:floating point types
// not addressed here
class portable_binary_iarchive : public boost::archive::basic_binary_iprimitive<
									 portable_binary_iarchive,
									 std::istream::char_type,
									 std::istream::traits_type>,
								 public boost::archive::detail::common_iarchive<
									 portable_binary_iarchive>
{
	typedef boost::archive::basic_binary_iprimitive<
		portable_binary_iarchive,
		std::istream::char_type,
		std::istream::traits_type>
		primitive_base_t;
	typedef boost::archive::detail::common_iarchive<
		portable_binary_iarchive>
		archive_base_t;
#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
  public:
#else
	friend archive_base_t;
	friend primitive_base_t; // since with override load below
	friend class boost::archive::detail::interface_iarchive<
		portable_binary_iarchive>;
	friend class boost::archive::load_access;

  protected:
#endif
	unsigned int m_flags;
	void load_impl(boost::intmax_t &l, char maxsize);

	// default fall through for any types not specified here
	template <class T>
	void load(T &t)
	{
		boost::intmax_t l;
		load_impl(l, sizeof(T));
		// use cast to avoid compile time warning
		//t = static_cast< T >(l);
		t = T(l);
	}
	void load(boost::serialization::item_version_type &t)
	{
		boost::intmax_t l;
		load_impl(l, sizeof(boost::serialization::item_version_type));
		// use cast to avoid compile time warning
		t = boost::serialization::item_version_type(l);
	}
	void load(boost::archive::version_type &t)
	{
		boost::intmax_t l;
		load_impl(l, sizeof(boost::archive::version_type));
		// use cast to avoid compile time warning
		t = boost::archive::version_type(l);
	}
	void load(boost::archive::class_id_type &t)
	{
		boost::intmax_t l;
		load_impl(l, sizeof(boost::archive::class_id_type));
		// use cast to avoid compile time warning
		t = boost::archive::class_id_type(static_cast<int>(l));
	}
	void load(std::string &t)
	{
		this->primitive_base_t::load(t);
	}
#ifndef BOOST_NO_STD_WSTRING
	void load(std::wstring &t)
	{
		this->primitive_base_t::load(t);
	}
#endif
	void load(float &t)
	{
		this->primitive_base_t::load(t);
		// floats not supported
		//BOOST_STATIC_ASSERT(false);
	}
	void load(double &t)
	{
		this->primitive_base_t::load(t);
		// doubles not supported
		//BOOST_STATIC_ASSERT(false);
	}
	void load(char &t)
	{
		this->primitive_base_t::load(t);
	}
	void load(unsigned char &t)
	{
		this->primitive_base_t::load(t);
	}
	typedef boost::archive::detail::common_iarchive<portable_binary_iarchive>
		detail_common_iarchive;
#if BOOST_VERSION > 105800
	template <class T>
	void load_override(T &t)
	{
		this->detail_common_iarchive::load_override(t);
	}
	void load_override(boost::archive::class_name_type &t);
	// binary files don't include the optional information
	void load_override(boost::archive::class_id_optional_type &) {}
#else
	template <class T>
	void load_override(T &t, int)
	{
		this->detail_common_iarchive::load_override(t, 0);
	}
	void load_override(boost::archive::class_name_type &t, int);
	// binary files don't include the optional information
	void load_override(boost::archive::class_id_optional_type &, int) {}
#endif

	void init(unsigned int flags);

  public:
	portable_binary_iarchive(std::istream &is, unsigned flags = 0) : primitive_base_t(
																		 *is.rdbuf(),
																		 0 != (flags & boost::archive::no_codecvt)),
																	 archive_base_t(flags),
																	 m_flags(0)
	{
		init(flags);
	}

	portable_binary_iarchive(
		std::basic_streambuf<
			std::istream::char_type,
			std::istream::traits_type> &bsb,
		unsigned int flags) : primitive_base_t(bsb,
											   0 != (flags & boost::archive::no_codecvt)),
							  archive_base_t(flags),
							  m_flags(0)
	{
		init(flags);
	}
};
}
}

// required by export in boost version > 1.34
#ifdef BOOST_SERIALIZATION_REGISTER_ARCHIVE
BOOST_SERIALIZATION_REGISTER_ARCHIVE(portable_binary_iarchive)
#endif

// required by export in boost <= 1.34
#define BOOST_ARCHIVE_CUSTOM_IARCHIVE_TYPES portable_binary_iarchive

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// portable_binary_iarchive.cpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com .
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <istream>
#include <string>

#include <boost/archive/archive_exception.hpp>
#if BOOST_VERSION >= 107200
#include <boost/predef/other/endian.h>
#else
#include <boost/detail/endian.hpp>
#endif
#include <boost/serialization/throw_exception.hpp>

namespace boost
{
namespace archive
{

inline void
portable_binary_iarchive::load_impl(boost::intmax_t &l, char maxsize)
{
	signed char size;
	l = 0;
	this->primitive_base_t::load(size);

	if(0 == size)
	{
		return;
	}

	bool negative = (size < 0);
	if(negative)
		size = -size;

	if(size > maxsize)
		boost::serialization::throw_exception(
			portable_binary_iarchive_exception());

	char *cptr = reinterpret_cast<char *>(&l);
#ifdef BOOST_BIG_ENDIAN
	cptr += (sizeof(boost::intmax_t) - size);
#endif
	this->primitive_base_t::load_binary(cptr, size);
	bool need_reverse = false;
#ifdef BOOST_BIG_ENDIAN
	if(m_flags & endian_little)
		need_reverse = true;
#else
	if(m_flags & endian_big)
		need_reverse = true;
#endif
	if (need_reverse)
		std::reverse(cptr, cptr + size -1);

	if(negative)
		l = -l;
}

#if BOOST_VERSION > 105800
inline void
portable_binary_iarchive::load_override(
	boost::archive::class_name_type &t)
{
	std::string cn;
	cn.reserve(BOOST_SERIALIZATION_MAX_KEY_SIZE);
	load_override(cn);
	if(cn.size() > (BOOST_SERIALIZATION_MAX_KEY_SIZE - 1))
		boost::serialization::throw_exception(
			boost::archive::archive_exception(
				boost::archive::archive_exception::invalid_class_name));
	std::memcpy(t, cn.data(), cn.size());
	// borland tweak
	t.t[cn.size()] = '\0';
}
#else
inline void
portable_binary_iarchive::load_override(
	boost::archive::class_name_type &t, int)
{
	std::string cn;
	cn.reserve(BOOST_SERIALIZATION_MAX_KEY_SIZE);
	load_override(cn, 0);
	if(cn.size() > (BOOST_SERIALIZATION_MAX_KEY_SIZE - 1))
		boost::serialization::throw_exception(
			boost::archive::archive_exception(
				boost::archive::archive_exception::invalid_class_name));
	std::memcpy(t, cn.data(), cn.size());
	// borland tweak
	t.t[cn.size()] = '\0';
}
#endif

inline void
portable_binary_iarchive::init(unsigned int flags)
{
	if(0 == (flags & boost::archive::no_header))
	{
		// read signature in an archive version independent manner
		std::string file_signature;
		*this >> file_signature;
		if(file_signature != boost::archive::BOOST_ARCHIVE_SIGNATURE())
			boost::serialization::throw_exception(
				boost::archive::archive_exception(
					boost::archive::archive_exception::invalid_signature));
		// make sure the version of the reading archive library can
		// support the format of the archive being read
		boost::archive::library_version_type input_library_version;
		*this >> input_library_version;

// ignore archive version checking
/*
        // extra little .t is to get around borland quirk
        if(boost::archive::BOOST_ARCHIVE_VERSION() < input_library_version)
            boost::serialization::throw_exception(
                boost::archive::archive_exception(
                    boost::archive::archive_exception::unsupported_version
                )
            );
        */

#if BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3205))
		this->set_library_version(input_library_version);
		//#else
		//#if ! BOOST_WORKAROUND(BOOST_MSVC, <= 1200)
		//detail::
		//#endif
		boost::archive::detail::basic_iarchive::set_library_version(
			input_library_version);
#endif
	}
	unsigned char x;
	load(x);
	m_flags = x << CHAR_BIT;
}
}
}

namespace boost
{
namespace archive
{

namespace detail
{
template class archive_serializer_map<portable_binary_iarchive>;
}

// template class basic_binary_iprimitive<
//    portable_binary_iarchive,
//    std::istream::char_type,
//    std::istream::traits_type
//> ;

} // namespace archive
} // namespace boost

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // PORTABLE_BINARY_IARCHIVE_HPP
