/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             houchengqiu<houchengqiu@uniontech.com>
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
#include "groupwidget_p.h"
#include "groupwidget.h"
#include "listview/grandsearchlistview.h"
#include "utils/utils.h"

#include <DLabel>
#include <DPushButton>
#include <DHorizontalLine>
#include <DApplicationHelper>

#include <QHBoxLayout>
#include <QVBoxLayout>

using namespace GrandSearch;

#define GROUP_MAX_SHOW 5
#define ListItemHeight            36       // 列表行高
#define GroupLabelHeight          28       // 组标签高度
#define MoreBtnMaxHeight          25       // 查看更多按钮最大高度
#define LayoutMagrinSize          10       // 布局边距
#define SpacerWidth               40       // 弹簧宽度
#define SpacerHeight              20       // 弹簧高度

GroupWidgetPrivate::GroupWidgetPrivate(GroupWidget *parent)
    : q_p(parent)
{

}

GroupWidget::GroupWidget(QWidget *parent)
    : DWidget(parent)
    , d_p(new GroupWidgetPrivate(this))
{
    m_firstFiveItems.clear();
    m_restShowItems.clear();
    m_cacheItems.clear();

    m_cacheItemsRecentFile.clear();

    initUi();
    initConnect();
}

GroupWidget::~GroupWidget()
{
    qDebug() << QString("groupWidget %1 destructed....").arg(m_groupName);
}

void GroupWidget::setGroupName(const QString &groupClassName)
{
    Q_ASSERT(m_groupLabel);

    const QString &groupName = GroupWidget::getGroupName(groupClassName);
    const QString &groupObjName = GroupWidget::getGroupObjName(groupClassName);

    m_groupClassName = groupClassName;
    m_groupName = groupName;

    m_groupLabel->setObjectName(groupObjName);
    m_groupLabel->setText(groupName);
}

void GroupWidget::appendMatchedItems(const MatchedItems &newItems, const QString& groupClassName)
{
    if (Q_UNLIKELY(newItems.isEmpty()))
        return;

    if (groupClassName == GRANDSEARCH_GROUP_RECENTFILE) {
        // 结果列表未展开
        if (!m_bListExpanded) {

            // 新来数据先放入最近文件缓存中
            m_cacheItemsRecentFile << newItems;

            // 显示不足5个，连带新增数据一起重排
            if (m_firstFiveItems.size() < GROUP_MAX_SHOW || m_listView->lastShowRow(GRANDSEARCH_GROUP_RECENTFILE) == -1) {

                // 清空列表数据，将已显示数据还原到各自缓存中
                m_firstFiveItems.clear();
                m_cacheItemsRecentFile << m_listView->groupItems(GRANDSEARCH_GROUP_RECENTFILE);
                m_cacheItems << m_listView->groupItems(m_groupClassName);

                // 拉通重排缓存中最近文件匹配结果
                Utils::sort(m_cacheItemsRecentFile);
                for (int i = 0; i < GROUP_MAX_SHOW; i++) {
                    if (!m_cacheItemsRecentFile.isEmpty())
                        m_firstFiveItems.push_back(m_cacheItemsRecentFile.takeFirst());
                }
                m_listView->setMatchedItems(m_firstFiveItems, GRANDSEARCH_GROUP_RECENTFILE);

                // 最近文件不足5个，从一般缓存中取剩余数据补齐5个
                if (m_firstFiveItems.size() < GROUP_MAX_SHOW) {
                    Utils::sort(m_cacheItems);

                    for (int i = m_firstFiveItems.size(); i < GROUP_MAX_SHOW; i++) {
                        if (!m_cacheItems.isEmpty()) {
                            MatchedItem item = m_cacheItems.takeFirst();
                            m_firstFiveItems.push_back(item);

                            m_listView->addRow(item, m_groupClassName);
                        }
                    }
                }
            }

            // 缓存中有数据，显示'查看更多'按钮
            m_viewMoreButton->setVisible(!m_cacheItemsRecentFile.isEmpty() || !m_cacheItems.isEmpty());
        }
        else {
            // 结果列表已展开
            // 对新数据排序，并插入到已显示最近文件结果末尾
            MatchedItems& tempNewItems = const_cast<MatchedItems&>(newItems);
            Utils::sort(tempNewItems);
            m_listView->insertRows(m_listView->lastShowRow(GRANDSEARCH_GROUP_RECENTFILE), tempNewItems, GRANDSEARCH_GROUP_RECENTFILE);
        }
    }
    else {
        // 结果列表未展开
        if (!m_bListExpanded) {

            // 新来的数据先放入缓存中
            m_cacheItems << newItems;

            // 显示结果不足5个，连带新增数据一起重新排序
            if (m_firstFiveItems.size() < GROUP_MAX_SHOW) {
                //当前已有最近文件显示，则在最近文件最后一行之后补齐显示新来的数据
                MatchedItems showedRecentFileItems = m_listView->groupItems(GRANDSEARCH_GROUP_RECENTFILE);
                if (!showedRecentFileItems.isEmpty()) {
                    m_cacheItems << m_listView->groupItems(m_groupClassName);
                    Utils::sort(m_cacheItems);

                    m_firstFiveItems.clear();

                    // 先置顶显示最近文件匹配结果
                    m_firstFiveItems << showedRecentFileItems;
                    m_listView->setMatchedItems(m_firstFiveItems, GRANDSEARCH_GROUP_RECENTFILE);

                    // 最近文件不足5个，从一般缓存中取剩余数据补齐5个
                    if (m_firstFiveItems.size() < GROUP_MAX_SHOW) {
                        for (int i = m_firstFiveItems.size(); i < GROUP_MAX_SHOW; i++) {
                            if (!m_cacheItems.isEmpty()) {
                                MatchedItem item = m_cacheItems.takeFirst();
                                m_firstFiveItems.push_back(item);

                                m_listView->addRow(item, m_groupClassName);
                            }
                        }
                    }
                }
                else {
                    m_cacheItems << m_firstFiveItems;
                    Utils::sort(m_cacheItems);

                    m_firstFiveItems.clear();
                    for (int i = 0; i < GROUP_MAX_SHOW; i++) {
                        if (!m_cacheItems.isEmpty())
                            m_firstFiveItems.push_back(m_cacheItems.takeFirst());
                    }

                    m_listView->setMatchedItems(m_firstFiveItems, groupClassName);
                }
            }

            // 缓存中有数据，显示'查看更多'按钮
            m_viewMoreButton->setVisible(!m_cacheItems.isEmpty());
        }
        else {
            // 结果列表已展开，已经显示的数据保持不变，仅对新增数据排序，然后追加到列表末尾
            MatchedItems& tempNewItems = const_cast<MatchedItems&>(newItems);
            Utils::sort(tempNewItems);
            m_listView->addRows(tempNewItems, groupClassName);
        }
    }
    layout();
}

