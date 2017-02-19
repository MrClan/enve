#ifndef QREALANIMATORVALUESLIDER_H
#define QREALANIMATORVALUESLIDER_H
#include "qdoubleslider.h"

class QrealAnimator;

class QrealAnimatorValueSlider : public QDoubleSlider
{
    Q_OBJECT
public:
    QrealAnimatorValueSlider(QString name, qreal minVal,
                             qreal maxVal, qreal prefferedStep,
                             QWidget *parent);
    QrealAnimatorValueSlider(qreal minVal, qreal maxVal, qreal prefferedStep,
                             QWidget *parent);
    QrealAnimatorValueSlider(qreal minVal, qreal maxVal, qreal prefferedStep,
                             QrealAnimator *animator, QWidget *parent = 0);
    QrealAnimatorValueSlider(QrealAnimator *animator, QWidget *parent = 0);
    QrealAnimatorValueSlider(QWidget *parent = 0);
    ~QrealAnimatorValueSlider() {}

    void paint(QPainter *p);

    void setAnimator(QrealAnimator *animator);
    bool hasTargetAnimator() {
        return mAnimator != NULL;
    }
    void openContextMenu(const QPoint &globalPos);
protected:
    void emitEditingStarted(qreal value);
    void emitValueChanged(qreal value);
    void emitEditingFinished(qreal value);
private:
    QrealAnimator *mAnimator = NULL;
private slots:
    void nullifyAnimator();
};

#endif // QREALANIMATORVALUESLIDER_H
