#include "ISprite.h"


void ISprite::SetLeftPercent(float percent)
{
	if (m_LeftPercent != percent)
	{
		m_LeftPercent = percent;
		m_bDirty = true;
	}
}

void ISprite::SetRightPercent(float percent)
{
	if (m_RightPercent != percent)
	{
		m_RightPercent = percent;
		m_bDirty = true;
	}
}

void ISprite::SetTopPercent(float percent)
{
	if (m_TopPercent != percent)
	{
		m_TopPercent = percent;
		m_bDirty = true;
	}
}

void ISprite::SetDownPercent(float percent)
{
	if (m_DownPercent != percent)
	{
		m_DownPercent = percent;
		m_bDirty = true;
	}
}

float ISprite::GetLeftPercent() const
{
	return m_LeftPercent;
}

float ISprite::GetRightPercent() const
{
	return m_RightPercent;
}

float ISprite::GetTopPercent() const
{
	return m_TopPercent;
}

float ISprite::GetDownPercent() const
{
	return m_DownPercent;
}

void ISprite::SetEdgePercents(float left, float right, float top, float down)
{
	bool changed = false;

	if (m_LeftPercent != left)
	{
		m_LeftPercent = left;
		changed = true;
	}
	if (m_RightPercent != right)
	{
		m_RightPercent = right;
		changed = true;
	}
	if (m_TopPercent != top)
	{
		m_TopPercent = top;
		changed = true;
	}
	if (m_DownPercent != down)
	{
		m_DownPercent = down;
		changed = true;
	}

	if (changed)
		m_bDirty = true;
}

void ISprite::GetEdgePercents(float& left, float& right, float& top, float& down) const
{
	left = m_LeftPercent;
	right = m_RightPercent;
	top = m_TopPercent;
	down = m_DownPercent;
}
