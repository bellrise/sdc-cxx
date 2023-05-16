/* sdc-cxx: reader.cc
   Copyright (c) 2023 bellrise */

#include <iostream>
#include <sdc-cxx.h>
#include <sdc.h>
#include <string.h>

using namespace sdc;

reader::reader() { }
reader::~reader() { }

void reader::read_from_stream(std::istream& stream)
{
	sdc_header header;

	stream.read(reinterpret_cast<char *>(&header), sizeof(header));

	if (memcmp(header.h_magic, SDC_MAGIC, 3)) {
		throw std::runtime_error(
		    "invalid input format: missing magic bytes");
	}

	if (header.h_version != sdc::details::supported_sdc_version) {
		throw std::runtime_error(
		    "reader supports different version than in the SDC format");
	}

	m_info.entries = header.h_entries;
	m_info.flags = header.h_flags;
	m_info.version = header.h_version;

	for (int i = 0; i < m_info.entries; i++)
		m_entries.push_back(read_entry(stream));
}

static void dump_value(const entry& e)
{
	switch (e.type()) {
	case entry::INT:
		std::cout << "int(" << e.get_int() << ')';
		break;
	case entry::LONG:
		std::cout << "long(" << e.get_long() << ')';
		break;
	case entry::UINT:
		std::cout << "uint(" << e.get_uint() << ')';
		break;
	case entry::ULONG:
		std::cout << "ulong(" << e.get_ulong() << ')';
		break;
	case entry::BOOL:
		std::cout << (e.get_bool() ? "true" : "false");
		break;
	case entry::STRING:
		std::cout << "string(" << e.get_string() << ')';
		break;
	case entry::ARRAY:
		std::cout << "array";
		break;
	case entry::BYTES:
		std::cout << "bytes";
		break;
	default:
		std::cout << "null";
	}

	if (e.type() == entry::ARRAY || e.type() == entry::BYTES) {
		std::cout << '[' << e.container_size() << ']';
	}

	std::cout << std::endl;
}

static void dump_entry(const entry& e, int indent)
{
	for (int i = 0; i < indent; i++)
		std::cout << "  ";

	if (e.has_name())
		std::cout << '"' << e.name() << "\": ";

	dump_value(e);

	if (e.type() == e.ARRAY) {
		for (const auto& e : e.get_array())
			dump_entry(e, indent + 1);
	}
}

void reader::dump_tree() const
{
	for (const auto& entry : m_entries)
		dump_entry(entry, 0);
}

int reader::entries_size() const
{
	return m_info.entries;
}

entry reader::as_entry() const
{
	return m_entries;
}

bool reader::has_named(const std::string& name)
{
	try {
		get_named(name);
		return true;
	} catch (std::runtime_error& _) {
		return false;
	}
}

const entry& reader::get_named(const std::string& name)
{
	for (const auto& e : m_entries) {
		if (e.has_name() && e.name() == name)
			return e;
	}

	throw std::runtime_error("no matching named element");
}

const entry& reader::at(size_t index)
{
	return m_entries[index];
}

const entry& reader::operator[](size_t index)
{
	return m_entries[index];
}

template <typename T>
static T streamread(std::istream& stream)
{
	T local;
	stream.read(reinterpret_cast<char *>(&local), sizeof(T));
	return local;
}

static std::string streamread_string(std::istream& stream)
{
	std::string str;
	char c;

	while (1) {
		stream.read(&c, 1);
		if (!c)
			break;
		str += c;
	}

	return str;
}

entry reader::read_entry(std::istream& stream)
{
	std::string name;
	sdc_entry header;
	entry val;

	stream.read(reinterpret_cast<char *>(&header), sizeof(header));

	if (header.e_flags & SDC_ENAMED)
		name = streamread_string(stream);

	switch (header.e_type) {
	case SDC_INT:
		val = streamread<int>(stream);
		break;
	case SDC_LONG:
		val = streamread<long>(stream);
		break;
	case SDC_UINT:
		val = streamread<uint>(stream);
		break;
	case SDC_ULONG:
		val = streamread<ulong>(stream);
		break;
	case SDC_BOOL:
		val = streamread<bool>(stream);
		break;
	case SDC_STRING:
		val = read_string_payload(stream, header.e_size);
		break;
	case SDC_ARRAY:
		val = read_array_payload(stream);
		break;
	case SDC_BYTES:
		val = read_bytes_payload(stream, header.e_size);
		break;
	}

	val.name(name);
	return val;
}

entry reader::read_string_payload(std::istream& stream, int len)
{
	std::string content;

	content.resize(len);
	stream.read(content.data(), len);

	return content;
}

entry reader::read_bytes_payload(std::istream& stream, int len)
{
	std::vector<char> content;

	content.resize(len);
	stream.read(content.data(), len);

	return content;
}

entry reader::read_array_payload(std::istream& stream)
{
	std::vector<entry> fields;
	sdc_array header;

	stream.read(reinterpret_cast<char *>(&header), sizeof(header));

	for (int i = 0; i < header.a_entries; i++)
		fields.push_back(read_entry(stream));

	return fields;
}
