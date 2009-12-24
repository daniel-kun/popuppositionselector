//*****************************************************************************
// copyright  :    (c) 2009 Daniel Albuschat
//*****************************************************************************

#include "PopupPositionSelector.h"

#include <QDesktopWidget>
#include <QResizeEvent>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

namespace
{
   const double cornerFactor = 0.3;

   void BuildCornerRects(const QRect &aSource, QRect aDest[])
   {
      int size;
      if (aSource.width() > aSource.height())
         size = aSource.height() * cornerFactor;
      else
         size = aSource.width() * cornerFactor;
      aDest[0] = aSource;
      aDest[0].setWidth(size);
      aDest[0].setHeight(size);
      aDest[3] = aDest[2] = aDest[1] = aDest[0];
      aDest[1].moveTopRight(aSource.topRight());
      aDest[2].moveBottomLeft(aSource.bottomLeft());
      aDest[3].moveBottomRight(aSource.bottomRight());
   }
}

//*****************************************************************************
// PopupPositionSelector
//*****************************************************************************
PopupPositionSelector::PopupPositionSelector():
   m_previewWidget(
      0, 
      Qt::Tool | 
      Qt::WindowStaysOnTopHint |
      Qt::FramelessWindowHint),
   m_previewEnabled(true)
{
   m_previewWidget.setFrameShape(QFrame::Box);
   m_position.screen = 0;
   m_position.corner = 0;

   m_mousePosition.corner = -1;
   m_mousePosition.screen = -1;

   QDesktopWidget *desktop = QApplication::desktop();

   int smallestX = 0, largestX = 0,
      smallestY = 0, largestY = 0;

   for (int i = 0; i < desktop->numScreens(); ++i)
   {
      QRect r = desktop->screenGeometry(i);
      if (r.x() < smallestX)
         smallestX = r.x();
      if (r.right() > largestX)
         largestX = r.right();
      if (r.y() < smallestY)
         smallestY = r.y();
      if (r.bottom() > largestY)
         largestY = r.bottom();
      m_originalRects.append(r);
   }
   m_totalRect.setRect(
      smallestX,
      smallestY,
      largestX - smallestX,
      largestY - smallestY);

   updateRects(sizeHint());

   setMouseTracking(true);
}

//-----------------------------------------------------------------------------
void PopupPositionSelector::setPosition(const PopupPosition &aPosition)
{
   m_position = aPosition;
   // We simply need to redraw, since m_position is used in paintEvent().
   update();

   emit positionChanged(m_position);
}

//-----------------------------------------------------------------------------
PopupPosition PopupPositionSelector::position() const
{
   return m_position;
}

//-----------------------------------------------------------------------------
void PopupPositionSelector::enablePreview(bool aEnabled)
{
   m_previewEnabled = aEnabled;
   updateMouse(m_mousePosition.screen, m_mousePosition.corner, true);
}

//-----------------------------------------------------------------------------
bool PopupPositionSelector::isPreviewEnabled() const
{
   return m_previewEnabled;
}

//-----------------------------------------------------------------------------
QSize PopupPositionSelector::sizeHint() const
{
   QSize scaled(m_totalRect.width(), m_totalRect.height());
   scaled.scale(300, 300, Qt::KeepAspectRatio);
   return scaled;
}

//-----------------------------------------------------------------------------
void PopupPositionSelector::resizeEvent(QResizeEvent *aEvent)
{
   updateRects(aEvent->size());
   QWidget::resizeEvent(aEvent);
}

//-----------------------------------------------------------------------------
void PopupPositionSelector::showEvent(QShowEvent *aEvent)
{
   m_screenText = tr("Screen %1");
   QWidget::showEvent(aEvent);
}

//-----------------------------------------------------------------------------
void PopupPositionSelector::mouseMoveEvent(QMouseEvent *aEvent)
{
   int newMouseScreen = -1,
      newMouseCorner = -1;
   for (int i = 0; i < m_screenRects.size(); ++i)
   {
      const QRect &rect(m_screenRects[i]);
      if (rect.contains(aEvent->pos()))
      {
         newMouseScreen = i;
         QRect r[4];
         BuildCornerRects(rect, r);
         for (int j = 0; j < 4; ++j)
         {
            if (r[j].contains(aEvent->pos()))
            {
               newMouseCorner = j;
               break;
            }
         }
         break;
      }
   }
   updateMouse(newMouseScreen, newMouseCorner);
   QWidget::mouseMoveEvent(aEvent);
}

//-----------------------------------------------------------------------------
void PopupPositionSelector::leaveEvent(QEvent *aEvent)
{
   updateMouse(-1, -1);
   QWidget::leaveEvent(aEvent);
}

//-----------------------------------------------------------------------------
void PopupPositionSelector::mouseReleaseEvent(QMouseEvent *aEvent)
{
   if (m_mousePosition.screen >= 0 && m_mousePosition.corner >= 0)
   {
      setPosition(m_mousePosition);
      update();
   }
   QWidget::mouseReleaseEvent(aEvent);
}

