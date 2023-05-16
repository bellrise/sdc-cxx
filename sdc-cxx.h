/* sdc-cxx.h - C++ SDC library
   Copyright (c) 2023 bellrise */

#ifndef SDC_CXX_H
#define SDC_CXX_H 1

#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace sdc {

/* Represents a single entry (or value) in the SDC format. Note that an array is
   also an entry, with multiple entries within. */

struct entry
{
	friend struct serializer;

	entry();
	entry(const entry&);
	~entry();

	template <typename T>
	entry(const T& inplace_ref)
	{
		set(inplace_ref);
	}

	/* Interchangable with the values from <sdc.h>. */
	enum entry_type
	{
		_NULL = 0,
		INT,
		LONG,
		UINT,
		ULONG,
		BOOL,
		STRING,
		ARRAY,
		BYTES,
	};

	void set(int);
	void set(long);
	void set(unsigned int);
	void set(unsigned long);
	void set(bool);
	void set(const char *);
	void set(const std::string&);
	void set(const std::span<char>&);
	void set(const std::vector<char>&);
	void set(const std::span<entry>&);
	void set(const std::vector<entry>&);

	entry_type type() const;

	/* Several getters, they will throw if the current type is not the
	   correct one. Note that these will not cast one type to another,
	   so first check with .type(). */

	int get_int() const;
	long get_long() const;
	unsigned int get_uint() const;
	unsigned long get_ulong() const;
	bool get_bool() const;
	std::string get_string() const;
	std::vector<char> get_bytes() const;
	std::vector<entry> get_array() const;

	/* Returns the size() of the string, bytes or array value. */

	int container_size() const;

	/* Set & get the field name. */

	void name(const std::string& name);
	std::string name() const;
	bool has_name() const;

	entry& operator=(const entry&);
	const entry& operator[](size_t index) const;

private:
	int m_type;
	std::string m_name;

	union
	{
		int m_int;
		long m_long;
		unsigned int m_uint;
		unsigned long m_ulong;
		bool m_bool;
	};

	std::string m_string;
	std::vector<char> m_bytes;
	std::vector<entry> m_array;

	void copy_entry_from(const entry&);
};

/* Serializer for putting data into a binary SDC file. Use add() and add_named()
   methods to insert entries, and insert the serialized binary data into a
   stream using write_to_stream(). */

struct serializer
{
	serializer();
	~serializer();

	void write_to_stream(std::ostream&);
	void write_entry(std::ostream&, const entry&);
	void write_array(std::ostream&, const std::vector<entry>&);

	void add(const entry&);
	void add_named(const entry&, const std::string& name);

private:
	std::vector<entry> m_entries;

	void write_header(std::ostream&);
};

/* Read serialized SDC data from a binary stream. Call read_from_stream() on
   said stream, and then use the several accessors to get the parsed entries. */

struct reader
{
	reader();
	~reader();

	void read_from_stream(std::istream&);
	void dump_tree() const;
	int entries_size() const;

	/* Accessors */

	entry as_entry() const;

	bool has_named(const std::string& name);
	const entry& get_named(const std::string& name);

	const entry& at(size_t index);
	const entry& operator[](size_t index);

private:
	std::vector<entry> m_entries;
	struct metadata
	{
		int entries;
		int version;
		int flags;
	} m_info;

	entry read_entry(std::istream&);
	entry read_string_payload(std::istream&, int len);
	entry read_bytes_payload(std::istream&, int len);
	entry read_array_payload(std::istream&);
};

namespace details {

/* If this value is different from the SDC_VERSION macro defined in sdc.h which
   is included in the source files, then the library will throw if you try to
   serialize or read from a stream. */
inline constexpr int supported_sdc_version = 0;

}

}

#endif /* SDC_CXX_H */
