#pragma once


class ISystemRender: public PrimaryID
{
public:
	ISystemRender() = default;
	virtual ~ISystemRender() = default;
	ISystemRender(const ISystemRender&) = delete;
	ISystemRender(ISystemRender&&) = delete;
	ISystemRender& operator=(const ISystemRender&) = delete;
	ISystemRender& operator=(ISystemRender&&) = delete;

	virtual void RenderBegin() = 0;
	virtual void RenderExecute() = 0;
	virtual void RenderEnd() = 0;
};
