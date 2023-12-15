#pragma once

template <typename T>
class TSingleton
{
public:
	static T& GetInstance()
	{
		static T Instance;
		return Instance;
	}
};
