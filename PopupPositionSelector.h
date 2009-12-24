//*****************************************************************************
// copyright  :    (c) 2009 Daniel Albuschat
//
// description:
// PopupPositionSelector is a widget that can be used to let the user 
// visually select the position that a TsInfoPopup should use.
// All monitors are displayed in the viewable area retaining their aspects.
// The user can then click on the corner of the desired screen where he wants
// to display the popup. When the mouse moves over a designated corner-area
// of a screen, a tool-window is displayed at that position of the real
// screen.
//*****************************************************************************

#ifndef TS_POPUPPOSITIONSELECTOR_H_23122009
#define TS_POPUPPOSITIONSELECTOR_H_23122009

#include <QWidget>
#include <QFrame>

//*****************************************************************************
// PopupPosition specifies the screen and the corner-position where a popup
// should be displayed.
//*****************************************************************************

struct PopupPosition
{
   int screen;
   int corner;
};

//*****************************************************************************
// PopupPositionSelector
//*****************************************************************************

class PopupPositionSelector: public QWidget
{
   Q_OBJECT

signals:
   // cornerHovered() is emitted when the user moves the mouse over a corner
   // of a miniature-screen. aPosition can have an invalid screen and corner
   // when the mouse is leaves a corner or a screen. In that case, screen or
   // corner are set to -1.
   void cornerHovered(PopupPosition aPosition);

   // positionChanged() is similar to cornerHovered(), but it is emitted when
   // the user clicks a position.
   void positionChanged(PopupPosition aPosition);

public:
   PopupPositionSelector();

   // Returns the position that has been set with setPosition() or has been
   // selected by the user.
   PopupPosition position() const;

   // Returns whether the preview is enabled.
   // See enablePreview().
   bool isPreviewEnabled() const;

   // The sizeHint is calculated from the available screens. The width and 
   // height are constrained to be 300 pixels for the sizeHint, although the
   // widget can be displayed in any size.
   virtual QSize sizeHint() const;

public slots:
   // Sets the current position to be aPosition. The current position is 
   // highlighted and can be changed by the user.
   void setPosition(const PopupPosition &aPosition);

   // When the preview is enabled, a window is shown on the real screen when
   // the user hovers a corner of the miniature-screens.
   // The default is enabled.
   void enablePreview(bool aEnabled = true);

protected:
   // When the widget is resized, the precalculated rects of the 
   // miniature-screens needs to be updated.
   virtual void resizeEvent(QResizeEvent *aEvent);

   // Localization happens in the showEvent()
   virtual void showEvent(QShowEvent *aEvent);

   // When the mouse moves over a miniature-screen's corner, the tool-window is 
   // displayed at the corner of the real monitor.
   virtual void mouseMoveEvent(QMouseEvent *aEvent);
   virtual void leaveEvent(QEvent *aEvent);

   // When the user releases the mouse over a miniature-screen's corner, the
   // position is set to that corner.
   virtual void mouseReleaseEvent(QMouseEvent *aEvent);

   // Paint the miniature-screens.
   virtual void paintEvent(QPaintEvent *aEvent);

private:
   // This widget is used to display a tool-window when the user hovers a
   // corner of the miniature-window.
   QFrame m_previewWidget;
   bool m_previewEnabled;

   // m_mouseScreen and m_mouseCorner are set when the mouse hovers over a 
   // miniature-screen and probably it's corner.
   // When the mouse is currently not over a screen or a corner, it is 
   // set to -1.
   PopupPosition m_mousePosition;

   PopupPosition m_position;

   // The total rect is the rectangle that surrounds all monitors.
   QRect m_totalRect;

   // The original rects store the positions and sizes of the real monitors.
   QVector<QRect> m_originalRects;

   // The key is the number of the screen, while the value specifies the
   // rectangle in the viewable area. The rect is recalculated on each resize.
   QVector<QRect> m_screenRects;

   // Each rect in m_screenRects has 4 rects in m_cornerRects. They are the 
   // rects for the highlight-zones in the 4 corners, in this order:
   // top left, top right, bottom left, bottom right
   QVector<QRect> m_cornerRects;

   // m_screenText is the localized text that is visible inside each 
   // miniature-screen.
   QString m_screenText;

   // Update m_screenRects
   void updateRects(QSize aSize);
   void updateRects(int aWidth, int aHeight);

   // Update mouse-position, corner-highlights and preview widget
   void updateMouse(
      int aNewScreen, 
      int aNewCorner,
      bool aForce = false);
};

#endif
