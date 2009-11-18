#ifndef traktor_db_IMessageListener_H
#define traktor_db_IMessageListener_H

#include "Core/Object.h"

namespace traktor
{
	namespace db
	{

class IMessage;

/*! \brief Message listener interface.
 * \ingroup Database
 */
class IMessageListener : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool notify(const IMessage* message) = 0;
};

	}
}

#endif	// traktor_db_IMessageListener_H
