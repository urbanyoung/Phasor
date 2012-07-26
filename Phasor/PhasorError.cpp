#include "PhasorError.h"

namespace Phasor
{
	PhasorError::PhasorError()
	{
		hasErr = false;
	}

	PhasorError::~PhasorError()
	{

	}

	void PhasorError::SetError(const std::string& error)
	{
		printf("Error: %s\n", error.c_str());
		this->err = error;
		hasErr = true;
	}

	std::string PhasorError::GetError() const
	{
		return err;
	}

	bool PhasorError::hasError() const
	{
		return hasErr;
	}
}