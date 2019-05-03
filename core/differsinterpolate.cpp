#include "differsinterpolate.h"
#include "Segments/qcubicsegment1d.h"
#include "Animators/SmartPath/smartpathcontainer.h"

bool gDiffers(const QString &val1, const QString &val2) {
    return val1 != val2;
}

void gInterpolate(const qreal &val1, const qreal &val2,
                  const qreal &t, qreal &val) {
    val = val1*(1 - t) + val2*t;
}

bool gDiffers(const bool &val1, const bool &val2) {
    return val1 != val2;
}

bool gDiffers(const qCubicSegment1D &val1,
              const qCubicSegment1D &val2) {
    return val1 != val2;
}

void gInterpolate(const qCubicSegment1D &val1,
                  const qCubicSegment1D &val2,
                  const qreal &t, qCubicSegment1D &val) {
    val = val1*(1 - t) + val2*t;
}

bool gDiffers(const SmartPath &path1, const SmartPath &path2) {
    return SmartPath::sDifferent(path1, path2);
}

void gInterpolate(const SmartPath &path1, const SmartPath &path2,
                  const qreal &path2Weight, SmartPath &target) {
    SmartPath::sInterpolate(path1, path2, path2Weight, target);
}

#include "brushpolyline.h"
bool gDiffers(const BrushPolyline& path1, const BrushPolyline& path2) {
    return &path1 != &path2;
}

void gInterpolate(const BrushPolyline &path1, const BrushPolyline &path2,
                  const qreal &path2Weight, BrushPolyline &target) {
    target = BrushPolyline::sInterpolate(path1, path2, path2Weight);
}
