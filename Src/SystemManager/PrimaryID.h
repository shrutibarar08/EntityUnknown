#pragma once
#include <cstdint>

using ID = uint32_t;

class PrimaryID
{
public:
	PrimaryID() : m_AssignedID(++ID_GENERATOR)
	{}

	ID GetAssignedID() const { return m_AssignedID; }

private:
	inline static ID ID_GENERATOR{ 0 };
	ID m_AssignedID;
};
