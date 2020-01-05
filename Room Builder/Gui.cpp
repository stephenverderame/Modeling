#include "Gui.h"
#include "Font.h"
#include <vector>
#include "ShaderManager.h"
#include "Window.h"
#include <sstream>
#include <time.h>

#include <iostream>

Control::Control() : visible(false) {}

Control::Control(float x, float y, float w, float h) : visible(false), x(x), y(y), w(w), h(h)
{
}

const float CELL_HEIGHT = 0.05;

struct ltImpl
{
	std::unique_ptr<GuiRect> bg;
	std::vector<std::shared_ptr<Text>> labels;
	float top;
	float mx, my;
	bool cursorHand;
	std::shared_ptr<Font> f;
	bool click;
};

GuiList::GuiList(float x, float y, float w, float h, std::shared_ptr<Font> f) : Control(x, y, w, h)
{
	lPimpl = std::make_unique<ltImpl>();
	lPimpl->bg = std::make_unique<GuiRect>();
	lPimpl->bg->setColor(glm::vec4(0.2, 0.2, 0.2, 0.8));
	lPimpl->bg->setPos(glm::vec3(x, y, 0.0));
	lPimpl->bg->scale(glm::vec3(w, h, 1.0));
	sid = shaderID::gui;
	lPimpl->top = 0;
	lPimpl->mx = 0;
	lPimpl->my = 0;
	lPimpl->cursorHand = false;
	lPimpl->f = f;
	lPimpl->click = false;
}
GuiList::~GuiList() = default;

void GuiList::nvi_draw(int p)
{
	if (visible) {
		lPimpl->bg->draw();
		int i = 1;
		bool hand = false;
		if (lPimpl->top > 0) lPimpl->top = 0;
		else if (lPimpl->top < -h + CELL_HEIGHT) lPimpl->top = -h + CELL_HEIGHT;
		for (auto& txt : lPimpl->labels) {
			glm::vec3 pos(x, y + h - (CELL_HEIGHT * i++) + lPimpl->top, 0.0);
			if (pos.y >= y - 0.05 && pos.y < y + h) {
				if (lPimpl->mx > x && lPimpl->mx < x + w && lPimpl->my >= pos.y && lPimpl->my <= pos.y + CELL_HEIGHT) {
					if (lPimpl->click) {
						lPimpl->click = false;
						if (onClick != nullptr) onClick(i - 2, txt->getText());
					}
					txt->setColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
					hand = true;
					if (!lPimpl->cursorHand) {
						Window::setCursor(cursorType::hand);
						lPimpl->cursorHand = true;
					}
				}
				else
					txt->setColor(glm::vec4(1.0));
				txt->setPos(pos);
				txt->draw();
			}
		}
		if (!hand && lPimpl->cursorHand) {
			lPimpl->cursorHand = false;
			Window::setCursor(cursorType::normal);
		}
	}

}

void GuiList::notify(const command & c)
{
	switch (c.cmd)
	{
		case msg::gui_scroll:
			lPimpl->top += CELL_HEIGHT * (int)c.args[0];
			break;
		case msg::gui_mouseHover:
		{
			float * args = (float*)c.args[0];
			lPimpl->mx = args[0];
			lPimpl->my = 1.0 - args[1];
			break;
		}
		case msg::gui_click:
			lPimpl->click = true;
			break;
	}
}

bool GuiList::isInterested(msg m)
{
	return true;
}

void GuiList::addItem(const char * name)
{
	auto i = std::make_shared<Text>(lPimpl->f);
	i->setText(name);
	i->setColor(glm::vec4(1));
	lPimpl->labels.push_back(i);
}

struct tfImpl
{
	std::unique_ptr<GuiRect> bg, iBeam;
	std::shared_ptr<Font> fnt;
	std::unique_ptr<Text> t;
	std::stringstream txt;
	bool caps;
	bool typing;
	clock_t iBeamBlink, iBeamStartBlink;
	bool cursorBeam;
	std::string label;
};


TextField::TextField(float x, float y, float w, float h, std::shared_ptr<class Font> f) : Control(x, y, w, h)
{
	sid = shaderID::gui;
	tPimpl = std::make_unique<tfImpl>();
	tPimpl->bg = std::make_unique<GuiRect>();
	tPimpl->iBeam = std::make_unique<GuiRect>();
	tPimpl->bg->setPos(glm::vec3(x, y, 1.0));
	tPimpl->bg->scale(glm::vec3(w, h, 1.0));
	tPimpl->bg->setColor(glm::vec4(1));
	tPimpl->bg->setStroke(glm::vec4(0, 0, 0, 1.0), 0.001);
	tPimpl->iBeam->scale(glm::vec3(0.0001, h, 1.0));
	tPimpl->iBeam->setColor(glm::vec4(0, 0, 0, 1.0));
	tPimpl->fnt = f;
	tPimpl->caps = false;
	tPimpl->iBeamBlink = 0;
	tPimpl->t = std::make_unique<Text>(f);
	tPimpl->t->setPos(glm::vec3(x, y, 1.0));
	tPimpl->t->scale(glm::vec3(1.0));
	tPimpl->t->setColor(glm::vec4(0, 0, 0, 1));
	tPimpl->cursorBeam = false;
}

TextField::~TextField() = default;

