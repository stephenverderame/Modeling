#pragma once
#include "Object.h"
#include "Observer.h"
#include <functional>
#include <string>

class Control : public Object, public Observer
{
protected:
	bool visible;
	float x, y, w, h;
public:
	Control();
	Control(float x, float y, float w, float h);
	void setVisible(bool v) { visible = v; }
	bool isVisible() const { return visible; }
};

struct ltImpl;
class GuiList : public Control
{
private:
	std::unique_ptr<ltImpl> lPimpl;
	int scrollPos;
	std::function<void(int index, const char * name)> onClick;
protected:
	void nvi_draw(int p) override;
public:
	GuiList(float x, float y, float w, float h, std::shared_ptr<class Font> f);
	~GuiList();
	void notify(const command & c) override;
	bool isInterested(msg m) override;
	void addItem(const char * name);
	void setOnClick(std::function<void(int index, const char * name)> c) { onClick = c; }
};
struct tfImpl;
class TextField : public Control
{
private:
	std::unique_ptr<tfImpl> tPimpl;
protected:
	void nvi_draw(int p) override;
public:
	TextField(float x, float y, float w, float h, std::shared_ptr<class Font> f);
	~TextField();
	void notify(const command & c) override;
	bool isInterested(msg m) override;
	void setLabel(const char * title);
	std::string getText();
};
struct bImpl;
class Button : public Control
{
	std::unique_ptr<bImpl> bPimpl;
	std::function<void()> onClick;
protected:
	void nvi_draw(int p) override;
public:
	Button(float x, float y, float w, float h, std::shared_ptr<class Font> f);
	~Button();
	void setText(const char * txt);
	void setCallback(std::function<void()> f) { onClick = f; }
	void notify(const command & c) override;
	bool isInterested(msg m) override;
};