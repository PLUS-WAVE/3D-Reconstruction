#pragma once
#include <QWidget>
#include <QEvent>

class Viewer :public QWidget
{
	Q_OBJECT
public:
	Viewer(QWidget* parent = nullptr);
	Viewer(int i, QWidget* parent = nullptr);
	void resizeEvent(QResizeEvent* event);
	~Viewer();
private:
};
