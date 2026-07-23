#pragma once

#include <qobject.h>

// 这是一个轻量抽象类，配合function.h中的模板使用，构造继承于它的子类时，请使用
// newWindow<ChildWidget>(args) 来构造，并重载虚函数，连接你想连接到主窗口的槽函数，
// 只是为了避免不长期加载的窗口长期占用内存，每次新建又要手动connect
class SecondaryWindow
{
public:
	SecondaryWindow() {};

	virtual void connectToMainWindow() = 0;

	virtual ~SecondaryWindow() = default;
};

