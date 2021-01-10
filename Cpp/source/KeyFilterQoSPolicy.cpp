#include "OPSTypeDefs.h"
#include "KeyFilterQoSPolicy.h"
#include "OPSObject.h"
namespace ops
{
	KeyFilterQoSPolicy::KeyFilterQoSPolicy() noexcept
	{
	}
	KeyFilterQoSPolicy::KeyFilterQoSPolicy(ObjectKey_T const keyString)
	{
		keyStrings.push_back(keyString);
	}
	KeyFilterQoSPolicy::KeyFilterQoSPolicy(std::vector<ObjectKey_T> const keyStrings)
	{
		this->keyStrings = keyStrings;
	}
	void KeyFilterQoSPolicy::setKeys(std::vector<ObjectKey_T> const keyStrings)
	{
		const SafeLock lock(*this);

		this->keyStrings = keyStrings;
	}
	void KeyFilterQoSPolicy::setKey(ObjectKey_T const key)
	{
		const SafeLock lock(*this);

		this->keyStrings.clear();
		this->keyStrings.push_back(key);
	}
	std::vector<ObjectKey_T> KeyFilterQoSPolicy::getKeys()
	{
		const SafeLock lock(*this);

		return keyStrings;
	}
    
	KeyFilterQoSPolicy::~KeyFilterQoSPolicy()
    {
    }
    
    bool KeyFilterQoSPolicy::applyFilter(const OPSObject* const o)
	{
		const SafeLock lock(*this);

		// An empty key filter is the same as no filter
		if (keyStrings.size() == 0) { return true; }

		for (unsigned int i = 0; i < keyStrings.size(); i++) {
			if(o->getKey() == keyStrings[i]) {
				return true;	// match, so keep this object
			}
		}
		return false;	// no match, skip this object
    }
}