void GroupWidget::showHorLine(bool bShow)
{
    Q_ASSERT(m_line);

    m_line->setVisible(bShow);

    layout();
}

bool GroupWidget::isHorLineVisilbe()
{
    Q_ASSERT(m_line);

    m_line->isVisible();

    return false;
}

GrandSearchListview *GroupWidget::getListView()
{
    return m_listView;
}

int GroupWidget::itemCount()
{
    Q_ASSERT(m_listView);

    return m_listView->rowCount();
}

int GroupWidget::getCurSelectHeight()
{
    Q_ASSERT(m_listView);

    int nHeight = 0;
    if (m_listView->currentIndex().isValid()) {
        nHeight += m_groupLabel->height();
        nHeight += (m_listView->currentIndex().row() + 1) * ListItemHeight;
    }

    return nHeight;
}

void GroupWidget::reLayout()
{
    Q_ASSERT(m_listView);
    Q_ASSERT(m_groupLabel);
    Q_ASSERT(m_line);

    m_listView->setFixedHeight(m_listView->rowCount() * ListItemHeight);

    int nHeight = 0;
    nHeight += m_groupLabel->height();
    nHeight += m_listView->height();

    if (!m_line->isHidden()) {
        nHeight += m_line->height();
        nHeight += LayoutMagrinSize;
        m_vContentLayout->setSpacing(10);
    } else {
        m_vContentLayout->setSpacing(0);
    }

    this->setFixedHeight(nHeight);

    layout();
}

void GroupWidget::clear()
{
    Q_ASSERT(m_listView);

    m_firstFiveItems.clear();
    m_restShowItems.clear();
    m_cacheItems.clear();

    m_cacheItemsRecentFile.clear();

    m_bListExpanded = false;

    m_listView->clear();
    setVisible(false);
}

QString GroupWidget::groupName()
{
    Q_ASSERT(m_groupLabel);

    return m_groupLabel->text();
}

QString GroupWidget::getGroupName(const QString &groupClassName)
{
    QString strName = groupClassName;

    if (GRANDSEARCH_GROUP_APP == groupClassName)
        strName = GroupName_App;
    else if (GRANDSEARCH_GROUP_FOLDER == groupClassName)
        strName = GroupName_Folder;
    else if (GRANDSEARCH_GROUP_FILE == groupClassName)
        strName = GroupName_File;

    return strName;
}

QString GroupWidget::getGroupObjName(const QString &groupClassName)
{
    QString strObjName = groupClassName;

    if (GRANDSEARCH_GROUP_APP == groupClassName)
        strObjName = GroupObjName_App;
    else if (GRANDSEARCH_GROUP_FOLDER == groupClassName)
        strObjName = GroupObjName_Folder;
    else if (GRANDSEARCH_GROUP_FILE == groupClassName)
        strObjName = GroupObjName_File;

    return strObjName;
}

