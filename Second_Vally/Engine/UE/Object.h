#pragma once

#include "Tickable.h"
#include "NameTypes.h"

class COREUOBJECT_API UObject : public FTickableGameObject
{
public:
	explicit UObject(const FName& _Name = NAME_None);
	virtual ~UObject();

public:
	const FName& GetFName() const;
	FString GetName() const;

private:
	FName __Name;
	std::vector<std::string>	__Tag;

public:
	template<typename T>
	void AddTag(T&& _Tag);

	void RemoveTag(const std::string& _Tag);
	bool HasTag(const std::string& _Tag) const;
};
