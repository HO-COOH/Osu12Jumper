#include "include/Mania.Pattern.hpp"
#include <unordered_set>
#include <algorithm>

bool Mania::Pattern::colunmHasObject(int column) const
{
	return std::any_of(hitObjects.cbegin(), hitObjects.cend(), [column, this](auto const& objPtr) { return objPtr->getColumnIndex(totalColumn) == column; });
}

int Mania::Pattern::numColumnWithObject() const
{
	std::unordered_set<int> column;
	for (auto const& hitObject : hitObjects)
		column.insert(hitObject->getColumnIndex(totalColumn));
	return static_cast<int>(column.size());
}

Mania::Pattern& Mania::Pattern::operator+=(std::unique_ptr<HitObject>&& hitObject)
{
	hitObjects.emplace_back(std::move(hitObject));
	return *this;
}

Mania::Pattern& Mania::Pattern::operator+=(Pattern&& pattern)
{
	hitObjects.reserve(hitObjects.size() + pattern.hitObjects.size());
	//hitObjects.insert(hitObjects.end(), pattern.hitObjects.begin(), pattern.hitObjects.end());
	std::move(pattern.hitObjects.begin(), pattern.hitObjects.end(), std::back_inserter(hitObjects));
	pattern.hitObjects.clear();
	return *this;
}

std::ostream& Mania::operator<<(std::ostream& os, Pattern::Type type)
{
	if (type == Pattern::Type::None)
	{
		os << "None";
		return os;
	}

	Pattern::Type const copy = type;
	for (int i = 1; i < sizeof(type) * 8; ++i)
	{
		reinterpret_cast<unsigned&>(type) &= (1 << (i - 1));

		if (type & Pattern::Type::ForceStack)
			os << "ForceStack, ";
		else if (type & Pattern::Type::ForceNotStack)
			os << "ForceNotStack, ";
		else if (type & Pattern::Type::KeepSingle)
			os << "KeepSingle, ";
		else if (type & Pattern::Type::LowProbability)
			os << "LowProbability, ";
		else if (type & Pattern::Type::Alternate)
			os << "Alternate, ";
		else if (type & Pattern::Type::ForceSigSlider)   
			os << "ForceSigSlider, ";
		else if (type & Pattern::Type::ForceNotSlider)
			os << "ForceNotSlider, ";
		else if (type & Pattern::Type::Gathered)
			os << "Gathered, ";
		else if (type & Pattern::Type::Mirror)
			os << "Mirror, ";
		else if (type & Pattern::Type::Reverse)
			os << "Reverse, ";
		else if (type & Pattern::Type::Cycle)
			os << "Cycle, ";
		else if (type & Pattern::Type::Stair)
			os << "Stair, ";
		else if (type & Pattern::Type::ReverseStair)
			os << "ReverseStair, ";

		type = copy;
	}
	return os;
}
