#pragma once

#include "nest.h"

#include "SFML/Graphics.hpp"

class Debug
{
public:
	enum class Overlay
	{
		NONE,
		NAMETABLE,
		PATTERNTABLE,
		TEXT
	};

public:
	Debug();

public:
	const std::vector<sf::Drawable*>& GetObjects();

public:
	void SetOverlayType(Overlay type);
	
public:
	int AddText();
	void RemoveText(int handle);
	void UpdateText(int handle, std::string text);

private:
	std::vector<sf::Drawable*> mObjects;
	sf::Font mFont;
};

