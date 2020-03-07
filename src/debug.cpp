#include "debug.h"

Debug::Debug() : mFont()
{
	//mImage.create(512, 480, sf::Color::Transparent);
	//mTexture.create(512, 480);
	//mTexture.update(mImage);
	//mSprite.setTexture(mTexture, true);
	mFont.loadFromFile("consolab.ttf");
	
	auto text = new sf::Text("THIS IS A TEST STRING", mFont, 16);
	text->setOutlineColor(sf::Color::Black);
	text->setOutlineThickness(1);
	mObjects.push_back(text);

}

const std::vector<sf::Drawable*>& Debug::GetObjects()
{
	return mObjects;
}

void Debug::SetOverlayType(Overlay type)
{
}

int Debug::AddText()
{
	return 0;
}

void Debug::RemoveText(int handle)
{
}

void Debug::UpdateText(int handle, std::string text)
{
}
