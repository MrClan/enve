#ifndef EFFECTANIMATORS_H
#define EFFECTANIMATORS_H
#include "complexanimator.h"

namespace fmt_filters { struct image; }

class PixmapEffect;
class BoundingBox;
class SkImage;
class SkCanvas;
class SkBitmap;
struct BoundingBoxRenderData;
struct PixmapEffectRenderData;

#include "smartPointers/sharedpointerdefs.h"

class EffectAnimators : public ComplexAnimator {
    friend class SelfRef;
public:
    void addEffect(const qsptr<PixmapEffect> &effect);

    qreal getEffectsMargin() const;

    void setParentBox(BoundingBox *box);
    BoundingBox *getParentBox() {
        return mParentBox_k;
    }

    bool hasEffects();

    bool SWT_isPixmapEffectAnimators() { return true; }
    qreal getEffectsMarginAtRelFrame(const int &relFrame) const;
    qreal getEffectsMarginAtRelFrameF(const qreal &relFrame) const;

    void addEffectRenderDataToListF(const qreal &relFrame,
                                    BoundingBoxRenderData *data);

    void ca_removeAllChildAnimators();

    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
    void readPixmapEffect(QIODevice *target);
protected:
    EffectAnimators(BoundingBox *parentBox);
private:
    BoundingBox * const mParentBox_k;
};

#endif // EFFECTANIMATORS_H