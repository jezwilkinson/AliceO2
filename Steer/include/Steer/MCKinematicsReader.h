// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "SimulationDataFormat/DigitizationContext.h"
#include "SimulationDataFormat/MCTrack.h"
#include "SimulationDataFormat/MCCompLabel.h"
#include "SimulationDataFormat/MCEventHeader.h"
#include <vector>

class TChain;

namespace o2
{

namespace steer
{

class MCKinematicsReader
{
 public:
  enum class Mode {
    kDigiContext,
    kMCKine
  };

  /// default constructor
  MCKinematicsReader() = default;

  /// constructor taking a name and mode (either kDigiContext or kMCKine)
  /// In case of "context", the name is the filename of the digitization context.
  /// In case of MCKine mode, the name is the "prefix" referencing a single simulation production.
  /// The default mode is kDigiContext.
  MCKinematicsReader(std::string_view name, Mode mode = Mode::kDigiContext)
  {
    if (mode == Mode::kMCKine) {
      initFromKinematics(name);
    } else if (mode == Mode::kDigiContext) {
      initFromDigitContext(name);
    }
  }

  /// inits the reader from a digitization context
  /// returns true if successful
  bool initFromDigitContext(std::string_view filename);

  /// inits the reader from a simple kinematics file
  bool initFromKinematics(std::string_view filename);

  bool isInitialized() const { return mInitialized; }

  /// query an MC track given a basic label object
  /// returns nullptr if no track was found
  MCTrack const* getTrack(o2::MCCompLabel const&) const;

  /// query an MC track given source, event, track IDs
  /// returns nullptr if no track was found
  MCTrack const* getTrack(int source, int event, int track) const;

  /// query an MC track given event, track IDs
  /// returns nullptr if no track was found
  MCTrack const* getTrack(int event, int track) const;

  /// variant returning all tracks for source and event at once
  std::vector<MCTrack> const& getTracks(int source, int event) const;

  /// variant returning all tracks for source and event at once
  std::vector<MCTrack> const& getTracks(int event) const;

  /// get all primaries for a certain event

  /// get all secondaries of the given label

  /// get all mothers/daughters of the given label

  /// retrieves the MCEventHeader for a given eventID and sourceID
  o2::dataformats::MCEventHeader const& getMCEventHeader(int source, int event) const;

  DigitizationContext const* getDigitizationContext() const
  {
    return mDigitizationContext;
  }

 private:
  void loadTracksForSource(int source) const;
  void loadHeadersForSource(int source) const;

  DigitizationContext const* mDigitizationContext = nullptr;

  // chains for each source
  std::vector<TChain*> mInputChains;

  // a vector of tracks foreach source and each collision
  mutable std::vector<std::vector<std::vector<o2::MCTrack>>> mTracks; // the in-memory track container
  mutable std::vector<std::vector<o2::dataformats::MCEventHeader>> mHeaders; // the in-memory header container

  bool mInitialized = false; // whether initialized
};

inline MCTrack const* MCKinematicsReader::getTrack(o2::MCCompLabel const& label) const
{
  const auto source = label.getSourceID();
  const auto event = label.getEventID();
  const auto track = label.getTrackID();
  return getTrack(source, event, track);
}

inline MCTrack const* MCKinematicsReader::getTrack(int source, int event, int track) const
{
  if (mTracks[source].size() == 0) {
    loadTracksForSource(source);
  }
  return &mTracks[source][event][track];
}

inline MCTrack const* MCKinematicsReader::getTrack(int event, int track) const
{
  return getTrack(0, event, track);
}

inline std::vector<MCTrack> const& MCKinematicsReader::getTracks(int source, int event) const
{
  if (mTracks[source].size() == 0) {
    loadTracksForSource(source);
  }
  return mTracks[source][event];
}

inline std::vector<MCTrack> const& MCKinematicsReader::getTracks(int event) const
{
  return getTracks(0, event);
}

inline o2::dataformats::MCEventHeader const& MCKinematicsReader::getMCEventHeader(int source, int event) const
{
  if (mHeaders.at(source).size() == 0) {
    loadHeadersForSource(source);
  }
  return mHeaders.at(source)[event];
}

} // namespace steer
} // namespace o2
