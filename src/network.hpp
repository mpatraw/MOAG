
#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <cassert>
#include <algorithm>
#include <memory>
#include <vector>

#include <enet/enet.h>

namespace m {

class network_impl;

const uint8_t packet_type_connection = 0;
const uint8_t packet_type_disconnection = 1;
const uint8_t packet_type_user_defined = 2;

class packet final {
public:
	packet() : pos{ 0 } {}

	bool empty() const { return chunk.empty(); }
	size_t size() const { return chunk.size(); }
	const uint8_t *data() const { return chunk.data(); }
	const uint8_t *remaining_data() const { return data() + pos; }
	size_t remaining_size() const { return size() - pos; }
	bool can_read(size_t amount = 1) const { return remaining_size() >= amount; }

	void reread() { pos = 0; }
	void rewrite() { chunk.clear(); }

	void load(uint8_t *data, size_t len) {
		std::copy_n(data, len, std::back_inserter(chunk));
	}

	packet &operator <<(uint8_t i) {
		chunk.push_back(i);
		return *this;
	}
	packet &operator <<(int8_t i) {
		*this << static_cast<uint8_t>(i);
		return *this;
	}
	packet &operator <<(uint16_t i) {
		auto v = htons(i);
		chunk.push_back((v >> 0) & 0xff);
		chunk.push_back((v >> 8) & 0xff);
		return *this;
	}
	packet &operator <<(int16_t i) {
		*this << static_cast<uint16_t>(i);
		return *this;
	}
	packet &operator <<(uint32_t i) {
		auto v = htonl(i);
		chunk.push_back((v >> 0) & 0xff);
		chunk.push_back((v >> 8) & 0xff);
		chunk.push_back((v >> 16) & 0xff);
		chunk.push_back((v >> 24) & 0xff);
		return *this;
	}
	packet &operator <<(int32_t i) {
		*this << static_cast<uint32_t>(i);
		return *this;
	}
	packet &operator <<(const char *str) {
		while (*str) {
			chunk.push_back(*str++);
		}
		chunk.push_back(0);
		return *this;
	}


	packet &operator >>(uint8_t &i) {
		assert(can_read(1));
		i = chunk[pos++];
		return *this;
	}
	packet &operator >>(int8_t &i) {
		*this >> reinterpret_cast<uint8_t &>(i);
		return *this;
	}
	packet &operator >>(uint16_t &i) {
		assert(can_read(2));
		uint16_t v{ 0 };
		v |= chunk[pos++] << 0;
		v |= chunk[pos++] << 8;
		i = ntohs(v);
		return *this;
	}
	packet &operator >>(int16_t &i) {
		*this >> reinterpret_cast<uint16_t &>(i);
		return *this;
	}
	packet &operator >>(uint32_t &i) {
		assert(can_read(4));
		uint32_t v{ 0 };
		v |= chunk[pos++] << 0;
		v |= chunk[pos++] << 8;
		v |= chunk[pos++] << 16;
		v |= chunk[pos++] << 24;
		i = ntohl(v);
		return *this;
	}
	packet &operator >>(int32_t &i) {
		*this >> reinterpret_cast<uint32_t &>(i);
		return *this;
	}
	packet &operator >>(std::string &str) {
		while (auto c = chunk[pos++]) {
			str.push_back(c);
		}
		return *this;
	}

private:
	std::vector<uint8_t> chunk;
	size_t pos;
};

class client final {
public:
	client();
	~client();
	client(const client &) = delete;
	client(client &&) = delete;
	client &operator =(client) = delete;

	packet &recv();
	void send(const packet &p, bool reliable = true);

	uint32_t rtt() const;
private:
	std::unique_ptr<network_impl> impl;
};

class server final {
public:
	server();
	~server();
	server(const server &) = delete;
	server(server &&) = delete;
	server &operator =(server) = delete;

	bool is_connected(int id) const;

	packet &recv(int &id);
	void send(const packet &p, int id, bool reliable = true);
	void broadcast(const packet &p, bool reliable = true);

	uint32_t rtt(int id) const;
private:
	std::unique_ptr<network_impl> impl;
};

}

#endif