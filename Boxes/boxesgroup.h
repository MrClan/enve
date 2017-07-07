﻿#ifndef BOXESGROUP_H
#define BOXESGROUP_H
#include "Boxes/boundingbox.h"
#include "Boxes/vectorpath.h"
#include "fillstrokesettings.h"

#define Q_FOREACHBoxInListInverted(boxesList) BoundingBox *box = getAtIndexOrGiveNull((boxesList).count() - 1, (boxesList)); \
    for(int i = (boxesList).count() - 1; i >= 0; i--, box = getAtIndexOrGiveNull(i, (boxesList)) )



class MainWindow;

class VectorPathEdge;

struct BoxesGroupRenderData : public BoundingBoxRenderData {
    QList<BoundingBoxRenderData*> childrenRenderData;

    bool shouldPaintOnImage = true;

    void drawRenderedImage(SkCanvas *canvas) {
        if(shouldPaintOnImage) {
            BoundingBoxRenderData::drawRenderedImage(canvas);
        } else {
            SkPaint paint;
            paint.setAlpha(qRound(opacity*2.55));
            paint.setBlendMode(blendMode);
            canvas->saveLayer(NULL, &paint);
            Q_FOREACH(BoundingBoxRenderData *renderData,
                      childrenRenderData) {
                renderData->drawRenderedImage(canvas);
            }
            canvas->restore();
        }
    }

private:
    void drawSk(SkCanvas *canvas) {
        canvas->save();
        canvas->concat(QMatrixToSkMatrix(transform.inverted()));
        Q_FOREACH(BoundingBoxRenderData *renderData,
                  childrenRenderData) {
            //box->draw(p);
            renderData->drawRenderedImage(canvas);
        }

        canvas->restore();
    }
};

class BoxesGroup : public BoundingBox {
    Q_OBJECT
public:
    BoxesGroup(BoxesGroup *parent);
    BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting);
    ~BoxesGroup();
    virtual void prp_loadFromSql(const int &boundingBoxId);

    BoundingBox *createLink(BoxesGroup *parent);
    BoundingBox *createSameTransformationLink(BoxesGroup *parent);

    void deselectAllBoxesFromBoxesGroup();
    void selectAllBoxesFromBoxesGroup();

    BoundingBox *getBoxAt(const QPointF &absPos);
    //MovablePoint *getPointAt(const QPointF &absPos, const CanvasMode &currentMode);

    void addContainedBoxesToSelection(const QRectF &rect);

    void setIsCurrentGroup(const bool &bT);

    BoundingBox *getPathAtFromAllAncestors(const QPointF &absPos);

    void setCurrentFillStrokeSettingsFromBox(BoundingBox *box);

    void ungroup();

    int saveToSql(QSqlQuery *query, const int &parentId);
    BoxesGroup *loadChildrenFromSql(const int &thisBoundingBoxId,
                                    const bool &loadInBox);

    bool isCurrentGroup();
    void addChild(BoundingBox *child);
    void addChildToListAt(const int &index, BoundingBox *child,
                          const bool &saveUndoRedo = true);
    void updateChildrenId(const int &firstId,
                          const bool &saveUndoRedo = true);
    void updateChildrenId(const int &firstId,
                          const int &lastId,
                          const bool &saveUndoRedo = true);
    void removeChild(BoundingBox *child);
    void increaseChildZInList(BoundingBox *child);
    void decreaseChildZInList(BoundingBox *child);
    void bringChildToEndList(BoundingBox *child);
    void bringChildToFrontList(BoundingBox *child);
    void moveChildInList(BoundingBox *child,
                         const int &from,
                         const int &to,
                         const bool &saveUndoRedo = true);
    void moveChildBelow(BoundingBox *boxToMove,
                        BoundingBox *below);
    void moveChildAbove(BoundingBox *boxToMove,
                        BoundingBox *above);

    void removeChildFromList(const int &id,
                             const bool &saveUndoRedo = true);

    void setStrokeCapStyle(const Qt::PenCapStyle &capStyle);
    void setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle);
    void setStrokeWidth(const qreal &strokeWidth, const bool &finish);

    PaintSettings *getFillSettings();
    StrokeSettings *getStrokeSettings();
    void updateAllBoxes();

    void updateRelBoundingRect();
    void applyCurrentTransformation();

    bool relPointInsidePath(const QPointF &relPos);
//    QPointF getRelCenterPosition();
    void updateEffectsMargin();

    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     ScrollWidgetVisiblePart *visiblePartWidget);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);
    void startSelectedStrokeWidthTransform();
    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    void makeDuplicate(Property *targetBox);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);

    void applyPaintSetting(
            const PaintSetting &setting) {
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
            box->applyPaintSetting(setting);
        }
    }

    void setFillColorMode(const ColorMode &colorMode) {
        Q_FOREACH(const QSharedPointer<BoundingBox> &box,  mChildBoxes) {
            box->setFillColorMode(colorMode);
        }
    }
    void setStrokeColorMode(const ColorMode &colorMode) {
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
            box->setStrokeColorMode(colorMode);
        }
    }
    void clearAllCache();
    void drawPixmapSk(SkCanvas *canvas);
    void setDescendantCurrentGroup(const bool &bT);
    bool isDescendantCurrentGroup();
    bool shouldPaintOnImage();

    virtual void addChildAwaitingUpdate(BoundingBox *child);
    void updateAfterCombinedTransformationChanged();
    void updateCombinedTransformTmp();
    void updateAfterCombinedTransformationChangedAfterFrameChagne();

    bool SWT_isBoxesGroup() { return true; }
    void drawSk(SkCanvas *canvas);
    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);
    void prp_setAbsFrame(const int &frame);
    void schedulerProccessed();

    void createCurrentRenderData() {
        mCurrentRenderData = new BoxesGroupRenderData();
    }

    void setupBoundingBoxRenderDataForRelFrame(const int &relFrame,
                                               BoundingBoxRenderData *data);

protected:
    static bool mCtrlsAlwaysVisible;
    FillStrokeSettingsWidget *mFillStrokeSettingsWidget;
    bool mIsCurrentGroup = false;
    bool mIsDescendantCurrentGroup = false;
    QList<QSharedPointer<BoundingBox> > mChildBoxes;

    QList<QSharedPointer<BoundingBox> > mChildrenAwaitingUpdate;
    //QList<QSharedPointer<BoundingBox> > mUpdateChildrenAwaitingUpdate;
    int getChildBoxIndex(BoundingBox *child);
signals:
    void changeChildZSignal(int, int);
    void removeAnimatedBoundingBoxSignal(BoundingBox*);
    void addAnimatedBoundingBoxSignal(BoundingBox*);
};

#endif // BOXESGROUP_H
