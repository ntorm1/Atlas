export module AtlasException;

import <exception>;

import AtlasTypes;

namespace Atlas
{

export class AtlasException : public std::exception
{
private:
	String m_message;

public:
	AtlasException(String message) noexcept : m_message(std::move(message)) {}
	~AtlasException() noexcept override = default;
	//AtlasException(const AtlasException&) = delete;
	//AtlasException(AtlasException&&) = delete;
	//AtlasException& operator=(const AtlasException&) = delete;
	//AtlasException& operator=(AtlasException&&) = delete;

	const char* what() const noexcept override
	{
		return m_message.c_str();
	}

};	


}