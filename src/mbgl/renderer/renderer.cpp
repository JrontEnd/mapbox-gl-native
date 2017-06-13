#include <mbgl/renderer/renderer.hpp>
#include <mbgl/renderer/renderer_impl.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/map/backend_scope.hpp>

namespace mbgl {

Renderer::Renderer(Backend& backend_,
                   float pixelRatio_,
                   Scheduler& scheduler_,
                   FileSource& fileSource_,
                   MapMode mode_,
                   GLContextMode contextMode_,
                   const optional<std::string> programCacheDir_)
        : impl(std::make_unique<Impl>(backend_, pixelRatio_, scheduler_, fileSource_, mode_,
                                      contextMode_, std::move(programCacheDir_))) {
}
    
Renderer::~Renderer() = default;

void Renderer::setObserver(RendererObserver* observer) {
    impl->setObserver(observer);
}

void Renderer::update(std::unique_ptr<UpdateParameters> parameters, UpdateCallback callback) {
    impl->update(*parameters);
    callback();
}

void Renderer::render(std::unique_ptr<RenderParameters> parameters) {
    assert(BackendScope::exists());
    impl->render(*parameters);
}

std::vector<Feature> Renderer::queryRenderedFeatures(const ScreenLineString& geometry,
                                                     const TransformState& transformState,
                                                     const RenderedQueryOptions& options) const {
    return impl->queryRenderedFeatures(geometry, transformState, options);
}

std::vector<Feature> Renderer::querySourceFeatures(const std::string& sourceID, const SourceQueryOptions& options) const {
    return impl->querySourceFeatures(sourceID, options);
}

void Renderer::dumpDebugLogs() {
    impl->dumDebugLogs();
}

void Renderer::setSourceTileCacheSize(size_t size) {
    impl->setSourceTileCacheSize(size);
}

void Renderer::onLowMemory() {
    impl->onLowMemory();
}
    
} // namespace mbgl
