#pragma once

#include <mbgl/map/mode.hpp>
#include <mbgl/map/query.hpp>
#include <mbgl/util/feature.hpp>
#include <mbgl/util/geo.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace mbgl {

class Backend;
class FileSource;
class UpdateParameters;
class RenderParameters;
class RendererObserver;
class Scheduler;
class TransformState;

class Renderer {
public:
    Renderer(Backend&, float pixelRatio_, Scheduler&, FileSource&,
             MapMode, GLContextMode, const optional<std::string>);
    ~Renderer();

    using UpdateCallback = std::function<void()>;

    void update(std::unique_ptr<UpdateParameters>, UpdateCallback);

    void render(std::unique_ptr<RenderParameters>);

    void setObserver(RendererObserver*);

    std::vector<Feature> queryRenderedFeatures(const ScreenLineString&,
                                               const TransformState&,
                                               const RenderedQueryOptions&) const;

    std::vector<Feature> querySourceFeatures(const std::string&,
                                             const SourceQueryOptions&) const;

    void dumpDebugLogs();

    void onLowMemory();

    void setSourceTileCacheSize(size_t);

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace mbgl