void TextField::notify(const command & c)
{
	switch (c.cmd) {
	case msg::gui_keydown:
		if (tPimpl->typing) {
			if ((int)c.args[0] == (int)keyCode::capsLock) tPimpl->caps = !tPimpl->caps;
			if (tPimpl->caps) {
				if (isalpha((int)c.args[0])) tPimpl->txt << (char)(Window::isKeyPress(keyCode::shift) ? tolower((int)c.args[0]) : (int)c.args[0]);
			}
			else {
				if (isalpha((int)c.args[0])) tPimpl->txt << (char)(Window::isKeyPress(keyCode::shift) ? ((int)c.args[0]) : tolower((int)c.args[0]));
			}
			if ((int)c.args[0] == ' ') tPimpl->txt << ' ';
			else if ((int)c.args[0] == (int)keyCode::back && tPimpl->txt.str().size() >= 1) {
				std::string str = tPimpl->txt.str();
				tPimpl->txt.str(str.substr(0, str.size() - 1));
			}
			else if (!isalpha((int)c.args[0])) {
				char ch = Window::keyToLetter((keyCode)(int)c.args[0]);
				if (ch != 0) tPimpl->txt << ch;
			}
		}
		break;
	case msg::gui_mouseHover:
	case msg::gui_click:
	{
		float * ndc = (float*)c.args[0];
		float mx = ndc[0];
		float my = 1.0 - ndc[1];
		if (mx >= x && mx <= x + w && my >= y && my <= y + h) {
			if (c.cmd == msg::gui_click) tPimpl->typing = !tPimpl->typing;
			if (c.cmd == msg::gui_mouseHover || tPimpl->typing) {
				if (!tPimpl->cursorBeam) {
					Window::setCursor(cursorType::typing);
					tPimpl->cursorBeam = true;
				}
			}
		}
		else {
			if (tPimpl->cursorBeam) {
				Window::setCursor(cursorType::normal);
				tPimpl->cursorBeam = false;
			}
			if (c.cmd == msg::gui_click) tPimpl->typing = false;
		}
		break;
	}
	}
}

bool TextField::isInterested(msg m)
{
	return true;
}

void TextField::setLabel(const char * title)
{
	tPimpl->label = title;
}

std::string TextField::getText()
{
	return tPimpl->txt.str();
}

void TextField::nvi_draw(int p)
{
	tPimpl->bg->draw();
	tPimpl->t->setText(tPimpl->txt.str().c_str());
	if (tPimpl->typing) {
		if (clock() - tPimpl->iBeamBlink > CLOCKS_PER_SEC) {
			tPimpl->iBeamBlink = clock();
			tPimpl->iBeamStartBlink = clock();
		}
		if (clock() - tPimpl->iBeamStartBlink < CLOCKS_PER_SEC / 5.0) {
			float w = tPimpl->fnt->getNDCWidth(tPimpl->txt.str().c_str(), 1.0);
			tPimpl->iBeam->setPos(glm::vec3(x + w, y, 1.0));
			tPimpl->iBeam->draw();
		}
	}
	tPimpl->t->draw();
	if (tPimpl->label.size() >= 1) {
		tPimpl->t->setText(tPimpl->label.c_str());
		tPimpl->t->setPos(glm::vec3(x, y + h, 1.0));
		tPimpl->t->draw();
		tPimpl->t->setPos(glm::vec3(x, y, 1.0));
	}
}
struct bImpl
{
	std::unique_ptr<Text> label;
	std::unique_ptr<GuiRect> bg;
	bool hover;
};
void Button::nvi_draw(int p)
{
	bPimpl->bg->draw();
	bPimpl->label->draw();
}
Button::Button(float x, float y, float w, float h, std::shared_ptr<class Font> f) : Control(x, y, w, h)
{
	sid = shaderID::gui;
	bPimpl = std::make_unique<bImpl>();
	bPimpl->label = std::make_unique<Text>(f);
	bPimpl->bg = std::make_unique<GuiRect>();
	bPimpl->bg->setPos(glm::vec3(x, y, 1.0));
	bPimpl->bg->scale(glm::vec3(w, h, 1.0));
	bPimpl->bg->setColor(glm::vec4(0.2, 0.2, 0.2, 0.8));
	bPimpl->bg->setStroke(glm::vec4(1), 0.001);
	bPimpl->label->setColor(glm::vec4(1));
	bPimpl->hover = false;
}

Button::~Button() = default;

void Button::setText(const char * txt)
{
	bPimpl->label->setText(txt);
	bPimpl->label->setPos(glm::vec3(x + w / 2.0 - strlen(txt) * 0.015 / 2.0, y, 1.0));
}

void Button::notify(const command & c)
{
	if (c.cmd == msg::gui_mouseHover || c.cmd == msg::gui_click) {
		float * ndc = (float*)c.args[0];
		float mx = ndc[0];
		float my = 1.0 - ndc[1];
		if (mx > x && mx < x + w && my >= y && my <= y + h) {
			if (!bPimpl->hover) {
				bPimpl->hover = true;
				Window::setCursor(cursorType::hand);
			}
			if (c.cmd == msg::gui_click && onClick != nullptr) onClick();
		}
		else if (bPimpl->hover) {
			bPimpl->hover = false;
			Window::setCursor(cursorType::normal);
		}
	}
}

bool Button::isInterested(msg m)
{
	return true;
}
