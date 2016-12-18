#ifndef POINTHELPERS_H
#define POINTHELPERS_H
#include <QPointF>
#include <QString>

enum CtrlsMode {
    CTRLS_SMOOTH,
    CTRLS_SYMMETRIC,
    CTRLS_CORNER
};

extern qreal qRandF(qreal fMin, qreal fMax);

extern QPointF symmetricToPos(QPointF toMirror, QPointF mirrorCenter);
extern QPointF symmetricToPosNewLen(QPointF toMirror, QPointF mirrorCenter,
                                    qreal newLen);
extern qreal pointToLen(QPointF point);
extern bool isPointZero(QPointF pos);
extern QPointF scalePointToNewLen(QPointF point, qreal newLen);

extern void getCtrlsSymmetricPos(QPointF endPos, QPointF startPos,
                                 QPointF centerPos, QPointF *newEndPos,
                                 QPointF *newStartPos);
extern void getCtrlsSmoothPos(QPointF endPos, QPointF startPos,
                              QPointF centerPos, QPointF *newEndPos,
                              QPointF *newStartPos);

extern qreal clamp(qreal val, qreal min, qreal max);

extern qreal qclamp(qreal val, qreal min, qreal max);

extern qreal calcCubicBezierVal(qreal p0, qreal p1, qreal p2, qreal p3, qreal t);

extern qreal tFromX(qreal p0x, qreal p1x, qreal p2x, qreal p3x, qreal x);

extern bool isZero(qreal val);

extern bool isZero2Dec(qreal val);

extern bool isZero1Dec(qreal val);

extern QString boolToSql(bool bT);

#endif // POINTHELPERS_H
