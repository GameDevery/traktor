#ifndef Indentation_H
#define Indentation_H

#include "Condition.h"

class Indentation : public Condition
{
	T_RTTI_CLASS;

public:
	virtual void check(const traktor::Path& fileName, const Source& source, bool isHeader, traktor::OutputStream& report) const;
};

#endif	// Indentation_H
