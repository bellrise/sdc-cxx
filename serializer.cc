/* sdc-cxx: serializer.cc
   Copyright (c) 2023 bellrise */

#include <sdc-cxx.h>
#include <sdc.h>
#include <string.h>

using namespace sdc;

serializer::serializer()
{
	if (sdc::details::supported_sdc_version != SDC_VERSION) {
		throw std::logic_error(
		    "invalid sdc.h header - serializer supports different "
		    "version than SDC_VERSION is defined as");
	}
}

serializer::~serializer() { }

void serializer::add(const entry& val)
{
	m_entries.push_back(val);
}

void serializer::add_named(const entry& val, const std::string& name)
{
	entry copy = val;

	copy.name(name);
	m_entries.push_back(std::move(copy));
}

void serializer::write_to_stream(std::ostream& stream)
{
	write_header(stream);

	for (const auto& entry : m_entries)
		write_entry(stream, entry);
}

void serializer::write_header(std::ostream& stream)
{
	sdc_header header;

	memcpy(header.h_magic, SDC_MAGIC, 3);
	header.h_version = SDC_VERSION;
	header.h_flags = 0;
	header.h_customflags = 0;
	header.h_entries = m_entries.size();

	stream.write(reinterpret_cast<char *>(&header), sizeof(header));
}

template <typename T>
static void streamput(std::ostream& stream, const T *value)
{
	stream.write(reinterpret_cast<const char *>(value), sizeof(*value));
}

void serializer::write_entry(std::ostream& stream, const entry& entry)
{
	const std::string& name = entry.m_name;
	std::array<char, SDC_MAXLONGNAME> namebuf;
	size_t namesiz = 0;
	sdc_entry header;

	header.e_type = entry.m_type;
	header.e_size = 0;
	header.e_flags = 0;

	if (!name.empty()) {
		header.e_flags |= SDC_ENAMED;
		if (name.size() > SDC_MAXNAME)
			header.e_flags |= SDC_ELONGNAME;

		namesiz = std::min(name.size(),
				   static_cast<size_t>(SDC_MAXLONGNAME - 1));
		memcpy(namebuf.data(), name.data(), namesiz);
	}

	if (entry.m_type == SDC_STRING)
		header.e_size = entry.m_string.size();
	if (entry.m_type == SDC_BYTES)
		header.e_size = entry.m_bytes.size();

	// Write the header, then the name, and finally the payload.

	stream.write(reinterpret_cast<char *>(&header), sizeof(header));
	if (namesiz) {
		stream.write(namebuf.data(), namesiz);
		stream.put(0);
	}

	switch (entry.m_type) {
	case SDC_INT:
		streamput(stream, &entry.m_int);
		break;
	case SDC_LONG:
		streamput(stream, &entry.m_long);
		break;
	case SDC_UINT:
		streamput(stream, &entry.m_uint);
		break;
	case SDC_ULONG:
		streamput(stream, &entry.m_ulong);
		break;
	case SDC_BOOL:
		streamput(stream, &entry.m_bool);
		break;
	case SDC_STRING:
		stream.write(entry.m_string.data(), entry.m_string.size());
		break;
	case SDC_BYTES:
		stream.write(entry.m_bytes.data(), entry.m_bytes.size());
		break;
	case SDC_ARRAY:
		write_array(stream, entry.m_array);
		break;
	default:
		break;
	}
}

void serializer::write_array(std::ostream& stream,
			     const std::vector<entry>& array)
{
	sdc_array header;

	header.a_entries = array.size();

	stream.write(reinterpret_cast<const char *>(&header), sizeof(header));

	for (const auto& entry : array)
		write_entry(stream, entry);
}
