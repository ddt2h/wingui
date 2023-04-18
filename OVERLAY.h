#pragma once
#include <SFML/Graphics.hpp>
#include <Windows.h>

#include <iostream>

#include <vector>


class SingleClick
{
private:
	bool locked;
public:
	SingleClick()
	{

	}
	bool isClicked(sf::Keyboard::Key key)
	{
		bool ret = false;
		if (sf::Keyboard::isKeyPressed(key) && !locked)
		{
			locked = true;
			ret = true;
		}
		if (!sf::Keyboard::isKeyPressed(key))
			locked = false;
		return ret;
	}
};

class Label
{
	HDC wdc;
	COLORREF color;
	RECT rect;
	bool isActive = false;
	sf::Clock clock;
	float self_incr;
public:
	int x, y;
	RECT crect;
	std::wstring txt;
	
	Label()
	{

	}
	void create(int x, int y, std::wstring text, HDC wdc, COLORREF color)
	{
		this->x = x;
		this->y = y;
		this->txt = text;
		this->wdc = wdc;
		this->color = color;

		rect.left = x;
		rect.top = y;

		draw(2);//is needed for proper visibility
	}
	void setActive(bool ac)
	{
		isActive = ac;
	}
	void activeState()
	{
		self_incr = clock.getElapsedTime().asSeconds() * 500;
		SetTextColor(wdc, RGB(255 - self_incr, self_incr, self_incr));
	}
	void draw(int state = 1)
	{
	
		SetBkMode(wdc, state); //use 2 for clearing

		if (!isActive)
			SetTextColor(wdc, color);
		if (isActive)
			activeState();
	
		DrawText(wdc, txt.c_str(), -1, &rect, DT_NOCLIP);
			
	}
};
class Column
{
private:
	Label title;
	
	std::vector<Label> names;

	bool isSelected;

	SingleClick arrow_down;
	SingleClick arrow_up;

	int selected_id;
	
	void addLabel(HDC hdc, int xx, int yy, std::wstring title, COLORREF clr)
	{
		Label label;
		label.create(xx, yy, title, hdc, clr);
		label.draw(2); //must have bc idk y
		names.push_back(label);
	}
	void arrowsHandle()
	{
		if (arrow_down.isClicked(sf::Keyboard::Down))
		{
			selected_id++;

			if (selected_id > names.size() - 1)
				selected_id = names.size() - 1;
			deactiveAll();
			names[selected_id].setActive(true);

		}
		if (arrow_up.isClicked(sf::Keyboard::Up))
		{
			selected_id--;
			if (selected_id <= 1) //set so the title cant be selected
				selected_id = 1;

			deactiveAll();
			if (selected_id != 0) //exclude title
				names[selected_id].setActive(true);

		}
	}
	void updateSelection()
	{
		if (names.size() > 1 && isSelected) //not only the title
		{
			arrowsHandle();
		}
		
	}
public:
	bool isEmpty = true;
	void deactiveAll()
	{
		for (int i = 0; i < names.size(); i++)
		{
			names[i].setActive(false);
		}
	}
	Column()
	{

	}
	void setSelectionState(bool state)
	{
		isSelected = state;
		selected_id = 1;
		names[selected_id].setActive(true);
	}
	bool getSelectionState()
	{
		return isSelected;
	}
	std::wstring getSelectedName()
	{
		return names[selected_id].txt;
	}
	void draw()
	{
		updateSelection();

		for (int i = 0; i < names.size(); i++)
			names[i].draw();
	}
	
	void createColumn(HDC hdc, std::wstring title, std::vector<std::wstring> names, int x = 0, int y = 0)
	{
		addLabel(hdc, x, y, title, RGB(255, 0, 0));

		for (int i = 0; i < names.size(); i++)
		{
			isEmpty = false;
			addLabel(hdc, x, y + (16 * (i + 1)), names[i], RGB(0, 0, 0));
		}
	}
};
class Overlay
{
private:
	std::vector<Column> columns;
	std::vector<Label> labels;
	HDC hdc;

	int selected_id;
	bool is_visible;
	SingleClick arrow_left;
	SingleClick arrow_right;

	SingleClick enter;
	SingleClick enable_key;
	sf::Keyboard::Key enable_key_sf;

	void deactiveAll()
	{
		
		for (int i = 0; i < columns.size(); i++)
		{
			columns[i].setSelectionState(false);
			columns[i].deactiveAll();
		}
		columns[selected_id].setSelectionState(true);
	}
	void updateArrows()
	{
		if (arrow_right.isClicked(sf::Keyboard::Right))
		{
			selected_id++;

			if (selected_id > columns.size() - 1)
				selected_id = columns.size() - 1;
			deactiveAll();
			

		}
		if (arrow_left.isClicked(sf::Keyboard::Left))
		{
			selected_id--;
			if (selected_id <= 0) //set so the title cant be selected
				selected_id = 0;

			deactiveAll();
			
			

		}
	}
	void updateEnabler()
	{
		if (enable_key.isClicked(enable_key_sf))
			is_visible = !is_visible;
	}
	void updateSelection()
	{
		
		updateArrows();
	}
public:
	void drawAll()
	{
		updateEnabler();
		if (is_visible)
		{
			updateSelection();
			for (int i = 0; i < columns.size(); i++)
				columns[i].draw();
		}
		//enabler is visible
		for (int i = 0; i < labels.size(); i++)
			labels[i].draw();
		
	}
	
	void addColumn(std::wstring title, std::vector<std::wstring> names, int x = 0, int y = 0)
	{
		Column col;
		col.createColumn(hdc, title, names, x, y);
		if (!columns.size()) //so the first one is selected auto
		{
			col.setSelectionState(true);		
		}
			
		columns.push_back(col);
	}
	void addEnableKey(std::wstring title, sf::Keyboard::Key enabler, int x = 0, int y = 0)
	{
		Label label;
		label.create(x, y, title, hdc, RGB(255, 0, 0));
		label.draw(2); //must have bc idk y
		labels.push_back(label);

		enable_key_sf = enabler;
	}
	bool getChosenEvent(std::wstring name)
	{
		if (columns[selected_id].getSelectedName() == name)
			if (enter.isClicked(sf::Keyboard::Key::Enter))
				return true;
		return false;
	}
	Overlay(HWND hwnd)
	{
		hdc = GetDC(hwnd);
	}
	Overlay()
	{
		hdc = GetDC(GetDesktopWindow());
	}
};