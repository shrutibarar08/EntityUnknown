#pragma once


class IRender: public PrimaryID
{
public:
	IRender() = default;
	virtual ~IRender() = default;
	IRender(const IRender&) = delete;
	IRender(IRender&&) = delete;
	IRender& operator=(const IRender&) = delete;
	IRender& operator=(IRender&&) = delete;

	virtual void RenderBegin() = 0;
	virtual void RenderExecute() = 0;
	virtual void RenderEnd() = 0;
};
