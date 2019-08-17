#ifndef IMAGECACHEHANDLER_H
#define IMAGECACHEHANDLER_H
#include "skia/skiahelpers.h"
#include "filecachehandler.h"
#include "Tasks/updatable.h"
class ImageDataHandler;
class ImageLoader : public HDDTask {
    e_OBJECT
protected:
    ImageLoader(const QString &filePath,
                ImageDataHandler * const handler);
public:
    void process();
    void afterProcessing();
    void afterCanceled();
private:
    ImageDataHandler * const mTargetHandler;
    const QString mFilePath;
    sk_sp<SkImage> mImage;
};

class ImageDataHandler : public FileDataCacheHandler {
    e_OBJECT
    friend class ImageLoader;
protected:
    ImageDataHandler();
public:
    void afterSourceChanged() {}

    void clearCache() {
        mImage.reset();
        mImageLoader.reset();
    }

    ImageLoader * scheduleLoad();
    bool hasImage() const { return mImage.get(); }
    sk_sp<SkImage> getImage() const { return mImage; }
    sk_sp<SkImage> getImageCopy() const {
        return SkiaHelpers::makeCopy(mImage);
    }
protected:
    void setImage(const sk_sp<SkImage>& img) {
        mImage = img;
        mImageLoader.reset();
    }
private:
    sk_sp<SkImage> mImage;
    stdsptr<ImageLoader> mImageLoader;
};

class ImageFileHandler : public FileCacheHandler {
    e_OBJECT
protected:
    ImageFileHandler() {}

    void afterPathSet(const QString& path) {
        mFileMissing = !QFile(path).exists();
        if(mFileMissing) return mDataHandler.reset();
        const auto current = ImageDataHandler::sGetDataHandler<ImageDataHandler>(path);
        if(current) mDataHandler = current->ref<ImageDataHandler>();
        else mDataHandler = ImageDataHandler::sCreateDataHandler<ImageDataHandler>(path);
    }

    void reload() {
        mDataHandler->clearCache();
    }
public:
    void replace(QWidget * const parent);

    ImageLoader * scheduleLoad() {
        if(!mDataHandler) return nullptr;
        return mDataHandler->scheduleLoad();
    }

    bool hasImage() const {
        if(!mDataHandler) return false;
        return mDataHandler->hasImage();
    }

    sk_sp<SkImage> getImage() const {
        if(!mDataHandler) return nullptr;
        return mDataHandler->getImage();
    }
    sk_sp<SkImage> getImageCopy() const {
        if(!mDataHandler) return nullptr;
        return mDataHandler->getImageCopy();
    }
private:
    qsptr<ImageDataHandler> mDataHandler;
};

#endif // IMAGECACHEHANDLER_H