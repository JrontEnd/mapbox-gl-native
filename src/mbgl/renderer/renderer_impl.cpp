#include <mbgl/renderer/renderer_impl.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/storage/file_source.hpp>
#include <mbgl/renderer/render_style.hpp>
#include <mbgl/renderer/painter.hpp>
#include <mbgl/renderer/update_parameters.hpp>
// TODO move to renderer?
#include <mbgl/map/backend_scope.hpp>

namespace mbgl {

static RendererObserver& nullObserver() {
    static RendererObserver observer;
    return observer;
}

Renderer::Impl::Impl(Backend& backend_,
                     float pixelRatio_,
                     Scheduler& scheduler_,
                     FileSource& fileSource_,
                     MapMode mode_,
                     GLContextMode contextMode_,
                     const optional<std::string> programCacheDir_)
        : observer(&nullObserver())
        , backend(backend_)
        , mode(mode_)
        , contextMode(contextMode_)
        , pixelRatio(pixelRatio_),
          programCacheDir(programCacheDir_)
        , renderStyle(std::make_unique<RenderStyle>(scheduler_, fileSource_)) {

    renderStyle->setObserver(this);
}

Renderer::Impl::~Impl() = default;

void Renderer::Impl::setObserver(RendererObserver* observer_) {
    observer = observer_ ? observer_ : &nullObserver();
}

void Renderer::Impl::update(const UpdateParameters& parameters) {
    // Keep the time point for following render calls
    timePoint = parameters.timePoint;

    renderStyle->update(parameters);
}

void Renderer::Impl::render(const RenderParameters& params, RenderCallback callback) {

    // Initialize painter
    if (!painter) {
        gl::Context& context = backend.getContext();
        painter = std::make_unique<Painter>(context,
                                            pixelRatio,
                                            programCacheDir);
    }

    // Update transform state on painter
    painter->state = params.transformState;

    bool loaded = params.styleLoaded && renderStyle->isLoaded();

    if (mode == MapMode::Continuous) {
        if (renderState == RenderState::Never) {
            observer->onWillStartRenderingMap();
        }

        observer->onWillStartRenderingFrame();

        FrameData frameData { timePoint,
                              pixelRatio,
                              mode,
                              contextMode,
                              params.debugOptions };

        backend.updateAssumedState();

        painter->render(*renderStyle,
                        //tODO params.transformState,
                        frameData,
                        params.view);

        painter->cleanup();

        observer->onDidFinishRenderingFrame(loaded
            ? RendererObserver::RenderMode::Full
            : RendererObserver::RenderMode::Partial);

        if (!loaded) {
            renderState = RenderState::Partial;
        } else if (renderState != RenderState::Fully) {
            renderState = RenderState::Fully;
            observer->onDidFinishRenderingMap(RendererObserver::RenderMode::Full);
        }

        // Schedule an update if we need to paint another frame due to transitions or
        // animations that are still in progress
        callback(renderStyle->hasTransitions() || painter->needsAnimation());
    } else if (params.stillImageCallback && loaded) {
        FrameData frameData { timePoint,
                              pixelRatio,
                              mode,
                              contextMode,
                              params.debugOptions };

        backend.updateAssumedState();

        painter->render(*renderStyle,
                        frameData,
                        params.view);

        //auto request = std::move(stillImageRequest);
        params.stillImageCallback->operator()(nullptr);

        painter->cleanup();
    }
}

std::vector<Feature> Renderer::Impl::queryRenderedFeatures(const ScreenLineString& geometry,
                                                     const TransformState& transformState,
                                                     const RenderedQueryOptions& options) const {
    if (!renderStyle) return {};

    return renderStyle->queryRenderedFeatures(geometry, transformState, options);
}

std::vector<Feature> Renderer::Impl::querySourceFeatures(const std::string& sourceID,
                                                         const SourceQueryOptions& options) const {
    if (!renderStyle) return {};

    const RenderSource* source = renderStyle->getRenderSource(sourceID);
    if (!source) return {};

    return source->querySourceFeatures(options);
}

void Renderer::Impl::onInvalidate() {
    observer->onInvalidate();
};

void Renderer::Impl::onResourceError(std::exception_ptr ptr) {
    observer->onResourceError(ptr);
}

void Renderer::Impl::onLowMemory() {
    // TODO
    if (painter) {
        BackendScope guard(backend);
        painter->cleanup();
    }
    if (renderStyle) {
        renderStyle->onLowMemory();
        backend.invalidate();
    }
}

void Renderer::Impl::setSourceTileCacheSize(size_t size) {
    if (!renderStyle) return;

    if (size != sourceCacheSize) {
        sourceCacheSize = size;
        renderStyle->setSourceTileCacheSize(size);
        backend.invalidate();
    }
};

void Renderer::Impl::dumDebugLogs() {
    if (!renderStyle) return;

    renderStyle->dumpDebugLogs();
};


}
