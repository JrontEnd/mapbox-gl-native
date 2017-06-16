#include "asset_manager_file_source.hpp"

#include <mbgl/storage/file_source_request.hpp>
#include <mbgl/storage/response.hpp>
#include <mbgl/util/util.hpp>
#include <mbgl/util/threaded_object.hpp>
#include <mbgl/util/url.hpp>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

namespace mbgl {

class AssetManagerFileSource::Impl {
public:
    Impl(ActorRef<Impl>, AAssetManager* assetManager_) : assetManager(assetManager_) {
    }

    void request(const std::string& url, FileSource::Callback callback) {
        // Note: AssetManager already prepends "assets" to the filename.
        const std::string path = mbgl::util::percentDecode(url.substr(8));

        Response response;

        if (AAsset* asset = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_BUFFER)) {
            response.data = std::make_shared<std::string>(
                reinterpret_cast<const char*>(AAsset_getBuffer(asset)), AAsset_getLength64(asset));
            AAsset_close(asset);
        } else {
            response.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound,
                                                               "Could not read asset");
        }

        callback(response);
    }

private:
    AAssetManager* assetManager;
};

AssetManagerFileSource::AssetManagerFileSource(jni::JNIEnv& env, jni::Object<android::AssetManager> assetManager_)
    : assetManager(assetManager_.NewGlobalRef(env)),
      impl(std::make_unique<util::ThreadedObject<Impl>>("AssetManagerFileSource",
          AAssetManager_fromJava(&env, jni::Unwrap(**assetManager)))),
      thread(std::make_unique<ActorRef<Impl>>(impl->actor())) {
}

AssetManagerFileSource::~AssetManagerFileSource() = default;

std::unique_ptr<AsyncRequest> AssetManagerFileSource::request(const Resource& resource, Callback callback) {
    auto mailbox = std::make_shared<Mailbox>(*util::RunLoop::Get());
    auto request = std::make_unique<FileSourceRequest>(mailbox);

    request->onResponse(std::move(callback));

    thread->invoke(&Impl::request, resource.url, [ref =
            ActorRef<FileSourceRequest>(*request, mailbox)] (const Response& res) mutable {
        ref.invoke(&FileSourceRequest::setResponse, res);
    });

    return std::move(request);
}

} // namespace mbgl
