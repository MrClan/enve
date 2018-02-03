#include "canvas.h"
#include "Animators/PathAnimators/vectorpathanimator.h"
#include "nodepoint.h"
#include "Animators/pathanimator.h"

void Canvas::connectPoints() {
    QList<NodePoint*> selectedNodePoints;
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        if(point->isNodePoint()) {
            if(((NodePoint*)point)->isEndPoint()) {
                selectedNodePoints.append( (NodePoint*) point);
            }
        }
    }
    clearPointsSelection();

    if(selectedNodePoints.count() == 2) {
        NodePoint *firstPoint = selectedNodePoints.first();
        NodePoint *secondPoint = selectedNodePoints.last();
        VectorPathAnimator *firstSinglePath = firstPoint->getParentPath();
        VectorPathAnimator *secondSinglePath = secondPoint->getParentPath();
        PathAnimator *firstParentPath = firstSinglePath->getParentPathAnimator();
        PathAnimator *secondParentPath = secondSinglePath->getParentPathAnimator();
        if(firstParentPath == secondParentPath) {
            if(firstSinglePath == secondSinglePath) {
                firstSinglePath->connectPoints(firstPoint, secondPoint);
            } else {
                if(firstPoint->isSeparateNodePoint()) {
                    firstSinglePath->revertAllPointsForAllKeys();
                }
                if(!secondPoint->isSeparateNodePoint()) {
                    secondSinglePath->revertAllPointsForAllKeys();
                }
                firstSinglePath->connectWith(secondSinglePath);
            }
        } else {
            if(firstPoint->isSeparateNodePoint()) {
                firstSinglePath->revertAllPointsForAllKeys();
            }
            if(!secondPoint->isSeparateNodePoint()) {
                secondSinglePath->revertAllPointsForAllKeys();
            }
            QMatrix firstMatrix =
                    firstParentPath->getCombinedTransform();
            QMatrix secondMatrix =
                    secondParentPath->getCombinedTransform();
            QMatrix effectiveMatrix =
                    secondMatrix*firstMatrix.inverted();
            secondSinglePath->applyTransformToPoints(effectiveMatrix);
            firstParentPath->addSinglePathAnimator(secondSinglePath);
            secondSinglePath->removeFromParent();
            secondSinglePath->setParentPath(firstParentPath);

            firstSinglePath->connectWith(secondSinglePath);
        }
    }
}

void Canvas::disconnectPoints() {
    QList<NodePoint*> selectedNodePoints;
        Q_FOREACH(MovablePoint *point, mSelectedPoints) {
            if(point->isNodePoint()) {
                NodePoint *nextPoint = ((NodePoint*)point)->getNextPoint();
                if(nextPoint == NULL) continue;
                if(nextPoint->isSelected()) {
                    selectedNodePoints.append( (NodePoint*) point);
                }
            }
        }
        clearPointsSelection();
        Q_FOREACH(NodePoint *point, selectedNodePoints) {
            NodePoint *secondPoint = point->getNextPoint();
            if(secondPoint == NULL) secondPoint = point->getPreviousPoint();
            if(point->getParentPath() ==
                    secondPoint->getParentPath()) {
                point->getParentPath()->
                        disconnectPoints(point, secondPoint);
            }
        }
}

