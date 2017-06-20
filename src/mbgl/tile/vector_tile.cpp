#include <mbgl/tile/vector_tile.hpp>
#include <mbgl/tile/tile_loader_impl.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/renderer/tile_parameters.hpp>

#include <mapbox/vector_tile.hpp>
#include <protozero/pbf_reader.hpp>

#include <unordered_map>
#include <functional>
#include <utility>

namespace mbgl {

class VectorTileFeature : public GeometryTileFeature {
public:
    VectorTileFeature(const mapbox::vector_tile::layer& layer,
                      const protozero::data_view& view)
        : feature(view, layer) {
    }

    FeatureType getType() const override {
        switch (feature.getType()) {
        case mapbox::vector_tile::GeomType::POINT:
            return FeatureType::Point;
        case mapbox::vector_tile::GeomType::LINESTRING:
            return FeatureType::LineString;
        case mapbox::vector_tile::GeomType::POLYGON:
            return FeatureType::Polygon;
        default:
            return FeatureType::Unknown;
        }
    }

    optional<Value> getValue(const std::string& key) const override {
        return feature.getValue(key);
    }

    std::unordered_map<std::string, Value> getProperties() const override {
        return feature.getProperties();
    }

    optional<FeatureIdentifier> getID() const override {
        return feature.getID();
    }

    GeometryCollection getGeometries() const override {
        return fixupPolygons(feature.getGeometries<GeometryCollection>(2.0));
    }

private:
    mapbox::vector_tile::feature feature;
};

class VectorTileLayer : public GeometryTileLayer {
public:
    VectorTileLayer(const protozero::data_view& view)
        : layer(view) {
    }

    std::size_t featureCount() const override {
        return layer.featureCount();
    }

    std::unique_ptr<GeometryTileFeature> getFeature(std::size_t i) const override {
        return std::make_unique<VectorTileFeature>(layer, layer.getFeature(i));
    }

    std::string getName() const override {
        return layer.getName();
    }

private:
    mapbox::vector_tile::layer layer;
};

class VectorTileData : public GeometryTileData {
public:
    VectorTileData(std::shared_ptr<const std::string> data_) : data(std::move(data_)) {
    }

    std::unique_ptr<GeometryTileData> clone() const override {
        return std::make_unique<VectorTileData>(data);
    }

    const GeometryTileLayer* getLayer(const std::string& name) const override {
        if (!parsed) {
            // We're parsing this lazily so that we can construct VectorTileData objects on the main
            // thread without incurring the overhead of parsing immediately.
            for (const auto& pair : mapbox::vector_tile::buffer(*data).getLayers()) {
                layers.emplace(pair.first, VectorTileLayer{ pair.second });
            }
            parsed = true;
        }

        auto it = layers.find(name);
        if (it != layers.end()) {
            return &it->second;
        }
        return nullptr;
    }

private:
    std::shared_ptr<const std::string> data;
    mutable bool parsed = false;
    mutable std::unordered_map<std::string, VectorTileLayer> layers;
};

VectorTile::VectorTile(const OverscaledTileID& id_,
                       std::string sourceID_,
                       const TileParameters& parameters,
                       const Tileset& tileset)
    : GeometryTile(id_, sourceID_, parameters), loader(*this, id_, parameters, tileset) {
}

void VectorTile::setNecessity(Necessity necessity) {
    loader.setNecessity(necessity);
}

void VectorTile::setData(std::shared_ptr<const std::string> data_,
                         optional<Timestamp> modified_,
                         optional<Timestamp> expires_) {
    modified = modified_;
    expires = expires_;

    GeometryTile::setData(data_ ? std::make_unique<VectorTileData>(data_) : nullptr);
}

} // namespace mbgl
