#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <mutex>

#include "nes.h"

#include "video.h"
#include "audio.h"
#include "input.h"

#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "SFML/System.hpp"
#include <SFML/Window.hpp>


bool wait = false;


class KeyboardInput : public Input
{
public:
	const ButtonState& GetButtons()
	{
		state.a = sf::Keyboard::isKeyPressed(sf::Keyboard::F);
		state.b = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
		state.select = sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
		state.start = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter);
		state.up = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
		state.down = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
		state.left = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
		state.right = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
		return state;
	}

private:
	ButtonState state = { 0 };
};


class AudioDevice : public Audio, public sf::SoundStream
{
public:
	AudioDevice() : mSamples(4096)
	{
		initialize(1, 48000);
		//play();
		time0 = std::chrono::high_resolution_clock::now();
	}

public:
	void Update(const AudioBuffer& buffer) override
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (finished)
		{
			mSamples.clear();
			finished = false;
		}
			
		
		for (int i = 0; i < buffer.size(); i += 37)
		{
			sbyte sample = buffer[i];
			//sample -= 8;
			//std::cout << (int)buffer[i] << " " << (int)sample << "\n";
			sample *= 2048;
			//mSamples.push_back(sample);
		}

		//std::cout << mSamples.size() << "\n";
	}

	bool onGetData(Chunk& data) override
	{
		auto now = std::chrono::high_resolution_clock::now();
		//std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(now - time0).count() << "\n";
		//std::cout << mSamples.size() << "\n";
		time0 = now;

		std::lock_guard<std::mutex> lock(mutex);
		data.samples = mSamples.data();
		data.sampleCount = mSamples.size();
		//std::cout << "ON GET DATA" << std::endl;
		finished = true;
		return true;
	}

	void onSeek(sf::Time timeOffset) override
	{
		//std::cout << "ON SEEK\n";
	}

private:
	bool finished = false;
	std::vector<sword> mSamples;
	std::mutex mutex;
	decltype(std::chrono::high_resolution_clock::now()) time0;
	decltype(std::chrono::high_resolution_clock::now()) time1;
};


//using ScreenBuffer = uint8_t[256][240];

class VideoDevice : public Video
{
public:
	VideoDevice(sf::RenderWindow* window, sf::Text& text) : mWindow(window), mText(text)
	{
		mTarget.create(512, 480);
		mImage.create(256, 240, sf::Color::Cyan);
		mTexture.create(256, 240);
		mTexture.update(mImage);
		mSprite.setTexture(mTexture, true);
		mSprite.setScale(2, 2);
		mText.setPosition(30, 30);
	}

public:
	void Update(const ScreenBuffer& buffer)
	{
		for (int y = 0; y < 240; y++)
		{
			for (int x = 0; x < 256; x++)
			{
				mImage.setPixel(x, y, palette[buffer[y][x]]);
			}
		}
		
		mTexture.update(mImage);

		mTarget.clear();
		mTarget.draw(mSprite);
		//if (frames < 300)
			//mTarget.draw(mText);
		mTarget.display();
		
		mWindow->clear();
		//mWindow->draw(mSprite);
		//mWindow->draw(*debug.GetObjects()[0]);
		sf::Sprite sprite(mTarget.getTexture());

		mWindow->draw(sprite);
		//mWindow->draw(mText);
		mWindow->display();

	
		//std::cout << frames++ << std::endl;
	}

private:
	sf::RenderTexture mTarget;
	sf::RenderWindow* mWindow;
	sf::Image mImage;
	sf::Texture mTexture;
	sf::Sprite mSprite;
	sf::Text& mText;
	int frames = 0;

private:
	//Debug debug;

private:
	sf::Color palette[64] = {
		sf::Color(101, 101, 101),
		sf::Color(0, 45, 105),
		sf::Color(19, 31, 127),
		sf::Color(60, 19, 124),
		sf::Color(96, 11, 98),
		sf::Color(115, 10, 55),
		sf::Color(113, 15, 7),
		sf::Color(90, 26, 0),
		sf::Color(52, 40, 0),
		sf::Color(11, 52, 0),
		sf::Color(0, 60, 0),
		sf::Color(0, 61, 16),
		sf::Color(0, 56, 64),
		sf::Color(0, 0, 0),
		sf::Color(0, 0, 0),
		sf::Color(0, 0, 0),
		sf::Color(174, 174, 174),
		sf::Color(15, 99, 179),
		sf::Color(64, 81, 208),
		sf::Color(120, 65, 204),
		sf::Color(167, 54, 169),
		sf::Color(192, 52, 112),
		sf::Color(189, 60, 48),
		sf::Color(159, 74, 0),
		sf::Color(109, 92, 0),
		sf::Color(54, 109, 0),
		sf::Color(7, 119, 4),
		sf::Color(0, 121, 61),
		sf::Color(0, 114, 125),
		sf::Color(0, 0, 0),
		sf::Color(0, 0, 0),
		sf::Color(0, 0, 0),
		sf::Color(254, 254, 255),
		sf::Color(93, 179, 255),
		sf::Color(143, 161, 255),
		sf::Color(200, 144, 255),
		sf::Color(247, 133, 250),
		sf::Color(255, 131, 192),
		sf::Color(255, 139, 127),
		sf::Color(239, 154, 73),
		sf::Color(189, 172, 44),
		sf::Color(133, 188, 47),
		sf::Color(85, 199, 83),
		sf::Color(60, 201, 140),
		sf::Color(62, 194, 205),
		sf::Color(78, 78, 78),
		sf::Color(0, 0, 0),
		sf::Color(0, 0, 0),
		sf::Color(254, 254, 255),
		sf::Color(188, 223, 255),
		sf::Color(209, 216, 255),
		sf::Color(232, 209, 255),
		sf::Color(251, 205, 253),
		sf::Color(255, 204, 229),
		sf::Color(255, 207, 202),
		sf::Color(248, 213, 180),
		sf::Color(228, 220, 168),
		sf::Color(204, 227, 169),
		sf::Color(185, 232, 184),
		sf::Color(174, 232, 208),
		sf::Color(175, 229, 234),
		sf::Color(182, 182, 182),
		sf::Color(0, 0, 0),
		sf::Color(0, 0, 0)
	};
};


int main()
{
	sf::RenderWindow window(sf::VideoMode(512, 480), "NESt");

	sf::Font mFont;
	mFont.loadFromFile("consolab.ttf");

	sf::Text text("THIS IS A TEST STRING", mFont, 16);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(1);
	
	KeyboardInput input;
	AudioDevice audio;
	VideoDevice video(&window, text);

	NES nes(&video, &audio, &input);
	//nes.LoadGame("donkey_kong.nes");
	nes.LoadGame("Super_mario_brothers.nes");

	


	while (window.isOpen())
	{

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
				goto EXIT;
			}
			
			if (event.type == sf::Event::KeyPressed)
			{
				nes.DebugKey(event.key.code);
				if (event.key.code == 1)
					wait = true;
				else if (event.key.code == 2)
				{
					wait = false;
				}
			}
		}

		if (!wait)
			nes.Step();
		//window.draw(text);
		//window.display();
	}

EXIT:
	return 0;
}
