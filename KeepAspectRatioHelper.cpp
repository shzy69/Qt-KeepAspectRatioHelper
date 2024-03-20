#pragma once


#include <math.h>
#include <vector>
#include <queue>

#include <QWidget>
#include <QTimer>
#include <QResizeEvent>
#include <QBoxLayout>


/*
* Written by shzy.
* Happy for using.
*/


// The helped widget must be in some layout.
// all functions should be called at the main UI thread.
class KeepAspectRatioHelper : public QObject
{
	Q_OBJECT

public:
	explicit KeepAspectRatioHelper(QObject *parent = nullptr)
		: QObject(parent)
	{
		wrapper_list_.reserve(16);
	}
	~KeepAspectRatioHelper() = default;

public:
	bool AddHelped(QWidget *widget, float ratio = KeepAspectRatioHelper::ORIGIN_RATIO)
	{
		//ratio
		if (ratio == ORIGIN_RATIO)
		{
			ratio = (float)widget->width() / widget->height();
		}

		QWidget *parent = widget->parentWidget();

		QWidget *wrapper = new QWidget();

		//replace widget
		auto old_item = parent->layout()->replaceWidget(widget, wrapper);
		if (!old_item)
		{
			delete wrapper;
			return false;
		}
		delete old_item;

		//add to wrapper list
		if (empty_index_.empty())
		{
			wrapper_list_.push_back(WrapperRatio{wrapper, ratio});
		}
		else
		{
			int index = empty_index_.front();
			empty_index_.pop();
			wrapper_list_.at(index).wrapper = wrapper;
			wrapper_list_.at(index).ratio = ratio;
		}

		//install a QHBoxLayout inside the wrapper
		QHBoxLayout *hlayout = new QHBoxLayout(wrapper); //equal to wrapper.setLayout(hlayout);
		hlayout->setContentsMargins(0, 0, 0, 0);
		//add item
		hlayout->addItem(new QSpacerItem(0, 0));
		hlayout->addWidget(widget);
		hlayout->addItem(new QSpacerItem(0, 0));

		wrapper->installEventFilter(this);
		printf("install success\n");
		wrapper->resize(wrapper->size());

		return true;
	}

	// if you want to make any change of the layout or parent of the helped widget,
	// call this function first.
	void RemoveHelped(QWidget *widget)
	{
		QWidget *wrapper = widget->parentWidget();

		for (int i = 0; i < wrapper_list_.size(); i++)
		{
			auto &item = wrapper_list_[i];
			if (wrapper == item.wrapper)
			{
				item.wrapper = nullptr;
				item.ratio = 0;
				empty_index_.push(i);
				wrapper->removeEventFilter(this);

				//recover as origin
				widget->hide();
				widget->setParent(nullptr);
				auto old_item = wrapper->parentWidget()->layout()->replaceWidget(wrapper, widget);
				widget->show();

				delete wrapper;
				delete old_item;
			}
		}
	}

public:
	enum RATIO_VALUE { ORIGIN_RATIO = 0 };

protected:
	bool eventFilter(QObject *watched, QEvent *e)
	{
		if (e->type() != QEvent::Resize)
		{
			return QObject::eventFilter(watched, e);
		}

		QResizeEvent *resize_event = static_cast<QResizeEvent*>(e);
		QWidget *wrapper = static_cast<QWidget*>(watched);
		QBoxLayout *layout = static_cast<QBoxLayout*>(wrapper->layout());

		//ratio
		float aspect_ratio = 0;
		for (auto const &item : wrapper_list_)
		{
			if (item.wrapper == wrapper)
			{
				aspect_ratio = item.ratio;
				break;
			}
		}
		if (aspect_ratio == 0)
			return false;

		int widget_stretch = 0;
		int out_stretch = 0;

		float cur_ratio = (float)resize_event->size().width() / resize_event->size().height();

		if (cur_ratio > aspect_ratio)
		{
			layout->setDirection(QBoxLayout::LeftToRight);
			widget_stretch = round(wrapper->height() * aspect_ratio);
			out_stretch = round((wrapper->width() - widget_stretch) / 2);
		}
		else
		{
			layout->setDirection(QBoxLayout::TopToBottom);
			widget_stretch = round(wrapper->width() / aspect_ratio);
			out_stretch = round((wrapper->height() - widget_stretch) / 2);
		}

		layout->setStretch(0, out_stretch);
		layout->setStretch(1, widget_stretch);
		layout->setStretch(2, out_stretch);

		return false;
	}


private:
	struct WrapperRatio
	{
		QWidget *wrapper;
		float ratio;
	};

	std::vector<WrapperRatio> wrapper_list_;

	//any index of the wrapper_list_ been removed
	std::queue<int> empty_index_;

};

