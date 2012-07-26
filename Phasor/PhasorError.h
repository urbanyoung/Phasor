#pragma once

#include <string>

namespace Phasor
{
	class PhasorError
	{
	private:
		std::string err;
		bool hasErr;

	protected:
		void SetError(const std::string& error);

	public:
		PhasorError();
		~PhasorError();
		std::string GetError() const;
		bool hasError() const;

	};
}