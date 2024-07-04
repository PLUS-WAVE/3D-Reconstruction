#include "Viewer.h"
#include <qstring>
#include "Windows.h"
#include "qwindow.h"
Viewer::Viewer(QWidget* parent)
	: QWidget(parent)
{
}

Viewer::Viewer(int i, QWidget* parent)
	: QWidget(parent)
{
	i++;
	WId wid = (WId)FindWindowA("GLFW30", "MVSViewer");
	QWindow* m_window;
	m_window = QWindow::fromWinId(wid);
	m_window->setFlags(m_window->flags() | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
	QWidget* m_widget;
	m_widget = QWidget::createWindowContainer(m_window, this);
	m_widget->setMinimumSize(841, 421);
}
Viewer::~Viewer() {}

void Viewer::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
}