#pragma once
#include <mbgl/renderer/renderer.hpp>
#include <mbgl/renderer/renderer_observer.hpp>
#include <mbgl/renderer/render_style_observer.hpp>
#include <mbgl/style/style.hpp>

// TODO extract StillImageCallback
#include <mbgl/map/map.hpp>
// TODO: rename to render_mode and move
#include <mbgl/map/mode.hpp>
#include <mbgl/map/backend.hpp>
#include <mbgl/map/transform_state.hpp>

#include <memory>
#include <string>

namespace mbgl {

enum class RenderState : uint8_t {
    Never,
    Partial,
    Fully,
};

class Painter;
class RenderStyle;
class View;

// TODO find a decent place for this
class RenderParameters {
public:
    bool styleLoaded;
    const TransformState transformState;
    MapDebugOptions debugOptions;
    View& view;

    // For still image requests
    optional<Map::StillImageCallback> stillImageCallback;
};

class Renderer::Impl : public RenderStyleObserver {
public:
    Impl(Backend&, float pixelRatio_, Scheduler&, FileSource&,
         MapMode, GLContextMode, const optional<std::string>);
    ~Impl() final;

    void setObserver(RendererObserver*);

    void update(const UpdateParameters&);

    void render(const RenderParameters&, RenderCallback);

    std::vector<Feature> queryRenderedFeatures(const ScreenLineString&,
                                               const TransformState&,
                                               const RenderedQueryOptions&) const;

    std::vector<Feature> querySourceFeatures(const std::string&,
                                             const SourceQueryOptions&) const;


    void onLowMemory();
    void setSourceTileCacheSize(size_t);

    void dumDebugLogs() ;

    // RenderStyleObserver implementation
    void onInvalidate()override;
    void onResourceError(std::exception_ptr) override;

private:
    friend class Renderer;

    RendererObserver* observer;
    Backend& backend;

    const MapMode mode;
    const GLContextMode contextMode;
    const float pixelRatio;
    const optional<std::string> programCacheDir;

    RenderState renderState = RenderState::Never;

    std::unique_ptr<RenderStyle> renderStyle;
    std::unique_ptr<Painter> painter;

    size_t sourceCacheSize;

    // Gets updated on update calls
    TimePoint timePoint;

};

} // namespace mbgl