void GroupWidget::initUi()
{
    // 获取设置当前窗口文本颜色
    QColor groupTextColor = QColor(0, 0, 0, static_cast<int>(255*0.6));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        groupTextColor = QColor(255, 255, 255, static_cast<int>(255*0.6));
    QPalette labelPalette;
    labelPalette.setColor(QPalette::WindowText, groupTextColor);

    // 组列表内控件沿垂直方向布局
    m_vLayout = new QVBoxLayout(this);
    m_vLayout->setContentsMargins(0, 0, LayoutMagrinSize, 0);
    m_vLayout->setSpacing(0);
    this->setLayout(m_vLayout);

    // 组名标签
    m_groupLabel = new DLabel(tr(""), this);
    m_groupLabel->setFixedHeight(GroupLabelHeight);
    m_groupLabel->setPalette(labelPalette);
    m_groupLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_groupLabel->setContentsMargins(0, 0, 0, 0);

    // 查看更多按钮
    m_viewMoreButton = new DPushButton(tr("More"), this);
    m_viewMoreButton->setMaximumHeight(MoreBtnMaxHeight);
    m_viewMoreButton->setFlat(true);

    // 设置查看按钮文本颜色
    QColor moreTextColor = QColor(0, 0, 0, static_cast<int>(255*0.4));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        moreTextColor = QColor(255, 255, 255, static_cast<int>(255*0.4));
    QPalette palette = m_viewMoreButton->palette();
    palette.setColor(QPalette::ButtonText, moreTextColor);
    // 使'查看更多'按钮按下时，背景色变淡
    QBrush brush(QColor(0,0,0,0));
    palette.setBrush(QPalette::Active, QPalette::Button, brush);
    palette.setBrush(QPalette::Active, QPalette::Light, brush);
    palette.setBrush(QPalette::Active, QPalette::Midlight, brush);
    palette.setBrush(QPalette::Active, QPalette::Dark, brush);
    palette.setBrush(QPalette::Active, QPalette::Mid, brush);
    m_viewMoreButton->setPalette(palette);
    DFontSizeManager::instance()->bind(m_viewMoreButton, DFontSizeManager::T8, QFont::Normal);
    QFont fontMoreBtn = m_viewMoreButton->font();
    fontMoreBtn.setWeight(QFont::Normal);
    fontMoreBtn = DFontSizeManager::instance()->get(DFontSizeManager::T8, fontMoreBtn);
    m_viewMoreButton->setFont(fontMoreBtn);
    m_viewMoreButton->hide();

    // 组列表标题栏布局
    m_hTitelLayout = new QHBoxLayout();
    m_hTitelLayout->setContentsMargins(LayoutMagrinSize,0,0,0);
    m_hTitelLayout->setSpacing(0);
    m_hTitelLayout->addWidget(m_groupLabel);
    m_hTitelLayout->addSpacerItem(new QSpacerItem(SpacerWidth,SpacerHeight,QSizePolicy::Expanding, QSizePolicy::Minimum));
    m_hTitelLayout->addWidget(m_viewMoreButton);

    // 组内结果列表
    m_listView = new GrandSearchListview(this);

    // 分割线
    m_line = new DHorizontalLine;
    m_line->setFrameShadow(DHorizontalLine::Raised);
    m_line->setLineWidth(1);

    // 列表和分割线放到内容布局内
    m_vContentLayout = new QVBoxLayout();
    m_vContentLayout->setMargin(0);
    m_vContentLayout->setSpacing(0);
    m_vContentLayout->addWidget(m_listView);
    m_vContentLayout->addWidget(m_line);

    // 标题栏布局和内容布局放到主布局内
    m_vLayout->addLayout(m_hTitelLayout);
    m_vLayout->addLayout(m_vContentLayout);
}

void GroupWidget::initConnect()
{
    Q_ASSERT(m_viewMoreButton);

    connect(m_viewMoreButton, &DPushButton::clicked, this, &GroupWidget::onMoreBtnClcked);
}

void GroupWidget::paintEvent(QPaintEvent *event)
{
// 调试使用，最后发布时需删除todo
#ifdef SHOW_BACKCOLOR
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setBrush(Qt::green);
    painter.drawRect(rect());
#else
    DWidget::paintEvent(event);
#endif
}

void GroupWidget::onMoreBtnClcked()
{
    Q_ASSERT(m_listView);
    Q_ASSERT(m_viewMoreButton);

    // 在已显示最近文件的最后一行，显示缓存中的最近文件匹配结果
    if (!m_cacheItemsRecentFile.isEmpty()) {
        Utils::sort(m_cacheItemsRecentFile);
        m_listView->insertRows(m_listView->lastShowRow(GRANDSEARCH_GROUP_RECENTFILE) + 1, m_cacheItemsRecentFile, GRANDSEARCH_GROUP_RECENTFILE);
        m_cacheItemsRecentFile.clear();
    }

    // 将缓存中的数据转移到剩余显示结果中
    m_restShowItems << m_cacheItems;
    Utils::sort(m_restShowItems);

    // 剩余显示结果追加显示到列表中
    m_listView->addRows(m_restShowItems, m_groupClassName);

    // 清空缓存中数据
    m_cacheItems.clear();

    reLayout();

    emit showMore();
    m_viewMoreButton->hide();

    m_bListExpanded = true;
}


