#include <mbgl/storage/file_source_request.hpp>

#include <mbgl/actor/mailbox.hpp>

namespace mbgl {

FileSourceRequest::FileSourceRequest(std::shared_ptr<Mailbox> mailbox_) : mailbox(mailbox_) {
}

FileSourceRequest::~FileSourceRequest() {
    cancelCallback();
}

void FileSourceRequest::onResponse(FileSource::Callback&& callback) {
    responseCallback = std::move(callback);
}

void FileSourceRequest::onCancel(std::function<void()>&& callback) {
    cancelCallback = std::move(callback);
}

void FileSourceRequest::setResponse(const Response& response) {
    // Copy, because calling the callback will
    // sometimes self destroy this object.
    auto callback = responseCallback;
    callback(response);
}

} // namespace mbgl
