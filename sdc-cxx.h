/* sdc-cxx.h - C++ SDC library
   Copyright (c) 2023 bellrise */

#ifndef SDC_CXX_H
#define SDC_CXX_H 1

#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace sdc {

struct entry
{
	friend struct serializer;

	entry();
	entry(const entry&);
	~entry();

	void set(int);
	void set(long);
	void set(unsigned int);
	void set(unsigned long);
	void set(bool);
	void set(const std::string&);
	void set(const std::span<char>&);
	void set(const std::span<entry>&);

	void name(const std::string& name);
	std::string name() const;

	entry& operator=(const entry&);

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
		std::string m_string;
		std::vector<char> m_bytes;
		std::vector<entry> m_array;
	};

	void copy_entry_from(const entry&);
};

struct serializer
{
	serializer();
	~serializer();

	void write_to_stream(std::ostream&);
	void write_entry(std::ostream&, const entry&);
	void write_array(std::ostream&, const std::vector<entry>&);

	void add(const entry&);

private:
	std::vector<entry> m_entries;

	void write_header(std::ostream&);
};

struct reader
{
	reader();
	~reader();

	void read_from_stream(std::ifstream&);
};

}

#endif /* SDC_CXX_H */