void Canvas::mergePoints() {
    QList<NodePoint*> selectedNodePoints;
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        if(point->isNodePoint()) {
            //if(((NodePoint*)point)->isEndPoint()) {
                selectedNodePoints.append( (NodePoint*) point);
            //}
        }
    }
    clearPointsSelection();
    if(selectedNodePoints.count() == 2) {
        NodePoint *firstPoint = selectedNodePoints.first();
        NodePoint *secondPoint = selectedNodePoints.last();
        if(firstPoint->isEndPoint() &&
           secondPoint->isEndPoint()) {
            VectorPathAnimator *firstParentPath = firstPoint->getParentPath();
            VectorPathAnimator *secondParentPath = secondPoint->getParentPath();
            if(firstParentPath == secondParentPath) {
                secondParentPath->connectPoints(firstPoint, secondPoint);
            } else {
                if(firstPoint->isSeparateNodePoint()) {
                    firstParentPath->revertAllPointsForAllKeys();
                }
                if(!secondPoint->isSeparateNodePoint()) {
                    secondParentPath->revertAllPointsForAllKeys();
                }
                PathAnimator *firstPathAnimator =
                        firstParentPath->getParentPathAnimator();
                PathAnimator *secondPathAnimator =
                        secondParentPath->getParentPathAnimator();
                const QList<SkPoint> &listT =
                        secondParentPath->getElementsPosList();
                const QList<NodeSettings*> &nodeStts =
                        secondParentPath->getNodeSettingsList();
                if(firstPathAnimator == secondPathAnimator) {
                    for(int i = 0; i < listT.count() - 2; i += 3) {
                        QPointF pos = SkPointToQPointF(listT.at(i + 1));
                        firstParentPath->addNodeRelPos(
                                    SkPointToQPointF(listT.at(i)) + pos,
                                    pos,
                                    SkPointToQPointF(listT.at(i + 2)) + pos,
                                    -1, *nodeStts.at(i/3));
                    }
                } else {
                    QMatrix firstMatrix =
                            firstPathAnimator->getCombinedTransform();
                    QMatrix secondMatrix =
                            secondPathAnimator->getCombinedTransform();
                    QMatrix effectiveMatrix =
                            secondMatrix*firstMatrix.inverted();

                    for(int i = 0; i < listT.count() - 2; i += 3) {
                        QPointF pos = SkPointToQPointF(listT.at(i + 1));
                        pos = effectiveMatrix.map(pos);

                        QPointF startPos = SkPointToQPointF(listT.at(i)) + pos;
                        startPos = effectiveMatrix.map(startPos);

                        QPointF endPos = SkPointToQPointF(listT.at(i + 2)) + pos;
                        endPos = effectiveMatrix.map(endPos);
                        firstParentPath->addNodeRelPos(startPos, pos, endPos,
                                                       -1, *nodeStts.at(i/3));
                    }
                }
                secondParentPath->removeFromParent();
                return;
            }
        } else if(firstPoint->getNextPoint() != secondPoint &&
                  firstPoint->getPreviousPoint() != secondPoint) {
            return;
        }
        QPointF sumPos = firstPoint->getAbsolutePos() +
                secondPoint->getAbsolutePos();
        bool firstWasPrevious = firstPoint ==
                secondPoint->getPreviousPoint();

        secondPoint->startTransform();
        secondPoint->moveToAbs(sumPos/2);
        if(firstWasPrevious) {
            secondPoint->moveStartCtrlPtToAbsPos(
                        firstPoint->getStartCtrlPtAbsPos());
        } else {
            secondPoint->moveEndCtrlPtToAbsPos(
                        firstPoint->getEndCtrlPtAbsPos());
        }
        secondPoint->finishTransform();

        firstPoint->removeFromVectorPath();
    }
}

void Canvas::setPointCtrlsMode(CtrlsMode mode) {
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        if(point->isNodePoint()) {
            ( (NodePoint*)point)->setCtrlsMode(mode);
        }
    }
}

void Canvas::makeSelectedPointsSegmentsCurves() {
    QList<NodePoint*> selectedNodePoints;
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        if(point->isNodePoint()) {
            selectedNodePoints.append( (NodePoint*) point);
        }
    }
    Q_FOREACH(NodePoint *selectedPoint,
            selectedNodePoints) {
        NodePoint *nextPoint = selectedPoint->getNextPoint();
        NodePoint *prevPoint = selectedPoint->getPreviousPoint();
        if(selectedNodePoints.contains(nextPoint)) {
            selectedPoint->setEndCtrlPtEnabled(true);
        }
        if(selectedNodePoints.contains(prevPoint)) {
            selectedPoint->setStartCtrlPtEnabled(true);
        }
    }
}

void Canvas::makeSelectedPointsSegmentsLines() {
    QList<NodePoint*> selectedNodePoints;
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        if(point->isNodePoint()) {
            selectedNodePoints.append( (NodePoint*) point);
        }
    }
    Q_FOREACH(NodePoint *selectedPoint,
            selectedNodePoints) {
        NodePoint *nextPoint = selectedPoint->getNextPoint();
        NodePoint *prevPoint = selectedPoint->getPreviousPoint();
        if(selectedNodePoints.contains(nextPoint)) {
            selectedPoint->setEndCtrlPtEnabled(false);
        }
        if(selectedNodePoints.contains(prevPoint)) {
            selectedPoint->setStartCtrlPtEnabled(false);
        }
    }
}

void Canvas::finishSelectedPointsTransform() {
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        point->finishTransform();
    }
}

void Canvas::startSelectedPointsTransform() {
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        point->startTransform();
    }
}

void Canvas::cancelSelectedPointsTransform() {
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        point->cancelTransform();
    }
}

void Canvas::moveSelectedPointsByAbs(const QPointF &by,
                                     const bool &startTransform) {
    if(startTransform) {
        startSelectedPointsTransform();
        Q_FOREACH(MovablePoint *point, mSelectedPoints) {
            point->moveByAbs(by);
        }
    } else {
        Q_FOREACH(MovablePoint *point, mSelectedPoints) {
            point->moveByAbs(by);
        }
    }
}

void Canvas::selectAndAddContainedPointsToSelection(QRectF absRect) {
    Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
        box->selectAndAddContainedPointsToList(absRect, &mSelectedPoints);
    }
}

