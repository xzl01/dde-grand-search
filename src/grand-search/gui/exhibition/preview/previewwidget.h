/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
 *
 * Maintainer: houchengqiu<houchengqiu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include "previewpluginmanager.h"
#include <DWidget>

#include <QScopedPointer>

class GeneralToolBar;;
class QVBoxLayout;
class QSpacerItem;
class DetailInfoWidget;
class PluginProxy;
class PreviewWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
    friend class PluginProxy;
public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    ~PreviewWidget() override;

    // 预览指定搜索项
    bool previewItem(const GrandSearch::MatchedItem &item);

private:
    void initUi();
    void initConnect();
    void reLayout();
    void clearLayoutWidgets();

    void onOpenClicked();
    void onOpenpathClicked();
    void onCopypathClicked();
private:
    QVBoxLayout *m_vMainLayout = nullptr;               // 预览界面主体布局

    QSpacerItem* m_vSpaceItem = nullptr;       // 垂直弹簧，用于将工具栏置底显示

    QSharedPointer<GrandSearch::PreviewPlugin> m_preview = nullptr;        // 当前正在预览的插件界面

    QSharedPointer<GrandSearch::PreviewPlugin> m_generalPreview = nullptr; // 默认预览插件界面

    DetailInfoWidget* m_detailInfoWidget = nullptr; // 通用属性详情部件
    GeneralToolBar *m_generalToolBar = nullptr;     // 通用工具栏部件
    PluginProxy *m_proxy = nullptr; //用于预览插件回调预览框架的接口


private:
    GrandSearch::MatchedItem m_item; //当前正在预览的匹配结果
    PreviewPluginManager m_pluginManager; //预览插件管理对象
};

#endif // PREVIEWWIDGET_H
