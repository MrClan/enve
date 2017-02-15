#ifndef TEXTBOX_H
#define TEXTBOX_H
#include "Boxes/pathbox.h"

class TextBox : public PathBox
{
public:
    TextBox(BoxesGroup *parent);

    void setText(QString text);

    void setFont(QFont font);
    void setSelectedFontSize(qreal size);
    void setSelectedFontFamilyAndStyle(QString fontFamily,
                               QString fontStyle);
    void drawSelected(QPainter *p,
                      const CanvasMode &currentCanvasMode);

    void openTextEditor();
    int saveToSql(QSqlQuery *query, int parentId);
    void loadFromSql(int boundingBoxId);
    void updatePath();
    MovablePoint *getPointAt(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode);
//    QRectF getTextRect();
    void setPathText(bool pathText);
private:
    QString mText;
    QFont mFont;
    Qt::Alignment mAlignment = Qt::AlignLeft;
};

#endif // TEXTBOX_H