void Canvas::addPointToSelection(MovablePoint *point) {
    if(point->isSelected()) {
        return;
    }
    point->select();
    mSelectedPoints.append(point); schedulePivotUpdate();
}

void Canvas::removePointFromSelection(MovablePoint *point) {
    point->deselect();
    mSelectedPoints.removeOne(point); schedulePivotUpdate();
}

void Canvas::updateSelectedPointsAfterCtrlsVisiblityChanged() {
    if(!BoxesGroup::mCtrlsAlwaysVisible) {
        QList<MovablePoint*> pointsToDeselect;
        Q_FOREACH(MovablePoint *point, mSelectedPoints) {
            pointsToDeselect << point;
        }
        Q_FOREACH(MovablePoint *point, pointsToDeselect) {
            removePointFromSelection(point);
        }
    }
}

void Canvas::removeSelectedPointsApproximateAndClearList() {
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        point->deselect();
        point->removeApproximate();
    }
    mSelectedPoints.clear(); schedulePivotUpdate();
}

void Canvas::removeSelectedPointsAndClearList() {
    if(mIsMouseGrabbing) {
        if(!BoxesGroup::mCtrlsAlwaysVisible ||
            mSelectedPoints.count() == 1) {
            if(mLastPressedPoint != NULL) {
                if(mLastPressedPoint->isCtrlPoint()) {
                    mLastPressedPoint->cancelTransform();
                    mLastPressedPoint->deselect();
                    mLastPressedPoint->removeFromVectorPath();
                    mSelectedPoints.removeOne(mLastPressedPoint);
                    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                        point->deselect();
                    }
                    goto DONE;
                }
            }
        }
    }
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        point->deselect();
        point->removeFromVectorPath();
    }
DONE:
    mSelectedPoints.clear(); schedulePivotUpdate();
}

void Canvas::clearPointsSelection()
{
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        point->deselect();
    }
    mSelectedPoints.clear();
    if(mCurrentMode == MOVE_POINT) {
        schedulePivotUpdate();
    }
//    if(mLastPressedPoint != NULL) {
//        mLastPressedPoint->deselect();
//        mLastPressedPoint = NULL;
//    }
//    setCurrentEndPoint(NULL);
}

void Canvas::clearLastPressedPoint() {
    if(mLastPressedPoint != NULL) {
        mLastPressedPoint->deselect();
        mLastPressedPoint = NULL;
    }
}

void Canvas::clearCurrentEndPoint() {
    setCurrentEndPoint(NULL);
}

QPointF Canvas::getSelectedPointsAbsPivotPos() {
    if(mSelectedPoints.isEmpty()) return QPointF(0., 0.);
    QPointF posSum = QPointF(0., 0.);
    Q_FOREACH(MovablePoint *point, mSelectedPoints) {
        posSum += point->getAbsolutePos();
    }
    qreal invCount = 1./mSelectedPoints.length();
    return posSum*invCount;
}

bool Canvas::isPointsSelectionEmpty() {
    return mSelectedPoints.isEmpty();
}

int Canvas::getPointsSelectionCount() {
    return mSelectedPoints.length();
}

void Canvas::rotateSelectedPointsBy(const qreal &rotBy,
                                    const QPointF &absOrigin,
                                    const bool &startTrans) {
    if(mSelectedPoints.isEmpty()) return;
    if(mLocalPivot) {
        if(startTrans) {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->saveTransformPivotAbsPos(point->getAbsolutePos());
                point->rotateRelativeToSavedPivot(rotBy);
            }
        } else {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->rotateRelativeToSavedPivot(rotBy);
            }
        }
    } else {
        if(startTrans) {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->saveTransformPivotAbsPos(absOrigin);
                point->rotateRelativeToSavedPivot(rotBy);
            }
        } else {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->rotateRelativeToSavedPivot(rotBy);
            }
        }
    }
}

void Canvas::scaleSelectedPointsBy(const qreal &scaleXBy,
                                   const qreal &scaleYBy,
                                   const QPointF &absOrigin,
                                   const bool &startTrans) {
    if(mSelectedPoints.isEmpty()) return;
    if(mLocalPivot) {
        if(startTrans) {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->saveTransformPivotAbsPos(point->getAbsolutePos());
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy );
            }
        } else {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        }
    } else {
        if(startTrans) {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->startTransform();
                point->saveTransformPivotAbsPos(absOrigin);
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        } else {
            Q_FOREACH(MovablePoint *point, mSelectedPoints) {
                point->scaleRelativeToSavedPivot(scaleXBy, scaleYBy);
            }
        }
    }
}

void Canvas::clearPointsSelectionOrDeselect() {
    if(mSelectedPoints.isEmpty() ) {
        deselectAllBoxes();
    } else {
        clearPointsSelection();
        clearCurrentEndPoint();
        clearLastPressedPoint();
    }
}
