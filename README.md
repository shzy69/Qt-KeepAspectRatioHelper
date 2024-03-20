# KeepAspectRatioHelper
In Qt, when a widget is inside layout,  
it's not easy to keep the widget's aspect ratio during window's resizing.  
But with this helper class, it's becoming quite easy.

# Demo
```
/* Just add the widget to the helper class. That is all */
auto keep_ratio_helper = new KeepAspectRatioHelper(this);
keep_ratio_helper->AddHelped(some_widget_inside_layout);
keep_ratio_helper->AddHelped(another_widget_inside_layout);
//...


```