#pragma once

#include <mbgl/storage/file_source.hpp>
#include <mbgl/util/async_request.hpp>

#include <memory>
#include <functional>

namespace mbgl {

class Mailbox;

class FileSourceRequest : public AsyncRequest {
public:
    FileSourceRequest(std::shared_ptr<Mailbox> mailbox);
    ~FileSourceRequest() final;

    void onResponse(FileSource::Callback&& callback);
    void onCancel(std::function<void()>&& callback);
    void setResponse(const Response& res);

private:
    void runResponseCallback();

    std::shared_ptr<Mailbox> mailbox;

    FileSource::Callback responseCallback = [](Response) {};
    std::function<void()> cancelCallback = []{};
};

} // namespace mbgl
