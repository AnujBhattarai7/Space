#pragma once

typedef unsigned long long   uint64_t;
typedef long unsigned int _size_t_;

namespace Space {

	class UUID
	
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};

}

namespace std {
	template <typename T> struct hash;

	template<>
	struct hash<Space::UUID>
	{
		_size_t_ operator()(const Space::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};

}