#pragma once
#include <memory>
struct fnImpl;
class Font
{
	std::unique_ptr<fnImpl> pimpl;
public:
	Font(const char * fontFile);
	void drawText(const char * txt, float x, float y, float scale);
	~Font();
};

