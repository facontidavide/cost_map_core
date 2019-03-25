/**
 * @file src/lib/cost_map.cpp
 */

#include <grid_map_core/GridMapMath.hpp>
#include <cost_map_core/cost_map.hpp>
#include <cost_map_core/iterators/costmap_iterator.hpp>
#include <iostream>
#include <cassert>
#include <math.h>
#include <algorithm>
#include <stdexcept>
#include "../include/cost_map_core/submap_geometry.hpp"

#include <Eigen/Dense>

namespace cost_map {

CostMap::CostMap(const std::vector<std::string>& layers)
{
  position_.setZero();
  length_.setZero();
  resolution_ = 0.0;
  size_.setZero();
  startIndex_.setZero();
  timestamp_ = 0;
  layers_ = layers;

  for (auto& layer : layers_) {
    data_.insert(std::pair<std::string, Matrix>(layer, Matrix()));
  }
}

CostMap::CostMap() :
    CostMap(std::vector<std::string>())
{
}

CostMap::~CostMap()
{
}

void CostMap::setGeometry(const cost_map::Length& length, const double resolution,
                          const cost_map::Position& position)
{
  assert(length(0) > 0.0);
  assert(length(1) > 0.0);
  assert(resolution > 0.0);

  Size size;
  size(0) = static_cast<int>(round(length(0) / resolution)); // There is no round() function in Eigen.
  size(1) = static_cast<int>(round(length(1) / resolution));
  resize(size);
  clearAll();

  resolution_ = resolution;
  length_ = (size_.cast<double>() * resolution_).matrix();
  position_ = position;
  startIndex_.setZero();

  return;
}

void CostMap::setGeometry(const SubmapGeometry& geometry)
{
  setGeometry(geometry.getLength(), geometry.getResolution(), geometry.getPosition());
}

void CostMap::setBasicLayers(const std::vector<std::string>& basicLayers)
{
  basicLayers_ = basicLayers;
}

const std::vector<std::string>& CostMap::getBasicLayers() const
{
  return basicLayers_;
}

bool CostMap::hasSameLayers(const cost_map::CostMap& other) const
{
  for (const auto& layer : layers_) {
    if (!other.exists(layer)) return false;
  }
  return true;
}

void CostMap::add(const std::string& layer, const DataType value)
{
  add(layer, Matrix::Constant(size_(0), size_(1), value));
}

void CostMap::add(const std::string& layer, const Matrix& data)
{
  assert(size_(0) == data.rows());
  assert(size_(1) == data.cols());

  if (exists(layer)) {
    // Type exists already, overwrite its data.
    data_.at(layer) = data;
  } else {
    // Type does not exist yet, add type and data.
    data_.insert(std::pair<std::string, Matrix>(layer, data));
    layers_.push_back(layer);
  }
}

bool CostMap::exists(const std::string& layer) const
{
  return !(data_.find(layer) == data_.end());
}

const cost_map::Matrix& CostMap::get(const std::string& layer) const
{
  try {
    return data_.at(layer);
  } catch (const std::out_of_range& exception) {
    throw std::out_of_range("CostMap::get(...) : No map layer '" + layer + "' available.");
  }
}

cost_map::Matrix& CostMap::get(const std::string& layer)
{
  try {
    return data_.at(layer);
  } catch (const std::out_of_range& exception) {
    throw std::out_of_range("CostMap::get(...) : No map layer of type '" + layer + "' available.");
  }
}

const cost_map::Matrix& CostMap::operator [](const std::string& layer) const
{
  return get(layer);
}

cost_map::Matrix& CostMap::operator [](const std::string& layer)
{
  return get(layer);
}

bool CostMap::erase(const std::string& layer)
{
  const auto dataIterator = data_.find(layer);
  if (dataIterator == data_.end()) return false;
  data_.erase(dataIterator);

  const auto layerIterator = std::find(layers_.begin(), layers_.end(), layer);
  if (layerIterator == layers_.end()) return false;
  layers_.erase(layerIterator);

  const auto basicLayerIterator = std::find(basicLayers_.begin(), basicLayers_.end(), layer);
  if (basicLayerIterator != basicLayers_.end()) basicLayers_.erase(basicLayerIterator);

  return true;
}

const std::vector<std::string>& CostMap::getLayers() const
{
  return layers_;
}

DataType& CostMap::atPosition(const std::string& layer, const cost_map::Position& position)
{
  Eigen::Array2i index;
  if (getIndex(position, index)) {
    return at(layer, index);
  }
  throw std::out_of_range("CostMap::atPosition(...) : Position is out of range.");
}

DataType CostMap::atPosition(const std::string& layer,
                             const cost_map::Position& position,
                             grid_map::InterpolationMethods interpolation_method
                             ) const
{
  if ( interpolation_method == grid_map::InterpolationMethods::INTER_LINEAR) {
    float value;
    if (atPositionLinearInterpolated(layer, position, value)) {
      return value;
    } else {
      interpolation_method = grid_map::InterpolationMethods::INTER_NEAREST;
    }
  }
  if ( interpolation_method == grid_map::InterpolationMethods::INTER_NEAREST)
  {
    Index index;
    if (getIndex(position, index)) {
      return at(layer, index);
    } else {
      throw std::out_of_range("CostMap::atPosition(...) : position is out of range.");
    }
  }
  // should have handled by here...
  throw std::runtime_error("CostMap::atPosition(...) : specified interpolation method not implemented.");
}

DataType& CostMap::at(const std::string& layer, const cost_map::Index& index)
{
  try {
    return data_.at(layer)(index(0), index(1));
  } catch (const std::out_of_range& exception) {
    throw std::out_of_range("CostMap::at(...) : No map layer '" + layer + "' available.");
  }
}

DataType CostMap::at(const std::string& layer, const Eigen::Array2i& index) const
{
  try {
    return data_.at(layer)(index(0), index(1));
  } catch (const std::out_of_range& exception) {
    throw std::out_of_range("CostMap::at(...) : No map layer '" + layer + "' available.");
  }
}

bool CostMap::getIndex(const cost_map::Position& position, cost_map::Index& index) const
{
  return grid_map::getIndexFromPosition(index, position, length_, position_, resolution_, size_, startIndex_);
}

bool CostMap::getPosition(const cost_map::Index& index, cost_map::Position& position) const
{
  return grid_map::getPositionFromIndex(position, index, length_, position_, resolution_, size_, startIndex_);
}

bool CostMap::isInside(const cost_map::Position& position) const
{
  return grid_map::checkIfPositionWithinMap(position, length_, position_);
}

bool CostMap::isValid(const cost_map::Index& index) const
{
  return isValid(index, basicLayers_);
}

bool CostMap::isValid(const cost_map::Index& index, const std::string& layer) const
{
  return (at(layer, index) != NO_INFORMATION);
}

bool CostMap::isValid(const cost_map::Index& index, const std::vector<std::string>& layers) const
{
  if (layers.empty()) return false;
  for (auto& layer : layers) {
    if (at(layer, index) == NO_INFORMATION) return false;
  }
  return true;
}

bool CostMap::getPosition3(const std::string& layer, const cost_map::Index& index,
                           cost_map::Position3& position) const
{
  if (!isValid(index, layer)) return false;
  Position position2d;
  getPosition(index, position2d);
  position.head(2) = position2d;
  position.z() = at(layer, index);
  return true;
}

bool CostMap::getVector(const std::string& layerPrefix, const cost_map::Index& index,
                        Eigen::Vector3d& vector) const
{
  std::vector<std::string> layers;
  layers.push_back(layerPrefix + "x");
  layers.push_back(layerPrefix + "y");
  layers.push_back(layerPrefix + "z");
  if (!isValid(index, layers)) return false;
  for (size_t i = 0; i < 3; ++i) {
    vector(i) = at(layers[i], index);
  }
  return true;
}

CostMap CostMap::getSubmap(const cost_map::Position& position, const cost_map::Length& length,
                           bool& isSuccess) const
{
  Index index;
  return getSubmap(position, length, index, isSuccess);
}

CostMap CostMap::getSubmap(const cost_map::Position& position, const cost_map::Length& length,
                           cost_map::Index& indexInSubmap, bool& isSuccess) const
{
  // Submap the generate.
  CostMap submap(layers_);
  submap.setBasicLayers(basicLayers_);
  submap.setTimestamp(timestamp_);
  submap.setFrameId(frameId_);

  // Get submap geometric information.
  SubmapGeometry submapInformation(*this, position, length, isSuccess);
  if (isSuccess == false) return CostMap(layers_);
  submap.setGeometry(submapInformation);
  submap.startIndex_.setZero(); // Because of the way we copy the data below.

  // Copy data.
  std::vector<BufferRegion> bufferRegions;

  if (!getBufferRegionsForSubmap(bufferRegions, submapInformation.getStartIndex(),
                                 submap.getSize(), size_, startIndex_)) {
    std::cout << "Cannot access submap of this size." << std::endl;
    isSuccess = false;
    return CostMap(layers_);
  }

  for (auto& data : data_) {
    for (const auto& bufferRegion : bufferRegions) {
      Index index = bufferRegion.getStartIndex();
      Size size = bufferRegion.getSize();

      if (bufferRegion.getQuadrant() == BufferRegion::Quadrant::TopLeft) {
        submap.data_[data.first].topLeftCorner(size(0), size(1)) = data.second.block(index(0), index(1), size(0), size(1));
      } else if (bufferRegion.getQuadrant() == BufferRegion::Quadrant::TopRight) {
        submap.data_[data.first].topRightCorner(size(0), size(1)) = data.second.block(index(0), index(1), size(0), size(1));
      } else if (bufferRegion.getQuadrant() == BufferRegion::Quadrant::BottomLeft) {
        submap.data_[data.first].bottomLeftCorner(size(0), size(1)) = data.second.block(index(0), index(1), size(0), size(1));
      } else if (bufferRegion.getQuadrant() == BufferRegion::Quadrant::BottomRight) {
        submap.data_[data.first].bottomRightCorner(size(0), size(1)) = data.second.block(index(0), index(1), size(0), size(1));
      }

    }
  }

  isSuccess = true;
  return submap;
}

bool CostMap::move(const cost_map::Position& position, std::vector<BufferRegion>& newRegions)
{
  Index indexShift;
  Position positionShift = position - position_;
  grid_map::getIndexShiftFromPositionShift(indexShift, positionShift, resolution_);
  Position alignedPositionShift;
  grid_map::getPositionShiftFromIndexShift(alignedPositionShift, indexShift, resolution_);

  // Delete fields that fall out of map (and become empty cells).
  for (int i = 0; i < indexShift.size(); i++) {
    if (indexShift(i) != 0) {
      if (abs(indexShift(i)) >= getSize()(i)) {
        // Entire map is dropped.
        clearAll();
        newRegions.push_back(BufferRegion(Index(0, 0), getSize(), BufferRegion::Quadrant::Undefined));
      } else {
        // Drop cells out of map.
        int sign = (indexShift(i) > 0 ? 1 : -1);
        int startIndex = startIndex_(i) - (sign < 0 ? 1 : 0);
        int endIndex = startIndex - sign + indexShift(i);
        int nCells = abs(indexShift(i));
        int index = (sign > 0 ? startIndex : endIndex);
        grid_map::boundIndexToRange(index, getSize()(i));

        if (index + nCells <= getSize()(i)) {
          // One region to drop.
          if (i == 0) {
            clearRows(index, nCells);
            newRegions.push_back(BufferRegion(Index(index, 0), Size(nCells, getSize()(1)), BufferRegion::Quadrant::Undefined));
          } else if (i == 1) {
            clearCols(index, nCells);
            newRegions.push_back(BufferRegion(Index(0, index), Size(getSize()(0), nCells), BufferRegion::Quadrant::Undefined));
          }
        } else {
          // Two regions to drop.
          int firstIndex = index;
          int firstNCells = getSize()(i) - firstIndex;
          if (i == 0) {
            clearRows(firstIndex, firstNCells);
            newRegions.push_back(BufferRegion(Index(firstIndex, 0), Size(firstNCells, getSize()(1)), BufferRegion::Quadrant::Undefined));
          } else if (i == 1) {
            clearCols(firstIndex, firstNCells);
            newRegions.push_back(BufferRegion(Index(0, firstIndex), Size(getSize()(0), firstNCells), BufferRegion::Quadrant::Undefined));
          }

          int secondIndex = 0;
          int secondNCells = nCells - firstNCells;
          if (i == 0) {
            clearRows(secondIndex, secondNCells);
            newRegions.push_back(BufferRegion(Index(secondIndex, 0), Size(secondNCells, getSize()(1)), BufferRegion::Quadrant::Undefined));
          } else if (i == 1) {
            clearCols(secondIndex, secondNCells);
            newRegions.push_back(BufferRegion(Index(0, secondIndex), Size(getSize()(0), secondNCells), BufferRegion::Quadrant::Undefined));
          }
        }
      }
    }
  }

  // Update information.
  startIndex_ += indexShift;
  grid_map::boundIndexToRange(startIndex_, getSize());
  position_ += alignedPositionShift;

  // Check if map has been moved at all.
  return (indexShift.any() != 0);
}

bool CostMap::move(const cost_map::Position& position)
{
  std::vector<BufferRegion> newRegions;
  return move(position, newRegions);
}

bool CostMap::addDataFrom(const cost_map::CostMap& other, bool extendMap, bool overwriteData,
                          bool copyAllLayers, std::vector<std::string> layers)
{
  // Set the layers to copy.
  if (copyAllLayers) layers = other.getLayers();

  // Resize map.
  if (extendMap) extendToInclude(other);

  // Check if all layers to copy exist and add missing layers.
  for (const auto& layer : layers) {
    if (std::find(layers_.begin(), layers_.end(), layer) == layers_.end()) {
      add(layer);
    }
  }
  // Copy data.
  for (CostMapIterator iterator(*this); !iterator.isPastEnd(); ++iterator) {
    if (isValid(*iterator) && !overwriteData) continue;
    Position position;
    getPosition(*iterator, position);
    Index index;
    if (!other.isInside(position)) continue;
    other.getIndex(position, index);
    for (const auto& layer : layers) {
      if (!other.isValid(index, layer)) continue;
      at(layer, *iterator) = other.at(layer, index);
    }
  }

  return true;
}

bool CostMap::extendToInclude(const cost_map::CostMap& other)
{
  // Get dimension of maps.
  Position topLeftCorner(position_.x() + length_.x() / 2.0, position_.y() + length_.y() / 2.0);
  Position bottomRightCorner(position_.x() - length_.x() / 2.0, position_.y() - length_.y() / 2.0);
  Position topLeftCornerOther(other.getPosition().x() + other.getLength().x() / 2.0, other.getPosition().y() + other.getLength().y() / 2.0);
  Position bottomRightCornerOther(other.getPosition().x() - other.getLength().x() / 2.0, other.getPosition().y() - other.getLength().y() / 2.0);
  // Check if map needs to be resized.
  bool resizeMap = false;
  Position extendedMapPosition = position_;
  Length extendedMapLength = length_;
  if (topLeftCornerOther.x() > topLeftCorner.x()) {
    extendedMapPosition.x() += (topLeftCornerOther.x() - topLeftCorner.x()) / 2.0;
    extendedMapLength.x() += topLeftCornerOther.x() - topLeftCorner.x();
    resizeMap = true;
  }
  if (topLeftCornerOther.y() > topLeftCorner.y()) {
    extendedMapPosition.y() += (topLeftCornerOther.y() - topLeftCorner.y()) / 2.0;
    extendedMapLength.y() += topLeftCornerOther.y() - topLeftCorner.y();
    resizeMap = true;
  }
  if (bottomRightCornerOther.x() < bottomRightCorner.x()) {
    extendedMapPosition.x() -= (bottomRightCorner.x() - bottomRightCornerOther.x()) / 2.0;
    extendedMapLength.x() += bottomRightCorner.x() - bottomRightCornerOther.x();
    resizeMap = true;
  }
  if (bottomRightCornerOther.y() < bottomRightCorner.y()) {
    extendedMapPosition.y() -= (bottomRightCorner.y() - bottomRightCornerOther.y()) / 2.0;
    extendedMapLength.y() += bottomRightCorner.y() - bottomRightCornerOther.y();
    resizeMap = true;
  }

  // Resize map and copy data to new map.
  if (resizeMap) {
    CostMap mapCopy = *this;
    setGeometry(extendedMapLength, resolution_, extendedMapPosition);
    // Align new map with old one.
    Vector shift = position_ - mapCopy.getPosition();
    shift.x() = std::fmod(shift.x(), resolution_);
    shift.y() = std::fmod(shift.y(), resolution_);
    if (std::abs(shift.x()) < resolution_ / 2.0) {
      position_.x() -= shift.x();
    } else {
      position_.x() += resolution_ - shift.x();
    }
    if (size_.x() % 2 != mapCopy.getSize().x() % 2) {
      position_.x() += -std::copysign(resolution_ / 2.0, shift.x());
    }
    if (std::abs(shift.y()) < resolution_ / 2.0) {
      position_.y() -= shift.y();
    } else {
      position_.y() += resolution_ - shift.y();
    }
    if (size_.y() % 2 != mapCopy.getSize().y() % 2) {
      position_.y() += -std::copysign(resolution_ / 2.0, shift.y());
    }
    // Copy data.
    for (CostMapIterator iterator(*this); !iterator.isPastEnd(); ++iterator) {
      if (isValid(*iterator)) continue;
      Position position;
      getPosition(*iterator, position);
      Index index;
      if (!mapCopy.isInside(position)) continue;
      mapCopy.getIndex(position, index);
      for (const auto& layer : layers_) {
        at(layer, *iterator) = mapCopy.at(layer, index);
      }
    }
  }
  return true;
}

void CostMap::setTimestamp(const Time timestamp)
{
  timestamp_ = timestamp;
}

Time CostMap::getTimestamp() const
{
  return timestamp_;
}

void CostMap::resetTimestamp()
{
  timestamp_ = 0.0;
}

void CostMap::setFrameId(const std::string& frameId)
{
  frameId_ = frameId;
}

const std::string& CostMap::getFrameId() const
{
  return frameId_;
}

const Eigen::Array2d& CostMap::getLength() const
{
  return length_;
}

const Eigen::Vector2d& CostMap::getPosition() const
{
  return position_;
}

double CostMap::getResolution() const
{
  return resolution_;
}

const cost_map::Size& CostMap::getSize() const
{
  return size_;
}

void CostMap::setStartIndex(const cost_map::Index& startIndex) {
  startIndex_ = startIndex;
}

const cost_map::Index& CostMap::getStartIndex() const
{
  return startIndex_;
}

void CostMap::clear(const std::string& layer)
{
  try {
    data_.at(layer).setConstant(NO_INFORMATION);
  } catch (const std::out_of_range& exception) {
    throw std::out_of_range("CostMap::clear(...) : No map layer '" + layer + "' available.");
  }
}

void CostMap::clearBasic()
{
  for (auto& layer : basicLayers_) {
    clear(layer);
  }
}

void CostMap::clearAll()
{
  for (auto& data : data_) {
    data.second.setConstant(NO_INFORMATION);
  }
}

void CostMap::clearRows(unsigned int index, unsigned int nRows)
{
  std::vector<std::string> layersToClear;
  if (basicLayers_.size() > 0) layersToClear = basicLayers_;
  else layersToClear = layers_;
  for (auto& layer : layersToClear) {
    data_.at(layer).block(index, 0, nRows, getSize()(1)).setConstant(NO_INFORMATION);
  }
}

void CostMap::clearCols(unsigned int index, unsigned int nCols)
{
  std::vector<std::string> layersToClear;
  if (basicLayers_.size() > 0) layersToClear = basicLayers_;
  else layersToClear = layers_;
  for (auto& layer : layersToClear) {
    data_.at(layer).block(0, index, getSize()(0), nCols).setConstant(NO_INFORMATION);
  }
}

bool CostMap::atPositionLinearInterpolated(const std::string& layer, const Position& position,
                                           float& value) const
{
  std::vector<Position> points(4);
  std::vector<Index> indices(4);
  getIndex(position, indices[0]);
  getPosition(indices[0], points[0]);

  if (position.x() >= points[0].x()) {
    // Second point is above first point.
    indices[1] = indices[0] + Index(-1, 0);
    if (!getPosition(indices[1], points[1])) return false; // Check if still on map.
  } else {
    indices[1] = indices[0] + Index(1, 0);
    if (!getPosition(indices[1], points[1])) return false;
  }

  if (position.y() >= points[0].y()) {
    // Third point is right of first point.
    indices[2] = indices[0] + Index(0, -1);
    if (!getPosition(indices[2], points[2])) return false;
  } else {
    indices[2] = indices[0] + Index(0, 1);
    if (!getPosition(indices[2], points[2])) return false;
  }

  indices[3].x() = indices[1].x();
  indices[3].y() = indices[2].y();
  if (!getPosition(indices[3], points[3])) return false;

  Eigen::Vector4d b;
  Eigen::Matrix4d A;

  for (unsigned int i = 0; i < points.size(); ++i) {
    b(i) = at(layer, indices[i]);
    A.row(i) << 1, points[i].x(), points[i].y(), points[i].x() * points[i].y();
  }

  Eigen::Vector4d x = A.colPivHouseholderQr().solve(b);
  //Eigen::Vector4d x = A.fullPivLu().solve(b);

  value = x(0) + x(1) * position.x() + x(2) * position.y() + x(3) * position.x() * position.y();
  return true;
}

void CostMap::resize(const Eigen::Array2i& size)
{
  size_ = size;
  for (auto& data : data_) {
    data.second.resize(size_(0), size_(1));
  }
}

} /* namespace */