//-----------------------------------------------------------------------------
void PopupPositionSelector::paintEvent(QPaintEvent *aEvent)
{
   QWidget::paintEvent(aEvent);

   QPainter painter(this);
   painter.setBrush(Qt::white);
   painter.setPen(Qt::black);
   // Since I expect the two drawRects() calls to be faster than individual
   // calls to drawRect(), we first draw all rects and later overwrite the 
   // highlighted rect (if any).
   painter.drawRects(m_screenRects);
   painter.setBrush(QColor(188, 188, 188));
   painter.drawRects(m_cornerRects);

   for (int i = 0; i < m_screenRects.size(); ++i)
   {
      QFontMetrics metrics(font());
      QString screenText(m_screenText.arg(i + 1));
      QRect textRect(metrics.boundingRect(screenText));
      const QRect &r(m_screenRects[i]);
      painter.drawText(
         r.x() + (r.width() - textRect.width()) / 2,
         r.y() + (r.height() + textRect.height()) / 2,
         screenText);
   }

   if (m_mousePosition.screen != m_position.screen ||
      m_mousePosition.corner != m_position.corner)
   {
      if (m_mousePosition.screen >= 0)
      {
         if (m_mousePosition.corner >= 0)
         {
            const QRect &r(m_cornerRects[m_mousePosition.screen * 4 + m_mousePosition.corner]);
            QLinearGradient gradient(r.x(), r.y(), r.right(), r.bottom());
            gradient.setColorAt(0, QColor(204, 204, 204));
            gradient.setColorAt(1, QColor(219, 219, 219));
            painter.setBrush(gradient);
            painter.drawRect(r);
         }
      }
   }

   if (m_position.screen >= 0)
   {
//      painter.setBrush(Qt::red);
//      painter.drawRect(m_screenRects[m_position.screen]);
      if (m_position.corner >= 0)
      {
         const QRect &r(m_cornerRects[m_position.screen * 4 + m_position.corner]);
         QLinearGradient gradient(r.x(), r.y(), r.right(), r.bottom());
         gradient.setColorAt(0, QColor(255, 255, 255));
         gradient.setColorAt(1, QColor(252, 224, 185));
         painter.setBrush(gradient);
         painter.drawRect(r);
      }
   }
      
}

//-----------------------------------------------------------------------------
void PopupPositionSelector::updateRects(QSize aSize)
{
   updateRects(aSize.width(), aSize.height());
}

//-----------------------------------------------------------------------------
void PopupPositionSelector::updateRects(int aWidth, int aHeight)
{
   m_screenRects.resize(m_originalRects.size());
   m_cornerRects.resize(m_originalRects.size() * 4);

   QSize ratio(m_totalRect.width(), m_totalRect.height());
   ratio.scale(aWidth, aHeight, Qt::KeepAspectRatio);

   double 
      ratioX(static_cast<double>(ratio.width()) / m_totalRect.width()), 
      ratioY(static_cast<double>(ratio.height()) / m_totalRect.height());

   // 4 pixels margin to each side
   int margin = 4;

   for (int i = 0; i < m_originalRects.size(); ++i)
   {
      QRect rect(
         m_originalRects[i].x() * ratioX - m_totalRect.left() * ratioX + margin,
         m_originalRects[i].y() * ratioY - m_totalRect.top() * ratioY + margin,
         m_originalRects[i].width() * ratioX - margin * 2,
         m_originalRects[i].height() * ratioY - margin * 2);
      m_screenRects[i] = rect;

      QRect r[4];
      BuildCornerRects(rect, r);
      for (int x = 0; x < 4; ++x)
         m_cornerRects[i * 4 + x] = r[x];
   }
}

//-----------------------------------------------------------------------------
void PopupPositionSelector::updateMouse(
   int aNewScreen, 
   int aNewCorner,
   bool aForce)
{
   if (aNewScreen != m_mousePosition.screen || 
       aNewCorner != m_mousePosition.corner ||
       aForce)
   {
      m_mousePosition.screen = aNewScreen;
      m_mousePosition.corner = aNewCorner;

      // Lazy: We could find out which parts of the widgets need to be
      // updated, but we simply update the whole widget.
      update();

      if (m_mousePosition.corner >= 0)
      {
         setCursor(Qt::PointingHandCursor);
         if (m_previewEnabled)
         {
            QDesktopWidget *desktop = QApplication::desktop();
            QRect rect(desktop->availableGeometry(m_mousePosition.screen));
            QRect r[4];
            BuildCornerRects(rect, r);
            m_previewWidget.setGeometry(r[m_mousePosition.corner]);
         }
         m_previewWidget.setVisible(m_previewEnabled);
      }
      else
      {
         m_previewWidget.hide();
         setCursor(Qt::ArrowCursor);
      }

      emit cornerHovered(m_mousePosition);
   }
}
