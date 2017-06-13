#pragma once

#include <exception>

namespace mbgl {

class RendererObserver {
public:
    virtual ~RendererObserver() = default;

    enum class RenderMode : uint32_t {
        Partial,
        Full
    };

    virtual void onInvalidate() {}
    virtual void onResourceError(std::exception_ptr) {}

    virtual void onWillStartRenderingFrame() {}
    virtual void onDidFinishRenderingFrame(RenderMode) {}
    virtual void onWillStartRenderingMap() {}
    virtual void onDidFinishRenderingMap(RenderMode) {}
};

} // namespace mbgl
