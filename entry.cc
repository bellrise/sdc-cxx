#include <sdc-cxx.h>
#include <sdc.h>
#include <stdexcept>
#include <string.h>
#include <vector>

using namespace sdc;

entry::entry()
    : m_type(SDC_NULL)
    , m_name()
{ }

entry::entry(const entry& other)
    : m_type(other.m_type)
    , m_name(other.m_name)
{
	copy_entry_from(other);
}

entry::~entry() { }

void entry::set(int entry)
{
	m_type = SDC_INT;
	m_int = entry;
}

void entry::set(long entry)
{
	m_type = SDC_LONG;
	m_long = entry;
}

void entry::set(unsigned int entry)
{
	m_type = SDC_UINT;
	m_uint = entry;
}

void entry::set(unsigned long entry)
{
	m_type = SDC_ULONG;
	m_ulong = entry;
}

void entry::set(bool entry)
{
	m_type = SDC_BOOL;
	m_bool = entry;
}

void entry::set(const char *string)
{
	m_type = SDC_STRING;
	m_string = string;
}

void entry::set(const std::string& string)
{
	m_type = SDC_STRING;
	m_string = string;
}

void entry::set(const std::span<char>& bytes)
{
	m_type = SDC_BYTES;
	m_bytes = std::vector(bytes.begin(), bytes.end());
}

void entry::set(const std::vector<char>& bytes)
{
	m_type = SDC_BYTES;
	m_bytes = bytes;
}

void entry::set(const std::span<entry>& array)
{
	m_type = SDC_ARRAY;
	m_array = std::vector(array.begin(), array.end());
}

void entry::set(const std::vector<entry>& array)
{
	m_type = SDC_ARRAY;
	m_array = array;
}

entry::entry_type entry::type() const
{
	return static_cast<entry::entry_type>(m_type);
}

int entry::get_int() const
{
	if (m_type != SDC_INT)
		throw std::runtime_error("entry has different type");
	return m_int;
}

long entry::get_long() const
{
	if (m_type != SDC_LONG)
		throw std::runtime_error("entry has different type");
	return m_long;
}

unsigned int entry::get_uint() const
{
	if (m_type != SDC_UINT)
		throw std::runtime_error("entry has different type");
	return m_uint;
}

unsigned long entry::get_ulong() const
{
	if (m_type != SDC_ULONG)
		throw std::runtime_error("entry has different type");
	return m_ulong;
}

bool entry::get_bool() const
{
	if (m_type != SDC_BOOL)
		throw std::runtime_error("entry has different type");
	return m_bool;
}

std::string entry::get_string() const
{
	if (m_type != SDC_STRING)
		throw std::runtime_error("entry has different type");
	return m_string;
}

std::vector<char> entry::get_bytes() const
{
	if (m_type != SDC_BYTES)
		throw std::runtime_error("entry has different type");
	return m_bytes;
}

std::vector<entry> entry::get_array() const
{
	if (m_type != SDC_ARRAY)
		throw std::runtime_error("entry has different type");
	return m_array;
}

int entry::container_size() const
{
	if (m_type == SDC_ARRAY)
		return m_array.size();
	if (m_type == SDC_BYTES)
		return m_bytes.size();
	if (m_type == SDC_STRING)
		return m_string.size();
	return 0;
}

void entry::name(const std::string& name)
{
	m_name = name;
}

std::string entry::name() const
{
	return m_name;
}

bool entry::has_name() const
{
	return !m_name.empty();
}

entry& entry::operator=(const entry& other)
{
	m_type = other.m_type;
	m_name = other.m_name;
	copy_entry_from(other);
	return *this;
}

const entry& entry::operator[](size_t index) const
{
	if (m_type != SDC_ARRAY) {
		throw std::runtime_error(
		    "trying to index into non-array entry");
	}

	return m_array[index];
}

void entry::copy_entry_from(const entry& other)
{
	// Call the copy constructors of complex types, otherwise just memcpy()
	// the entry.

	switch (m_type) {
	case SDC_BYTES:
		m_bytes = other.m_bytes;
		break;
	case SDC_STRING:
		m_string = other.m_string;
		break;
	case SDC_ARRAY:
		m_array = other.m_array;
		break;
	default:
		m_ulong = other.m_ulong;
	}
}
